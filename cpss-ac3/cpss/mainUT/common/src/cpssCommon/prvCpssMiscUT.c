/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssBuffManagerPoolUT.c
*
* DESCRIPTION:
*       Unit tests for Miscellaneous operations for CPSS.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

/* includes */
#include <cpssCommon/private/prvCpssMisc.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#define MEM_ARRAY_SIZE_CNS  20
#define VALUES_ARRAY_SIZE_CNS    6

/*----------------------------------------------------------------------------*/
/*
GT_STATUS  prvCpssFieldValueSet
(
    IN GT_U32                  *startMemPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits,
    IN GT_U32                  value
);
*/
UTF_TEST_CASE_MAC(prvCpssFieldValueSet)
{
    GT_STATUS   st;/*return value*/
    static GT_U32   memArray[MEM_ARRAY_SIZE_CNS];/* array of words */
    GT_U32  ii; /* iterator */
    GT_U32  indexMax;/* max index in array memArray[] */
    GT_U32  startIndex;/* start index of filed */
    GT_U32  numOfBits; /*number of bits in the filed */
    GT_U32  valuesArray[VALUES_ARRAY_SIZE_CNS] = {/* value to write to field */
            0xFFFFFFFF , 0xA5A5A5A5 , 0x5A5A5A5A , 0x12345678 , 0x87654321 , 0 };
    GT_U32  value;/* actual value*/
    GT_U32  expectedValue;/* expected value*/

    indexMax = 32 * (MEM_ARRAY_SIZE_CNS - 1);

    for(startIndex = 0; startIndex < indexMax ; startIndex++)
    {
        for(numOfBits = 1 ; numOfBits <= 32 ; numOfBits++)
        {
            for(ii = 0; ii < VALUES_ARRAY_SIZE_CNS ; ii ++)
            {
                st = prvCpssFieldValueSet(memArray,startIndex,numOfBits,valuesArray[ii]);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, startIndex, numOfBits, valuesArray[ii]);

                st = prvCpssFieldValueGet(memArray,startIndex,numOfBits,&value);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, startIndex, numOfBits);

                expectedValue =  numOfBits < 32 ?
                                 U32_GET_FIELD_MAC(valuesArray[ii],0,numOfBits) :
                                 valuesArray[ii];
                /* compare the 'set' with the 'get' */
                UTF_VERIFY_EQUAL2_PARAM_MAC(expectedValue, value, startIndex, numOfBits);
            }
        }
    }

    startIndex = 54;
    /*****************************************/
    /* check for 'errors' -- on the set API  */
    /*****************************************/
    numOfBits = 0;
    value = valuesArray[0];
    st = prvCpssFieldValueSet(memArray,startIndex,numOfBits,value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

    numOfBits = 33;
    value = valuesArray[0];
    st = prvCpssFieldValueSet(memArray,startIndex,numOfBits,value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

    numOfBits = 0;
    st = prvCpssFieldValueSet(NULL,startIndex,numOfBits,value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);


    /*****************************************/
    /* check for 'errors' -- on the set API  */
    /*****************************************/

    numOfBits = 0;
    st = prvCpssFieldValueGet(memArray,startIndex,numOfBits,&value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

    numOfBits = 33;
    st = prvCpssFieldValueGet(memArray,startIndex,numOfBits,&value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

    numOfBits = 0;
    st = prvCpssFieldValueGet(memArray,startIndex,numOfBits,NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    numOfBits = 0;
    st = prvCpssFieldValueGet(NULL,startIndex,numOfBits,&value);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

}


/*----------------------------------------------------------------------------*/
/*
GT_U64 prvCpssMathMul64
(
    IN GT_U32 x,
    IN GT_U32 y
);
*/
UTF_TEST_CASE_MAC(prvCpssMathMul64)
{
    GT_U32   aArray[]    = {1,        0xFFF0,    0x56FDEC,    0x7AD0234, 0xFFFFFFFF, 0xFFFFFFFF, 0x75300};
    GT_U32   bArray[]    = {0x17,     0xABCE,       0xDEC,    0x101DFEC, 0xFF0FFFF1, 0xFFFFFFFF, 0x2300};
    GT_U32   resLsbArray[]  = {0x17, 0xABC34320, 0xBB171190, 0xFE9D53F0, 0x00F0000F, 0x00000001, 0x00590000};
    GT_U32   resMsbArray[]  = {0x00, 0x00000000, 0x00000004, 0x0007BB65, 0xFF0FFFF0, 0xFFFFFFFE, 0x00000001};
    GT_U32  ii; /* iterator */
    GT_U32  indexMax;
    GT_U64  value;/* actual value*/
    GT_U64  expectedValue;/* expected value*/

    indexMax = sizeof(aArray) / sizeof(aArray[0]);

    for(ii = 0; ii < indexMax ; ii++)
    {
        expectedValue.l[0] = resLsbArray[ii];
        expectedValue.l[1] = resMsbArray[ii];

        value = prvCpssMathMul64(aArray[ii], bArray[ii]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedValue.l[0], value.l[0], 0, ii);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedValue.l[1], value.l[1], 1, ii);
    }


    for(ii = 0; ii < indexMax ; ii++)
    {
        expectedValue.l[0] = resLsbArray[ii];
        expectedValue.l[1] = resMsbArray[ii];

        value = prvCpssMathMul64(bArray[ii], aArray[ii]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedValue.l[0], value.l[0], 0, ii);
        UTF_VERIFY_EQUAL2_PARAM_MAC(expectedValue.l[1], value.l[1], 1, ii);
    }
}

/*
GT_U64 prvCpssMathDiv64By32
(
    IN  GT_U64  x,
    IN  GT_U32  y,
    OUT GT_U32  *modPtr
);
*/
/* devide devident by div and get quotient and reminder */
/* calculate x64 = ((quotient * div) + reminder)        */
/* compare devident and x64                             */
static void check_case_prvCpssMathDiv64By32(
    GT_U64 devident, GT_U32 div)
{
    GT_U64  x64, w64;
    GT_U32  reminder;
    GT_U64  quotient;

    /* devide devident by div and get quotient and reminder */
    quotient = prvCpssMathDiv64By32(devident, div, &reminder);

    PRV_UTF_LOG6_MAC(
        "devident 0x%08X:%08X quotient 0x%08X:%08X div 0x%08X reminder 0x%08X\n",
        devident.l[1], devident.l[0], quotient.l[1], quotient.l[0], div, reminder);

    /* calculate x64 = ((quotient * div) + reminder)        */
    x64 = prvCpssMathMul64(quotient.l[0], div);
    x64.l[1] += quotient.l[1] * div;
    w64.l[0] = reminder;
    w64.l[1] = 0;
    x64 = prvCpssMathAdd64(x64, w64);

    /* compare devident and x64                             */
    UTF_VERIFY_EQUAL0_STRING_MAC(
        devident.l[0], x64.l[0], "back calculated low is different\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        devident.l[1], x64.l[1], "back calculated high is different\n");
}

UTF_TEST_CASE_MAC(prvCpssMathDiv64By32)
{
    static GT_U32 baseArr[][2] =
    {{0, 1}, {0x1000, 0}, {0xFFFF, 0xFFFF}, {0, 0xFFFFFFF},
     {0xFFFFFFFF, 0}, {0xFFFFFFFF, 0xFFFFFFFF}};
    static GT_U32 baseArrSize = sizeof(baseArr) / sizeof(baseArr[0]);
    static GT_U32 divArr[] = {1, 2, 0x123, 0xFFFFFF, 0xFFFFFFFF};
    static GT_U32 divArrSize = sizeof(divArr) / sizeof(divArr[0]);
    GT_U32 baseIdx, divIdx;
    GT_U64 x64;

    for (baseIdx = 0; (baseIdx < baseArrSize); baseIdx++)
    {
        /* coded in <high, low> order */
        x64.l[0] = baseArr[baseIdx][1];
        x64.l[1] = baseArr[baseIdx][0];
        for (divIdx = 0; (divIdx < divArrSize); divIdx++)
        {
            check_case_prvCpssMathDiv64By32(x64, divArr[divIdx]);
        }
    }
}

UTF_TEST_CASE_MAC(prvCpssBitmapLastOneBitIndexInRangeFind)
{
    static GT_U32 bmp0Arr[] = {0, 0x80000000, 0};
    static GT_U32 bmp1Arr[] = {0x000F0FFF};
    static GT_U32 bmp2Arr[] = {1, 0xFFFFFFF, 0x8F008000};
    struct
    {
        GT_U32 *bpmPtr;
        GT_U32 startBit;
        GT_U32 endBit;
        GT_U32 expectedResult;
    } testArr[] =
    {
        {bmp0Arr,  0,  95, 63},
        {bmp0Arr,  0,  63, 63},
        {bmp0Arr, 63,  95, 63},
        {bmp0Arr,  0,  62, 0xFFFFFFFF},
        {bmp1Arr, 12,  31, 19},
        {bmp1Arr,  0,  12, 11},
        {bmp1Arr,  0,  11, 11},
        {bmp1Arr,  0,   0,  0},
        {bmp2Arr,  0,  79, 79},
        {bmp2Arr,  0,  94, 91},
        {bmp2Arr,  0,   0,  0}
    };
    GT_U32 testArrSize = (sizeof(testArr) / sizeof(testArr[0]));
    GT_U32 i;
    GT_U32 result;

    for (i = 0; (i < testArrSize); i++)
    {
       result = prvCpssBitmapLastOneBitIndexInRangeFind(
           testArr[i].bpmPtr,
           testArr[i].startBit,
           testArr[i].endBit,
           0xFFFFFFFF);
       UTF_VERIFY_EQUAL0_STRING_MAC(
           testArr[i].expectedResult, result, "prvCpssBitmapLastOneBitIndexInRangeFind");
    }
}

UTF_TEST_CASE_MAC(prvCpssMathDiv64)
{
    GT_U64 x;
    GT_U64 y;
    GT_U64 rem;
    GT_U64 res;
    GT_U64 expRem;
    GT_U64 expRes;
    GT_STATUS st;

    /* Divide by zero */
    x.l[1] = 0;
    x.l[0] = 5;
    y.l[1] = 0;
    y.l[0] = 0;
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "Failed to check zero divisor\n");

    /* Divide by one */
    y.l[0] = 1;
    expRem.l[0] = 0;
    expRem.l[1] = 0;
    expRes.l[0] = 5;
    expRes.l[1] = 0;
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "Unexpected retval\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[0], rem.l[0], "Reminder expected to be zero\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[1], rem.l[1], "Reminder expected to be zero\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[0], res.l[0], "Unexpected result\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[1], res.l[1], "Unexpected result\n");

    /* Divide zero by one */
    x.l[0] = 0;
    expRes.l[0] = 0;
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "Unexpected retval\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[0], rem.l[0], "Reminder expected to be zero\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[1], rem.l[1], "Reminder expected to be zero\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[0], res.l[0], "Unexpected result\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[1], res.l[1], "Unexpected result\n");

    /* Divisor bigger than divident */
    y.l[0] = 1713;
    x.l[0] = 5;
    expRem.l[0] = 5;
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "Unexpected retval\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[0], rem.l[0], "Unexpected reminder\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[1], rem.l[1], "Unexpected reminder\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[0], res.l[0], "Unexpected result\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[1], res.l[1], "Unexpected result\n");

    /* Divide huge numbers */
    x.l[1] = 0x00FA3ABC;
    x.l[0] = 0xFEEDBABE;
    y.l[1] = 0x99;
    y.l[0] = 0xFFFFAAAA;
    expRem.l[1] = 0x27;
    expRem.l[0] = 0x89967AB8;
    expRes.l[1] = 0;
    expRes.l[0] = 0x19FF7;
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "Unexpected retval\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[0], rem.l[0], "Unexpected reminder\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRem.l[1], rem.l[1], "Unexpected reminder\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[0], res.l[0], "Unexpected result\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[1], res.l[1], "Unexpected result\n");

    /* Bad pointer to result */
    st = prvCpssMathDiv64(x, y, NULL, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st, "Unexpected retval\n");

    /* NULL pointer to the reminder */
    st = prvCpssMathDiv64(x, y, &res, &rem);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "Unexpected retval\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[0], res.l[0], "Unexpected result\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(expRes.l[1], res.l[1], "Unexpected result\n");
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of prvCpssMisc suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(prvCpssMisc)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssFieldValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssMathMul64)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssMathDiv64By32)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssBitmapLastOneBitIndexInRangeFind)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssMathDiv64)
UTF_SUIT_END_TESTS_MAC(prvCpssMisc)
