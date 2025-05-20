#include <errno.h>
#include <reent.h>
#include <defines/psp_defines.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>

unsigned *_newlib_heap_size;
char **_newlib_heap_base, **_newlib_heap_end, **_newlib_heap_cur;
SceKernelLwMutexWork *_newlib_sbrk_mutex;

int _newlib_vm_memblock;
int _newlib_vm_size;

extern int _newlib_vm_size_user __attribute__((weak));

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

void *_sbrk_r(struct _reent *reent, ptrdiff_t incr)
{
	if (sceKernelLockLwMutex(_newlib_sbrk_mutex, 1, 0) < 0)
		goto fail;
	if (!*_newlib_heap_base || *_newlib_heap_cur + incr >= *_newlib_heap_end)
	{
		sceKernelUnlockLwMutex(_newlib_sbrk_mutex, 1);
	fail:
		reent->_errno = ENOMEM;
		return (void *)-1;
	}

	char *prev_heap_end = *_newlib_heap_cur;
	*_newlib_heap_cur += incr;

	sceKernelUnlockLwMutex(_newlib_sbrk_mutex, 1);

	printf("core alloc at %08x, size: %08x", prev_heap_end, incr);

	return (void *)prev_heap_end;
}

void _init_vita_heap(void)
{
	if (&_newlib_vm_size_user != NULL)
	{
		printf("_newlib_vm_size_user %x\n", _newlib_vm_size_user);
		_newlib_vm_size = ALIGN(_newlib_vm_size_user, 0x100000);
		_newlib_vm_memblock = sceKernelAllocMemBlockForVM("code", _newlib_vm_size);

		if (_newlib_vm_memblock < 0)
		{
			// sceClibPrintf("sceKernelAllocMemBlockForVM failed\n");
		}
	}
	else
	{
		_newlib_vm_size = 0;
		_newlib_vm_memblock = 0;
	}

	return;
failure:
	_newlib_vm_memblock = 0;
}

int getVMBlock()
{
	return _newlib_vm_memblock;
}

void _free_vita_heap(void)
{
	if (_newlib_vm_memblock > 0)
		sceKernelFreeMemBlock(_newlib_vm_memblock);
}
