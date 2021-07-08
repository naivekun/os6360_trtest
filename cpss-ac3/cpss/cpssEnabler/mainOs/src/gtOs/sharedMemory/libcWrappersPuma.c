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
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <gtOs/gtOs.h>

/* Define this macro to enable wrapper logging */
/*#define LIBC_WRAPPERS_DEBUG_MAC*/

/************* Functions ******************************************************/
#if 0
/**
* @internal __wrap_alarm function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to alarm.
*
*/
unsigned __wrap_alarm(unsigned seconds)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_alarm called with %d\n", seconds);
#endif
    
    return alarm(seconds);
}

/**
* @internal __wrap_atexit function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to atexit.
*
*/
int __wrap_atexit(void (*function)(void))
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_atexit called with pointer to function  %x\n", function);
#endif

    return atexit(function);
}

/**
* @internal __wrap_chdir function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to chdir.
*
*/
int __wrap_chdir(const char *path)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_chdir called with %s\n", path);
#endif

    return chdir(path);
}

/**
* @internal __wrap_closedir function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to closedir.
*
*/
int __wrap_closedir(DIR *dir)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_closedir called with %s\n", dir);
#endif

    return closedir(dir);
}

/**
* @internal __wrap_fflush function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to fflush.
*
*/
int __wrap_fflush(FILE *stream)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_fflush called with %s\n", stream);
#endif

    return fflush(stream);
}

/*******************************************************************************
* __wrap_getcwd
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for getcwd.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for getcwd.
*
* COMMENTS:
*       Redirects call to getcwd.
*
*******************************************************************************/
char *__wrap_getcwd(char *buf, size_t size)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_getcwd called with buf = %s, size = %d\n", buf, size);
#endif

    return getcwd(buf, size);
}

/*******************************************************************************
* __wrap_getenv
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for getenv.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for getenv.
*
* COMMENTS:
*       Redirects call to getenv.
*
*******************************************************************************/
char *__wrap_getenv(const char *name)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_getenv called with name = %s\n", name);
#endif

    return getenv(name);
}

/*******************************************************************************
* __wrap_getwd
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for getwd.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for getwd.
*
* COMMENTS:
*       Redirects call to getwd.
*
*******************************************************************************/
char *__wrap_getwd(char *buf)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_getwd called with buf = %s\n", buf);
#endif

    return getwd(buf);
}

/**
* @internal __wrap_isspace function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to isspace.
*
*/
int __wrap_isspace(int c)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_isspace called with c = %d\n", c);
#endif

    return isspace(c);
}

/**
* @internal __wrap_log10 function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to log10.
*
*/
double __wrap_log10(double x)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_log10 called with x = %d\n", x);
#endif

    return log10(x);
}

/**
* @internal __wrap_lseek function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to lseek.
*
*/
off_t __wrap_lseek(int fildes, off_t offset, int whence)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_lseek called with fildes  = %d, offset = %d, whence= %d\n", fildes, offset, whence);
#endif

    return lseek(fildes, offset, whence);
}

/*******************************************************************************
* __wrap_memmove
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for memmove.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for memmove.
*
* COMMENTS:
*       Redirects call to memmove.
*
*******************************************************************************/
void *__wrap_memmove(void *dest, const void *src, size_t n)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_memmove called with src  = %x, n = %d\n", src, n);
#endif

    return memmove(dest, src, n);
}

/**
* @internal __wrap_nanosleep function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to nanosleep.
*
*/
int __wrap_nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_nanosleep called with rqtp  = %x, rmtp = %x\n", rqtp, rmtp);
#endif

    return nanosleep(rqtp, rmtp);
}

/**
* @internal __wrap_open function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to open.
*
*/
int __wrap_open(const char *pathname, int flags, ...)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_open called with pathname  = %s, flags = %d\n", pathname, flags);
#endif
    char buff[2048];
    va_list args;
    mode_t mode;

    if(flags & O_CREAT)
    {
        va_start(args, flags);
        vsprintf(buff, "%d", args);
        va_end(args);
        mode = atoi(buff);
        return open(pathname, flags, mode);
    }
    else
        return open(pathname, flags);
}

/*******************************************************************************
* __wrap_opendir
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for opendir.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for opendir.
*
* COMMENTS:
*       Redirects call to opendir.
*
*******************************************************************************/
DIR *__wrap_opendir(const char *name)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_opendir called with name  = %s\n", name);
#endif

    return opendir(name);
}

/*******************************************************************************
* __wrap_popen
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for popen.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for popen.
*
* COMMENTS:
*       Redirects call to popen.
*
*******************************************************************************/
FILE *__wrap_popen(const char *command, const char *type)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_popen called with command  = %s, type = %s\n", command, type);
#endif

    return popen(command, type);
}

/**
* @internal __wrap_pow function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to pow.
*
*/
double __wrap_pow(double x, double y)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_pow called with x = %f, y = %f\n", x, x);
#endif

    return pow(x, y);
}

/**
* @internal __wrap_pthread_mutex_lock function
* @endinternal
*
* @brief   Wrapper function for pthread .
*
* @note Redirects call to pthread_mutex_lock.
*
*/
int __wrap_pthread_mutex_lock(pthread_mutex_t *mutex)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_pthread_mutex_lock called with mutex = %x\n", mutex);
#endif

    return pthread_mutex_lock(mutex);
}

/**
* @internal __wrap_pthread_mutex_unlock function
* @endinternal
*
* @brief   Wrapper function for pthread .
*
* @note Redirects call to pthread_mutex_unlock.
*
*/
int __wrap_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_pthread_mutex_unlock called with mutex = %x\n", mutex);
#endif

    return pthread_mutex_unlock(mutex);
}

/**
* @internal __wrap_putchar function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to putchar.
*
*/
int __wrap_putchar(int c)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_putchar called with c = %d\n", c);
#endif

    return putchar(c);
}

/**
* @internal __wrap_puts function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to puts.
*
*/
int __wrap_puts(const char *s)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_puts called with s = %s\n", s);
#endif

    return puts(s);
}

/**
* @internal __wrap_raise function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to raise.
*
*/
int __wrap_raise(int sig)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_raise called with sig = %s\n", sig);
#endif

    return raise(sig);
}


/*******************************************************************************
* __wrap_readdir
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for readdir.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for readdir.
*
* COMMENTS:
*       Redirects call to readdir.
*
*******************************************************************************/
struct dirent *__wrap_readdir(DIR *dir)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_readdir called with dir = %s\n", dir);
#endif

    return readdir(dir);
}

/**
* @internal __wrap_setvbuf function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to setvbuf.
*
*/
int __wrap_setvbuf(FILE *stream, char *buf, int mode , size_t size)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_setvbuf called with stream = %x, buf = %s, mode = %d, size = %d\n", stream, buf, mode, size);
#endif

    return setvbuf(stream, buf, mode, size);
}

/*******************************************************************************
* __wrap_signal
*
* DESCRIPTION:
*       Wrapper function for libc .
*
* INPUTS:
*       Same as for signal.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Same as for signal.
*
* COMMENTS:
*       Redirects call to signal.
*
*******************************************************************************/
void  (*__wrap_signal(int sig, void (*func)(int)))(int)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_signal called with signum = %d\n", sig);
#endif

    signal(sig, func);
}


/**
* @internal __wrap_srand function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to srand.
*
*/
void __wrap_srand(unsigned int seed)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_srand called with seed = %d\n", seed);
#endif

    srand(seed);
}

/**
* @internal __wrap_time function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to time.
*
*/
time_t __wrap_time(time_t *t)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_time called with t = %x\n", t);
#endif

    return time(t);
}

/**
* @internal __wrap_write function
* @endinternal
*
* @brief   Wrapper function for libc .
*
* @note Redirects call to write.
*
*/
ssize_t __wrap_write(int fd, const void *buf, size_t count)
{
#ifdef LIBC_WRAPPERS_DEBUG_MAC
    osPrintf("__wrap_write called with fd = %d, buf = %s, count = %d\n", fd, buf, count);
#endif

    return write(fd, buf, count);
}
#endif


