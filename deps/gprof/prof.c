/*
 * PSP Software Development Kit - https://github.com/pspdev
 * vitaprof
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * prof.c - Main profiler code
 *
 * Copyright (c) 2005 urchin <c64psp@gmail.com>
 * Copyright (c) 2025 William Bonnaventure
 *
 */
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "vitagprof.h"

#define	GMON_PROF_ON	0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	3

#define GMONVERSION	0x00051879

#define VITA_TEXT_SEGEMENT_START 0x81000000

#include <psp2common/types.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>


/// define sample frequency - 1000 hz = 1ms
#define SAMPLE_FREQ     1000

/** gmon.out file header */
struct gmonhdr 
{
    int lpc;        /* lowest pc address */
    int hpc;        /* highest pc address */
    int ncnt;       /* size of samples + size of header */
    int version;    /* version number */
    int profrate;   /* profiling clock rate */
    int resv[3];    /* reserved */
};

/** frompc -> selfpc graph */
struct rawarc 
{
    unsigned int frompc;
    unsigned int selfpc;
    long count;
};

// Contains an arc + number of samples
// This is the main structure in memory
struct rawdata
{
    unsigned int samples;
    struct rawarc arc;
};

/** context */
struct gmonparam 
{
    int state;
    unsigned int lowpc;
    unsigned int highpc;
    unsigned int textsize;

    int ndata;
    struct rawdata **datas_ptr; // This is a list of pointers to rawdata elements

    int thread_id;
        
    unsigned int pc;
};

/// holds context statistics
static struct gmonparam gp;

/// one histogram per four bytes of text space
#define	HISTFRACTION	4

/// have we allocated memory and registered already
static int initialized = 0;

/// The offset from the standard .text segment start address
static int delta = 0;

/// defined by linker
extern int __executable_start;
extern int __etext;

/* forward declarations */
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void __gprof_cleanup(void);
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void _mcount_internal(unsigned int, unsigned int);
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
static int profiler_thread(SceSize args, void *argp);

__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void free_gprof()
{
    if (!gp.datas_ptr) {
        return;
    }

    unsigned int total_ram_usage = sizeof(struct rawdata *) * gp.ndata;

    for (int i = 0; i < gp.ndata; i++) {
        if (gp.datas_ptr[i]) {
            total_ram_usage += sizeof(struct rawdata);
            free(gp.datas_ptr[i]);
        }
    }

    free(gp.datas_ptr);
    gp.datas_ptr = NULL;
    sceClibPrintf("vitagprof: %d total bytes were allocated\n", total_ram_usage);
}

/** Initializes pg library

    After calculating the text size, initialize() allocates enough
    memory to allow fastest access to arc structures, and some more
    for sampling statistics. Note that this also installs a timer that
    runs at 1000 hert.
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
static void initialize()
{
    initialized = 1;

    sceClibPrintf("vitagprof: initializing\n");

    memset(&gp, '\0', sizeof(gp));
    gp.state = GMON_PROF_ON;
    // __executable_start has 0xB more bytes than the address we are looking for to match elf
    gp.lowpc = (unsigned int)&__executable_start - 0xB;
    gp.highpc = (unsigned int)&__etext;
    gp.textsize = gp.highpc - gp.lowpc;

    gp.ndata = (gp.textsize + HISTFRACTION - 1) / HISTFRACTION;
    gp.datas_ptr = (struct rawdata **)malloc(sizeof(struct rawdata *) * gp.ndata);
    if (gp.datas_ptr == NULL)
    {
        gp.state = GMON_PROF_ERROR;
        sceClibPrintf("vitagprof: error allocating %d bytes\n", sizeof(struct rawdata *) * gp.ndata);
        return;
    }

    memset((void *)gp.datas_ptr, '\0', gp.ndata * (sizeof(struct rawdata *)));

    sceClibPrintf("vitagprof: %d bytes allocated\n", sizeof(struct rawdata *) * gp.ndata);

    delta = VITA_TEXT_SEGEMENT_START - (unsigned int)&__executable_start;

    int thid = 0;
    thid = sceKernelCreateThread("profilerThread", profiler_thread, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid < 0) {
        sceClibPrintf("vitagprof: sceKernelCreateThread failed with error code %i\n", thid);
        free_gprof();
        gp.state = GMON_PROF_ERROR;
        return;
    }

    gp.thread_id = thid;

    sceKernelStartThread(thid, 0, 0);
}

__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_start(void) {
    // There is already a profiling session running, let's stop it and ignore the result
    if (gp.state == GMON_PROF_ON) {
        gprof_stop(NULL, 0);
    }
    initialize();
}

__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_stop(const char* filename, int should_dump)
{
    FILE *fp;
    int i;
    struct gmonhdr hdr;

    if (gp.state != GMON_PROF_ON)
    {
        /* profiling was disabled anyway */
        return;
    }

    // Disable profiling to end profiler_thread
    gp.state = GMON_PROF_OFF;

    sceClibPrintf("vitagprof: stopping\n");

    // Wait for end of thread, timeout is 2 seconds
    int exitstatus = 0;
    SceUInt timeout = 2000000;
    int ret = sceKernelWaitThreadEnd(gp.thread_id, &exitstatus, &timeout);
    if (ret < 0 || exitstatus != 0)
    {
        sceClibPrintf("vitagprof: error on profiler exit. Exit status %i, return code %i\n", exitstatus, ret);
    }

    sceKernelDeleteThread(gp.thread_id);

    if (should_dump) {
        sceClibPrintf("vitagprof: dumping data to %s\n", filename);
        fp = fopen(filename, "wb");
        hdr.lpc = gp.lowpc + delta;
        hdr.hpc = gp.highpc + delta;
        hdr.ncnt = sizeof(hdr) + (sizeof(unsigned int) * gp.ndata);
        hdr.version = GMONVERSION;
        hdr.profrate = SAMPLE_FREQ;
        hdr.resv[0] = 0;
        hdr.resv[1] = 0;
        hdr.resv[2] = 0;
        fwrite(&hdr, 1, sizeof(hdr), fp);

        unsigned int buffer[1024]; // Here to help write faster to file with 4k pages
        memset(buffer, '\0', sizeof(unsigned int) * 1024);
        int j = 0;
        for (i = 0; i < gp.ndata; i++) {
            j = i % 1024;

            if (gp.datas_ptr[i]) {
                buffer[j] = gp.datas_ptr[i]->samples;
            }

            if (j == 1023) {
                // We have a full buffer
                fwrite(buffer, sizeof(unsigned int), 1024, fp);
                memset(buffer, '\0', sizeof(unsigned int) * 1024);
            }
        }

        if (j < 1023 && i > 0) {
            // We have remaining data in buffer
            fwrite(buffer, sizeof(unsigned int), j + 1, fp);
        }

        for (i = 0; i < gp.ndata; i++)
        {
            if (gp.datas_ptr[i] && gp.datas_ptr[i]->arc.selfpc) {
                struct rawarc rc;
                memcpy(&rc, &gp.datas_ptr[i]->arc, sizeof(struct rawarc));
                rc.frompc += delta;
                rc.selfpc += delta;
                fwrite(&rc, sizeof(struct rawarc), 1, fp);
            }
        }

        fclose(fp);
        sceClibPrintf("vitagprof: dumping data done\n");
    }

    free_gprof();
    sceClibPrintf("vitagprof: stopped\n");
}

/** Writes gmon.out dump file and stops profiling
    Called from atexit() handler; will dump out a gmon.out file 
    at cwd with all collected information.
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void __gprof_cleanup()
{
    // TODO : not called ?
    gprof_stop("ux0:/data/gmon.out", 1);
}

/** Internal C handler for _mcount_internal()
    @param frompc    pc address of caller
    @param selfpc    pc address of current function

    Called from mcount.S to make life a bit easier. _mcount_internal is called
    right before a function starts. GCC generates a tiny stub at the very
    beginning of each compiled routine, which eventually brings the 
    control to here. 
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void _mcount_internal(unsigned int frompc, unsigned int selfpc)
{ 
    int e;
    struct rawdata *data;

    if (initialized == 0)
    {
        initialize();
    }

    if (gp.state != GMON_PROF_ON)
    {
        /* returned off for some reason */
        return;
    }

    /* call might come from stack */
    if (frompc >= gp.lowpc && frompc <= gp.highpc)
    {
        gp.pc = selfpc;
        e = (frompc - gp.lowpc) / HISTFRACTION;
        data = gp.datas_ptr[e];
        if (!data) {
            // No data yet for this function, allocate data
            data = malloc(sizeof(struct rawdata));
            if (!data) {
                // Not enough memory ?
                return;
            }
            data->samples = 0;
            data->arc.count = 0;
            gp.datas_ptr[e] = data;
        }
        data->arc.frompc = frompc;
        data->arc.selfpc = selfpc;
        data->arc.count++;
    }
}

/** Internal profiler thread
*/
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
static int profiler_thread(SceSize args, void *argp)
{
    while (gp.state == GMON_PROF_ON)
    {
        unsigned int frompc = gp.pc;

        /* call might come from stack */
        if (frompc >= gp.lowpc && frompc <= gp.highpc)
        {
            // Increment sample
            int e = (frompc - gp.lowpc) / HISTFRACTION;
            struct rawdata *data = gp.datas_ptr[e];
            if (!data) {
                // No data yet for this function, allocate data
                data = malloc(sizeof(struct rawdata));
                if (!data) {
                    // Not enough memory ?
                    continue;
                }
                data->samples = 1;
                data->arc.count = 0;
                data->arc.frompc = 0;
                data->arc.selfpc = 0;
                gp.datas_ptr[e] = data;
            } else {
                data->samples++;
            }
        }

        // Delay until next sample increment
        sceKernelDelayThread(1000 * 1000 / SAMPLE_FREQ);
    }

    return 0;
}
