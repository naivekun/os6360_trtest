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
* @file osVxIo.c
*
* @brief VxWorks Operating System wrapper. Input/output facility.
*
* @version   4
********************************************************************************
*/

#include <vxWorks.h>
#include <string.h>
#include <stdio.h>

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsGen.h>


/************* Global Arguments ***********************************************/

GT_STATUS (*osPrintSyncUartBindFunc)(char *, GT_U32) = NULL;

/************* Static variables ***********************************************/
static OS_BIND_STDOUT_FUNC_PTR writeFunctionPtr = NULL;
static void* writeFunctionParam = NULL;

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

    if(format == NULL)
	return 0;

    IS_WRAPPER_OPEN_PTR;

    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

    /* The %s is to overcam a compiler warning in ARM5 */
    if (writeFunctionPtr != NULL)
    {
        return writeFunctionPtr(writeFunctionParam, buff, i);
    }

    return printf("%s", buff);
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

    if(format == NULL || buffer == NULL)
	return 0;

    IS_WRAPPER_OPEN_PTR;
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

    IS_WRAPPER_OPEN_PTR;
    va_start(args, format);
    i = vsnprintf(buffer, (size_t)size, format, args);
    va_end(args);

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
    return vsnprintf(buffer, (size_t)size, format, args);
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
    IS_WRAPPER_OPEN_PTR;
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

    IS_WRAPPER_OPEN_PTR;

    va_start(args, format);
    i = vsprintf(buff, format, args);
    va_end(args);

    if ( osPrintSyncUartBindFunc != NULL )
      retVal = (int)((osPrintSyncUartBindFunc)(buff, (GT_U32)strlen(buff)));
      
    return (retVal);
}


