/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssFormatConvert.c
*
* @brief General functions for the bidirectional structured to raw format
* data conversion. The structured data may be either C language struct
* or union.The utility uses array of 32 bits words for raw data structures.
* All functions place the data to the appropriate positions
* of destination buffer using OR operation.
* The destination buffer must be cleared by a caller.
* The sequence of calls with the same destination buffer is
* legitimate if it accumulates the different data.
*
* @version   6
********************************************************************************
*/

#include <cpssCommon/cpssFormatConvert.h>
#include <cpssCommon/cpssPresteraDefs.h>

/**
* @struct PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC
 *
 * @brief Field conversion context
*/
typedef struct{

    CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC *conversionTabPtr;

    /** the conversion function pointer */
    CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_FUNC conversionFuncPtr;

    /** the maximal value to chech */
    GT_U32 maxValue;

    /** @brief the result of last condition
     *  Comments:
     */
    GT_BOOL conditionResult;

} PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC;


/**
* @internal prvStatCpssFmtStcValueGet function
* @endinternal
*
* @brief   Retrieve field from SW structure, union or external memory.
*         Field type should not be array of bytes.
* @param[in] fieldType                - field type
* @param[in] dataOffset               - Relative offset in bytes within the SW structure,
*                                      union or work memory.
* @param[in] stcDataPtr               - software formatted Data
* @param[in] extMemoPtr               - external memory for not standard fields
*
* @param[out] dataPtr                  - raw formatted Data
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported field type
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtStcValueGet
(
    IN  CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT  fieldType,
    IN  GT_U32                              dataOffset,
    IN  GT_VOID                             *stcDataPtr,
    IN  GT_VOID                             *extMemoPtr,
    OUT GT_U32                              *dataPtr
)
{
    GT_U8  *swDataPtr;        /* structured data address      */
    GT_U8  *workMemoPtr;      /* external memory data address */

    swDataPtr   = (GT_U8*)stcDataPtr + dataOffset;
 /*   workMemoPtr assignment is in an appropriate case */

    switch (fieldType)
    {
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_1BYTE_E:
            *dataPtr = *(GT_U8*)swDataPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_2BYTES_E:
            *dataPtr = *(GT_U16*)swDataPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_4BYTES_E:
            *dataPtr = *(GT_U32*)swDataPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_BOOL_E:
            *dataPtr = (*(GT_BOOL*)swDataPtr == GT_FALSE) ? 0 : 1;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_INVERTED_BOOL_E:
            *dataPtr = (*(GT_BOOL*)swDataPtr == GT_FALSE) ? 1 : 0;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_1BYTE_E:
            workMemoPtr = (GT_U8*)extMemoPtr + dataOffset;
            *dataPtr = *(GT_U8*)workMemoPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_2BYTES_E:
            workMemoPtr = (GT_U8*)extMemoPtr + dataOffset;
            *dataPtr = *(GT_U16*)workMemoPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_4BYTES_E:
            workMemoPtr = (GT_U8*)extMemoPtr + dataOffset;
            *dataPtr = *(GT_U32*)workMemoPtr;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_CONSTANT_VALUE_E:
            *dataPtr = dataOffset;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvStatCpssFmtContextManage function
* @endinternal
*
* @brief   The function manages the context of the conversion
*
* @param[in,out] savedContextPtr          - saved context
* @param[in] extraInfoPtr             - value of extra info from conversion entry
* @param[in] managmentCmd             - managment command
* @param[in] dataType                 - extraInfoPtr data type
* @param[in] stcDataPtr               - pointer to stc data
* @param[in] extMemoPtr               - pointer to external memory
* @param[in,out] savedContextPtr          - saved context
*
* @param[out] currentContextPtr        - current context
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - sourcve value not foun in table
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtContextManage
(
    INOUT  PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC *savedContextPtr,
    OUT    PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC *currentContextPtr,
    IN     GT_VOID                                   *extraInfoPtr,
    IN     CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_ENT     managmentCmd,
    IN     CPSS_FORMAT_CONVERT_FIELD_CONTEXT_ENT     dataType,
    IN     GT_VOID                                   *stcDataPtr,
    IN     GT_VOID                                   *extMemoPtr
)
{
    CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC       *condTabPtr; /* condition          */
    GT_U32                                        data;        /* tested field value */
    GT_STATUS                                     rc;          /* return code        */
    GT_BOOL                                       result;      /* condition result   */

    /* check type of data stored in the extraInfoPtr*/
    switch (dataType)
    {
        case CPSS_FORMAT_CONVERT_FIELD_CONTEXT_CONDITION_E:
             /* condition is stored in the extraInfoPtr */
            condTabPtr = (CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC*)extraInfoPtr;

            /* get value of condition field and apply mask and pattern */
            rc = prvStatCpssFmtStcValueGet(
                condTabPtr->fieldTypeOrSizeOf, condTabPtr->stcOrExtMemByteOffset,
                stcDataPtr, extMemoPtr, &data);

            result = ((data & condTabPtr->mask) == condTabPtr->pattern)
                ? GT_TRUE : GT_FALSE;
            if (rc != GT_OK)
            {
                result = GT_FALSE;
            }

            /* handle condition's result */
            switch (managmentCmd)
            {
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_SAVE_NOT_USE_E:
                    /* save it to use in next conversion entry */
                    savedContextPtr->conditionResult = result;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_AND_SAVE_E:
                    /* save it to use in next conversion entry
                      and use in the current context */
                    savedContextPtr->conditionResult = result;
                    currentContextPtr->conditionResult = result;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_NOT_SAVE_E:
                    /* use it in currect context and don't save it to use
                       in next conversion entry */
                    currentContextPtr->conditionResult = result;
                    return GT_OK;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        case CPSS_FORMAT_CONVERT_FIELD_CONTEXT_CONVERT_FUNC_E:
            /* conversion function for fields values
               is stored in the extraInfoPtr */
            switch (managmentCmd)
            {
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_SAVE_NOT_USE_E:
                    /* save it to use in next conversion entry */
                    savedContextPtr->conversionFuncPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_FUNC)extraInfoPtr;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_AND_SAVE_E:
                    /* save it to use in next conversion entry
                      and use in the current context */
                    savedContextPtr->conversionFuncPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_FUNC)extraInfoPtr;
                    currentContextPtr->conversionFuncPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_FUNC)extraInfoPtr;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_NOT_SAVE_E:
                    /* use it in currect context and don't save it to use
                       in next conversion entry */
                    currentContextPtr->conversionFuncPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_FUNC)extraInfoPtr;
                    return GT_OK;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        case CPSS_FORMAT_CONVERT_FIELD_CONTEXT_CONVERT_TAB_E:
            /* conversion table for fields values
               is stored in the extraInfoPtr */
            switch (managmentCmd)
            {
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_SAVE_NOT_USE_E:
                    /* save it to use in next conversion entry */
                    savedContextPtr->conversionTabPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC*)extraInfoPtr;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_AND_SAVE_E:
                    /* save it to use in next conversion entry
                      and use in the current context */
                    savedContextPtr->conversionTabPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC*)extraInfoPtr;
                    currentContextPtr->conversionTabPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC*)extraInfoPtr;
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_NOT_SAVE_E:
                    /* use it in currect context and don't save it to use
                       in next conversion entry */
                    currentContextPtr->conversionTabPtr =
                        (CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC*)extraInfoPtr;
                    return GT_OK;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        case CPSS_FORMAT_CONVERT_FIELD_CONTEXT_MAXVALUE_E:
            /* maximal value for field check
               is stored in the extraInfoPtr */
            switch (managmentCmd)
            {
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_SAVE_NOT_USE_E:
                    /* save it to use in next conversion entry */
                    savedContextPtr->maxValue =
                        (GT_U32)((GT_U8*)extraInfoPtr - (GT_U8*)0);
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_AND_SAVE_E:
                    /* save it to use in next conversion entry
                      and use in the current context */
                    savedContextPtr->maxValue =
                        (GT_U32)((GT_U8*)extraInfoPtr - (GT_U8*)0);
                    currentContextPtr->maxValue =
                        (GT_U32)((GT_U8*)extraInfoPtr - (GT_U8*)0);
                    return GT_OK;
                case CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_USE_NOT_SAVE_E:
                    /* use it in currect context and don't save it to use
                       in next conversion entry */
                    currentContextPtr->maxValue =
                        (GT_U32)((GT_U8*)extraInfoPtr - (GT_U8*)0);
                    return GT_OK;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        default:
            break;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal prvStatCpssFmtValueConvert function
* @endinternal
*
* @brief   The function converts bidirectional the value by conversion table
*
* @param[in] contextPtr               - pointer to the currenr context
* @param[in] convertCmd               - conversion command
* @param[in] stc2raw                  - GT_TRUE - STC2RAW, GT_FALSE - RAW2STC
* @param[in] srcData                  - data to be converted
*
* @param[out] dstDataPtr               - (pointer to) converted data
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - sourcve value not foun in table
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtValueConvert
(
    IN  PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC *contextPtr,
    IN  CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT convertCmd,
    IN  GT_BOOL                                   stc2raw,
    IN  GT_U32                                    srcData,
    OUT GT_U32                                    *dstDataPtr
)
{

    CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC *cnvTblPtr; /* cnv Tbl Ptr */

    switch (convertCmd)
    {
        case CPSS_FORMAT_CONVERT_FIELD_CONVERT_BY_FUNCTION_E:
            if (contextPtr->conversionFuncPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            return contextPtr->conversionFuncPtr(stc2raw, srcData, dstDataPtr);

        case CPSS_FORMAT_CONVERT_FIELD_CONVERT_BY_TABLE_E:
            if (contextPtr->conversionTabPtr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            cnvTblPtr = contextPtr->conversionTabPtr;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* conversion by table */

    if (stc2raw == GT_FALSE)
    {
        /* convert raw value to STC one */
        for (;
              ((cnvTblPtr->stcValue != 0xFFFFFFFF)
               && (cnvTblPtr->rawValue != 0xFFFFFFFF));
              cnvTblPtr ++)
        {
            /* compare raw value of DB with source data */
            if (cnvTblPtr->rawValue == srcData)
            {
                /* ouput STC value of DB */
                *dstDataPtr = cnvTblPtr->stcValue;
                return GT_OK;
            }
        }
    }
    else
    {
        /* convert STC value to raw one */
        for (;
              ((cnvTblPtr->stcValue != 0xFFFFFFFF)
               && (cnvTblPtr->rawValue != 0xFFFFFFFF));
              cnvTblPtr ++)
        {
            /* compare STC value of DB with source data */
            if (cnvTblPtr->stcValue == srcData)
            {
                /* ouput raw value of DB */
                *dstDataPtr = cnvTblPtr->rawValue;
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}


/**
* @internal prvStatCpssFmtValueToRawPut function
* @endinternal
*
* @brief   The function puts data to HW buffer
*
* @param[in] rawBitOffset             - offset (in bits) in the raw data
* @param[in] rawBitLength             - amounts of bits in the raw data
* @param[in] stcBitOffset             - the offset (in bits) from the origin of the
*                                      structure field (for field splitted to
*                                      several segmetnts)
* @param[in] contextPtr               - pointer to the currenr context
* @param[in] convertCmd               - conversion command
* @param[in] data                     - source ata
*
* @param[out] rawDataPtr               - hardware formatted Data origin pointer
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - sourcve value not foun in table
* @retval GT_BAD_PARAM             - on wrong data
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtValueToRawPut
(
    IN  GT_U32                                          stcBitOffset,
    IN  GT_U32                                          rawBitOffset,
    IN  GT_U32                                          rawBitLength,
    IN  PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC       *contextPtr,
    IN  CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT       convertCmd,
    IN  GT_U32                                          data,
    OUT GT_U32                                          *rawDataPtr
)
{
    GT_STATUS  rc;              /* return code              */
    GT_U32     *outDataPtr;     /* pointer into rawDataPtr  */
    GT_U32     rawOff;          /* bit offset in raw buffer */
    GT_U32     bitLen;          /* length in bits           */

    if (rawBitLength > 32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (convertCmd != CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E)
    {
        rc = prvStatCpssFmtValueConvert(
                contextPtr, convertCmd, GT_TRUE, data, &data);
    }
    else
        rc = GT_OK;

    outDataPtr = rawDataPtr + (rawBitOffset >> 5) /* (rawBitOffset / 32) */;
    rawOff     = rawBitOffset & 0x1f /* rawBitOffset % 32 */ ;

    bitLen     = rawBitLength;
    data       >>= stcBitOffset;

    if ((bitLen + rawOff) > 32)
    {
        bitLen = (bitLen + rawOff) - 32;
        *(outDataPtr + 1) |= ((data >> (32 - rawOff)) & ((1 << bitLen) - 1));
        bitLen = rawBitLength - bitLen;
    }

    if (bitLen < 32)
    {
        /* clear all out of range bits */
        data &= ((1 << bitLen) - 1);
    }

    *outDataPtr |= (data << rawOff);

    return rc;
}

/**
* @internal prvStatCpssFmtValueFromRawGet function
* @endinternal
*
* @brief   The function gets data from raw buffer
*
* @param[in] stcBitOffset             - offset (in bits) from bit0 in output data
* @param[in] rawBitOffset             - offset (in bits) in the raw data
* @param[in] rawBitLength             - amounts of bits in the raw data
* @param[in] contextPtr               - pointer to the currenr context
* @param[in] convertCmd               - conversion command
* @param[in] rawDataPtr               - source raw Data buffer
*
* @param[out] dataPtr                  - STC formatted Data
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - sourcve value not found in table
* @retval GT_BAD_PARAM             - on wrong data
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtValueFromRawGet
(
    IN  GT_U32                                          stcBitOffset,
    IN  GT_U32                                          rawBitOffset,
    IN  GT_U32                                          rawBitLength,
    IN  PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC       *contextPtr,
    IN  CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT       convertCmd,
    IN  GT_U32                                          *rawDataPtr,
    OUT GT_U32                                          *dataPtr
)
{
    GT_STATUS  rc;          /* return code              */
    GT_U32     *inDataPtr;  /* pointer into rawDataPtr  */
    GT_U32     data;        /* work variable            */
    GT_U32     rawOff;      /* bit offset in raw buffer */

    if ((convertCmd != CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E)
        && (stcBitOffset != 0))
    {
        /* the splitted field conversion not supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check STC field boundary */
    if ((rawBitLength + stcBitOffset) > 32)
    {
        /* STC field doesn't support more then 32 bits */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    inDataPtr = rawDataPtr + (rawBitOffset >> 5) /* (rawBitOffset / 32) */;
    rawOff    = rawBitOffset & 0x1f /* rawBitOffset % 32 */;
    data      = 0;

    if ((rawBitLength + rawOff) > 32)
    {
        data = (*(inDataPtr + 1) << (32 - rawOff));
    }

    data |= (*inDataPtr >> rawOff);

    if (rawBitLength < 32)
    {
        /* clear all out of range bits */
        data &= ((1 << rawBitLength) - 1);
    }

    if (convertCmd != CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E)
    {
        rc = prvStatCpssFmtValueConvert(
            contextPtr, convertCmd, GT_FALSE, data, &data);
    }
    else
        rc = GT_OK;

    *dataPtr |= (data << stcBitOffset);

    return rc;
}

/**
* @internal prvStatCpssFmtStcValuePut function
* @endinternal
*
* @brief   The function puts short field and converts it to HW format
*
* @param[in] fieldType                - field type
* @param[in] data                     - SW data Offset
* @param[in] data                     - softare formatted Data
*
* @param[out] stcDataPtr               - software formatted Data
* @param[out] extMemoPtr               - external memory for not standard fields
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - sourcve value not foun in table
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtStcValuePut
(
    IN  CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT          fieldType,
    IN  GT_U32                                      dataOffset,
    IN  GT_U32                                      data,
    OUT GT_VOID                                     *stcDataPtr,
    OUT GT_VOID                                     *extMemoPtr
)
{
    GT_U8 *swDataPtr;          /* structured data */
    GT_U8 *workMemoPtr;        /* external memory */

    swDataPtr   = (GT_U8*)stcDataPtr + dataOffset;
    workMemoPtr = (GT_U8*)extMemoPtr + dataOffset;

    switch (fieldType)
    {
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_1BYTE_E:
            *(GT_U8*)swDataPtr |= (GT_U8)data;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_2BYTES_E:
            *(GT_U16*)swDataPtr |= (GT_U16)data;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_SIZEOF_4BYTES_E:
            *(GT_U32*)swDataPtr |= (GT_U32)data;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_BOOL_E:
            *(GT_BOOL*)swDataPtr |= (data == 0) ? GT_FALSE : GT_TRUE;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_INVERTED_BOOL_E:
            *(GT_BOOL*)swDataPtr |= (data == 0) ? GT_TRUE : GT_FALSE ;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_1BYTE_E:
            *(GT_U8*)workMemoPtr |= (GT_U8)data;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_2BYTES_E:
            *(GT_U16*)workMemoPtr |= (GT_U16)data;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_EXT_MEM_SIZEOF_4BYTES_E:
            *(GT_U32*)workMemoPtr |= (GT_U32)data;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvStatCpssFmtStcValueCheck function
* @endinternal
*
* @brief   Checkes the field value validity
*
* @param[in] checkCmd                 - check command
* @param[in] maxValue                 -  for CPSS_FORMAT_CONVERT_FIELD_CHECK_BY_MAXVALUE_E
* @param[in] bitLength                -  for CPSS_FORMAT_CONVERT_FIELD_CHECK_BITS_E
*                                      checked that not relevant bits are zeros
* @param[in] data                     - checked value
*
* @param[out] validPtr                 - (pointer to) GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported checkCmd
*
* @note NONE
*
*/
static GT_STATUS prvStatCpssFmtStcValueCheck
(
    IN  CPSS_FORMAT_CONVERT_FIELD_CHECK_ENT  checkCmd,
    IN  GT_U32                               maxValue,
    IN  GT_U32                               bitLength,
    IN  GT_U32                               data,
    OUT GT_BOOL                             *validPtr
)
{
    switch (checkCmd)
    {
        case CPSS_FORMAT_CONVERT_FIELD_CHECK_BITS_E:
            *validPtr =  ((data & (0xFFFFFFFF << bitLength)) == 0)
                         ? GT_TRUE : GT_FALSE;
            break;
        case CPSS_FORMAT_CONVERT_FIELD_CHECK_BY_MAXVALUE_E:
            *validPtr = (data <= maxValue) ? GT_TRUE : GT_FALSE;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssFormatConvertStcToRaw function
* @endinternal
*
* @brief   The function converts data from SW structure or union to
*         HW memory entry format
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] fieldsInfoArrPtr         - field descriptors array
* @param[in] stcDataPtr               - this is pointer to first byte of SW structure
* @param[in] extMemoPtr               - memory to store:
*                                      condition variables for fields with condition checks
*                                      field's HW values for fields that cannot be
*                                      converted by simple SW to HW conversion
*
* @param[out] rawDataPtr               - raw formatted Data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssFormatConvertStcToRaw
(
    IN  CPSS_FORMAT_CONVERT_FIELD_INFO_STC *fieldsInfoArrPtr,
    IN  GT_VOID                            *stcDataPtr,
    IN  GT_VOID                            *extMemoPtr,
    OUT GT_U32                             *rawDataPtr
)
{
    GT_STATUS                            rc;          /* return code          */
    GT_STATUS                            rc1;         /* return code          */
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC   *fieldPtr;   /* field Info Ptr       */
    GT_U32                               data;        /* data                 */
    GT_BOOL                              valid;       /* the value is valid   */
    GT_U32                               arrBitLen;   /* array copy BitLen    */
    GT_U32                               arrRawBitOff;/* array copy Raw Offset*/
    GT_U32                               i;           /* loop index           */
    GT_U32                               bitLen;      /* length in bites      */
    GT_U32                               bitOff;      /* offset in bites      */

    /* data, unpacked from fieldPtr->fieldCommand */
    CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT        fieldType;
    CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_ENT     ptrContextCmd;
    CPSS_FORMAT_CONVERT_FIELD_CONTEXT_ENT     ptrContextType;
    GT_U32                                    conditionDep;
    CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT convertCmd;
    CPSS_FORMAT_CONVERT_FIELD_CHECK_ENT       checkCmd;

    /* saved and current field contexts */
    PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC savedContext;
    PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC currentContext;

    /* initialize context */
    savedContext.conditionResult = GT_FALSE;
    savedContext.conversionFuncPtr = NULL;
    savedContext.conversionTabPtr = NULL;
    savedContext.maxValue = 0;

    rc = GT_OK;

    for (fieldPtr = fieldsInfoArrPtr; (1); fieldPtr++)
    {
        CPSS_FORMAT_CONVERT_FIELD_COMMAND_UNPACK_MAC(
            fieldPtr->fieldCommand, fieldType, ptrContextCmd,
            ptrContextType, conditionDep, convertCmd, checkCmd);

        /* copy saved context to the current */
        currentContext = savedContext;

        if (ptrContextCmd != CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_NONE_E)
        {
            /* for the substructure field the pointer is busy by substructure
               conversion table but not by context management*/
            if (fieldType != CPSS_FORMAT_CONVERT_FIELD_STC_PTR_E)
            {
                /* get updated current and saved contexts */
                rc1 = prvStatCpssFmtContextManage(
                    &savedContext, &currentContext, fieldPtr->extraInfoPtr,
                    ptrContextCmd, ptrContextType, stcDataPtr, extMemoPtr);
                if (rc1 != GT_OK)
                {
                    rc = GT_BAD_PARAM;
                    if (fieldType == CPSS_FORMAT_CONVERT_FIELD_ENDLIST_E)
                    {
                        /* to prevent endless loop on error */
                        break;
                    }
                    continue;
                }
            }
        }


        /* check condition for entries with condition dependency */
        if ((conditionDep != 0) && (currentContext.conditionResult == GT_FALSE))
        {
            /* current entry doesn't satisfy a condition.
               Goto next entry.*/
            continue;
        }

        switch (fieldType)
        {
            case CPSS_FORMAT_CONVERT_FIELD_ENDLIST_E:
                if(rc != GT_OK)
                {
                    /* dipper debug is needed if failed here */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                /* the END LIST is after condition check to provide the
                   conditioned termination */
                return rc;

            case CPSS_FORMAT_CONVERT_FIELD_DUMMY_E:
                continue;

            case CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E:
                /* ignore conversion */
                convertCmd = CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E;

                bitLen = fieldPtr->rawBitLength;
                bitOff = fieldPtr->stcBitOffset;
                if (bitOff > 7)
                {
                    bitOff %= 8;
                    rc = GT_BAD_PARAM;
                }

                /* from last network order byte to origin */

                /* last byte index */
                i      = (bitLen + bitOff - 1) >> 3; /* >> 3 is / 8*/

                /* raw bit offset */
                arrRawBitOff = fieldPtr->rawBitOffset;
                /* length (bits) of cuurently copied portion */
                arrBitLen    = ((bitLen > (8 - bitOff)) ? (8 - bitOff) : bitLen);

                for (; (bitLen > 0); i--)
                {
                    data = *((GT_U8*)stcDataPtr + fieldPtr->stcByteOffsetOrConst + i);

                    rc1= prvStatCpssFmtValueToRawPut(
                        bitOff, arrRawBitOff, arrBitLen,
                        &currentContext, convertCmd, data, rawDataPtr);
                    if (GT_OK != rc1)
                    {
                        rc = GT_BAD_PARAM;
                    }

                    arrRawBitOff  += arrBitLen;
                    bitOff        = 0;
                    bitLen        -= arrBitLen;
                    arrBitLen     = (bitLen >= 8) ? 8 : bitLen;
                }

                continue;
            case CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_ASCENT_E:
                /* ignore conversion */
                convertCmd = CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E;

                bitLen = fieldPtr->rawBitLength;
                bitOff = fieldPtr->stcBitOffset;
                if (bitOff > 7)
                {
                    bitOff %= 8;
                    rc = GT_BAD_PARAM;
                }

                /* raw bit offset */
                arrRawBitOff = fieldPtr->rawBitOffset;
                /* length (bits) of cuurently copied portion */
                arrBitLen    = ((bitLen > (8 - bitOff)) ? (8 - bitOff) : bitLen);

                for (i = 0; (bitLen > 0); i++)
                {
                    data = *((GT_U8*)stcDataPtr
                        + fieldPtr->stcByteOffsetOrConst + i);

                    rc1= prvStatCpssFmtValueToRawPut(
                        bitOff, arrRawBitOff, arrBitLen,
                        &currentContext, convertCmd, data, rawDataPtr);
                    if (GT_OK != rc1)
                    {
                        rc = GT_BAD_PARAM;
                    }

                    arrRawBitOff  += arrBitLen;
                    bitOff        = 0;
                    bitLen        -= arrBitLen;
                    arrBitLen     = (bitLen >= 8) ? 8 : bitLen;
                }

                continue;

            case CPSS_FORMAT_CONVERT_FIELD_STC_PTR_E:

                if (fieldPtr->extraInfoPtr == NULL)
                {
                    rc = GT_BAD_PARAM;
                    continue;
                }

                /* recursive handling of substructures */
                rc1 = cpssFormatConvertStcToRaw(
                    (CPSS_FORMAT_CONVERT_FIELD_INFO_STC*)fieldPtr->extraInfoPtr,
                    ((GT_U8*)stcDataPtr + fieldPtr->stcByteOffsetOrConst),
                    extMemoPtr, (rawDataPtr + fieldPtr->rawBitOffset));
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                }
                continue;

            default:
                /* all scalar fields 1-4 bytes */
                rc1 = prvStatCpssFmtStcValueGet(
                    fieldType, fieldPtr->stcByteOffsetOrConst,
                    stcDataPtr, extMemoPtr, &data);
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                    continue;
                }

                if(checkCmd != CPSS_FORMAT_CONVERT_FIELD_CHECK_NONE_E)
                {
                    /* splitted field */
                    if ((fieldPtr->stcBitOffset != 0)
                        && (checkCmd == CPSS_FORMAT_CONVERT_FIELD_CHECK_BITS_E))
                    {
                        checkCmd = CPSS_FORMAT_CONVERT_FIELD_CHECK_NONE_E;
                    }

                    rc1 = prvStatCpssFmtStcValueCheck(
                        checkCmd, currentContext.maxValue, fieldPtr->rawBitLength,
                        data, &valid);
                    if ((GT_OK != rc1) || (valid == GT_FALSE))
                    {
                        rc = GT_BAD_PARAM;
                    }
                }

                rc1 = prvStatCpssFmtValueToRawPut(
                    fieldPtr->stcBitOffset, fieldPtr->rawBitOffset,
                    fieldPtr->rawBitLength, &currentContext, convertCmd,
                    data, rawDataPtr);
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                }

                continue;
        }
    }

    if(rc != GT_OK)
    {
        /* dipper debug is needed if failed here */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssFormatConvertRawToStc function
* @endinternal
*
* @brief   The function converts data from raw formatted memory entry format to
*         SW structure or union.
*
* @note   APPLICABLE DEVICES:      All devices
* @param[in] fieldsInfoArrPtr         - field descriptors array
* @param[in] rawDataPtr               - raw formatted Data
* @param[in,out] extMemoPtr               - memory to store:
*                                      condition variables for fields with condition checks
*
* @param[out] stcDataPtr               - software formatted Data
* @param[in,out] extMemoPtr               - memory to get:
*                                      field's HW values for fields that cannot be converted by
*                                      simple HW to SW conversion
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssFormatConvertRawToStc
(
    IN    CPSS_FORMAT_CONVERT_FIELD_INFO_STC *fieldsInfoArrPtr,
    IN    GT_U32                             *rawDataPtr,
    OUT   GT_VOID                            *stcDataPtr,
    INOUT GT_VOID                            *extMemoPtr
)
{
    GT_STATUS                            rc;          /* return code          */
    GT_STATUS                            rc1;         /* return code          */
    CPSS_FORMAT_CONVERT_FIELD_INFO_STC   *fieldPtr;   /* field Info Ptr       */
    GT_U32                               data;        /* data                 */
    GT_U32                               arrBitLen;   /* array copy BitLen    */
    GT_U32                               arrRawBitOff;/* array copy Raw Offset*/
    GT_U32                               i;           /* loop index           */
    GT_U32                               bitLen;      /* length in bites      */
    GT_U32                               bitOff;      /* offset in bites      */

    /* data, unpacked from fieldPtr->fieldCommand */
    CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT        fieldType;
    CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_ENT     ptrContextCmd;
    CPSS_FORMAT_CONVERT_FIELD_CONTEXT_ENT     ptrContextType;
    GT_U32                                    conditionDep;
    CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT convertCmd;
    CPSS_FORMAT_CONVERT_FIELD_CHECK_ENT       checkCmd GT_UNUSED;

    /* saved and current field contexts */
    PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC savedContext;
    PRV_CPSS_FORMAT_CONVERT_FIELD_CONTEXT_STC currentContext;

    /* initialize contexts */
    savedContext.conditionResult = GT_FALSE;
    savedContext.conversionFuncPtr = NULL;
    savedContext.conversionTabPtr = NULL;
    savedContext.maxValue = 0;

    rc = GT_OK;

    for (fieldPtr = fieldsInfoArrPtr; (1); fieldPtr++)
    {
        CPSS_FORMAT_CONVERT_FIELD_COMMAND_UNPACK_MAC(
            fieldPtr->fieldCommand, fieldType, ptrContextCmd,
            ptrContextType, conditionDep, convertCmd, checkCmd);

        /* copy saved context to the current */
        currentContext = savedContext;

        if (ptrContextCmd != CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_NONE_E)
        {
            /* for the substructure field the pointer is busy by substructure
               conversion table but not by context management*/
            if (fieldType != CPSS_FORMAT_CONVERT_FIELD_STC_PTR_E)
            {
                /* get updated current and saved contexts */
                rc1 = prvStatCpssFmtContextManage(
                    &savedContext, &currentContext, fieldPtr->extraInfoPtr,
                    ptrContextCmd, ptrContextType, stcDataPtr, extMemoPtr);
                if (rc1 != GT_OK)
                {
                    rc = GT_BAD_PARAM;
                    if (fieldType == CPSS_FORMAT_CONVERT_FIELD_ENDLIST_E)
                    {
                        /* to prevent endless loop on error */
                        break;
                    }
                    continue;
                }
            }
        }

        /* check condition for entries with condition dependency */
        if ((conditionDep != 0) && (currentContext.conditionResult == GT_FALSE))
        {
            /* currect entry doesn't satisfy a condition.
               Goto next entry.*/
            continue;
        }

        switch (fieldType)
        {
            case CPSS_FORMAT_CONVERT_FIELD_ENDLIST_E:
                /* the END LIST is after condition check to provide the
                   conditioned termination */
                return rc;

            case CPSS_FORMAT_CONVERT_FIELD_DUMMY_E:
                continue;

            case CPSS_FORMAT_CONVERT_FIELD_CONSTANT_VALUE_E:
                continue;

            case CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_E:
                /* ignore conversion */
                convertCmd = CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E;

                bitLen = fieldPtr->rawBitLength;
                bitOff = fieldPtr->stcBitOffset;
                if (bitOff > 7)
                {
                    bitOff %= 8;
                    rc = GT_BAD_PARAM;
                }

                /* from last network order byte to origin */

                /* last byte index */
                i      = (bitLen + bitOff - 1) >> 3; /* >> 3 is / 8*/
                /* raw bit offset */
                arrRawBitOff = fieldPtr->rawBitOffset;
                /* length (bits) of cuurently copied portion */
                arrBitLen    = ((bitLen > (8 - bitOff)) ? (8 - bitOff) : bitLen);

                for (; (bitLen > 0); i--)
                {
                    data = 0;
                    rc1 = prvStatCpssFmtValueFromRawGet(
                        bitOff, arrRawBitOff, arrBitLen,
                        &currentContext, convertCmd, rawDataPtr, &data);
                    if (GT_OK != rc1)
                    {
                        rc = GT_BAD_PARAM;
                    }

                    *((GT_U8*)stcDataPtr + fieldPtr->stcByteOffsetOrConst + i) |=
                        (GT_U8)data ;

                    arrRawBitOff  += arrBitLen;
                    bitOff        = 0;
                    bitLen        -= arrBitLen;
                    arrBitLen     = (bitLen >= 8) ? 8 : bitLen;
                }

                continue;

            case CPSS_FORMAT_CONVERT_FIELD_BYTE_ARRAY_ASCENT_E:

                /* ignore conversion */
                convertCmd = CPSS_FORMAT_CONVERT_FIELD_CONVERT_NONE_E;

                bitLen = fieldPtr->rawBitLength;
                bitOff = fieldPtr->stcBitOffset;
                if (bitOff > 7)
                {
                    bitOff %= 8;
                    rc = GT_BAD_PARAM;
                }

                /* raw bit offset */
                arrRawBitOff = fieldPtr->rawBitOffset;
                /* length (bits) of cuurently copied portion */
                arrBitLen    = ((bitLen > (8 - bitOff)) ? (8 - bitOff) : bitLen);

                for (i = 0; (bitLen > 0); i++)
                {
                    data = 0;
                    rc1 = prvStatCpssFmtValueFromRawGet(
                        bitOff, arrRawBitOff, arrBitLen,
                        &currentContext, convertCmd, rawDataPtr, &data);
                    if (GT_OK != rc1)
                    {
                        rc = GT_BAD_PARAM;
                    }

                    *((GT_U8*)stcDataPtr + fieldPtr->stcByteOffsetOrConst + i) |=
                        (GT_U8)data ;

                    arrRawBitOff  += arrBitLen;
                    bitOff        = 0;
                    bitLen        -= arrBitLen;
                    arrBitLen     = (bitLen >= 8) ? 8 : bitLen;
                }

                continue;

            case CPSS_FORMAT_CONVERT_FIELD_STC_PTR_E:

                if (fieldPtr->extraInfoPtr == NULL)
                {
                    rc = GT_BAD_PARAM;
                    continue;
                }

                rc1 = cpssFormatConvertRawToStc(
                    (CPSS_FORMAT_CONVERT_FIELD_INFO_STC*)fieldPtr->extraInfoPtr,
                    (rawDataPtr + fieldPtr->rawBitOffset),
                    ((GT_U8*)stcDataPtr + fieldPtr->stcByteOffsetOrConst),
                    extMemoPtr);
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                }
                continue;

            default:

                /* all scalar fields 1-4 bytes */
                data = 0;
                rc1 = prvStatCpssFmtValueFromRawGet(
                    fieldPtr->stcBitOffset, fieldPtr->rawBitOffset,
                    fieldPtr->rawBitLength,
                    &currentContext, convertCmd, rawDataPtr, &data);
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                    continue;
                }

                rc1 = prvStatCpssFmtStcValuePut(
                    fieldType, fieldPtr->stcByteOffsetOrConst, data,
                    stcDataPtr, extMemoPtr);
                if (GT_OK != rc1)
                {
                    rc = GT_BAD_PARAM;
                }
                continue;

        }
    }

    return rc;
}


