/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief basic os functions implementation for TM
 *
* @file cpss_os_implementation.c
*
* $Revision: 2.0 $
 */

#include <cpss/extServices/cpssExtServices.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <string.h>
#include "tm_os_interface.h"


#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myCpssOsMalloc
(
    IN GT_U32 size
)
{
    return cpssOsMalloc_MemoryLeakageDbg(size,__FILE__ " for Traffic manager",__LINE__);
}
static GT_VOID myCpssOsFree
(
    IN GT_VOID* const memblock
)
{
    cpssOsFree_MemoryLeakageDbg(memblock,__FILE__ " for Traffic manager",__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/* memory */
void *tm_malloc(unsigned int size)
{
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
   return myCpssOsMalloc(size);
#else
   return cpssOsMalloc(size);
#endif
}


void tm_free(void *ptr)
{
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    myCpssOsFree(ptr);
#else
    cpssOsFree(ptr);
#endif
}

void *tm_realloc(void *ptr,unsigned int size)
{
        return cpssOsRealloc(ptr,size);
}

void *tm_memset(void *s, int c,unsigned int n)
{
    return cpssOsMemSet(s, c, n);
}


void *tm_memcpy(void *dest, const void *src, unsigned int n)
{
    return cpssOsMemCpy(dest, src, n);
}


/* mathematics */
double tm_round(double x)
{
    if (x >= 0)
        return (x+0.5);
    else
        return(x-0.5);
}


float tm_roundf(float x)
{
    if (x >= 0)
        return (x+(float)0.5);
    else
        return (x-(float)0.5);
}


int tm_ceil(float x)
{
    int inum = (int)x;
    if (x == (float)inum)
        {
        return inum;
    }
    return inum + 1;
}


float tm_fabs_fl(float x)
{
    if (x < 0)
        {
        return -x;
    }
    return x;
}


double tm_fabs_db(double x)
{
    if (x < 0)
        {
        return -x;
    }
    return x;
}


int tm_abs(int x)
{
    if (x < 0)
        {
        return -x;
    }
    return x;
}

int tm_printf(const char *format, ...)
{
        char                        buffer[TM_MAX_STR_SIZE];
        va_list                        argptr;
        unsigned int        ilen;

    va_start(argptr, format);
    cpssOsVsprintf(buffer, format, argptr);
    va_end(argptr);

    ilen = (unsigned int)cpssOsStrlen(buffer);
    if (ilen >= sizeof(buffer))  /* overflow */
        {
                buffer[sizeof(buffer)-5] = '.';
                buffer[sizeof(buffer)-4] = '.';
        buffer[sizeof(buffer)-3] = '.';
        buffer[sizeof(buffer)-2] = '\n';
        buffer[sizeof(buffer)-1] = '\0';
    }

        return cpssOsPrintf(buffer);
}
