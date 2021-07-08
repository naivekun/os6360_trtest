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
* @file gtOsIo.h
*
* @brief Operating System wrapper. Input/output facility.
*
* @version   9
********************************************************************************
*/

#ifndef __gtOsIoh
#define __gtOsIoh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#if (!defined(__KERNEL__) && !defined(KERNEL) && !defined(INKERNEL) && !defined(_KERNEL))
#include <stdarg.h>
#endif

/************* Definitions ****************************************************/

/* a type for file */
typedef GT_UINTPTR OS_FILE;
#define OS_FILE_INVALID 0
#define OS_FILE_STDOUT  1
#define OS_FILE_STDERR  2

/************* Global Arguments ***********************************************/

extern GT_STATUS (*osPrintSyncUartBindFunc)(char *, GT_U32);

/************* Functions ******************************************************/
typedef int (*OS_BIND_STDOUT_FUNC_PTR)(void* userPtr, const char* buffer, int length);

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
);

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
GT_STATUS osGetStdOutFunction(
    OUT OS_BIND_STDOUT_FUNC_PTR     *writeFunctionPtrPtr,
    OUT GT_VOID_PTR                 *writeFunctionParamPtrPtr
);

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
GT_STATUS osBindStdOut(
    IN OS_BIND_STDOUT_FUNC_PTR writeFunction,
    IN void* userPtr
);

/**
* @internal osPrintf function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osPrintf(const char* format, ...);

/**
* @internal osVprintf function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osVprintf(const char* format, va_list args);

/**
* @internal osSprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSprintf(char * buffer, const char* format, ...);

/**
* @internal osVsprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsprintf(char * buffer, const char* format, va_list args);

/**
* @internal osSnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSnprintf(char * buffer, int size, const char* format, ...);

/**
* @internal osVsnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsnprintf(char * buffer, int size, const char* format, va_list args);

/**
* @internal osPrintSync function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osPrintSync(const char* format, ...);

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
char * osGets(char * buffer);

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
const char * osFlastErrorStr(void);

/**
* @internal osFopen function
* @endinternal
*
* @brief   Opens the file whose name is specified in the parameter filename and associates it with a stream
*         that can be identified in future operations by the OS_FILE pointer returned.
*/
OS_FILE osFopen(const char * fileNamePtr, const char * modePtr);

/**
* @internal osFclose function
* @endinternal
*
* @brief   Closes the file associated with the stream and disassociates it.
*
* @note Nome
*
*/
int osFclose(OS_FILE stream);

/**
* @internal osRewind function
* @endinternal
*
* @brief   Sets the position indicator associated with stream to the beginning of the file.
*
* @note Nome
*
*/
void osRewind(OS_FILE stream);

/**
* @internal osFprintf function
* @endinternal
*
* @brief   Write a formatted string to the stream.
*/
int osFprintf(OS_FILE stream, const char* format, ...);

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
char * osFgets(char * bufferPtr, int num, OS_FILE stream);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsIoh */
/* Do Not Add Anything Below This Line */



