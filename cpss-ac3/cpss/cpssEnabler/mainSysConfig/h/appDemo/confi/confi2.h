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
* @file confi2.h
*
* @brief Contains API for configuration script processing.
*
* @version   3
********************************************************************************
*/
#ifndef __confi2h
#define __confi2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>

/**
* @struct GT_CONFI_BUFFER
 *
 * @brief This structure contains byte array data and length.
*/
typedef struct{

    GT_CHAR *buf;

    /** buffer size */
    GT_U32 bufSize;

    /** data size */
    GT_U32 dataSize;

} GT_CONFI_BUFFER;

/**
* @internal confi2Read function
* @endinternal
*
* @brief   Read configuration script from input stream.
*
* @retval GT_OK                    - if succeeded,
* @retval GT_FAIL                  - if failed
*/
GT_STATUS   confi2Read();


/**
* @internal confi2FileRead function
* @endinternal
*
* @brief   Read configuration script from file.
*
* @param[in] filename                 - filename.
*
* @retval GT_OK                    - if succeeded,
* @retval GT_FAIL                  - if failed
*/
GT_STATUS   confi2FileRead
(
    IN  GT_U8   *filename
);


/**
* @internal confi2Restore function
* @endinternal
*
* @brief   Restore the confi binary format from the flash
*
* @param[in] theConfiId               - the confi ID
*
* @retval GT_OK                    - succeeded
* @retval GT_OUT_OF_CPU_MEM        - out of CPU memory
* @retval GT_ERROR                 - failed
*
* @note Public.
*
*/
GT_STATUS confi2Restore
(
    IN GT_U8        theConfiId
);


/**
* @internal confi2Store function
* @endinternal
*
* @brief   Store the confi binary format to the flash
*
* @param[out] theConfiId               - the confi ID
*
* @retval GT_OK                    - succeeded
* @retval GT_OUT_OF_CPU_MEM        - out of CPU memory
* @retval GT_ERROR                 - failed
*
* @note Public.
*
*/
GT_STATUS confi2Store
(
    IN GT_U8        theConfiId
);

/**
* @internal confi2Print function
* @endinternal
*
* @brief   Writes the contents of the confi buffer to the standard output.
*
* @retval GT_OK                    - finished successfully
* @retval GT_ERROR                 - error
*
* @note Public
*
*/
GT_STATUS confi2Print
(
    GT_VOID
);


/**
* @internal confi2InitSystem function
* @endinternal
*
* @brief   This is the main board initialization function.
*
* @param[in] theConfiId               - the confi ID
* @param[in] echoOn                   - if GT_TRUE the echo is on, otherwise the echo is off.
*
* @retval GT_OK                    - if succeeded,
* @retval GT_FAIL                  - if failed
*/
GT_STATUS   confi2InitSystem
(
    IN GT_U8        theConfiId, 
    IN GT_BOOL      echoOn
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __confiLoaderh */


