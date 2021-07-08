/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osWin32Io.c
*
* @brief Win32 Operating System Simulation. Input/output facility.
*
* @version   6
********************************************************************************
*/

#include <windows.h>
#include <process.h>

#include <winbase.h>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <gtOs/gtOsIo.h>

#include <asicSimulation/SCIB/scib.h>

/* the lock,unlock of the 'scib sem' must be only after the scib initialized
   so before that we not take the semaphore , and not release it.
*/
#define PROTECT_TASK_DOING_IO_ACTION_START_MAC      \
    if(simulationInitReady)                         \
    {                                               \
        SCIB_SEM_TAKE;                              \
        protectionDone = 1;                         \
    }

/*unlock semaphore only if we know that we locked it ! */
#define PROTECT_TASK_DOING_IO_ACTION_END_MAC        \
    if(protectionDone)                              \
        SCIB_SEM_SIGNAL

extern GT_U32   simulationInitReady;
static GT_U32   protectionDone = 0;

/************* Global Arguments ***********************************************/

GT_STATUS (*osPrintSyncUartBindFunc)(char *, GT_U32) = NULL;

/************* Static variables ***********************************************/
static OS_BIND_STDOUT_FUNC_PTR writeFunctionPtr = NULL;
static GT_VOID_PTR             writeFunctionParam = NULL;

/************ Public Functions ************************************************/

/**
* @internal osNullStdOutFunction function
* @endinternal
*
* @brief   Get Stdout handler
*/
int osNullStdOutFunction
(
    GT_VOID_PTR         userPtr,
    const char*         buffer,
    int                 length
)
{
    return 0;
}

/**
* @internal osGetStdOutFunction function
* @endinternal
*
* @brief   Get Stdout handler
*
* @param[out] writeFunctionPtrPtr      - poiter to saved output function
* @param[out] writeFunctionParamPtrPtr - poiter to saved output function parameter
*                                       GT_OK
*/
GT_STATUS osGetStdOutFunction
(
    OUT OS_BIND_STDOUT_FUNC_PTR     *writeFunctionPtrPtr,
    OUT GT_VOID_PTR                 *writeFunctionParamPtrPtr
)
{
    *writeFunctionPtrPtr        = writeFunctionPtr;
    *writeFunctionParamPtrPtr   = writeFunctionParam;

    return GT_OK;
}

/**
* @internal osBindStdOut function
* @endinternal
*
* @brief   Bind Stdout to handler
*
* @param[in] writeFunction            - function to call for output
* @param[in] userPtr                  - first parameter to pass to write function
*                                       GT_OK
*/
GT_STATUS osBindStdOut
(
    IN OS_BIND_STDOUT_FUNC_PTR writeFunction,
    IN void* userPtr
)
{
    writeFunctionPtr = writeFunction;
    writeFunctionParam = userPtr;
    return GT_OK;
}

/**
* @internal osPrintf function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osPrintf(const char* format, ...)
{
    char buff[2048];
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

    PROTECT_TASK_DOING_IO_ACTION_START_MAC;

    if (writeFunctionPtr != NULL)
    {
        i =  writeFunctionPtr(writeFunctionParam, buff, i);
    }
    else
    {
        i =  printf("%s", buff);
    }

    PROTECT_TASK_DOING_IO_ACTION_END_MAC;


    return i;
}


/**
* @internal osSprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSprintf(char * buffer, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buffer, format, args);
    va_end(args);

    return i;
}

/**
* @internal osVsprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsprintf(char * buffer, const char* format, va_list args)
{
    return vsprintf(buffer, format, args);
}

/**
* @internal osSnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSnprintf(char * buffer, int size, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = _vsnprintf(buffer, (size_t)size, format, args);
    va_end(args);

    if (i >= size)
    {
        buffer[size-1] = 0;
        i = size-1;
    }

    return i;
}

/**
* @internal osVsnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsnprintf(char * buffer, int size, const char* format, va_list args)
{
    int i;
    i = _vsnprintf(buffer, (size_t)size, format, args);
    if (i >= size)
    {
        buffer[size-1] = 0;
        i = size-1;
    }
    return i;
}

/*******************************************************************************
* osGets
*
* DESCRIPTION:
*       Reads characters from the standard input stream into the array
*       'buffer' until end-of-file is encountered or a new-line is read.
*       Any new-line character is discarded, and a null character is written
*       immediately after the last character read into the array.
*
* INPUTS:
*       buffer  - pointer to buffer to write to
*
* OUTPUTS:
*       buffer  - buffer with read data
*
* RETURNS:
*       A pointer to 'buffer', or a null pointer if end-of-file is
*       encountered and no characters have been read, or there is a read error.
*
* COMMENTS:
*       None
*
*******************************************************************************/
char * osGets(char * buffer)
{
    return gets(buffer);
}

/**
* @internal osPrintSync function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream, in polling mode
*         The device driver will print the string in the same instance this function
*         is called, with no delay.
*/
int osPrintSync(const char* format, ...)
{
    char buff[2048];
    va_list args;
    int i, retVal = 0;


    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

    PROTECT_TASK_DOING_IO_ACTION_START_MAC;
    if ( osPrintSyncUartBindFunc != NULL )
    {
      retVal = (int)((osPrintSyncUartBindFunc)(buff, (GT_U32)strlen(buff)));
    }
    else /* similar to osPrintf(...) */
    {
        if (writeFunctionPtr != NULL)
        {
            i =  writeFunctionPtr(writeFunctionParam, buff, i);
        }
        else
        {
            i =  printf("%s", buff);
        }
    }
    PROTECT_TASK_DOING_IO_ACTION_END_MAC;


    return (retVal);
}

/*******************************************************************************
* osFlastErrorStr
*
* DESCRIPTION:
*       returns error string for last error
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
const char * osFlastErrorStr(void)
{
    return strerror(errno);
}

/**
* @internal osFopen function
* @endinternal
*
* @brief   Opens the file whose name is specified in the parameter filename and associates it with a stream
*         that can be identified in future operations by the OS_FILE pointer returned.
*/
OS_FILE osFopen(const char * fileNamePtr, const char * modePtr)
{
    FILE *fd = fopen(fileNamePtr, modePtr);
    if (!fd)
        return OS_FILE_INVALID;
    return (OS_FILE)fd;
}

#define OS_FILE_TO_FD(_stream) \
    FILE *fd = (FILE *)(_stream); \
    if ((_stream) == OS_FILE_STDOUT) \
        fd = stdout; \
    if ((_stream) == OS_FILE_STDERR) \
        fd = stderr;

/**
* @internal osFclose function
* @endinternal
*
* @brief   Closes the file associated with the stream and disassociates it.
*
* @note Nome
*
*/
int osFclose(OS_FILE stream)
{
    OS_FILE_TO_FD(stream);
    return fclose(fd);
}

/**
* @internal osRewind function
* @endinternal
*
* @brief   Sets the position indicator associated with stream to the beginning of the file.
*
* @note Nome
*
*/
void osRewind(OS_FILE stream)
{
    OS_FILE_TO_FD(stream);
    rewind(fd);
}

/**
* @internal osFprintf function
* @endinternal
*
* @brief   Write a formatted string to the stream.
*/
int osFprintf(OS_FILE stream, const char* format, ...)
{
    va_list args;
    char buffer[2048];
    OS_FILE_TO_FD(stream);

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    return fprintf(fd, "%s", buffer);
}

/*******************************************************************************
* osFgets
*
* DESCRIPTION:
*       Reads characters from stream and stores them as a string into buffer until (num-1) characters have been read
*       or either a newline or the end-of-file is reached, whichever happens first.
*
* INPUTS:
*       bufferPtr  - (pointer to) buffer to write to
*       num - maximum number of characters to be copied into str (including the terminating null-character)
*       stream  - OS_FILE object that specifies the stream.
*
* OUTPUTS:
*       bufferPtr  - (pointer to)buffer with read data
*
* RETURNS:
*       A pointer to 'bufferPtr', or a null pointer if end-of-file is
*       encountered and no characters have been read, or there is a read error.
*
* COMMENTS:
*       None
*
*******************************************************************************/
char * osFgets(char * bufferPtr, int num, OS_FILE stream)
{
    OS_FILE_TO_FD(stream);
    return fgets(bufferPtr, num, fd);
}

/**
* @internal osFwrite function
* @endinternal
*
* @brief   Writes nmemb elements of data, each size bytes long, to the stream,
*         obtaining them from the location given by ptr.
* @param[in] ptr                      - (pointer from) buffer to write from
* @param[in] size                     - element size
* @param[in] nmemb                    - number of elements to write
* @param[in] stream                   - OS_FILE object that specifies the stream.
*                                       Number of items written or less than 0
*/
int osFwrite(
    IN const void *ptr,
    IN int         size,
    IN int         nmemb,
    IN OS_FILE     stream
)
{
    OS_FILE_TO_FD(stream);
    return (int)fwrite(ptr,size,nmemb,fd);
}

/**
* @internal osFread function
* @endinternal
*
* @brief   Reads nmemb elements of data, each size bytes long, from the stream,
*         to the location given by ptr.
* @param[in] size                     - element size
* @param[in] nmemb                    - number of elements to write
* @param[in] stream                   - OS_FILE object that specifies the stream.
*
* @param[out] ptr                      - (pointer to) buffer to read to
*                                       Number of items read or less than 0
*/
int osFread(
    OUT void        *ptr,
    IN  int          size,
    IN  int          nmemb,
    IN  OS_FILE      stream
)
{
    OS_FILE_TO_FD(stream);
    return (int)fread(ptr,size,nmemb,fd);
}

/**
* @internal osFgetLength function
* @endinternal
*
* @brief   Returns lenhth of current file
*
* @param[in] stream                   - OS_FILE object that specifies the stream.
*                                       file length or 0
*/
GT_UINTPTR osFgetLength(
    IN  OS_FILE stream
)
{
    long curr, length;
    OS_FILE_TO_FD(stream);
    curr = ftell(fd);
    fseek(fd, 0, SEEK_END);
    length = ftell(fd);
    fseek(fd, curr, SEEK_SET);
    return (GT_UINTPTR)length;
}



