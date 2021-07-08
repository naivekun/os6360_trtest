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
* @file prvCpssMisc.h
*
* @brief Miscellaneous operations for CPSS.
*
* @version   5
********************************************************************************
*/
#ifndef __prvCpssMisch
#define __prvCpssMisch

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
/* Add support for EMULATOR mode */
#include <cpssCommon/private/prvCpssEmulatorMode.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* indication to set PRV_CPSS_ENTRY_FORMAT_TABLE_STC::startBit during run time */
#define PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS   0xFFFFFFFF
/* indication in PRV_CPSS_ENTRY_FORMAT_TABLE_STC::previousFieldType that 'current'
field is consecutive to the previous field */
#define PRV_CPSS_FIELD_CONSECUTIVE_CNS      0xFFFFFFFF

/**
* @struct PRV_CPSS_ENTRY_FORMAT_TABLE_STC
 *
 * @brief A structure to hold info about field in entry of table
*/
typedef struct{

    /** start bit of the field. filled in runtime when value != PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS */
    GT_U32 startBit;

    /** number of bits in the field */
    GT_U32 numOfBits;

    /** @brief 'point' to the previous field for calculation of startBit.
     *  used when != PRV_CPSS_FIELD_CONSECUTIVE_CNS
     */
    GT_U32 previousFieldType;

} PRV_CPSS_ENTRY_FORMAT_TABLE_STC;

/* macro to fill instance of PRV_CPSS_ENTRY_FORMAT_TABLE_STC with value that good for 'standard' field.
'standard' field is a field that comes after the field that is defined before it in the array of fields

the macro gets the <numOfBits> of the current field.
*/
#define PRV_CPSS_STANDARD_FIELD_MAC(numOfBits)     \
        {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,        \
         numOfBits,                       \
         PRV_CPSS_FIELD_CONSECUTIVE_CNS}


/* macro to set <startBit> and <numOfBits> in instance of PRV_CPSS_ENTRY_FORMAT_TABLE_STC
the macro gets the <numOfBits> of the current field.
*/
#define PRV_CPSS_EXPLICIT_FIELD_MAC(startBit,numOfBits)     \
        {startBit,        \
         numOfBits,                       \
         0/*don't care*/}


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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);


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
);

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
);

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
);

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
);

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
);

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
);

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

);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssMisch */

