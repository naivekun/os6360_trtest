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
* @file prvCpssMisc.c
*
* @brief Miscellaneous operations for CPSS.
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>

/* return GT_U32-mask with n lowest bits set to 0. Rest of mask's bits are 1. */
#define LO_ZEROES_MAC(n) ((n)>=32 ? (GT_U32)0  : (GT_U32)-1<<(n))

/* return GT_U32-mask with n lowest bits set to 1. Rest of mask's bits are 0. */
#define LO_ONES_MAC(n)   ((n)>=32 ? (GT_U32)-1 : (GT_U32)(1<<(n))-1)


/**
* @internal prvCpssPeriodicFieldValueSet function
* @endinternal
*
* @brief   Set a value of a field into entry that lay in array of words.
*         the entry length is in bits and can be any non-zero number
* @param[in,out] dataArray[]              - array of words to set the field value into.
* @param[in] entryIndex               - entry index inside dataArray[]
* @param[in] entryNumBits             - number of bits for each entry.(non-zero)
* @param[in] fieldStartBit            - start bit of field . this bit is counted from the start
*                                      of the entry(and not from entry[0]).
* @param[in] fieldNumBits             - number of bits to set (field length) (1..32)
* @param[in] fieldValue               - the value of the field that need to set.
* @param[in,out] dataArray[]              - array of words with updated data on the field.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter entryNumBits or fieldNumBits
*                                       or fieldNumBits > entryNumBits
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPeriodicFieldValueSet(
    INOUT GT_U32    dataArray[],
    IN GT_U32       entryIndex,
    IN GT_U32       entryNumBits,
    IN GT_U32       fieldStartBit,
    IN GT_U32       fieldNumBits,
    IN GT_U32       fieldValue
)
{
    /* the field bits that can be spread on 2 registers max */
    GT_U32  firstWordIndex;/* temporary index of the word to update in dataArray[] */
    GT_U32  offset;/* start bit index , in the first updated word */
    GT_U32  indexOfStartingBit; /* index of bit to start with , indexed from the
                                   start of the array dataArray[]*/
    GT_U32  freeBits;/*number of bits in the first updated word from the offset
                       to end of word --> meaning (32-offset)
                       this value allow as to understand if we update single
                       word or 2  words.
                       */

    CPSS_NULL_PTR_CHECK_MAC(dataArray);

    if(fieldNumBits > 32  || fieldNumBits == 0 ||
       entryNumBits == 0 ||
       ((fieldNumBits + fieldStartBit) > entryNumBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    indexOfStartingBit = entryNumBits * entryIndex + fieldStartBit;

    firstWordIndex =  indexOfStartingBit >> 5 /* /32 */;

    offset = indexOfStartingBit & 0x1f /* % 32 */;
    freeBits = 32 - offset;
    if(freeBits >= fieldNumBits)
    {
        /* set the value in the field -- all in single word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex],offset,fieldNumBits,fieldValue);
    }
    else
    {
        /* set the start of value in the first word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex],offset,freeBits,fieldValue);
        /* set the rest of the value in the second word */
        U32_SET_FIELD_MASKED_MAC(dataArray[firstWordIndex + 1],0,fieldNumBits - freeBits,(fieldValue>>freeBits));
    }

    return GT_OK;
}

/**
* @internal prvCpssPeriodicFieldValueGet function
* @endinternal
*
* @brief   Get a value of a field from entry that lay in array of words.
*         the entry length is in bits and can be any non-zero number
* @param[in] dataArray[]              - array of words to Get the field value from.
* @param[in] entryIndex               - entry index inside dataArray[]
* @param[in] entryNumBits             - number of bits for each entry.(non-zero)
* @param[in] fieldStartBit            - start bit of field . this bit is counted from the start
*                                      of the entry(and not from entry[0]).
* @param[in] fieldNumBits             - number of bits to get (field length) (1..32)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter entryNumBits or fieldNumBits
*                                       or fieldNumBits > entryNumBits
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPeriodicFieldValueGet(
    IN GT_U32       dataArray[],
    IN GT_U32       entryIndex,
    IN GT_U32       entryNumBits,
    IN GT_U32       fieldStartBit,
    IN GT_U32       fieldNumBits,
    OUT GT_U32      *fieldValuePtr
)
{
    /* the field bits that can be spread on 2 registers max */
    GT_U32  firstWordIndex;/* temporary index of the word to read from dataArray[] */
    GT_U32  offset;/* start bit index , in the first read word */
    GT_U32  indexOfStartingBit; /* index of bit to start with , indexed from the
                                   start of the array dataArray[]*/
    GT_U32  freeBits;/*number of bits in the first read word from the offset
                       to end of word --> meaning (32-offset)
                       this value allow as to understand if we read single
                       word or 2  words.
                       */

    CPSS_NULL_PTR_CHECK_MAC(dataArray);
    CPSS_NULL_PTR_CHECK_MAC(fieldValuePtr);

    if(fieldNumBits > 32  || fieldNumBits == 0 ||
       entryNumBits == 0 ||
       ((fieldNumBits + fieldStartBit)  > entryNumBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    indexOfStartingBit = entryNumBits * entryIndex + fieldStartBit;

    firstWordIndex =  indexOfStartingBit >> 5 /* /32 */;

    offset = indexOfStartingBit & 0x1f /* % 32 */;
    freeBits = 32 - offset;
    if(freeBits >= fieldNumBits)
    {
        /* get the value of the field -- all in single word */
        *fieldValuePtr = U32_GET_FIELD_MAC(dataArray[firstWordIndex],offset,fieldNumBits);
    }
    else
    {
        /* get the start of value from the first word */
        *fieldValuePtr = U32_GET_FIELD_MAC(dataArray[firstWordIndex],offset,freeBits);
        /* get the rest of the value from the second word */
        *fieldValuePtr |=
            ((U32_GET_FIELD_MAC(dataArray[firstWordIndex + 1],0,(fieldNumBits - freeBits))) << freeBits);
    }

    return GT_OK;
}


/**
* @internal prvCpssFieldValueGet function
* @endinternal
*
* @brief   get the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (1..32)
*
* @param[out] valuePtr                 - (pointer to) value get
*                                      Returns:
*                                      COMMENTS:
*                                      GT_OK - no error
*                                      GT_BAD_PTR - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
* @note GT_OK - no error
*       GT_BAD_PTR - on NULL pointer
*       GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
*/
GT_STATUS  prvCpssFieldValueGet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    OUT GT_U32                 *valuePtr
)
{
    GT_U32  value;    /* value of field */
    GT_U32  wordIndex = OFFSET_TO_WORD_MAC(startBit);/* word index in startMemPtr[]*/
    GT_U32  bitIndex  = OFFSET_TO_BIT_MAC(startBit); /* bit index in startMemPtr[wordIndex]*/
    GT_U32  len0;       /* length of field in first startMemPtr[wordIndex] */
    GT_U32  tmpValue;   /* temo value to help calculation */

    CPSS_NULL_PTR_CHECK_MAC(startMemPtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(numBits > 32 || numBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(wordIndex == OFFSET_TO_WORD_MAC(startBit + numBits - 1))
    {
        /* read in the same word */
        if(numBits == 32)
        {
            value = startMemPtr[wordIndex];
        }
        else
        {
            value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,(numBits));
        }
    }
    else
    {
        len0 = (32 - bitIndex);
        /* read from 2 words */
        /* in first word , start from bitIndex , and read to end of word */
        value = U32_GET_FIELD_MAC(startMemPtr[wordIndex],bitIndex,len0);

        /* in second word , start from bit 0 , and read the rest of data */
        tmpValue = U32_GET_FIELD_MAC(startMemPtr[wordIndex + 1],0,(numBits - len0));
        value |= tmpValue << len0;
    }

    *valuePtr =  value;

    return GT_OK;
}

/**
* @internal prvCpssFieldValueSet function
* @endinternal
*
* @brief   set the value of field (up to 32 bits) that located in any start bit in
*         memory
* @param[in] startMemPtr              - pointer to memory
* @param[in] startBit                 - start bit of field (0..)
* @param[in] numBits                  - number of bits of field (1..32)
* @param[in] value                    - the  to set
*                                      Returns:
*                                      COMMENTS:
*                                      GT_OK - no error
*                                      GT_BAD_PTR - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*
* @note GT_OK - no error
*       GT_BAD_PTR - on NULL pointer
*       GT_BAD_PARAM - on bad parameter : numBits > 32 or numBits == 0
*       COMMENTS:
*       None.
*
*/
GT_STATUS  prvCpssFieldValueSet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  value
)
{
    GT_U32  wordIndex = OFFSET_TO_WORD_MAC(startBit);/* word index in startMemPtr[]*/
    GT_U32  bitIndex  = OFFSET_TO_BIT_MAC(startBit); /* bit index in startMemPtr[wordIndex]*/
    GT_U32  len0;       /* length of field in first startMemPtr[wordIndex] */

    CPSS_NULL_PTR_CHECK_MAC(startMemPtr);

    if(numBits > 32 || numBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(wordIndex == OFFSET_TO_WORD_MAC(startBit + numBits - 1))
    {
        /* write in the same word */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[wordIndex],bitIndex,(numBits),(value));
    }
    else
    {
        len0 = (32 - bitIndex);
        /* write in 2 words */
        /* in first word , start from bitIndex , and write to end of word */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[wordIndex],bitIndex,len0,value);

        /* in second word , start from bit 0 , and write the rest of data */
        U32_SET_FIELD_MASKED_MAC(startMemPtr[wordIndex + 1],0,(numBits - len0),(value >> len0));
    }

    return GT_OK;
}

/**
* @internal prvCpssFieldFromEntry_GT_U32_Get function
* @endinternal
*
* @brief   Get GT_U32 value of a field from the table entry.
*
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] valuePtr                 - (pointer to) value get
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      GT_BAD_PARAM - on bad parameter : fieldInfo.numBits > 32 or fieldInfo.numBits == 0
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldFromEntry_GT_U32_Get(
    IN GT_U32                           *entryPtr,
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          *valuePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssFieldValueGet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits,valuePtr);
}

/**
* @internal prvCpssFieldToEntry_GT_U32_Set function
* @endinternal
*
* @brief   Set GT_U32 value to a field in a table entry.
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
* @param[in] value                    - the  to set
*/
GT_STATUS prvCpssFieldToEntry_GT_U32_Set(
    IN GT_U32                           *entryPtr,
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           value
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if(fieldsInfoArr[fieldIndex].numOfBits < 32 &&
        (value > BIT_MASK_MAC(fieldsInfoArr[fieldIndex].numOfBits)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssFieldValueSet(entryPtr,fieldsInfoArr[fieldIndex].startBit,fieldsInfoArr[fieldIndex].numOfBits,value);
}

/**
* @internal prvCpssFieldToEntry_GT_Any_Set function
* @endinternal
*
* @brief   Set 'any number of bits' value to a field in a table entry.
*         the bits are in little Endian order in the array of words.
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
* @param[in] valueArr[]               - the array of values to set
*/
GT_STATUS prvCpssFieldToEntry_GT_Any_Set(
    IN GT_U32                           *entryPtr,
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    IN GT_U32                           valueArr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numOfWords;/* number of words in the field */
    GT_U32  startBit;/* start bit of current section of the field */
    GT_U32  numOfBits;/* number of bits of current section of the field */
    GT_U32  value;/* value of current section of the field */

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valueArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if(fieldsInfoArr[fieldIndex].numOfBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numOfWords = (fieldsInfoArr[fieldIndex].numOfBits + 31) / 32;
    startBit = fieldsInfoArr[fieldIndex].startBit;
    numOfBits = 32;

    /* handle the 'Full words' (32 bits) sections */
    for(ii = 0 ; ii < (numOfWords - 1); ii++ , startBit += 32)
    {
        value = valueArr[ii];
        rc = prvCpssFieldValueSet(entryPtr,startBit,numOfBits,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    value = valueArr[ii];

    /* handle the last word */
    numOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;
    if(numOfBits == 0)
    {
        /* last word is full 32 bits */
        numOfBits = 32;
    }

    return prvCpssFieldValueSet(entryPtr,startBit,numOfBits,value);
}

/**
* @internal prvCpssFieldToEntry_GT_Any_Get function
* @endinternal
*
* @brief   Get 'any number of bits' value to a field in a table entry.
*         the bits returned are in little Endian order in the array of words.
* @param[in] entryPtr                 - pointer to memory.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] valueArr[]               - the array of values to get
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      GT_BAD_PARAM    - on bad parameter : info.length == 0
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldToEntry_GT_Any_Get(
    IN GT_U32                           *entryPtr,
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          valueArr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32  numOfWords;/* number of words in the field */
    GT_U32  startBit;/* start bit of current section of the field */
    GT_U32  numOfBits;/* number of bits of current section of the field */
    GT_U32  value;/* value of current section of the field */

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(valueArr);

    if(fieldsInfoArr[fieldIndex].startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    else if(fieldsInfoArr[fieldIndex].numOfBits == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    numOfWords = (fieldsInfoArr[fieldIndex].numOfBits + 31) / 32;
    startBit = fieldsInfoArr[fieldIndex].startBit;
    numOfBits = 32;

    /* handle the 'Full words' (32 bits) sections */
    for(ii = 0 ; ii < (numOfWords - 1); ii++ , startBit += 32)
    {
        rc = prvCpssFieldValueGet(entryPtr,startBit,numOfBits,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        valueArr[ii] = value;
    }

    /* handle the last word */
    numOfBits = fieldsInfoArr[fieldIndex].numOfBits & 0x1f;
    if(numOfBits == 0)
    {
        /* last word is full 32 bits */
        numOfBits = 32;
    }

    rc = prvCpssFieldValueGet(entryPtr,startBit,numOfBits,&value);

    valueArr[ii] = value;

    return rc;
}



/**
* @internal prvCpssFillFieldsStartBitInfo function
* @endinternal
*
* @brief   Fill during init the 'start bit' of the fields in the table format.
*
* @param[in] numOfFields              - the number of elements in in fieldsInfoArr[].
* @param[in,out] fieldsInfoArr[]          - array of fields info
* @param[in,out] fieldsInfoArr[]          - array of fields info , after modify the <startBit> of the fields.
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      COMMENTS:
*/
GT_STATUS prvCpssFillFieldsStartBitInfo(
    IN GT_U32                           numOfFields,
    INOUT PRV_CPSS_ENTRY_FORMAT_TABLE_STC   fieldsInfoArr[]
)
{
    GT_U32                      ii;
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *currentFieldInfoPtr;
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *prevFieldInfoPtr;
    GT_U32                      prevIndex;

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);

    for( ii = 0 ; ii < numOfFields ; ii++)
    {
        currentFieldInfoPtr = &fieldsInfoArr[ii];

        prevIndex = currentFieldInfoPtr->previousFieldType;

        if(currentFieldInfoPtr->startBit == PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS)
        {
            if(ii == 0)
            {
                /* first field got no options other then to start in bit 0 */
                currentFieldInfoPtr->startBit = 0;
            }
            else /* use the previous field info */
            {
                if(prevIndex == PRV_CPSS_FIELD_CONSECUTIVE_CNS)
                {
                    /* this field is consecutive to the previous field */
                    prevIndex = ii-1;
                }
                else
                {
                    /* this field come after other previous field */
                }

                prevFieldInfoPtr = &fieldsInfoArr[prevIndex];
                currentFieldInfoPtr->startBit = prevFieldInfoPtr->startBit + prevFieldInfoPtr->numOfBits;
            }
        }
        else
        {
            /* no need to calculate the start bit -- it is FORCED by the entry format */
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssFieldInEntryInfoGet function
* @endinternal
*
* @brief   Get the start bit and the length of specific field in entry format .
*
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldIndex               - the index of the field (used as index in fieldsInfoArr[])
*
* @param[out] startBitPtr              - (pointer to) the start bit of the field.
* @param[out] numOfBitsPtr             - (pointer to) the number of bits of the field.
*                                      RETURN:
*                                      GT_OK   -   on success
*                                      GT_BAD_PTR  - on NULL pointer
*                                      COMMENTS:
*/
GT_STATUS prvCpssFieldInEntryInfoGet(
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldIndex,
    OUT GT_U32                          *startBitPtr,
    OUT GT_U32                          *numOfBitsPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(startBitPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfBitsPtr);

    *startBitPtr = fieldsInfoArr[fieldIndex].startBit;
    *numOfBitsPtr = fieldsInfoArr[fieldIndex].numOfBits;

    return GT_OK;
}

/* generic GT_U32 bitmap functions */

/**
* @internal prvCpssBitmapNextOneBitIndexFind function
* @endinternal
*
* @brief   Find index of next bit in bitmap that contains one.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
* @param[in] startBit                 - index of start bit, to find first bit specify -1
*/
GT_32 prvCpssBitmapNextOneBitIndexFind
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32   bitmapWordsNum,
    IN   GT_32    startBit
)
{
    GT_U32  searchFrom;
    GT_U32  wordIdx;
    GT_U32  bitIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* table of 16 2-bit values of first one-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  -, 0, 1, 0, 2, 0, 1, 0, 3, 0,  1,  0,  2,  0,  1,  0 */
    static GT_U32 firstBitTable = 0x12131210;

    /* look for word containing the bit being searched */
    searchFrom = (startBit < 0) ? 0 : (GT_U32)(startBit + 1) ;
    wordIdx = (searchFrom / 32);
    if (wordIdx >= bitmapWordsNum) return -1;

    word = bitmapArrPtr[wordIdx];
    bitIdx  = (searchFrom % 32);
    if (bitIdx != 0)
    {
        /* relevant for first checked word only */
        word &= (~ ((1 << bitIdx) - 1));
    }
    if (word == 0)
    {
        /* continue search not zero word */
        for (wordIdx++; (wordIdx < bitmapWordsNum); wordIdx++)
        {
            word = bitmapArrPtr[wordIdx];
            if (word != 0) break;
        }
    }

    /* not zero word not found */
    if (word == 0) return -1;

    /* look for first one bit in already found word */
    shiftBits = 0;
    if ((word & 0xFFFF) == 0)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if ((word & 0xFF) == 0)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if ((word & 0x0F) == 0)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((firstBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapNextZeroBitIndexFind function
* @endinternal
*
* @brief   Find index of next bit in bitmap that contains zero.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
* @param[in] startBit                 - index of start bit, to find first bit specify -1
*/
GT_32 prvCpssBitmapNextZeroBitIndexFind
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32   bitmapWordsNum,
    IN   GT_32    startBit
)
{
    GT_U32  searchFrom;
    GT_U32  wordIdx;
    GT_U32  bitIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* table of 16 2-bit values of first zero-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  0, 1, 0, 2, 0, 1, 0, 3, 0, 1,  0,  2,  0,  1,  0,  - */
    static GT_U32 firstBitTable = 0x0484C484;

    /* look for word containing the bit being searched */
    searchFrom = (startBit < 0) ? 0 : (GT_U32)(startBit + 1) ;
    wordIdx = (searchFrom / 32);
    if (wordIdx >= bitmapWordsNum) return -1;

    word = bitmapArrPtr[wordIdx];
    bitIdx  = (searchFrom % 32);
    if (bitIdx != 0)
    {
        /* relevant for first checked word only */
        word |= ((1 << bitIdx) - 1);
    }
    if (word == 0xFFFFFFFF)
    {
        /* continue search not zero word */
        for (wordIdx++; (wordIdx < bitmapWordsNum); wordIdx++)
        {
            word = bitmapArrPtr[wordIdx];
            if (word != 0xFFFFFFFF) break;
        }
    }

    /* not full-ones word not found */
    if (word == 0xFFFFFFFF) return -1;

    /* look for first zero bit in already found word */
    shiftBits = 0;
    if ((word & 0xFFFF) == 0xFFFF)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if ((word & 0xFF) == 0xFF)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if ((word & 0x0F) == 0x0F)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((firstBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapOneBitsCount function
* @endinternal
*
* @brief   Count amount of bits that contains one in bitmap.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] bitmapWordsNum           - amount of words in bitmap
*/
GT_32 prvCpssBitmapOneBitsCount
(
    IN   GT_U32   *bitmapArrPtr,
    IN   GT_U32    bitmapWordsNum
)
{
    GT_U32  wordIdx;
    GT_U32 word;
    GT_U32  numOfBits;
    GT_U32  shiftBits;
    /* table of 16 2-bit values of (numOfBits - 1) of one-bits 0..3 */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15   */
    /* data  0, 0, 0, 1, 0, 1, 1, 2, 0, 1,  1,  2,  1,  2,  2,  3   */
    static GT_32 numOfBitsTable = 0xE9949440;

    numOfBits = 0;
    for (wordIdx = 0; (wordIdx < bitmapWordsNum); wordIdx++)
    {
        word = bitmapArrPtr[wordIdx];
        if (word == 0) continue;
        for (shiftBits = 0; (shiftBits < 32); /*none*/)
        {
            if ((word & 0xFFFF) == 0)
            {
                shiftBits += 16;
                word >>= 16;
                continue;
            }
            if ((word & 0xFF) == 0)
            {
                shiftBits += 8;
                word >>= 8;
                continue;
            }
            if ((word & 0x0F) == 0)
            {
                shiftBits += 4;
                word >>= 4;
                continue;
            }
            /* found non zero nibble - get 2-bit value from table */
            numOfBits += (((numOfBitsTable >> ((word & 0x0F) * 2)) & 3) + 1);
            shiftBits += 4;
            word >>= 4;
        }
    }
    return numOfBits;
}

/**
* @internal prvCpssBitmapLastOneBitIndexInRangeFind function
* @endinternal
*
* @brief   Find index of last bit in range of bits in bitmap that contains one.
*
* @param[in] bitmapArrPtr            - array of 32-bit words of bitmap
* @param[in] startBit                - index of start bit of search range
* @param[in] endBit                  - index of end bit of search range
* @param[in] notFoundReturnValue     - value to return if all bits in range are zeros
*/
GT_U32 prvCpssBitmapLastOneBitIndexInRangeFind
(
    IN   GT_U32    *bitmapArrPtr,
    IN   GT_U32    startBit,
    IN   GT_U32    endBit,
    IN   GT_U32    notFoundReturnValue
)
{
    GT_32   firstWordIdx;
    GT_32   lastWordIdx;
    GT_32   firstWordMask;
    GT_32   lastWordMask;
    GT_32   wordIdx;
    GT_U32  word;
    GT_U32  shiftBits;
    GT_U32  shiftIn4Bits;
    /* 4-bit field last one bit 0..3                             */
    /* table of 16 2-bit values of last one-bit 0..3             */
    /* index 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* data  -, 0, 1, 1, 2, 2, 2, 2, 3, 3,  3,  3,  3,  3,  3,  3 */
    static GT_U32 lastBitTable = 0xFFFFAA50;

    if (startBit > endBit) return notFoundReturnValue;

    word          = 0;
    firstWordIdx  = (GT_32)(startBit / 32);
    lastWordIdx   = (GT_32)(endBit / 32);
    firstWordMask = (GT_32)(0xFFFFFFFF << (startBit % 32));
    lastWordMask  = (GT_32)(((GT_U32)0xFFFFFFFF) >> (31 - (endBit % 32)));

    for (wordIdx = lastWordIdx; (wordIdx >= firstWordIdx); wordIdx--)
    {
        word = bitmapArrPtr[wordIdx];
        if (wordIdx == firstWordIdx) word &= firstWordMask;
        if (wordIdx == lastWordIdx) word &= lastWordMask;
        if (word != 0) break;
    }
    /* not found not-zero word */
    if (wordIdx < firstWordIdx) return notFoundReturnValue;

    /* search the highes one-bit in the word */
    shiftBits = 0;
    if (word & 0xFFFF0000)
    {
        shiftBits += 16;
        word >>= 16;
    }
    if (word & 0xFF00)
    {
        shiftBits += 8;
        word >>= 8;
    }
    if (word & 0xF0)
    {
        shiftBits += 4;
        word >>= 4;
    }
    /* found non zero nibble - get 2-bit value from table */
    shiftIn4Bits = ((lastBitTable >> ((word & 0x0F) * 2)) & 3);
    return (GT_32)((32 * wordIdx) + shiftBits + shiftIn4Bits);
}

/**
* @internal prvCpssBitmapRangeSet function
* @endinternal
*
* @brief   Set range of bits in bitmap to 0 or 1.
*
* @param[in] bitmapArrPtr             - array of 32-bit words of bitmap
* @param[in] startBit                 - index of start bit
* @param[in] numOfBits                - number of bits to set
* @param[in] value                    -  to set, any not zero treated as one
*/
GT_VOID prvCpssBitmapRangeSet
(
    IN   GT_U32    *bitmapArrPtr,
    IN   GT_U32    startBit,
    IN   GT_U32    numOfBits,
    IN   GT_U32    value
)
{
    GT_U32 wordIdx;
    GT_U32 low, high, mask, highZeros;

    while (numOfBits)
    {
        wordIdx = startBit / 32;
        low = startBit % 32;
        high = (low + numOfBits);
        if (high > 32) high = 32;

        /* update middle */
        if ((low == 0) && (high == 32))
        {
            bitmapArrPtr[wordIdx] = ((value == 0) ? 0 : 0xFFFFFFFF);
            startBit  += 32;
            numOfBits -= 32;
            continue;
        }

        /* update first or last word */
        highZeros = 32 - high;
        mask = ((GT_U32)(0xFFFFFFFF << (low + highZeros)) >> highZeros);
        if (value == 0)
        {
            bitmapArrPtr[wordIdx] &= (~ mask);
        }
        else
        {
            bitmapArrPtr[wordIdx] |= mask;
        }
        startBit  += (high - low);
        numOfBits -= (high - low);
    }
}

/**
* @internal prvCpssFieldToEntryAndMask_GT_U32_Set function
* @endinternal
*
* @brief   Set GT_U32 value to a field in a table entry and raise bits
*         appropriate to field in the mask
*         NOTE:the value MUST not be bigger then the max value for the field !
* @param[in] entryPtr                 - pointer to entry.
* @param[in] maskPtr                  - pointer to mask.
* @param[in] fieldsInfoArr[]          - array of fields info
* @param[in] fieldId                  - the index of the field (used as index in fieldsInfoArr[])
* @param[in] value                    - the  to set
*/
GT_STATUS prvCpssFieldToEntryAndMask_GT_U32_Set
(
    IN GT_U32                           *entryPtr,
    IN GT_U32                           *maskPtr,
    IN PRV_CPSS_ENTRY_FORMAT_TABLE_STC  fieldsInfoArr[],
    IN GT_U32                           fieldId,
    IN GT_U32                           value

)
{
    GT_STATUS rc;
    GT_U32    fieldStartBit;
    GT_U32    fieldLength;

    rc = prvCpssFieldToEntry_GT_U32_Set(entryPtr, fieldsInfoArr, fieldId, value);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* call prvCpssFieldInEntryInfoGet after prvCpssFieldToEntry_GT_U32_Set
       to be sure fieldStartBit, fieldLength are valid */
    rc = prvCpssFieldInEntryInfoGet(fieldsInfoArr,fieldId, &fieldStartBit, &fieldLength);
    if (rc != GT_OK)
    {
        return rc;
    }
    raiseBitsInMemory(maskPtr, fieldStartBit, fieldLength);
    return GT_OK;
}

/**
* @internal raiseBitsInMemory function
* @endinternal
*
* @brief   set specified bits in memory to 1.
*         Bits out of range specified by pair (startBit, numBits) stays the same.
* @param[in,out] memPtr                   - (pointer to) memory to update. if NULL do nothing
* @param[in] startBit                 - start bit in the memory to set.
* @param[in] numBits                  - number of bits (unlimited num of bits)
* @param[in,out] memPtr                   - (pointer to) updated memory
*                                      COMMENTS:
*/
void  raiseBitsInMemory(
    INOUT GT_U32                  *memPtr,
    IN    GT_U32                  startBit,
    IN    GT_U32                  numBits
)
{
    GT_U32 lastBit = (startBit + numBits - 1);
    GT_U32 i;
    GT_U32 firstIx;
    GT_U32 lastIx;

    if (memPtr == NULL)
    {
        return;
    }

    if (numBits)
    {
        firstIx   = startBit >> 5;  /* >>5 == /32 */
        lastIx    = lastBit  >> 5;  /* >>5 == /32 */
        startBit &= 0x1f;           /* &0x1f ==  %32 */
        lastBit  &= 0x1f;           /* &0x1f ==  %32 */

        /* set to 0xffffffff all range's words except first and last ones */
        for (i = firstIx + 1; i< lastIx; i++)
        {
            memPtr[i] = (GT_U32)(-1);
        }

        /* update first, last  words */
        if (firstIx != lastIx)
        {
            /* for first word: clear mask bits lower than startBit */
            memPtr[firstIx] |= LO_ZEROES_MAC(startBit);

            /* for last word: clear mask bits higher than lastBit */
            memPtr[lastIx] |= LO_ONES_MAC(lastBit+1);
        }
        else
        {
            /* first word is the last word. Combine (&) masks for first, last words  */
            memPtr[firstIx] |= LO_ZEROES_MAC(startBit) & LO_ONES_MAC(lastBit+1);
        }
    }
}

/**
* @internal printBitmapArr function
* @endinternal
*
* @brief   print a bitmap array .
*
* @note   APPLICABLE DEVICES:      ALL.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] bmpName                  - the 'name' of the BMP. (relevant only in case of error in expected_numOfSetBits)
* @param[in] bitmapArr                - pointer to bitmap array
* @param[in] numOfBits                - number of bits in bitmapArr (set or unset)
* @param[in] expected_numOfSetBits    - expected amount of set bits
*                                      value 0xFFFFFFFF meaning do not compare
*                                       number of bits that are set (not zero) in the bmp
*/
GT_U32 printBitmapArr
(
    IN GT_CHAR* bmpName,
    IN GT_U32   *bitmapArr,
    IN GT_U32   numOfBits,
    IN GT_U32   expected_numOfSetBits
)
{
    GT_U32 i,beginOfRange; /* indexes, helper variable for printing range */
    GT_BOOL inRange; /* helper variable for printing range */
    GT_U32  actualUsedAmount = 0;

    inRange = GT_FALSE;
    beginOfRange = 0;
    /* iterating over the bitmap */
    for (i = 0; i < numOfBits; i++)
    {
        if (0 == (bitmapArr[i>>5] & (1<<(i & 0x1f))))
        {
            /* end of range */
            if (inRange == GT_TRUE)
            {
                /* printing the range that just ended*/
                if ( i != 0 && ((i-1) != beginOfRange))
                {
                    cpssOsPrintf("..%d", i - 1);
                }
                if (i != (numOfBits-1))
                {
                    cpssOsPrintf(", ");
                }
                inRange = GT_FALSE;
            }
        }
        /* usedRulesBitmapArr[i] is on */
        else
        {
            actualUsedAmount++;
            /* beginning of range */
            if (inRange == GT_FALSE)
            {
                cpssOsPrintf("%d",i);
                inRange = GT_TRUE;
                beginOfRange = i;
            }
        }
    }
    /* dealing with the last range */
    if (inRange == GT_TRUE)
    {
        /* printing the range that just ended*/
        if ((i != 0) && ((i-1) != beginOfRange))
        {
            cpssOsPrintf("..%d", i - 1);
        }
    }

    if (actualUsedAmount == 0)
    {
        cpssOsPrintf("(empty)");
    }
    else
    {
        cpssOsPrintf("([%d] bits are set)",actualUsedAmount);
    }
    cpssOsPrintf("\n");

    if(expected_numOfSetBits != 0xFFFFFFFF &&
       actualUsedAmount != expected_numOfSetBits)
    {
        cpssOsPrintf("DB ERROR detected : [%s] : actualUsedAmount[%d] expected[%d] \n",
            bmpName,actualUsedAmount,expected_numOfSetBits);
    }

    return actualUsedAmount;
}

