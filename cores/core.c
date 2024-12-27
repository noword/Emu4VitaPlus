#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/clib.h>
#include <stdio.h>
#include <libretro.h>

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp)
{
	//printf("module_start\n");
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp)
{
	return SCE_KERNEL_STOP_SUCCESS;
}

int module_exit(SceSize args, void *argp)
{
	return SCE_KERNEL_START_SUCCESS;
}

int getVMBlock()
{
	return 0;
}


int main (int argc, char *const argv[]) {

   return 0;
}