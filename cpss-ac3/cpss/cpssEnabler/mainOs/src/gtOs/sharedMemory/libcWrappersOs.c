/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

/************* Includes *******************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <gtOs/gtOs.h>
#include <gtOs/gtOsMem.h>

/* Define this macro to enable wrapper logging */
/*#define LIBC_WRAPPERS_DEBUG_MAC*/

/************* Functions ******************************************************/
/**
* @internal __wrap_free function
* @endinternal
*
* @brief   Wrapper function for libc free.
*
* @note Redirects call to osFree.
*
*/
void __wrap_free(void *ptr)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_free called with ptr=%p\n", ptr);
#endif
    if(ptr != NULL)
        osFree(ptr);
}

/*******************************************************************************
* __wrap_malloc
*
* DESCRIPTION:
*       Wrapper function for libc malloc.
*
* INPUTS:
*       Same as for malloc.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for malloc.
*
* COMMENTS:
*       Redirects call to osMalloc.
*
*******************************************************************************/
void *__wrap_malloc(size_t size)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_malloc called with size=%d\n", size);
#endif
    return osMalloc(size);
}

/*******************************************************************************
* __wrap_realloc
*
* DESCRIPTION:
*       Wrapper function for libc realloc.
*
* INPUTS:
*       Same as for realloc.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for realloc.
*
* COMMENTS:
*       Redirects call to osRealloc.
*
*******************************************************************************/
void *__wrap_realloc(void* ptr, size_t size)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_realloc called with ptr=%p size=%d\n", ptr, size);
#endif
    return osRealloc(ptr, size);
}

/*******************************************************************************
* __wrap_calloc
*
* DESCRIPTION:
*       Wrapper function for libc calloc.
*
* INPUTS:
*       Same as for calloc.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for calloc.
*
* COMMENTS:
*       Redirects call to osMalloc/osMemSet.
*
*******************************************************************************/
void *__wrap_calloc(size_t nmemb, size_t size)
{
    size_t summary_size=nmemb*size;
    void *ptr;
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_calloc called with nmemb=%d, size=%d\n", nmemb, size);
#endif
    ptr=osMalloc(summary_size);
    if(ptr != NULL)
        osMemSet(ptr, 0, summary_size);
    return ptr;
}

#if 0
/**
* @internal __wrap_memcmp function
* @endinternal
*
* @brief   Wrapper function for libc memcmp.
*
* @note Redirects call to osMemCmp.
*
*/
int __wrap_memcmp(const void *s1, const void *s2, size_t n)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_memcmp called with s1=0x%p, s2=0x%p n=%d\n", s1, s2, n);
#endif
    return osMemCmp(s1, s2, n);
}

/*******************************************************************************
* __wrap_memcpy
*
* DESCRIPTION:
*       Wrapper function for libc memcpy.
*
* INPUTS:
*       Same as for memcpy.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for memcpy.
*
* COMMENTS:
*       Redirects call to osMemCpy.
*
*******************************************************************************/
void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_memcpy called with dest=0x%p, src=0x%p n=%d\n", dest, src, n);
#endif
    return osMemCpy(dest, src, n);
}

/*******************************************************************************
* __wrap_memset
*
* DESCRIPTION:
*       Wrapper function for libc memset.
*
* INPUTS:
*       Same as for memset.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for memset.
*
* COMMENTS:
*       Redirects call to osMemSet.
*
*******************************************************************************/
void *__wrap_memset(void *s, int c, size_t n)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_memset called with s=0x%p, c=%d n=%d\n", s, c, n);
#endif
    return osMemSet(s, c, n);
}

/**
* @internal __wrap_printf function
* @endinternal
*
* @brief   Wrapper function for libc printf.
*
* @note Redirects call to osPrintf.
*
*/
int __wrap_printf(const char *format, ...)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_printf called with format='%s'\n", format);
#endif
    char buff[2048];
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

    return osPrintf(buff);
}

/**
* @internal __wrap_rand function
* @endinternal
*
* @brief   Wrapper function for libc rand.
*
* @note Redirects call to osRand.
*
*/
int __wrap_rand(void)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_rand called\n");
#endif
    return osRand();
}

/*******************************************************************************
* __wrap_strcat
*
* DESCRIPTION:
*       Wrapper function for libc strcat.
*
* INPUTS:
*       Same as for strcat.
*
* OUTPUTS:
*       Same as for strcat.
*
* RETURNS:
*       Same as for strcat.
*
* COMMENTS:
*       Redirects call to osStrCat.
*
*******************************************************************************/
char *__wrap_strcat(char *dest, const char *src)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_strcat called with dest='%s', src='%s'\n", dest, src);
#endif
    return osStrCat(dest, src);
}

/**
* @internal __wrap_strcmp function
* @endinternal
*
* @brief   Wrapper function for libc strcmp.
*
* @note Redirects call to osStrCmp.
*
*/
int __wrap_strcmp(const char *s1, const char *s2)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_strcmp called with s1='%s', s2='%s'\n", s1, s2);
#endif
    return osStrCmp(s1, s2);
}

/*******************************************************************************
* __wrap_strcpy
*
* DESCRIPTION:
*       Wrapper function for libc strcpy.
*
* INPUTS:
*       Same as for strcpy.
*
* OUTPUTS:
*       Same as for strcpy.
*
* RETURNS:
*       Same as for strcpy.
*
* COMMENTS:
*       Redirects call to osStrCpy.
*
*******************************************************************************/
char *__wrap_strcpy(char *dest, const char *src)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_strcpy called with dest=0x%p, src='%s'\n", dest, src);
#endif
    return osStrCpy(dest, src);
}

/**
* @internal __wrap_strlen function
* @endinternal
*
* @brief   Wrapper function for libc strlen.
*
* @note Redirects call to osStrlen.
*
*/
size_t __wrap_strlen(const char *s)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_strlen called with s='%s'\n", s);
#endif
    return osStrlen(s);
}
#endif


