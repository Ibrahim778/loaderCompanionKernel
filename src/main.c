/*
  Under MIT
  by skgleba, teakhanirons and Ibrahim
*/

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>

#include <stdio.h>
#include <string.h>

#include <taihen.h>

#include <string.h>
#include <stdio.h>

#include <psp2/usbstorvstor.h>
#include <psp2kern/power.h> 
#include <psp2kern/ctrl.h> 
#include <psp2kern/display.h>
#include <psp2kern/io/fcntl.h> 
#include <psp2kern/io/dirent.h> 
#include <psp2kern/udcd.h>
#include <psp2kern/sblaimgr.h> 
#include <psp2kern/kernel/cpu.h> 
#include <psp2kern/kernel/sysmem.h> 
#include <psp2kern/kernel/suspend.h>
#include <psp2kern/kernel/modulemgr.h> 
#include <psp2kern/kernel/threadmgr.h> 
#include <psp2kern/kernel/dmac.h>

tai_module_info_t vstorinfo;

static SceUID hooks[3];

static tai_hook_ref_t ksceIoOpenRef;
static tai_hook_ref_t ksceIoReadRef;

char *path = NULL;
char *folder = NULL;
static int first = 1;
int select = 1;
int active = 0;
#define printf ksceDebugPrintf
int (*setname)(const char *name, const char *version);
int (*setpath)(const char *path);
int (*activate)(SceUsbstorVstorType type);
int (*stop)(void);
void StartUsb();
void StopUsb();

int pathCheck() {
  int fd = ksceIoOpen(path, SCE_O_RDONLY, 0777);
  if (fd < 0) {
  	ksceDebugPrintf("%s doesn't exist\n", path);
    return 0;
  }

  ksceIoClose(fd);
  ksceDebugPrintf("%s exists\n", path);
  return 1;
}

int fileExist(const char *file)
{
  int fd = ksceIoOpen(file, SCE_O_RDONLY, 0);
  if(fd < 0)
    return 0;
  ksceIoClose(fd);
  return 1;
}

int namesetter() { 
	int ret = 0;
	ret = setname("\"PS Vita\" MC", "1.00"); 
	return ret;
}

int pathsetter() { 
	int ret = 0;
	ret = setpath(path); 
	return ret;
}

void StartUsbOffCard()
{

  int kid = ksceKernelSearchModuleByName("udcd_uvc");

  if(kid > 0)
  {
    ksceDebugPrintf("Found udcd_uvc!\n");
    int ret = ksceKernelStopUnloadModule(kid, 0, NULL, 0, NULL, NULL);
    ksceDebugPrintf("Unload result: %d", ret);
  }

	path = "sdstor0:xmc-lp-ign-userext";
	if(em_iofix(pathCheck) == 0) 
	{ 
		path = NULL;
		ksceDebugPrintf("path doesn't exist!\n");
		return;
	}
	StopUsb();
	StartUsb();
}

void StartSd2Vita()
{
  int kid = ksceKernelSearchModuleByName("udcd_uvc");
  if(kid > 0)
  {
    ksceDebugPrintf("Found udcd_uvc!\n");
    int ret = ksceKernelStopUnloadModule(kid, 0, NULL, 0, NULL, NULL);
    ksceDebugPrintf("Unload result: %d", ret);
  }

	ksceDebugPrintf("Will start sd2vita usb!\n");
	path = "sdstor0:gcd-lp-ign-entire";
	if(em_iofix(pathCheck) == 0) 
	{ 
		path = NULL;
		ksceDebugPrintf("path doesn't exist!\n");
		return;
	}
	if(activate != 0) StopUsb();
	StartUsb();
}

void StopUsbExp()
{
  StopUsb();
}

int activator() { 
	int ret = 0;
	ret = activate(SCE_USBSTOR_VSTOR_TYPE_FAT);
	return ret;
}

int em_iofix(void *func) {
	int ret, state = 0;

	ENTER_SYSCALL(state);
	ret = em_iofix_threader(func);
	EXIT_SYSCALL(state);

	return ret;
}

int em_iofix_threader(void *func) { // can't access sdstor0: without a thread, this is a workaround
	int ret, res, uid = 0;
	ret = uid = ksceKernelCreateThread("em_iofix", func, 64, 0x10000, 0, 0, 0);
	if (ret < 0){ ret = -1; goto exit;}
	if ((ret = ksceKernelStartThread(uid, 0, NULL)) < 0) { ret = -1; goto exit;}
	if ((ret = ksceKernelWaitThreadEnd(uid, &res, NULL)) < 0) { ret = -1; goto exit;}
	ret = res;
exit:
	if (uid > 0) { ksceKernelDeleteThread(uid); }
	return ret;
}

static SceUID ksceIoOpenPatched(const char *file, int flags, SceMode mode) {
  first = 1;

  SceUID fd = TAI_CONTINUE(SceUID, ksceIoOpenRef, file, flags, mode);

  if (fd == 0x800F090D)
    return TAI_CONTINUE(SceUID, ksceIoOpenRef, file, flags & ~SCE_O_WRONLY, mode);

  return fd;
}

static int ksceIoReadPatched(SceUID fd, void *data, SceSize size) {
  int res = TAI_CONTINUE(int, ksceIoReadRef, fd, data, size);

  if (first) {
    first = 0;

    // Manipulate boot sector to support exFAT
    if (memcmp(data + 0x3, "EXFAT", 5) == 0) {
      // Sector size
      *(uint16_t *)(data + 0xB) = 1 << *(uint8_t *)(data + 0x6C);

      // Volume size
      *(uint32_t *)(data + 0x20) = *(uint32_t *)(data + 0x48);
    }
  }

  return res;
}


int sync() {
	ksceDebugPrintf("%s sync: %d\n", path, ksceIoSync(path, 0));
	return 1;
}


void StartUsb() {
	if(active == 0) {
	 	if(!path) {
			ksceDebugPrintf("path is null\n");
			return;
		}
  // Remove image path limitation
  	char zero[0x6E];
 	memset(zero, 0, 0x6E); // I can probably use DmacMemset here but I'll leave it as is since 0x6E bytes is small enough
  	hooks[0] = taiInjectDataForKernel(KERNEL_PID, vstorinfo.modid, 0, 0x1738, zero, 0x6E);

  // Add patches to support exFAT
  	hooks[1] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceIoOpenRef, "SceUsbstorVStorDriver", 0x40FD29C7, 0x75192972, ksceIoOpenPatched);
  	hooks[2] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceIoReadRef, "SceUsbstorVStorDriver", 0x40FD29C7, 0xE17EFC03, ksceIoReadPatched);
  	ksceDebugPrintf("exfat and image path limit patches = %x %x %x\n", hooks[0], hooks[1], hooks[2]);

  	ksceDebugPrintf("NAME: %x\n", em_iofix(namesetter));
  	ksceDebugPrintf("PATH: %x\n", em_iofix(pathsetter));
	ksceDebugPrintf("ACTI: %x\n", em_iofix(activator));
	active = 1;
	} else { ksceDebugPrintf("already activated\n"); }
}

void StopUsb() {
	if(active == 1) {
		active = 0;
		ksceDebugPrintf("STOP: %x\n", em_iofix(stop));
	} else { ksceDebugPrintf("usbstorvstor already stopped\n"); }
	ksceDebugPrintf("clear USB bus 2: %d\n", ksceUdcdStopCurrentInternal(2));
  	if (hooks[2] >= 0) { taiHookReleaseForKernel(hooks[2], ksceIoReadRef); }
	if (hooks[1] >= 0) { taiHookReleaseForKernel(hooks[1], ksceIoOpenRef); }
  	if (hooks[0] >= 0) { taiInjectReleaseForKernel(hooks[0]); }
  	em_iofix(sync);
}

#define DACR_OFF(stmt)                 \
do {                                   \
    unsigned prev_dacr;                \
    __asm__ volatile(                  \
        "mrc p15, 0, %0, c3, c0, 0 \n" \
        : "=r" (prev_dacr)             \
    );                                 \
    __asm__ volatile(                  \
        "mcr p15, 0, %0, c3, c0, 0 \n" \
        : : "r" (0xFFFF0000)           \
    );                                 \
    stmt;                              \
    __asm__ volatile(                  \
        "mcr p15, 0, %0, c3, c0, 0 \n" \
        : : "r" (prev_dacr)            \
    );                                 \
} while (0)

static char lpath[0x400]; // self path
static char larg[0x100]; // self args
static uint32_t largl; // self args len

int ksceAppMgrLaunchAppByPath(const char *name, const char *cmd, int cmdlen, int dynamic, void *opt, void *id);

static int launch_thread(SceSize args, void *argp) {
  int opt[52/4];
  memset(opt, 0, sizeof(opt));
  opt[0] = sizeof(opt);

  int ret = ksceAppMgrLaunchAppByPath(lpath, (largl) ? larg : 0, largl, 0, opt, NULL);
  
  ksceDebugPrintf("launch %s(%s) |=>| ret: 0x%X\n", lpath, larg, ret);

  return ksceKernelExitDeleteThread(0);
}

int launchAppFromFileExport(uintptr_t path, uintptr_t cmd, uint32_t cmdlen) {
  uint32_t state;
  ENTER_SYSCALL(state);
  
  largl = (cmdlen < 0x100) ? cmdlen : 0x100;
  
  ksceKernelStrncpyUserToKernel(lpath, path, 0x400);
  if (cmd)
    ksceKernelMemcpyUserToKernel(larg, cmd, largl);

  SceUID thid = ksceKernelCreateThread("launch_thread", (SceKernelThreadEntry)launch_thread, 0x40, 0x1000, 0, 0, NULL);
  if (thid < 0) {
    EXIT_SYSCALL(state);
    return thid;
  }

  ksceKernelStartThread(thid, 0, NULL);

  EXIT_SYSCALL(state);
  return 0;
}


void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize args, void *argp) {
  // patch thread watchdog and allowSelfArgs QA
  uintptr_t addr;
  int sysmem_id = ksceKernelSearchModuleByName("SceSysmem");
  module_get_offset(0x10005, sysmem_id, 0, 0, &addr); // either taiModuleUtils or tai_compat
  DACR_OFF(*(uint32_t*)(addr + 0x1f0e6) = 0xe0032001; *(uint32_t*)(addr + 0x205fe) = 0xe00b2000;);

  	if(ksceSblAimgrIsGenuineDolce()) return SCE_KERNEL_START_SUCCESS;

  	vstorinfo.size = sizeof(tai_module_info_t);

  if(taiGetModuleInfoForKernel(KERNEL_PID, "SceUsbstorVStorDriver", &vstorinfo) < 0) {
  	// ksceDebugPrintf("vstor not loaded\n");
  	return SCE_KERNEL_START_SUCCESS;
  }
  // ksceDebugPrintf("vstor loaded\n");
  	
  	module_get_offset(KERNEL_PID, vstorinfo.modid, 0, 0x16b8 | 1, &setname);
  	module_get_offset(KERNEL_PID, vstorinfo.modid, 0, 0x16d8 | 1, &setpath);
  	module_get_offset(KERNEL_PID, vstorinfo.modid, 0, 0x1710 | 1, &activate);
  	module_get_offset(KERNEL_PID, vstorinfo.modid, 0, 0x1858 | 1, &stop);

  	// ksceDebugPrintf("vstor hooks returns: %x %x %x %x\n", rname, rpath, racti, rstop);
  
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
  return SCE_KERNEL_STOP_SUCCESS;
}