/*
 * Originally created by devnoname120 (https://github.com/devnoname120)
 */

#include "vitaPackage.h"
#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/promoterutil.h>
#include <psp2/sysmodule.h>
#include <common/debugScreen.h>

#include "filesystem.hpp"
#include "sha1.h"
#include "zip.h"


extern unsigned char _binary_assets_head_bin_start;
extern unsigned char _binary_assets_head_bin_size;

std::string TitleID;

#define print psvDebugScreenPrintf


static void fpkg_hmac(const uint8_t* data, unsigned int len, uint8_t hmac[16]) {
	
	SHA1_CTX ctx;
	char sha1[20];
	char buf[64];

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)data, len);
	sha1_final(&ctx, (BYTE*)sha1);

	memset(buf, 0, 64);
	memcpy(&buf[0], &sha1[4], 8);
	memcpy(&buf[8], &sha1[4], 8);
	memcpy(&buf[16], &sha1[12], 4);
	buf[20] = sha1[16];
	buf[21] = sha1[1];
	buf[22] = sha1[2];
	buf[23] = sha1[3];
	memcpy(&buf[24], &buf[16], 8);

	sha1_init(&ctx);
	sha1_update(&ctx, (BYTE*)buf, 64);
	sha1_final(&ctx, (BYTE*)sha1);
	memcpy(hmac, sha1, 16);
}

typedef struct SfoHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t keyofs;
	uint32_t valofs;
	uint32_t count;
} __attribute__((packed)) SfoHeader;

typedef struct SfoEntry
{
	uint16_t nameofs;
	uint8_t alignment;
	uint8_t type;
	uint32_t valsize;
	uint32_t totalsize;
	uint32_t dataofs;
} __attribute__((packed)) SfoEntry;

int getSfoString(char* buffer, const char* name, char* string, unsigned int length) {
	
	auto* header = (SfoHeader*)buffer;
	auto* entries = (SfoEntry*)((uint32_t)buffer + sizeof(SfoHeader));

	if (header->magic != SFO_MAGIC)
		return -1;

	int i;
	for (i = 0; i < header->count; i++) {
		if (strcmp(buffer + header->keyofs + entries[i].nameofs, name) == 0) {
			memset(string, 0, length);
			strncpy(string, buffer + header->valofs + entries[i].dataofs, length);
			string[length - 1] = '\0';
			return 0;
		}
	}

	return -2;
}

int WriteFile(const char* file, const void* buf, unsigned int size) {

	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int checkFileExist(const char* file) {
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));

	return sceIoGetstat(file, &stat) >= 0;
}

int allocateReadFile(const char* file, char** buffer) {

	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	// FIXME Check if < 0
	int size = sceIoLseek32(fd, 0, SCE_SEEK_END);
	sceIoLseek32(fd, 0, SCE_SEEK_SET);

	*buffer = (char*)malloc(size);
	if (!*buffer) {
			sceIoClose(fd);
			return -1;
	}

	int read = sceIoRead(fd, *buffer, size);
	sceIoClose(fd);

	return read;
}

int makeHeadBin() {

	uint8_t hmac[16];
	uint32_t off;
	uint32_t len;
	uint32_t out;

	if (checkFileExist(PACKAGE_TEMP_FOLDER "sce_sys/package/head.bin"))
		return 0;

	// Read param.sfo
	char* sfo_buffer = nullptr;
	int res = allocateReadFile(PACKAGE_TEMP_FOLDER "sce_sys/param.sfo", &sfo_buffer);
	if (res < 0)
		return res;

	// Get title id
	char titleid[12];
	memset(titleid, 0, sizeof(titleid));
	getSfoString(sfo_buffer, "TITLE_ID", titleid, sizeof(titleid));

	// Enforce TITLE_ID format
	if (strlen(titleid) != 9)
		return -1;
	
	TitleID = std::string(titleid);
	
	// Get content id
	char contentid[48];
	memset(contentid, 0, sizeof(contentid));
	getSfoString(sfo_buffer, "CONTENT_ID", contentid, sizeof(contentid));

	// Free sfo buffer
	free(sfo_buffer);

	// Allocate head.bin buffer
	uint8_t* head_bin = (uint8_t*)malloc((size_t)&_binary_assets_head_bin_size);
	memcpy(head_bin, (void*)&_binary_assets_head_bin_start, (size_t)&_binary_assets_head_bin_size);

	// Write full title id
	char full_title_id[48];
	snprintf(full_title_id, sizeof(full_title_id), "EP9000-%s_00-0000000000000000", titleid);
	strncpy((char*)&head_bin[0x30], strlen(contentid) > 0 ? contentid : full_title_id, 48);

	// hmac of pkg header
	len = ntohl(*(uint32_t*)&head_bin[0xD0]);
	fpkg_hmac(&head_bin[0], len, hmac);
	memcpy(&head_bin[len], hmac, 16);

	// hmac of pkg info
	off = ntohl(*(uint32_t*)&head_bin[0x8]);
	len = ntohl(*(uint32_t*)&head_bin[0x10]);
	out = ntohl(*(uint32_t*)&head_bin[0xD4]);
	fpkg_hmac(&head_bin[off], len - 64, hmac);
	memcpy(&head_bin[out], hmac, 16);

	// hmac of everything
	len = ntohl(*(uint32_t*)&head_bin[0xE8]);
	fpkg_hmac(&head_bin[0], len, hmac);
	memcpy(&head_bin[len], hmac, 16);

	// Make dir
	sceIoMkdir(PACKAGE_TEMP_FOLDER "sce_sys/package", 0777);

	// Write head.bin
	WriteFile(PACKAGE_TEMP_FOLDER "sce_sys/package/head.bin", head_bin, (unsigned int)&_binary_assets_head_bin_size);

	free(head_bin);

	return 0;
}

VitaPackage::VitaPackage(const std::string vpk) : vpk_(vpk) {

	uint32_t ptr[0x100] = { 0 };
	ptr[0] = 0;
	ptr[1] = (uint32_t)&ptr[0];
	uint32_t scepaf_argp[] = { 0x400000, 0xEA60, 0x40000, 0, 0 };
	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(scepaf_argp), scepaf_argp, ptr);

	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	scePromoterUtilityInit();
}

VitaPackage::~VitaPackage() {
	scePromoterUtilityExit();
	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
}

void VitaPackage::Extract() {
	print("Extracting\n");
	Filesystem::removePath(std::string(PACKAGE_TEMP_FOLDER));

	sceIoMkdir(PACKAGE_TEMP_FOLDER, 0777);

	Zipfile vpk_file = Zipfile(vpk_);

	vpk_file.Unzip(std::string(PACKAGE_TEMP_FOLDER));
}

int VitaPackage::InstallExtracted() {
	print("Making HeadBin\n");
	int ret = makeHeadBin();
	if (ret < 0)
		psvDebugScreenPrintf("Error faking app signature\n");
	print("Promoting\n");
	ret = scePromoterUtilityPromotePkg(PACKAGE_TEMP_FOLDER, 0);
	if (ret < 0)
		psvDebugScreenPrintf("Error installing app\n");

	int state = 0;
	do {
		ret = scePromoterUtilityGetState(&state);
		if (ret < 0)
			psvDebugScreenPrintf("Error while instaling\n");

		sceKernelDelayThread(150 * 1000);
	} while (state);

	int result = 0;
	ret = scePromoterUtilityGetResult(&result);
	if (ret < 0)
		psvDebugScreenPrintf("Installation failed\n");
	else print("Success.\n");

	return 1;
}

int VitaPackage::Install() {
	Extract();
	return InstallExtracted();
}

void VitaPackage::DeletePackage()
{
	Filesystem::removePath(std::string(PACKAGE_TEMP_FOLDER));
}

void UpdatePackage::MakeHeadBin() {
	int ret = makeHeadBin();
	if (ret < 0)
		psvDebugScreenPrintf("Error faking app signature\n");
}

bool isPackageInstalled(std::string titleid) {
	// FIXME Don't reload the module every time

	// ScePaf is required for PromoterUtil
	uint32_t ptr[0x100] = {0};
	ptr[0] = 0;
	ptr[1] = (uint32_t)&ptr[0];
	uint32_t scepaf_argp[] = {0x400000, 0xEA60, 0x40000, 0, 0};
	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(scepaf_argp), scepaf_argp, ptr);

	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);

	int ret = scePromoterUtilityInit();
	if (ret < 0)
		psvDebugScreenPrintf("scePromoterUtilityInit() = " + ret);
	
	//Loading done
	int res;
	int installed = scePromoterUtilityCheckExist(titleid.c_str(), &res);

	scePromoterUtilityExit();

	return installed >= 0;
}

std::string getTitleID()
{
	return TitleID;
}


