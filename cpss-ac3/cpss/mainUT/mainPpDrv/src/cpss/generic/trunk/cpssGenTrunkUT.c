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
* @file cpssGenTrunkUT.c
*
* @brief Unit tests for cpssGenTrunk, that provides
* CPSS generic Trunk for setting, resetting and editing.
*
* @version   2./
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* defines */

/* Default valid value for port id */
#define TRUNK_VALID_VIRT_PORT_CNS 0

/* Invalid enum */
#define TRUNK_INVALID_ENUM_CNS    0x5AAAAAA5

/* Maximum value for trun id */
#define TRUNK_MAX_TRUNK_ID_CNS    256

/* Tested trunk id */
#define TRUNK_TESTED_TRUNK_ID_CNS 2

/* This value is used as null index. */
#define TRUNK_NULL_TRUNK_ID_CNS   0


/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumberOfTrunks
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkInit)
{
/*
    ITERATE_DEVICES
    1.1. Call with maxNumberOfTrunks [0 / maxTrunkNum - 1].
    Expected: GT_OK.
    1.2. Call with out of range maxNumberOfTrunks [maxTrunkNum].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      maxNumberOfTrunks = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with maxNumberOfTrunks [0 / maxTrunkNum - 1].
            Expected: GT_OK.
        */

        /* call with maxNumberOfTrunks [0] */
        maxNumberOfTrunks = 0;

        st = prvCpssGenericTrunkInit(dev, maxNumberOfTrunks,PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, maxNumberOfTrunks);

        /* call with maxNumberOfTrunks [maxTrunkNum - 1] */
        maxNumberOfTrunks = PRV_CPSS_PP_MAC(dev)->trunkInfo.numTrunksSupportedHw;

        st = prvCpssGenericTrunkInit(dev, maxNumberOfTrunks,PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, maxNumberOfTrunks);
    }

    maxNumberOfTrunks = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkInit(dev, maxNumberOfTrunks,PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkInit(dev, maxNumberOfTrunks,PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkMembersSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with trunkId [2],
                   numOfEnabledMembers [5],
                   enabledMembersArray [0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]},
                   numOfDisabledMembers [7],
                   disabledMembersArray [0 / ... / 7] {port [5 / ... / 11], device [devNum]}.
    Expected: GT_OK.
    1.2. Call with trunkId [3],
                   numOfEnabledMembers [2],
                   enabledMembersArray [0 / 1] (already added) {port [7 / 4], device [devNum]},
                   numOfDisabledMembers [2],
                   disabledMembersArray [0 / 1] {port [6 / 2], device [devNum]}.
    Expected: NON GT_OK.
    1.3. Call with trunkId [2],
                   numOfEnabledMembers [1],
                   enabledMembersArray[0] {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]},
                   numOfDisabledMembers [0].
    Expected: NON GT_OK.
    1.4. Call with trunkId [2],
                   numOfEnabledMembers [1],
                   enabledMembersArray[0] {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]},
                   numOfDisabledMembers [0].
    Expected: NON GT_OK.
    1.5. Call cpssExMxPmTrunkDbEnabledMembersGet with trunkId [2],
                                                      non-NULL numOfEnabledMembersPtr,
                                                      and allocated enabledMembersArray for 12 members.
    Expected: GT_OK, numOfEnabledMembersPtr [5], and the same enabledMembersArray as written.
    1.6. Call cpssExMxPmTrunkDbDisabledMembersGet with trunkId [2],
                                                       non-NULL numOfDisabledMembersPtr,
                                                       and allocated disabledMembersArray for 12 members.
    Expected: GT_OK, numOfDisabledMembersPtr [7], and the same disabledMembersArray as written.
    1.7. Check out of range for sum [>CPSS_EXMXPM_TRUNK_MAX_NUM_OF_MEMBERS_CNS = 12] of enabled and disabled members.
         Call with trunkId [2],
                   numOfEnabledMembers [6],
                   enabledMembersArray [0 / 1 / 2 / 3 / 4 / 5] {port [0 / 1 / 2 / 3 / 4 / 5], device [devNum]},
                   numOfDisabledMembers [7],
                   disabledMembersArray [tested dev id, virtual port id 6, 7, 8, 9, 10, 11, 12].
    Expected: NON GT_OK.
    1.8. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   numOfEnabledMembers [5],
                   enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],
                   numOfDisabledMembers [7],
                   disabledMembersArray [tested dev id, virtual port id 5 ... 11].
    Expected: NON GT_OK.
    1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   numOfEnabledMembers [5],
                   enabledMembersArray [0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]},
                   numOfDisabledMembers [7],
                   disabledMembersArray [0 /  / 6] {port [5 / .. / 11], device [devNum]}.
    Expected: NON GT_OK.
    1.10. Call with trunkId [2],
                    numOfEnabledMembers [0],
                    enabledMembersArray [NULL],
                    numOfDisabledMembers [0],
                    disabledMembersArray [NULL] to remove all members from the trunk.
    Expected: GT_OK.
    1.11. Call with trunkId [2],
                    numOfEnabledMembers [1],
                    enabledMembersArray [NULL],
                    numOfDisabledMembers [0],
                    non-null  disabledMembersArray.
    Expected: GT_BAD_PTR.
    1.12. Call with trunkId [2],
                    numOfEnabledMembers [0],
                    non-null enabledMembersArray,
                    numOfDisabledMembers [1],
                    disabledMembersArray[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID             trunkId              = 0;
    GT_U32                  numOfEnabledMembers  = 0;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray [PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  numOfDisabledMembers = 0;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_BOOL                 isEqual              = GT_FALSE;
    GT_U32                  numOfMembersWritten  = 0;
    CPSS_TRUNK_MEMBER_STC   membersArrayWritten [PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           numOfEnabledMembers [5],
                           enabledMembersArray [0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]},
                           numOfDisabledMembers [7],
                           disabledMembersArray [0 / ... / 7] {port [5 / ... / 11], device [devNum]}.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        numOfEnabledMembers = 5;

        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = 0;
        enabledMembersArray[1].hwDevice = dev;
        enabledMembersArray[1].port   = 1;
        enabledMembersArray[2].hwDevice = dev;
        enabledMembersArray[2].port   = 2;
        enabledMembersArray[3].hwDevice = dev;
        enabledMembersArray[3].port   = 3;
        enabledMembersArray[4].hwDevice = dev;
        enabledMembersArray[4].port   = 4;

        numOfDisabledMembers = 7;

        disabledMembersArray[0].hwDevice = dev;
        disabledMembersArray[0].port   = 5;
        disabledMembersArray[1].hwDevice = dev;
        disabledMembersArray[1].port   = 6;
        disabledMembersArray[2].hwDevice = dev;
        disabledMembersArray[2].port   = 7;
        disabledMembersArray[3].hwDevice = dev;
        disabledMembersArray[3].port   = 8;
        disabledMembersArray[4].hwDevice = dev;
        disabledMembersArray[4].port   = 9;
        disabledMembersArray[5].hwDevice = dev;
        disabledMembersArray[5].port   = 10;
        disabledMembersArray[6].hwDevice = dev;
        disabledMembersArray[6].port   = 11;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, numOfEnabledMembers = %d, numOfDisabledMembers = %d",
                                     dev, trunkId, numOfEnabledMembers, numOfDisabledMembers);

        /*
            1.2. Call with trunkId [3],
                           numOfEnabledMembers [2],
                           enabledMembersArray [0 / 1] (already added) {port [7 / 4], device [devNum]},
                           numOfDisabledMembers [2],
                           disabledMembersArray [0 / 1] {port [6 / 2], device [devNum]}.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS + 1;

        numOfEnabledMembers = 2;
        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = 7;
        enabledMembersArray[1].hwDevice = dev;
        enabledMembersArray[1].port   = 4;

        numOfDisabledMembers = 2;

        disabledMembersArray[0].hwDevice = dev;
        disabledMembersArray[0].port   = 6;
        disabledMembersArray[1].hwDevice = dev;
        disabledMembersArray[1].port   = 2;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with trunkId [2],
                           numOfEnabledMembers [1],
                           enabledMembersArray[0] {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]},
                           numOfDisabledMembers [0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        numOfEnabledMembers = 1;
        enabledMembersArray[0].hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        enabledMembersArray[0].port   = 0;

        numOfDisabledMembers = 0;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, enabledMembersArray[0]->device = %d",
                                         dev, enabledMembersArray[0].hwDevice);

        /*
            1.4. Call with trunkId [2],
                           numOfEnabledMembers [1],
                           enabledMembersArray[0] {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]},
                           numOfDisabledMembers [0].
            Expected: NON GT_OK.
        */
        numOfEnabledMembers = 1;
        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        numOfDisabledMembers = 0;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, enabledMembersArray[0]->port = %d",
                                         dev, enabledMembersArray[0].port);

        /*
            1.5. Call cpssExMxPmTrunkDbEnabledMembersGet with trunkId [2],
                                                              non-NULL numOfEnabledMembersPtr,
                                                              and allocated enabledMembersArray for 12 members.
            Expected: GT_OK, numOfEnabledMembersPtr [5], and the same enabledMembersArray as written.
        */
        numOfEnabledMembers = 5;

        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = 0;
        enabledMembersArray[1].hwDevice = dev;
        enabledMembersArray[1].port   = 1;
        enabledMembersArray[2].hwDevice = dev;
        enabledMembersArray[2].port   = 2;
        enabledMembersArray[3].hwDevice = dev;
        enabledMembersArray[3].port   = 3;
        enabledMembersArray[4].hwDevice = dev;
        enabledMembersArray[4].port   = 4;

        /* must initialize to the max number of members we wish to receive */
        numOfMembersWritten = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "prvCpssGenericTrunkDbEnabledMembersGet: %d, %d", dev, trunkId);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfEnabledMembers, numOfMembersWritten,
                   "get another numOfEnabledMembers than was set: %d, %d", dev, trunkId);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) enabledMembersArray,
                                     (GT_VOID*) membersArrayWritten,
                                     numOfEnabledMembers * sizeof(enabledMembersArray[0]))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                   "get another enabledMembersArray than was set: %d, %d", dev, trunkId);

        /*
            1.6. Call cpssExMxPmTrunkDbDisabledMembersGet with trunkId [2],
                                                               non-NULL numOfDisabledMembersPtr,
                                                               and allocated disabledMembersArray for 12 members.
            Expected: GT_OK, numOfDisabledMembersPtr [7], and the same disabledMembersArray as written.
        */
        numOfDisabledMembers = 7;

        disabledMembersArray[0].hwDevice = dev;
        disabledMembersArray[0].port   = 5;
        disabledMembersArray[1].hwDevice = dev;
        disabledMembersArray[1].port   = 6;
        disabledMembersArray[2].hwDevice = dev;
        disabledMembersArray[2].port   = 7;
        disabledMembersArray[3].hwDevice = dev;
        disabledMembersArray[3].port   = 8;
        disabledMembersArray[4].hwDevice = dev;
        disabledMembersArray[4].port   = 9;
        disabledMembersArray[5].hwDevice = dev;
        disabledMembersArray[5].port   = 10;
        disabledMembersArray[6].hwDevice = dev;
        disabledMembersArray[6].port   = 11;

        /* must initialize to the max number of members we wish to receive */
        numOfMembersWritten = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "prvCpssGenericTrunkDbEnabledMembersGet: %d, %d", dev, trunkId);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(numOfDisabledMembers, numOfMembersWritten,
                   "get another numOfDisabledMembers than was set: %d, %d", dev, trunkId);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) disabledMembersArray,
                                     (GT_VOID*) membersArrayWritten,
                                     numOfDisabledMembers * sizeof(disabledMembersArray[0]))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                   "get another disabledMembersArray than was set: %d, %d", dev, trunkId);

        /*
            1.7. Check out of range for sum [>CPSS_EXMXPM_TRUNK_MAX_NUM_OF_MEMBERS_CNS = 12] of enabled and disabled members.
                 Call with trunkId [2],
                           numOfEnabledMembers [6],
                           enabledMembersArray [0 / 1 / 2 / 3 / 4 / 5] {port [0 / 1 / 2 / 3 / 4 / 5], device [devNum]},
                           numOfDisabledMembers [7],
                           disabledMembersArray [tested dev id, virtual port id 6, 7, 8, 9, 10, 11, 12].
            Expected: NON GT_OK.
        */
        numOfEnabledMembers = 6;

        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = 0;
        enabledMembersArray[1].hwDevice = dev;
        enabledMembersArray[1].port   = 1;
        enabledMembersArray[2].hwDevice = dev;
        enabledMembersArray[2].port   = 2;
        enabledMembersArray[3].hwDevice = dev;
        enabledMembersArray[3].port   = 3;
        enabledMembersArray[4].hwDevice = dev;
        enabledMembersArray[4].port   = 4;
        enabledMembersArray[5].hwDevice = dev;
        enabledMembersArray[5].port   = 5;

        numOfDisabledMembers = 7;

        disabledMembersArray[0].hwDevice = dev;
        disabledMembersArray[0].port   = 6;
        disabledMembersArray[1].hwDevice = dev;
        disabledMembersArray[1].port   = 7;
        disabledMembersArray[2].hwDevice = dev;
        disabledMembersArray[2].port   = 8;
        disabledMembersArray[3].hwDevice = dev;
        disabledMembersArray[3].port   = 9;
        disabledMembersArray[4].hwDevice = dev;
        disabledMembersArray[4].port   = 10;
        disabledMembersArray[5].hwDevice = dev;
        disabledMembersArray[5].port   = 11;
        disabledMembersArray[6].hwDevice = dev;
        disabledMembersArray[6].port   = 12;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, numOfEnabledMembers + numOfDisabledMembers = %d",
                                         dev, trunkId, numOfEnabledMembers + numOfDisabledMembers);

        /*
            1.8. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           numOfEnabledMembers [5],
                           enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],
                           numOfDisabledMembers [7],
                           disabledMembersArray [tested dev id, virtual port id 5 ... 11].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        numOfEnabledMembers = 5;

        enabledMembersArray[0].hwDevice = dev;
        enabledMembersArray[0].port   = 0;
        enabledMembersArray[1].hwDevice = dev;
        enabledMembersArray[1].port   = 1;
        enabledMembersArray[2].hwDevice = dev;
        enabledMembersArray[2].port   = 2;
        enabledMembersArray[3].hwDevice = dev;
        enabledMembersArray[3].port   = 3;
        enabledMembersArray[4].hwDevice = dev;
        enabledMembersArray[4].port   = 4;

        numOfDisabledMembers = 7;

        disabledMembersArray[0].hwDevice = dev;
        disabledMembersArray[0].port   = 5;
        disabledMembersArray[1].hwDevice = dev;
        disabledMembersArray[1].port   = 6;
        disabledMembersArray[2].hwDevice = dev;
        disabledMembersArray[2].port   = 7;
        disabledMembersArray[3].hwDevice = dev;
        disabledMembersArray[3].port   = 8;
        disabledMembersArray[4].hwDevice = dev;
        disabledMembersArray[4].port   = 9;
        disabledMembersArray[5].hwDevice = dev;
        disabledMembersArray[5].port   = 10;
        disabledMembersArray[6].hwDevice = dev;
        disabledMembersArray[6].port   = 11;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           numOfEnabledMembers [5],
                           enabledMembersArray [0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]},
                           numOfDisabledMembers [7],
                           disabledMembersArray [0 /  / 6] {port [5 / .. / 11], device [devNum]}.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.10. Call with trunkId [2],
                            numOfEnabledMembers [0],
                            enabledMembersArray [NULL],
                            numOfDisabledMembers [0],
                            disabledMembersArray [NULL] to remove all members from the trunk.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        numOfEnabledMembers  = 0;
        numOfDisabledMembers = 0;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, NULL,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.11. Call with trunkId [2],
                            numOfEnabledMembers [1],
                            enabledMembersArray [NULL],
                            numOfDisabledMembers [0],
                            non-null  disabledMembersArray.
            Expected: GT_BAD_PTR.
        */

        numOfEnabledMembers  = 1;
        numOfDisabledMembers = 0;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, NULL,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, numOfEnabledMembers = %d, enabledMembersArray = NULL",
                                     dev, numOfEnabledMembers);

        /*
            1.12. Call with trunkId [2],
                            numOfEnabledMembers [0],
                            non-null enabledMembersArray,
                            numOfDisabledMembers [1],
                            disabledMembersArray[NULL].
            Expected: GT_BAD_PTR.
        */

        numOfEnabledMembers  = 0;
        numOfDisabledMembers = 1;

        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, numOfDisabledMembers = %d, disabledMembersArray = NULL",
                                     dev, numOfDisabledMembers);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    numOfEnabledMembers = 5;

    enabledMembersArray[0].hwDevice = dev;
    enabledMembersArray[0].port   = 0;
    enabledMembersArray[1].hwDevice = dev;
    enabledMembersArray[1].port   = 1;
    enabledMembersArray[2].hwDevice = dev;
    enabledMembersArray[2].port   = 2;
    enabledMembersArray[3].hwDevice = dev;
    enabledMembersArray[3].port   = 3;
    enabledMembersArray[4].hwDevice = dev;
    enabledMembersArray[4].port   = 4;

    numOfDisabledMembers = 7;

    disabledMembersArray[0].hwDevice = dev;
    disabledMembersArray[0].port   = 5;
    disabledMembersArray[1].hwDevice = dev;
    disabledMembersArray[1].port   = 6;
    disabledMembersArray[2].hwDevice = dev;
    disabledMembersArray[2].port   = 7;
    disabledMembersArray[3].hwDevice = dev;
    disabledMembersArray[3].port   = 8;
    disabledMembersArray[4].hwDevice = dev;
    disabledMembersArray[4].port   = 9;
    disabledMembersArray[5].hwDevice = dev;
    disabledMembersArray[5].port   = 10;
    disabledMembersArray[6].hwDevice = dev;
    disabledMembersArray[6].port   = 11;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                           numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                       numOfDisabledMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkMemberAdd)
{
/*
    ITERATE_DEVICES
    1.1. Call with trunkId [2],
                   memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.2. Call with trunkId [2]
                   and memberPtr {port [0], device [devNum]} (already added).
    Expected: GT_OK.
    1.3. Call with trunkId [3],
                   memberPtr {port [0], device [devNum]} (already added to another trunkId).
    Expected: NON GT_OK.
    1.4. Call with trunkId [2],
                   non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
    Expected: NON GT_OK.
    1.5. Call with trunkId [2],
                   non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
    Expected: NON GT_OK.
    1.6. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL memberPtr [tested dev, virtual port 0].
    Expected: NON GT_OK.
    1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   memberPtr [tested dev, virtual port 0].
    Expected: NON GT_OK.
    1.8. Call with trunkId [2],
                   memberPtr [NULL].
    Expected: GT_BAD_PTR.
    1.9. Call cpssExMxPmTrunkMemberRemove with trunkId [2],
                                               memberPtr {port [0], device [devNum]} to remove added member from the trunk.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID             trunkId = 0;
    CPSS_TRUNK_MEMBER_STC   member;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = dev;
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.2. Call with trunkId [2]
                           and memberPtr {port [0], device [devNum]} (already added).
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.3. Call with trunkId [3],
                           memberPtr {port [0], device [devNum]} (already added to another trunkId).
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS + 1;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                         dev, trunkId, member.hwDevice, member.port);

        /*
            1.4. Call with trunkId [2],
                           non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        member.hwDevice = dev;

        /*
            1.5. Call with trunkId [2],
                           non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
            Expected: NON GT_OK.
        */
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        member.port = TRUNK_VALID_VIRT_PORT_CNS;

        /*
            1.6. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.8. Call with trunkId [2],
                           memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /*
            1.9. Call cpssExMxPmTrunkMemberRemove with trunkId [2],
                                                       memberPtr {port [0], device [devNum]} to remove added member from the trunk.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    member.hwDevice = 0;
    member.port   = TRUNK_VALID_VIRT_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkMemberRemove)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssExMxPmTrunkMemberAdd with trunkId [2],
                                            non-NULL memberPtr {port [0], device [devNum]} to add new member.
    Expected: GT_OK.
    1.2. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.3. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]} (already removed).
    Expected: GT_OK.
    1.4. Call with trunkId [2],
                   non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
    Expected: NOT GT_OK.
    1.5. Call with trunkId [2],
                   non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
    Expected: NON GT_OK.
    1.6. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL memberPtr [tested dev, virtual port 0].
    Expected: NON GT_OK.
    1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   non-NULL memberPtr [tested dev, virtual port 0].
    Expected: NON GT_OK.
    1.8. Check for NULL handling.
         Call with trunkId [2],
                   memberPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID             trunkId = 0;
    CPSS_TRUNK_MEMBER_STC   member;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssExMxPmTrunkMemberAdd with trunkId [2],
                                                    non-NULL memberPtr {port [0], device [devNum]} to add new member.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = dev;
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.2. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.3. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]} (already removed).
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.4. Call with trunkId [2],
                           non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
            Expected: GT_OK.
        */
        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        member.hwDevice = dev;

        /*
            1.5. Call with trunkId [2],
                           non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
            Expected: GT_OK.
        */
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        member.port = TRUNK_VALID_VIRT_PORT_CNS;

        /*
            1.6. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           non-NULL memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.8. Check for NULL handling.
                 Call with trunkId [2],
                           memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    member.hwDevice = dev;
    member.port   = TRUNK_VALID_VIRT_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkMemberDisable)
{
/*
    ITERATE_DEVICES
    1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                            non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.2. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.3. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]} (already disabled).
    Expected: GT_OK.
    1.4. Call with trunkId [2],
                   non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}
    Expected: NON GT_OK.
    1.5. Call with trunkId [2],
                   non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
    Expected: NON GT_OK.
    1.6. Call prvCpssGenericTrunkMemberRemove with trunkId [2],
                                               non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.7. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]} (removed member).
    Expected: NON GT_OK.
    1.8. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL memberPtr [tested dev, virtual port 0].
    Expected: NON GT_OK.
    1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: NON GT_OK.
    1.10. Call with trunkId [2], memberPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID             trunkId = 0;
    CPSS_TRUNK_MEMBER_STC   member;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                                    non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = dev;
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.2. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.3. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]} (already disabled).
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.4. Call with trunkId [2],
                           non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}
            Expected: NON GT_OK.
        */
        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        member.hwDevice = dev;

        /*
            1.5. Call with trunkId [2],
                           non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
            Expected: NON GT_OK.
        */
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        member.port = TRUNK_VALID_VIRT_PORT_CNS;

        /*
            1.6. Call cpssExMxPmTrunkMemberRemove with trunkId [2],
                                                       non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.7. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]} (removed member).
            Expected: NON GT_OK.
        */
        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                         dev, trunkId, member.hwDevice, member.port);

        /*
            1.8. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           non-NULL memberPtr {port [0], device [devNum]}.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.10. Call with trunkId [2],
                            memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    member.hwDevice = 0;
    member.port   = TRUNK_VALID_VIRT_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkMemberDisable(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkMemberEnable)
{
/*
    ITERATE_DEVICES
    1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                            non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.2. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.3. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]} (already enabled).
    Expected: GT_OK.
    1.4. Call with trunkId [2],
                   non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
    Expected: NON GT_OK.
    1.5. Call with trunkId [2],
                   non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
    Expected: NON GT_OK.
    1.6. Call prvCpssGenericTrunkMemberRemove with trunkId [2],
                                               non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.7. Call with trunkId [2],
                   non-NULL memberPtr {port [0], device [devNum]} (removed member).
    Expected: NON GT_OK.
    1.8. Call with trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: NON GT_OK.
    1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   non-NULL memberPtr {port [0], device [devNum]}.
    Expected: NON GT_OK.
    1.10. Call with trunkId [2],
                    memberPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID             trunkId = 0;
    CPSS_TRUNK_MEMBER_STC   member;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                                    non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = dev;
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.2. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.3. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]} (already enabled).
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.4. Call with trunkId [2],
                           non-NULL memberPtr {port [0], out of range device [PRV_CPSS_MAX_PP_DEVICES_CNS]}
            Expected: NON GT_OK.
        */
        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        member.hwDevice = dev;

        /*
            1.5. Call with trunkId [2],
                           non-NULL memberPtr {out of range port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}.
            Expected: NON GT_OK.
        */
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        member.port = TRUNK_VALID_VIRT_PORT_CNS;

        /*
            1.6. Call prvCpssGenericTrunkMemberRemove with trunkId [2],
                                                       non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /*
            1.7. Call with trunkId [2],
                           non-NULL memberPtr {port [0], device [devNum]} (removed member).
            Expected: NON GT_OK.
        */
        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                         dev, trunkId, member.hwDevice, member.port);

        /*
            1.8. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL memberPtr [tested dev, virtual port 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           non-NULL memberPtr {port [0], device [devNum]}.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.10. Call with trunkId [2],
                            memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    member.hwDevice = 0;
    member.port   = TRUNK_VALID_VIRT_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkMemberEnable(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkNonTrunkPortsAdd)
{
/*
    ITERATE_DEVICES
    1.1. Call with trunkId [2],
                   nonTrunkPortsBmpPtr->ports [3, 0].
    Expected: GT_OK.
    1.2. Call with trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL nonTrunkPortsBmpPtr [3, 0].
    Expected: GT_OK.
    1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   non-NULL nonTrunkPortsBmpPtr [3, 0].
    Expected: NON GT_OK.
    1.4. Call with trunkId [2],
                   nonTrunkPortsBmpPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call prvCpssGenericTrunkNonTrunkPortsRemove with trunkId [2],
                                                 nonTrunkPortsBmp [3, 0] to remove all non trunk members from the trunk.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID         trunkId = 0;
    CPSS_PORTS_BMP_STC  nonTrunkPortsBmp;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           nonTrunkPortsBmpPtr->ports [3, 0].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&nonTrunkPortsBmp);

        nonTrunkPortsBmp.ports[0] = 3;
        nonTrunkPortsBmp.ports[1] = 0;

        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL nonTrunkPortsBmpPtr [3, 0].
            Expected: GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           non-NULL nonTrunkPortsBmpPtr [3, 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.4. Call with trunkId [2],
                           nonTrunkPortsBmpPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsBmpPtr = NULL", dev, trunkId);

        /*
            1.5. Call prvCpssGenericTrunkNonTrunkPortsRemove with trunkId [2],
                                                               nonTrunkPortsBmp [3, 0] to remove all non trunk members from the trunk.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkNonTrunkPortsRemove: %d, %d",
                                     dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&nonTrunkPortsBmp);
    nonTrunkPortsBmp.ports[0] = 3;
    nonTrunkPortsBmp.ports[1] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkNonTrunkPortsRemove)
{
/*
    ITERATE_DEVICES
    1.1. Call prvCpssGenericTrunkNonTrunkPortsAdd with trunkId [2],
                                                   nonTrunkPortsBmpPtr [3, 0].
    Expected: GT_OK.
    1.2. Call with trunkId [2],
                   non-NULL nonTrunkPortsBmpPtr [3, 0].
    Expected: GT_OK.
    1.3. Call with trunkId [NULL_TRUNK_ID_CNS = 0],
                   non-NULL nonTrunkPortsBmpPtr [3, 0].
    Expected: GT_OK.
    1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                   non-NULL nonTrunkPortsBmpPtr [3, 0].
    Expected: NON GT_OK.
    1.5. Call with trunkId [2],
                   nonTrunkPortsBmpPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID         trunkId = 0;
    CPSS_PORTS_BMP_STC  nonTrunkPortsBmp;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCpssGenericTrunkNonTrunkPortsAdd with trunkId [2],
                                                           nonTrunkPortsBmpPtr [3, 0].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&nonTrunkPortsBmp);
        nonTrunkPortsBmp.ports[0] = 3;
        nonTrunkPortsBmp.ports[1] = 0;

        st = prvCpssGenericTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssGenericTrunkNonTrunkPortsAdd: %d, %d",
                                     dev, trunkId);
        /*
            1.2. Call with trunkId [2],
                           nonTrunkPortsBmpPtr->ports [3, 0].
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with trunkId [NULL_TRUNK_ID_CNS = 0],
                           non-NULL nonTrunkPortsBmpPtr [3, 0].
            Expected: GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256],
                           non-NULL nonTrunkPortsBmpPtr [3, 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.5. Call with trunkId [2],
                           nonTrunkPortsBmpPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsBmpPtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&nonTrunkPortsBmp);
    nonTrunkPortsBmp.ports[0] = 3;
    nonTrunkPortsBmp.ports[1] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkDbEnabledMembersGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with trunkId [2],
                   non-NULL numOfEnabledMembers [12],
                   and allocated enabledMembersArray for 12 members.
    Expected: GT_OK.
    1.2. Call with trunk id [NULL_TRUNK_ID_CNS = 0].
    Expected: NON GT_OK.
    1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256].
    Expected: NON GT_OK.
    1.4. Call with trunkId [2],
                   numOfEnabledMembers [NULL],
                   and allocated enabledMembersArray.
    Expected: GT_BAD_PTR.
    1.5. Call with trunkId [2],
                   enabledMembersArray [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID                 trunkId        = 0;
    GT_U32                      numOfEnMembers = 0;
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           non-NULL numOfEnabledMembers [12],
                           and allocated enabledMembersArray for 12 members.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;
        /* must initialize to the max number of members we wish to receive */
        numOfEnMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, enabledMembersArray);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, numOfEnMembers);

        /*
            1.2. Call with trunk id [NULL_TRUNK_ID_CNS = 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        /* must initialize to the max number of members we wish to receive */
        numOfEnMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, enabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        /* must initialize to the max number of members we wish to receive */
        numOfEnMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, enabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.5. Call with trunkId [2],
                           numOfEnabledMembers [NULL],
                           and allocated enabledMembersArray.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, NULL, enabledMembersArray);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfEnabledMembers = NULL", dev);

        /*
            1.6. Call with trunkId [2],
                           enabledMembersArray [NULL].
            Expected: GT_BAD_PTR.
        */
        numOfEnMembers = 1;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enabledMembersArray = NULL", dev);
    }

    trunkId        = TRUNK_TESTED_TRUNK_ID_CNS;
    numOfEnMembers = 8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* must initialize to the max number of members we wish to receive */
        numOfEnMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, enabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* must initialize to the max number of members we wish to receive */
    numOfEnMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

    st = prvCpssGenericTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnMembers, enabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkDbDisabledMembersGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with trunkId [2],
                   non-NULL numOfDisabledMembersPtr [12],
                   and allocated disabledMembersArray for 12 members.
    Expected: GT_OK.
    1.2. Call with trunk id [NULL_TRUNK_ID_CNS = 0].
    Expected: NON GT_OK.
    1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256].
    Expected: NON GT_OK.
    1.4. Call with trunkId [2],
                   numOfDisabledMembersPtr [NULL],
                   and allocated disabledMembersArray.
    Expected: GT_BAD_PTR.
    1.5. Call with trunkId [2],
                   disabledMembersArray [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_TRUNK_ID                 trunkId         = 0;
    GT_U32                      numOfDisMembers = 0;
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           non-NULL numOfDisabledMembersPtr [12],
                           and allocated disabledMembersArray for 12 members.
            Expected: GT_OK.
        */
        trunkId         = TRUNK_TESTED_TRUNK_ID_CNS;
        /* must initialize to the max number of members we wish to receive */
        numOfDisMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, numOfDisMembers);

        /*
            1.2. Call with trunk id [NULL_TRUNK_ID_CNS = 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;

        /* must initialize to the max number of members we wish to receive */
        numOfDisMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 256].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_MAX_TRUNK_ID_CNS;

        /* must initialize to the max number of members we wish to receive */
        numOfDisMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        /*
            1.4. Call with trunkId [2],
                           numOfDisabledMembersPtr [NULL],
                           and allocated disabledMembersArray.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, NULL, disabledMembersArray);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfDisabledMembersPtr = NULL", dev);

        /*
            1.5. Call with trunkId [2],
                           disabledMembersArray [NULL].
            Expected: GT_BAD_PTR.
        */
        numOfDisMembers = 1;

        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, disabledMembersArray = NULL", dev);
    }

    trunkId         = TRUNK_TESTED_TRUNK_ID_CNS;
    numOfDisMembers = 8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* must initialize to the max number of members we wish to receive */
        numOfDisMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* must initialize to the max number of members we wish to receive */
    numOfDisMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

    st = prvCpssGenericTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssGenericTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
*/
UTF_TEST_CASE_MAC(prvCpssGenericTrunkDbIsMemberOfTrunk)
{
/*
    ITERATE_DEVICES (ExMxPm)
    1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                            and non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.2. Call with memberPtr {port [0], device [devNum]},
                   non-NULL trunkIdPtr.
    Expected: GT_OK and trunkIdPtr [2].
    1.3. Call with non-NULL memberPtr {port [0], device [devNum]},
                   trunkIdPtr [NULL].
    Expected: GT_OK.
    1.4. Call prvCpssGenericTrunkMemberRemove with trunkId [2],
                                               non-NULL memberPtr {port [0], device [devNum]}.
    Expected: GT_OK.
    1.5. Call with non-NULL memberPtr {port [0], device [devNum]} (removed member),
                   non-NULL trunkIdPtr.
    Expected: NOT GT_OK.
    1.6. Call with memberPtr [NULL],
                   non-NULL trunkIdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_TRUNK_MEMBER_STC   member;
    GT_TRUNK_ID             trunkId    = 0;
    GT_TRUNK_ID             trunkIdGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call prvCpssGenericTrunkMemberAdd with trunkId [2],
                                                    and non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS;

        member.hwDevice = dev;
        member.port   = TRUNK_VALID_VIRT_PORT_CNS;

        st = prvCpssGenericTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "prvCpssGenericTrunkMemberAdd: %d, %d", dev, trunkId);

        /*
            1.2. Call with memberPtr {port [0], device [devNum]},
                           non-NULL trunkIdPtr.
            Expected: GT_OK and trunkIdPtr [2].
        */
        st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, member.hwDevice, member.port);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkId, trunkIdGet,
                   "get another trunkId than was set: %d", dev);

        /*
            1.3. Call with non-NULL memberPtr {port [0], device [devNum]},
                           trunkIdPtr [NULL].
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, &member, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, trunkIdPtr = NULL", dev);

        /*
            1.4. Call prvCpssGenericTrunkMemberRemove with trunkId [2],
                                                       non-NULL memberPtr {port [0], device [devNum]}.
            Expected: GT_OK.
        */
        st = prvCpssGenericTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "prvCpssGenericTrunkMemberRemove: %d, %d", dev, trunkId);

        /*
            1.5. Call with non-NULL memberPtr {port [0], device [devNum]} (removed member),
                           non-NULL trunkIdPtr.
            Expected: NOT GT_OK.
        */
        st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdGet);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, member.hwDevice, member.port);

        /*
            1.6. Call with memberPtr [NULL],
                           non-NULL trunkIdPtr.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, NULL, &trunkIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, memberPtr = NULL", dev);
    }

    member.hwDevice = 0;
    member.port   = TRUNK_VALID_VIRT_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_ALL_FAMILIES_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = prvCpssGenericTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssGenTrunk suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssGenTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkInit)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkMembersSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkMemberAdd)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkMemberRemove)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkMemberDisable)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkMemberEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkNonTrunkPortsAdd)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkNonTrunkPortsRemove)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkDbEnabledMembersGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkDbDisabledMembersGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssGenericTrunkDbIsMemberOfTrunk)
UTF_SUIT_END_TESTS_MAC(cpssGenTrunk)


