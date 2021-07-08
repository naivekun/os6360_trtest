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
* @file cpssDxChTrunkUT.c
*
* @brief Unit Tests for Trunk module.
*
* Trunk module has two types of APIs: High-Level and Low-Level.
* It's not recommended to mix both APIs so this test plan describes
* two test suits. First suit is cpssDxChTrunkHighLevel which includes
* all functions for High-Level mode and second suit is
* cpssDxChTrunkLowLevel which includes all functions for Low-Level mode
* and functions compliance with both modes.
*
* For the cpssDxChTrunk HighLevel and LowLevel suits
* the following assumptions are done:
* 1. There is shadow DB for trunk's tables. Number of trunks is greater than zero.
* 2. Trunk ids 2, 3 are in range for trunk ids.
* 3. Virtual ports from 0 to 8 are valid for all tested device ids.
* 4. Test cases should think about cleaning trunk's tables after testing.
* 5. Test suits will be run separately after reset.
*
* @version   55
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* bookmark to state that we need last index in the L2 ECMP table to hold the NULL port */
/* see PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpIndex_NULL_port */
#define L2_ECMP_NULL_PORT_BOOKMARK


typedef struct{
    GT_U32  port;
    GT_U32  weight;
}PORT_WITH_WEIGHT_STC;

typedef struct{
    PORT_WITH_WEIGHT_STC  *portsArray;
    GT_U32  sizeofArray;
}PORTS_ARRAY_WITH_SIZE_STC;
#define PORTS_ARRAY_WITH_SIZE_MAC(portsArray) {portsArray,sizeof(portsArray)/sizeof(portsArray[0])}
#define NULL_ARRAY_WITH_SIZE_MAC {NULL,0}

    static PORT_WITH_WEIGHT_STC                  portWeightArr_1_members[]=
    {
        /*port*/    /*weight*/
        {5       ,      500}/* don't care on single port */
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_2_members[]=
    {
        /*port*/    /*weight*/
        {5       ,      3},
        {16      ,      5}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_3_members[]=
    {
        /*port*/    /*weight*/
        {5       ,      3},
        {16      ,      3},
        {26      ,      2}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_4_members[]=
    {
        /*port*/    /*weight*/
        {6       ,      2},
        {17      ,      2},
        {27      ,      1},
        {11      ,      3}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_5_members[]=
    {
        /*port*/    /*weight*/
        {6       ,      1},
        {17      ,      1},
        {27      ,      4},
        {11      ,      1},
        {20      ,      1}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_6_members[]=
    {
        /*port*/    /*weight*/
        {6       ,      1},
        {17      ,      1},
        {27      ,      2},
        {18      ,      1},
        {11      ,      2},
        {20      ,      1}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_7_members[]=
    {
        /*port*/    /*weight*/
        {6       ,      1},
        {17      ,      1},
        {27      ,      1},
        {18      ,      1},
        {11      ,      2},
        {20      ,      1},
        {23      ,      1}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_8_members[]=
    {
        /*port*/    /*weight*/
        {6       ,      1},
        {17      ,      1},
        {27      ,      1},
        {24      ,      1},
        {18      ,      1},
        {11      ,      1},
        {20      ,      1},
        {23      ,      1}
    };

    static PORT_WITH_WEIGHT_STC                  portWeightArr_3_members_case_1[]=
    {
        /*port*/    /*weight*/
        {5       ,      1},
        {16      ,      5},
        {26      ,      2}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_3_members_case_2[]=
    {
        /*port*/    /*weight*/
        {5       ,      4},
        {16      ,      3},
        {26      ,      1}
    };
    static PORT_WITH_WEIGHT_STC                  portWeightArr_3_members_case_3[]=
    {
        /*port*/    /*weight*/
        {5       ,      2},
        {16      ,      3},
        {26      ,      3}
    };

    static PORTS_ARRAY_WITH_SIZE_STC  portsArrayWithSize[]=
    {
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_2_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_6_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_4_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_5_members),
        NULL_ARRAY_WITH_SIZE_MAC,/*NULL*/
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_1_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_3_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_7_members),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_8_members),

        /* manipulation on weights of the same members*/
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_3_members_case_1),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_3_members_case_2),
        PORTS_ARRAY_WITH_SIZE_MAC(portWeightArr_3_members_case_3)


    };

/* sorting mode - by default = GT_FALSE */
static GT_BOOL  sortMode = GT_FALSE;

/* Trunk database starts from index 1. This value is used as null index.    */
#define TRUNK_NULL_TRUNK_ID_CNS         0
#define TRUNK_ID_ALL_CNS    0xFFFF
static void checkLowLevelMembers(
    IN  GT_U8   dev,
    IN  GT_TRUNK_ID trunkId,
    IN  GT_U32  db_numOfEnabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   db_EnabledMembersArr[],
    IN  GT_U32  db_numOfDisabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   db_DisabledMembersArr[]
);

/* get the number of 'mac trunks' that was initialized by cpssDxChTrunkInit(...)*/
static GT_U32   getInitMaxTrunks(IN GT_U8   dev)
{
    GT_STATUS   st;
    GT_U32  get_maxNumberOfTrunks;

    st = cpssDxChTrunkDbInitInfoGet(dev,&get_maxNumberOfTrunks,NULL);
    if(st != GT_OK)
    {
        return 0;
    }

    return get_maxNumberOfTrunks;
}

#define NUM_OF_MEMBERS_MAC(_x) (sizeof(_x)/sizeof(_x[0]))
#define MAX_TRUNKS_CNS(devNum)  \
    getInitMaxTrunks(devNum)

/* Tested trunk id. All tests uses this trunk id for testing. It means that     */
/* test must clear this trunk id after testing.is completed.                    */
/* NOTE: in xCat\xCat3 use trunk 2 because issues of 'trunk WA' that the appdemo enable for it
    so need to use range of 0..31 (that is converted to even numbers > 64 ...) */
#define TRUNK_TESTED_TRUNK_ID_CNS(devNum)                                             \
        (GT_TRUNK_ID)(((devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) &&                    \
        (0==PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(devNum) &&       \
            (MAX_TRUNKS_CNS(devNum) > 30)) ?    \
                (MAX_TRUNKS_CNS(devNum) - 30) : \
                2)

/* Tested virtual port id. All tests uses this port id for testing.     */
#define TRUNK_TESTED_VIRT_PORT_ID_CNS(devNum)                              \
        (GT_PHYSICAL_PORT_NUM)(                                                  \
           ((devNum) >= PRV_CPSS_MAX_PP_DEVICES_CNS)  ? 8 :/*device not exists*/ \
           (PRV_CPSS_PP_MAC(devNum)->numOfPorts > 28) ? (PRV_CPSS_PP_MAC(devNum)->numOfPorts - 6) : \
           /* support ch3 XG : ports: 0,4,10,12,16,22, 24..27*/ \
           (PRV_CPSS_PP_MAC(devNum)->existingPorts.ports[0] == 0xF411411) ? 12 : \
                8)

/* is eArch enabled in device */
#define TRUNK_E_ARCH_ENABLED_MAC(_devNum) \
    (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ? UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_devNum) : 0)

#define TRUNK_HASH_MASK_CRC_NUM_ENTRIES_CNS(_devNum)    \
    (GT_U32)(TRUNK_E_ARCH_ENABLED_MAC(_devNum) ? (BIT_4 + 16) : (BIT_4 + 12))

/* get HW device number */
#define TRUNK_HW_DEV_NUM_MAC(_devNum) \
    (((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ? PRV_CPSS_HW_DEV_NUM_MAC(_devNum) : _devNum)

/* force trunkId to be in valid range for cascade trunks */
#define FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(_trunkId) (GT_TRUNK_ID)((_trunkId) & 0xFF)

/* Default valid value for port id */
#define TRUNK_PORT_VALID_PHY_PORT_CNS  0

/* replace used members with other members */
GT_STATUS   mainUtTrunkReplaceUsedMembers(
    IN GT_U8                dev ,
    IN GT_TRUNK_ID         trunkId,
    INOUT CPSS_TRUNK_MEMBER_STC   membersArray[],
    IN GT_U32                  numOfMembers
)
{
    GT_STATUS   st;
    GT_U32  jj,kk;
    GT_U32  isLocalDev;
    GT_TRUNK_ID existsInTrunkId;

    /* replace ports that already in pre-defined cascade trunk(s) */
    for(jj = 0 ; jj < numOfMembers ; jj++)
    {
        isLocalDev = (membersArray[jj].hwDevice == TRUNK_HW_DEV_NUM_MAC(dev)) ? 1 : 0;

        find_proper_port:

        do
        {
            /* check if those ports are not members of some cascade trunk in the system */
            st = cpssDxChTrunkDbIsMemberOfTrunk(dev,&membersArray[jj],&existsInTrunkId);
            if(st == GT_OK && (trunkId != existsInTrunkId))
            {
                /* port already in trunk ... remove it */
                membersArray[jj].port++;
                if(isLocalDev)
                {
                    while(0 == PRV_CPSS_PHY_PORT_IS_EXIST_MAC(dev, membersArray[jj].port))
                    {
                        membersArray[jj].port++;
                        if(membersArray[jj].port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
                        {
                            membersArray[jj].port = 0;
                        }
                    }
                }

                if(membersArray[jj].port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
                {
                    membersArray[jj].port = 0;
                }
            }
            else
            {
                /* not in trunk , or already in the needed trunk (it's OK) */
                break;
            }

            if(membersArray[jj].port == CPSS_CPU_PORT_NUM_CNS)
            {
                /* I think that can find port within the first 64 ports */
                membersArray[jj].port = 0;

                goto find_proper_port;
            }
        }
        while(1);

        /* resolve duplications */
        for(kk = 0 ; kk < jj; kk++)
        {
            if(membersArray[jj].port == membersArray[kk].port)
            {
                membersArray[jj].port++;
                if(membersArray[jj].port >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))
                {
                    membersArray[jj].port = 0;
                }
                goto find_proper_port;
            }
        }
    }


    return GT_OK;
}

GT_STATUS   mainUtTrunkReplaceUsedWeightedMembers(
    IN GT_U8                dev ,
    IN GT_TRUNK_ID         trunkId,
    INOUT CPSS_TRUNK_WEIGHTED_MEMBER_STC   weightMembersArray[],
    IN GT_U32                  numOfMembers
)
{
    GT_STATUS   rc;
    CPSS_TRUNK_MEMBER_STC   membersArray[64];
    GT_U32  maxLoop = (numOfMembers < 64) ? numOfMembers : 64;
    GT_U32  ii;

    /* build members array from weighted members */
    for(ii = 0 ; ii < maxLoop ; ii++)
    {
        membersArray[ii] = weightMembersArray[ii].member;
    }

    rc = mainUtTrunkReplaceUsedMembers(dev,trunkId,membersArray,maxLoop);

    /* build weighted members from members array */
    for(ii = 0 ; ii < maxLoop ; ii++)
    {
        weightMembersArray[ii].member = membersArray[ii];
    }

    return rc;
}


/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMembersSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with trunkId [2], numOfEnabledMembers [5], enabledMembersArray[0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]}, numOfDisabledMembers [3], disabledMembersArray [0 / 1 / 2] {port [5 / 6 / 7], device [devNum]}. Expected: GT_OK.
1.2. Try to set already written trunk members to another trunk id. Call with trunk id [3], numOfEnabledMembers [2], enabledMembersArray [0 / 1] {port [7 / 4], device[devNum]},  numOfDisabledMembers [2], disabledMembersArray [0 / 1] {port [6 / 2], device [devNum]}. Expected: NON GT_OK.
1.3. Try to set invalid device id and legal port id as trunk entry member. Call with trunk id [2], numOfEnabledMembers [1], enabledMembersArray[0] {port [0], device [PRV_CPSS_MAX_PP_DEVICES_CNS]}, numOfDisabledMembers [0]. Expected: NON GT_OK.
1.4. Try to set legal device id and invalid port id as trunk entry member. Call with trunk id [2], numOfEnabledMembers [1], enabledMembersArray[0] {port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}, numOfDisabledMembers [0]. Expected: NON GT_OK.
1.5. Call cpssDxChTrunkDbEnabledMembersGet with trunkId [2], non-NULL numOfEnabledMembersPtr, and allocated enabledMembersArray for 8 members. Expected: GT_OK, numOfEnabledMembersPtr [5], and the same enabledMembersArray as written.
1.6. Call cpssDxChTrunkDbDisabledMembersGet with trunkId [2], non-NULL numOfDisabledMembersPtr, and allocated disabledMembersArray for 8 members. Expected: GT_OK, numOfDisabledMembersPtr [3], and the same disabledMembersArray as written.
1.7. Check out of range for sum [>CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS = 8] of enabled and disabled members. Call with trunkId [2], numOfEnabledMembers [6], enabledMembersArray [0 / 1 / 2 / 3 / 4 / 5] {port [0 / 1 / 2 / 3 / 4 / 5], device [devNum]}, numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 6, 7, 8]. Expected: NON GT_OK.
1.8. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], numOfEnabledMembers [5], enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4], numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7]. Expected: NON GT_OK.
1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], numOfEnabledMembers [5], enabledMembersArray [0 / 1 / 2 / 3 / 4] {port [0 / 1 / 2 / 3 / 4], device [devNum]}, numOfDisabledMembers [3], disabledMembersArray [0 / 1 / 2] {port [5 / 6 / 7], device [devNum]}. Expected: NON GT_OK.
1.10. Call with trunkId [2], numOfEnabledMembers [0], enabledMembersArray [NULL], numOfDisabledMembers [0], disabledMembersArray [NULL] to remove all members from the trunk. Expected: GT_OK.
1.11. Call with trunkId [2], numOfEnabledMembers [1], enabledMembersArray [NULL], numOfDisabledMembers [0], non-null  disabledMembersArray.
Expected: GT_BAD_PTR.
1.12. Call with trunkId [2], numOfEnabledMembers [0], non-null enabledMembersArray, numOfDisabledMembers [1], disabledMembersArray[NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  numOfDisabledMembers;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_32                   res;
    GT_U32                  numOfMembersWritten;
    CPSS_TRUNK_MEMBER_STC   membersArrayWritten[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_HW_DEV_NUM           hwDevNum;
    GT_U32                  jj,kk;
    GT_U32                  error_numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   error_enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  error_numOfDisabledMembers;
    CPSS_TRUNK_MEMBER_STC   error_disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)membersArrayWritten, sizeof(membersArrayWritten));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], numOfEnabledMembers [5],                                         */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],                          */
        /* numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7].     */
        /* Expected: GT_OK.                                                                             */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        numOfEnabledMembers = 5;
        enabledMembersArray[0].hwDevice = hwDevNum;
        enabledMembersArray[0].port = 4;
        enabledMembersArray[1].hwDevice = hwDevNum;
        enabledMembersArray[1].port = 1;
        enabledMembersArray[2].hwDevice = hwDevNum;
        enabledMembersArray[2].port = 3;
        enabledMembersArray[3].hwDevice = hwDevNum;
        enabledMembersArray[3].port = 2;
        enabledMembersArray[4].hwDevice = hwDevNum;
        enabledMembersArray[4].port = 0;

        numOfDisabledMembers = 3;
        disabledMembersArray[0].hwDevice = hwDevNum;
        disabledMembersArray[0].port = 6;
        disabledMembersArray[1].hwDevice = hwDevNum;
        disabledMembersArray[1].port = 5;
        disabledMembersArray[2].hwDevice = hwDevNum;
        disabledMembersArray[2].port = 7;

        if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* support for CH3 device with 10 XG ports */
            if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
            {
                enabledMembersArray[1].port = 16/*1*/;
                enabledMembersArray[2].port = 10/*3*/;
                enabledMembersArray[3].port = 22/*2*/;

                disabledMembersArray[0].port = 26/*6*/;
                disabledMembersArray[1].port = 25/*5*/;
                disabledMembersArray[2].port = 27/*7*/;
            }
        }

        st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                     numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.2. Try to set already written trunk members to another trunk id.                           */
        /* Call with trunk id [3], numOfEnabledMembers [2],                                             */
        /* enabledMembersArray [tested dev id, virtual port 7, 4],                                      */
        /* numOfDisabledMembers [2], disabledMembersArray [tested dev id, virtual port id 6, 2].        */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev + 1);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        error_numOfEnabledMembers = 2;
        error_enabledMembersArray[0].hwDevice = hwDevNum;
        error_enabledMembersArray[0].port = 7;
        error_enabledMembersArray[1].hwDevice = hwDevNum;
        error_enabledMembersArray[1].port = 4;

        error_numOfDisabledMembers = 2;
        error_disabledMembersArray[0].hwDevice = hwDevNum;
        error_disabledMembersArray[0].port = 6;
        error_disabledMembersArray[1].hwDevice = hwDevNum;
        error_disabledMembersArray[1].port = 2;

        if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* support for CH3 device with 10 XG ports */
            if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
            {
                error_enabledMembersArray[0].port = 16/*7*/;

                error_disabledMembersArray[0].port = 26/*6*/;
                error_disabledMembersArray[1].port = 25/*2*/;
            }
        }

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.3. Try to set invalid device id and legal port id as trunk entry member.                   */
        /* Call with trunk id [2], numOfEnabledMembers [1],                                             */
        /* enabledMembersArray [out of range device id, virtual port 8], numOfDisabledMembers [0].      */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev + 2);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        error_numOfEnabledMembers = 1;
        error_enabledMembersArray[0].hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        error_enabledMembersArray[0].port = 0;

        error_numOfDisabledMembers = 0;

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, enabledMembersArray[0]->hwDevice = %d",
                                         dev, error_enabledMembersArray[0].hwDevice);

        /* 1.4. Try to set legal device id and invalid port id as trunk entry member.                   */
        /* Call with trunk id [2], numOfEnabledMembers [1],                                             */
        /* enabledMembersArray [tested device id, out of range virtual port],                           */
        /* numOfDisabledMembers [0].                                                                    */
        /* Expected: NON GT_OK.                                                                         */
        error_numOfEnabledMembers = 1;
        error_enabledMembersArray[0].hwDevice = hwDevNum;
        error_enabledMembersArray[0].port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        error_numOfDisabledMembers = 0;

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, enabledMembersArray[0]->port = %d",
                                         dev, error_enabledMembersArray[0].port);

        /* 1.5. Call cpssDxChTrunkDbEnabledMembersGet with trunkId [2],                                 */
        /* non-NULL numOfEnabledMembersPtr, and allocated enabledMembersArray for 8 members.            */
        /* Expected: GT_OK, numOfEnabledMembersPtr [5], and the same enabledMembersArray as written.    */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        numOfMembersWritten = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbEnabledMembersGet: %d, %d", dev, trunkId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numOfEnabledMembers, numOfMembersWritten,
                   "get another numOfEnabledMembersPtr than was set: %d, %d", dev, trunkId);

        if(sortMode == GT_TRUE)
        {
            if(numOfEnabledMembers == numOfMembersWritten)
            {
                /* find the members */
                for(jj = 0 ; jj < numOfMembersWritten; jj++)
                {
                    for(kk = 0 ; kk < numOfEnabledMembers; kk++)
                    {
                        if(enabledMembersArray[kk].port    == membersArrayWritten[jj].port &&
                           enabledMembersArray[kk].hwDevice  == membersArrayWritten[jj].hwDevice)
                        {
                            break;
                        }
                    }

                    if(kk == numOfEnabledMembers)
                    {
                        /* member was not found */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                    }
                }
            }

            if(numOfMembersWritten > 1)
            {
                /* check that members are sorted */
                for(jj = 0 ; jj < (numOfMembersWritten - 1); jj++)
                {
                    if((membersArrayWritten[jj].hwDevice > membersArrayWritten[jj+1].hwDevice) ||
                       ((membersArrayWritten[jj].hwDevice == membersArrayWritten[jj+1].hwDevice) &&
                        (membersArrayWritten[jj].port   > membersArrayWritten[jj+1].port)))
                    {
                        /* member not sorted  */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member not sorted");
                    }
                }
            }
        }
        else
        {
            res = cpssOsMemCmp((const GT_VOID*)enabledMembersArray, (const GT_VOID*)membersArrayWritten,
                                numOfEnabledMembers * sizeof(enabledMembersArray[0]));
            UTF_VERIFY_EQUAL2_STRING_MAC(0, res,
                       "get another enabledMembersArray than was set: %d, %d", dev, trunkId);
        }


        /* 1.6. Call cpssDxChTrunkDbDisabledMembersGet with trunkId [2],                                */
        /* non-NULL numOfDisabledMembersPtr, and allocated disabledMembersArray for 8 members.          */
        /* Expected: GT_OK, numOfDisabledMembersPtr [3], and the same disabledMembersArray as written.  */

        numOfMembersWritten = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbDisabledMembersGet: %d, %d", dev, trunkId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numOfDisabledMembers, numOfMembersWritten,
                   "get another numOfDisabledMembersPtr than was set: %d, %d", dev, trunkId);

        if(sortMode == GT_TRUE)
        {
            if(numOfDisabledMembers == numOfMembersWritten)
            {
                /* find the members */
                for(jj = 0 ; jj < numOfMembersWritten; jj++)
                {
                    for(kk = 0 ; kk < numOfDisabledMembers; kk++)
                    {
                        if(disabledMembersArray[kk].port    == membersArrayWritten[jj].port &&
                           disabledMembersArray[kk].hwDevice  == membersArrayWritten[jj].hwDevice)
                        {
                            break;
                        }
                    }

                    if(kk == numOfDisabledMembers)
                    {
                        /* member was not found */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                    }
                }
            }

            if(numOfMembersWritten > 1)
            {
                /* check that members are sorted */
                for(jj = 0 ; jj < (numOfMembersWritten - 1); jj++)
                {
                    if((membersArrayWritten[jj].hwDevice > membersArrayWritten[jj+1].hwDevice) ||
                       ((membersArrayWritten[jj].hwDevice == membersArrayWritten[jj+1].hwDevice) &&
                        (membersArrayWritten[jj].port   > membersArrayWritten[jj+1].port)))
                    {
                        /* member not sorted  */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member not sorted");
                    }
                }
            }

        }
        else
        {
            res = cpssOsMemCmp((const GT_VOID*)disabledMembersArray, (const GT_VOID*)membersArrayWritten,
                                numOfDisabledMembers * sizeof(disabledMembersArray[0]));
            UTF_VERIFY_EQUAL2_STRING_MAC(0, res,
                       "get another disabledMembersArray than was set: %d, %d", dev, trunkId);
        }

        /* 1.7. Check out of range for sum [>CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS = 8]                     */
        /* of enabled and disabled members. Call with trunkId [2], numOfEnabledMembers [6],             */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4, 5],                       */
        /*  numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 6, 7, 8].    */
        /* Expected: NON GT_OK.                                                                         */
        error_numOfEnabledMembers = 6;
        error_enabledMembersArray[0].hwDevice = hwDevNum;
        error_enabledMembersArray[0].port = 4;
        error_enabledMembersArray[1].hwDevice = hwDevNum;
        error_enabledMembersArray[1].port = 1;
        error_enabledMembersArray[2].hwDevice = hwDevNum;
        error_enabledMembersArray[2].port = 5;
        error_enabledMembersArray[3].hwDevice = hwDevNum;
        error_enabledMembersArray[3].port = 3;
        error_enabledMembersArray[4].hwDevice = hwDevNum;
        error_enabledMembersArray[4].port = 0;
        error_enabledMembersArray[5].hwDevice = hwDevNum;
        error_enabledMembersArray[5].port = 2;

        error_numOfDisabledMembers = 3;
        error_disabledMembersArray[0].hwDevice = hwDevNum;
        error_disabledMembersArray[0].port = 7;
        error_disabledMembersArray[1].hwDevice = hwDevNum;
        error_disabledMembersArray[1].port = 8;
        error_disabledMembersArray[2].hwDevice = hwDevNum;
        error_disabledMembersArray[2].port = 6;

        if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* support for CH3 device with 10 XG ports */
            if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
            {
                error_enabledMembersArray[1].port = 16/*1*/;
                error_enabledMembersArray[2].port = 10/*5*/;
                error_enabledMembersArray[3].port = 22/*3*/;
                error_enabledMembersArray[5].port = 12/*2*/;

                error_disabledMembersArray[0].port = 27/*7*/;
                error_disabledMembersArray[1].port = 24/*8*/;
                error_disabledMembersArray[2].port = 26/*6*/;
            }
        }

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        numOfDisabledMembers--;/*2*/
        /* check add/remove members to this trunk */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, &disabledMembersArray[0]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "removed %d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, disabledMembersArray[0].hwDevice, disabledMembersArray[0].port);
        disabledMembersArray[0] = disabledMembersArray[1];
        disabledMembersArray[1] = disabledMembersArray[2];



        enabledMembersArray[5].hwDevice = (hwDevNum + 2) & PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev);
        enabledMembersArray[5].port = 1;
        numOfEnabledMembers++;
        /* support for CH3 device with 10 XG ports */
        if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
        {
            enabledMembersArray[5].port = 12/*1*/;
        }

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &enabledMembersArray[5]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "added %d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, enabledMembersArray[5].hwDevice, enabledMembersArray[5].port);
        numOfDisabledMembers--;/*1*/
        enabledMembersArray[6] = disabledMembersArray[1];
        numOfEnabledMembers++; /*7*/
        /* check enable/disable members to this trunk */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, &enabledMembersArray[6]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "removed %d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, enabledMembersArray[6].hwDevice, enabledMembersArray[6].port);

        disabledMembersArray[1] = enabledMembersArray[0];
        enabledMembersArray[0] = enabledMembersArray[6];
        numOfEnabledMembers--;/*6*/
        numOfDisabledMembers++;/*2*/
        st = cpssDxChTrunkMemberDisable(dev, trunkId, &disabledMembersArray[1]);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "removed %d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, disabledMembersArray[1].hwDevice, disabledMembersArray[1].port);

        numOfMembersWritten = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbEnabledMembersGet: %d, %d", dev, trunkId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numOfEnabledMembers, numOfMembersWritten,
                   "get another numOfEnabledMembersPtr than was set: %d, %d", dev, trunkId);

        if(numOfEnabledMembers == numOfMembersWritten)
        {
            /* find the members */
            for(jj = 0 ; jj < numOfMembersWritten; jj++)
            {
                for(kk = 0 ; kk < numOfEnabledMembers; kk++)
                {
                    if(enabledMembersArray[kk].port    == membersArrayWritten[jj].port &&
                       enabledMembersArray[kk].hwDevice  == membersArrayWritten[jj].hwDevice)
                    {
                        break;
                    }
                }

                if(kk == numOfEnabledMembers)
                {
                    /* member was not found */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                }
            }
        }

        if(sortMode == GT_TRUE)
        {
            if(numOfMembersWritten > 1)
            {
                /* check that members are sorted */
                for(jj = 0 ; jj < (numOfMembersWritten - 1); jj++)
                {
                    if((membersArrayWritten[jj].hwDevice > membersArrayWritten[jj+1].hwDevice) ||
                       ((membersArrayWritten[jj].hwDevice == membersArrayWritten[jj+1].hwDevice) &&
                        (membersArrayWritten[jj].port   > membersArrayWritten[jj+1].port)))
                    {
                        /* member not sorted  */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member not sorted");
                    }
                }
            }

        }

        numOfMembersWritten = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbDisabledMembersGet: %d, %d", dev, trunkId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numOfDisabledMembers, numOfMembersWritten,
                   "get another numOfDisabledMembersPtr than was set: %d, %d", dev, trunkId);

        if(numOfDisabledMembers == numOfMembersWritten)
        {
            /* find the members */
            for(jj = 0 ; jj < numOfMembersWritten; jj++)
            {
                for(kk = 0 ; kk < numOfDisabledMembers; kk++)
                {
                    if(disabledMembersArray[kk].port    == membersArrayWritten[jj].port &&
                       disabledMembersArray[kk].hwDevice  == membersArrayWritten[jj].hwDevice)
                    {
                        break;
                    }
                }

                if(kk == numOfDisabledMembers)
                {
                    /* member was not found */
                    UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                }
            }
        }

        if(sortMode == GT_TRUE)
        {
            if(numOfMembersWritten > 1)
            {
                /* check that members are sorted */
                for(jj = 0 ; jj < (numOfMembersWritten - 1); jj++)
                {
                    if((membersArrayWritten[jj].hwDevice > membersArrayWritten[jj+1].hwDevice) ||
                       ((membersArrayWritten[jj].hwDevice == membersArrayWritten[jj+1].hwDevice) &&
                        (membersArrayWritten[jj].port   > membersArrayWritten[jj+1].port)))
                    {
                        /* member not sorted  */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member not sorted");
                    }
                }
            }

        }


        /* 1.8. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], numOfEnabledMembers [5],               */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],                          */
        /* numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7].     */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        error_numOfEnabledMembers = 5;
        error_enabledMembersArray[0].hwDevice = hwDevNum;
        error_enabledMembersArray[0].port = 0;
        error_enabledMembersArray[1].hwDevice = hwDevNum;
        error_enabledMembersArray[1].port = 1;
        error_enabledMembersArray[2].hwDevice = hwDevNum;
        error_enabledMembersArray[2].port = 3;
        error_enabledMembersArray[3].hwDevice = hwDevNum;
        error_enabledMembersArray[3].port = 4;
        error_enabledMembersArray[4].hwDevice = hwDevNum;
        error_enabledMembersArray[4].port = 2;

        error_numOfDisabledMembers = 2;
        error_disabledMembersArray[0].hwDevice = hwDevNum;
        error_disabledMembersArray[0].port = 5;
        error_disabledMembersArray[1].hwDevice = hwDevNum;
        error_disabledMembersArray[1].port = 6;
        error_disabledMembersArray[2].hwDevice = hwDevNum;
        error_disabledMembersArray[2].port = 7;

        if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* support for CH3 device with 10 XG ports */
            if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
            {
                error_enabledMembersArray[1].port = 16/*1*/;
                error_enabledMembersArray[2].port = 22/*3*/;
                error_enabledMembersArray[4].port = 12/*2*/;

                error_disabledMembersArray[0].port = 25/*5*/;
                error_disabledMembersArray[1].port = 26/*6*/;
                error_disabledMembersArray[2].port = 27/*7*/;
            }
        }

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], numOfEnabledMembers [5],        */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],                          */
        /* numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7].     */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMembersSet(dev, trunkId, error_numOfEnabledMembers, error_enabledMembersArray,
                                     error_numOfDisabledMembers, error_disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.10. Call cpssDxChTrunkMembersSet with trunkId [2], numOfEnabledMembers [0],                */
        /* enabledMembersArray [NULL], numOfDisabledMembers [0], disabledMembersArray [NULL]            */
        /* to remove all members from the trunk.                                                        */
        /* Expected: GT_OK.                                                                             */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        numOfEnabledMembers = 0;
        numOfDisabledMembers = 0;

        st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, NULL,
                                     numOfDisabledMembers, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.11. Call with trunkId [2], numOfEnabledMembers [1],
           enabledMembersArray [NULL], numOfDisabledMembers [0], non-null
           disabledMembersArray. Expected: GT_BAD_PTR.  */

        numOfEnabledMembers = 1;
        numOfDisabledMembers = 0;

        st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, NULL,
                                     numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, numOfEnabledMembers = %d, enabledMembersArray = NULL",
                                     dev, numOfEnabledMembers);

        /* 1.12. Call with trunkId [2], numOfEnabledMembers [0], non-null
           enabledMembersArray, numOfDisabledMembers [1], disabledMembersArray[NULL].
           Expected: GT_BAD_PTR.    */

        numOfEnabledMembers = 0;
        numOfDisabledMembers = 1;

        st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                     numOfDisabledMembers, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, numOfDisabledMembers = %d, disabledMembersArray = NULL",
                                     dev, numOfDisabledMembers);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    numOfEnabledMembers = 5;
    enabledMembersArray[0].hwDevice = 0;
    enabledMembersArray[0].port = 0;
    enabledMembersArray[1].hwDevice = 0;
    enabledMembersArray[1].port = 1;
    enabledMembersArray[2].hwDevice = 0;
    enabledMembersArray[2].port = 2;
    enabledMembersArray[3].hwDevice = 0;
    enabledMembersArray[3].port = 3;
    enabledMembersArray[4].hwDevice = 0;
    enabledMembersArray[4].port = 4;

    numOfDisabledMembers = 2;
    disabledMembersArray[0].hwDevice = 0;
    disabledMembersArray[0].port = 5;
    disabledMembersArray[1].hwDevice = 0;
    disabledMembersArray[1].port = 6;
    disabledMembersArray[2].hwDevice = 0;
    disabledMembersArray[2].port = 7;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                     numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                 numOfDisabledMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberAdd)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.2. Try to add the same trunk member again, accordingly to header the result must be OK. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.3. Try to add already added trunk members to another trunk id. Call with trunk id [3], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
1.4. Try to add invalid device id and legal port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [8], device [PRV_CPSS_MAX_PP_DEVICES_CNS]}. Expected: NON GT_OK.
1.5. Try to add legal device id and invalid port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}. Expected: NON GT_OK.
1.6. Check for NULL handling. Call with trunk id [2], memberPtr [NULL]. Expected: GT_BAD_PTR.
1.7. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr [tested dev, virtual port 8]. Expected: NON GT_OK.
1.8. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL memberPtr [tested dev, virtual port 8]. Expected: NON GT_OK.
1.9. Call cpssDxChTrunkMemberRemove with trunkId [2], memberPtr {port [8], device [devNum]} to remove added member from the trunk. Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].             */
        /* Expected: GT_OK.                                                                             */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.2. Try to add the same trunk member again, accordingly to header the result must be OK.    */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev, virtual port 8].                     */
        /* Expected: GT_OK.                                                                             */
        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.3. Try to add already added trunk members to another trunk id.                             */
        /* Call with trunk id [3], non-NULL memberPtr [tested dev id, virtual port 8].                  */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 1);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                         dev, trunkId, member.hwDevice, member.port);

        /* 1.4. Try to add invalid device id and legal port id as trunk entry member.                   */
        /* Call with trunk id [2], non-NULL memberPtr [invalid dev id, virtual port 8].                 */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        /* 1.5. Try to add legal device id and invalid port id as trunk entry member.                   */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, invalid port].                    */
        /* Expected: NON GT_OK.                                                                         */
        member.hwDevice = hwDevNum;
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        /* 1.6. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].                      */
        /* Expected: GT_BAD_PTR.                                                                        */
        st = cpssDxChTrunkMemberAdd(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /* 1.7. Call with null trunk id [NULL_TRUNK_ID_CNS = 0],                                        */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                             */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.8. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                                 */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                             */
        /* Expected: NON GT_OK.                                                                         */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.9. Call cpssDxChTrunkMemberRemove with trunkId [2],                                        */
        /* memberPtr [tested dev id, virtual port 8] to remove added member from the trunk.             */
        /* Expected: GT_OK                                                                              */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberRemove)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Add member to tested trunk id before call remove function. Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.2. Try to remove invalid device id and legal port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [8], device [PRV_CPSS_MAX_PP_DEVICES_CNS]}. Expected: GT_OK.
1.3. Try to remove legal device id and invalid port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}. Expected: NON GT_OK.
1.4. Try to remove added member. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.5. Try to remove already removed trunk member again, accordingly to header the result must be OK. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.6. Check for NULL handling. Call with trunk id [2], memberPtr [NULL]. Expected: GT_BAD_PTR.
1.7. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr [tested dev, virtual port 8]. Expected: NON GT_OK.
1.8. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL memberPtr [tested dev, virtual port 8]. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Add member to tested trunk id before call remove function.                                      */
        /* Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].   */
        /* Expected: GT_OK.                                                                                     */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.2. Try to remove invalid device id and legal port id as trunk
           entry member. Call with trunk id [2], non-NULL memberPtr {port [8],
           device [PRV_CPSS_MAX_PP_DEVICES_CNS]}. Expected: GT_OK.
        */

        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                     dev, trunkId, member.hwDevice);

        /* 1.3. Try to remove legal device id and invalid port id as trunk entry member.                        */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, invalid port].                            */
        /* Expected: GT_OK.                                                                                 */
        member.hwDevice = hwDevNum;
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                     dev, trunkId, member.port);

        /* 1.4. Try to remove added member.                                                                     */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                          */
        /* Expected: GT_OK.                                                                                     */
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.5. Try to remove already removed trunk member again, accordingly to header the result must be OK.  */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev, virtual port 8].                             */
        /* Expected: GT_OK.                                                                                     */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.6. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].                              */
        /* Expected: GT_BAD_PTR.                                                                                */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /* 1.7. Call with null trunk id [NULL_TRUNK_ID_CNS = 0],                                                */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                                     */
        /* Expected: NON GT_OK.                                                                                 */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.8. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                                         */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                                     */
        /* Expected: NON GT_OK.                                                                                 */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberDisable)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Add member to tested trunk before disable it. Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.2. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.3. Try to disable already disabled trunk member, accordingly to header the result must be OK. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.4. Try to disable invalid device id and legal port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [8], device [PRV_CPSS_MAX_PP_DEVICES_CNS]} Expected: NON GT_OK.
1.5. Try to disable legal device id and invalid port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS], device [devNum]}. Expected: NON GT_OK.
1.6. Remove member from trunk and try to disable it then. Call cpssDxChTrunkMemberRemove with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.7. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
1.8. Check for NULL handling. Call with trunk id [2], memberPtr [NULL]. Expected: GT_BAD_PTR.
1.9. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr [tested dev, virtual port 8]. Expected: NON GT_OK.
1.10. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Add member to tested trunk before disable it.                                                       */
        /* Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].       */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.2. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                         */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.3. Try to disable already disabled trunk member, accordingly to header the result must be OK.          */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                              */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.4. Try to disable invalid device id and legal port id as trunk entry member.                           */
        /* Call with trunk id [2], non-NULL memberPtr [invalid dev id, virtual port 8]                              */
        /* Expected: NON GT_OK.                                                                                     */
        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        /* 1.5. Try to disable legal device id and invalid port id as trunk entry member.                           */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, invalid port].                                */
        /* Expected: NON GT_OK.                                                                                     */
        member.hwDevice = hwDevNum;
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        /* 1.6. Remove member from trunk and try to disable it then.                                                */
        /* Call cpssDxChTrunkMemberRemove with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].    */
        /* Expected: GT_OK.                                                                                         */
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.7. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                         */
        /* Expected: NON GT_OK.                                                                                     */
        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.8. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].                                  */
        /* Expected: GT_BAD_PTR.                                                                                    */
        st = cpssDxChTrunkMemberDisable(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /* 1.9. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr [tested dev, virtual port 8].   */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.10. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                                            */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                                         */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberDisable(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberEnable)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Add member to tested trunk before enable it. Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.2. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.3. Try to enable already enabled trunk member, accordingly to header the result must be OK. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.4. Try to enable invalid device id and legal port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [8], device [PRV_CPSS_MAX_PP_DEVICES_CNS]} Expected: NON GT_OK.
1.5. Try to enable legal device id and invalid port id as trunk entry member. Call with trunk id [2], non-NULL memberPtr { UTF_CPSS_PP_MAX_PORTS_NUM_CNS, device [devNum]}. Expected: NON GT_OK.
1.6. Remove member from trunk and try to enable it then. Call cpssDxChTrunkMemberRemove with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.7. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
1.8. Check for NULL handling. Call with trunk id [2], memberPtr [NULL]. Expected: GT_BAD_PTR.
1.9. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
1.10. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL memberPtr {port [8], device [devNum]}. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Add member to tested trunk before enable it.                                                        */
        /* Call cpssDxChTrunkMemberAdd with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].       */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.2. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                         */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.3. Try to enable already enabled trunk member, accordingly to header the result must be OK.            */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                              */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.4. Try to enable invalid device id and legal port id as trunk entry member.                            */
        /* Call with trunk id [2], non-NULL memberPtr [invalid dev id, virtual port 8]                              */
        /* Expected: NON GT_OK.                                                                                     */
        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        /* 1.5. Try to enable legal device id and invalid port id as trunk entry member.                            */
        /* Call with trunk id [2], non-NULL memberPtr [tested dev id, invalid port].                                */
        /* Expected: NON GT_OK.                                                                                     */
        member.hwDevice = hwDevNum;
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        /* 1.6. Remove member from trunk and try to enable it then.                                                 */
        /* Call cpssDxChTrunkMemberRemove with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].    */
        /* Expected: GT_OK.                                                                                         */
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.7. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].                         */
        /* Expected: NON GT_OK.                                                                                     */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, member.hwDevice, member.port);

        /* 1.8. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].                                  */
        /* Expected: GT_BAD_PTR.                                                                                    */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /* 1.9. Call with null trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL memberPtr [tested dev, virtual port 8].   */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.10. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                                            */
        /* non-NULL memberPtr [tested dev, virtual port 8].                                                         */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberEnable(dev, trunkId, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode ? (in ExMx - High)

GT_STATUS cpssDxChTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkNonTrunkPortsAdd)
{
/*
ITERATE_DEVICES
1.1. Call with trunk id [2], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00] to add first eight virtual ports to the trunk's non-trunk entry. Expected: GT_OK.
1.2. Check for NULL handling. Call with trunk id [2], nonTrunkPortsBmpPtr [NULL]. Expected: GT_BAD_PTR.
1.3. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00]. Expected: GT_OK.
1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00]. Expected: NON GT_OK.
1.5. Call cpssDxChTrunkNonTrunkPortsRemove with trunkId [2], nonTrunkPortsBmp [0xFF, 0x00] to remove all non trunk members from the trunk  Expected: GT_OK.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_TRUNK_ID         trunkId;
    CPSS_PORTS_BMP_STC  nonTrunkPortsBmp;
    GT_U8               port;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nonTrunkPortsBmp, sizeof(nonTrunkPortsBmp));
    for (port = 0; port < CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS; port++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&nonTrunkPortsBmp, port);
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunk id [2], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00] to add first eight                */
        /* virtual ports to the trunk's non-trunk entry.                                                            */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, nonTrunkPortsBmp.ports[0], nonTrunkPortsBmp.ports[1]);

        /* 1.2. Check for NULL handling. Call with trunk id [2], nonTrunkPortsBmpPtr [NULL].                        */
        /* Expected: GT_BAD_PTR.                                                                                  */
        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsBmpPtr = NULL", dev, trunkId);

        /* 1.3. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0],                      */
        /* non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00].                                                               */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00].  */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.5. Call cpssDxChTrunkNonTrunkPortsRemove with trunkId [2], nonTrunkPortsBmp [0xFF, 0x00]               */
        /* to remove all non trunk members from the trunk                                                           */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkNonTrunkPortsRemove: %d, %d, %d, %d",
                                     dev, trunkId, nonTrunkPortsBmp.ports[0], nonTrunkPortsBmp.ports[1]);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode ? (in ExMx - High)

GT_STATUS cpssDxChTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkNonTrunkPortsRemove)
{
/*
ITERATE_DEVICES
1.1. Add first eight virtual ports to the trunk's non-trunk entry. Call cpssDxChTrunkNonTrunkPortsAdd with trunkId [2], nonTrunkPortsBmpPtr [0xFF, 0x00]. Expected: GT_OK.
1.2. Call with trunk id [2], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00] to remove first eight virtual ports from the trunk's non-trunk entry. Expected: GT_OK.
1.3. Check for NULL handling. Call with trunk id [2], nonTrunkPortsBmpPtr [NULL]. Expected: GT_BAD_PTR.
1.4. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00]. Expected: GT_OK.
1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00]. Expected: NON GT_OK.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_TRUNK_ID         trunkId;
    CPSS_PORTS_BMP_STC  nonTrunkPortsBmp;
    GT_U8               port;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nonTrunkPortsBmp, sizeof(nonTrunkPortsBmp));
    for (port = 0; port < CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS; port++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&nonTrunkPortsBmp, port);
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Add first eight virtual ports to the trunk's non-trunk entry.                                       */
        /* Call cpssDxChTrunkNonTrunkPortsAdd with trunkId [2], nonTrunkPortsBmpPtr [0xFF, 0x00].                   */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsAdd(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkNonTrunkPortsAdd: %d, %d, %d, %d",
                                     dev, trunkId, nonTrunkPortsBmp.ports[0], nonTrunkPortsBmp.ports[1]);

        /* 1.2. Call with trunk id [2], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00] to remove first eight             */
        /* virtual ports to the trunk's non-trunk entry.                                                            */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, nonTrunkPortsBmp.ports[0], nonTrunkPortsBmp.ports[1]);

        /* 1.3. Check for NULL handling. Call with trunk id [2], nonTrunkPortsBmpPtr [NULL].                        */
        /* Expected: GT_BAD_PTR.                                                                                  */
        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsBmpPtr = NULL", dev, trunkId);

        /* 1.4. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0],                      */
        /* non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00].                                                               */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsBmpPtr [0xFF, 0x00].  */
        /* Expected: NON GT_OK.                                                                                     */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkNonTrunkPortsRemove(dev, trunkId, &nonTrunkPortsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode

GT_STATUS cpssDxChTrunkPortTrunkIdSet
(
    IN GT_U8            devNum,
    IN GT_U8            portNum,
    IN GT_BOOL          memberOfTrunk,
    IN GT_TRUNK_ID      trunkId
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkPortTrunkIdSet)
{
/*
ITERATE_DEVICES_VIRT_PORTS (DxChx)
1.1.1. Call with memberOfTrunk [GT_TRUE], trunkId [2]. Expected: GT_OK.
1.1.2. Call cpssDxChTrunkPortTrunkIdGet with non-NULL memberOfTrunkPtr, and non-NULL trunkIdPtr. Expected: GT_OK, memberOfTrunkPtr [GT_TRUE] and trunkIdPtr [2].
1.1.3. Call with memberOfTrunk [GT_FALSE], trunkid [2] (but trunkid will be set to 0). Expected: GT_OK.
1.1.4. Call cpssDxChTrunkPortTrunkIdGet with non-NULL memberOfTrunkPtr, and non-NULL trunkIdPtr. Expected: GT_OK, memberOfTrunkPtr [GT_FALSE] and trunkIdPtr [0].
1.1.5. Call with null trunk id [NULL_TRUNK_ID_CNS = 0] and memberOfTrunk [GT_TRUE]. Expected: NON GT_OK.
1.1.6. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128] memberOfTrunk [GT_TRUE]. Expected: NON GT_OK.
1.1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128] (this value relevant only when memberOfTrunk = GT_TRUE) memberOfTrunk [GT_FALSE]. Expected: GT_OK.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_BOOL     memberOfTrunk;
    GT_TRUNK_ID trunkId;
    GT_BOOL         memberOfTrunkWritten;
    GT_TRUNK_ID     trunkIdWritten;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with memberOfTrunk [GT_TRUE], trunkId [2].
               Expected: GT_OK. */

            trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            memberOfTrunk = GT_TRUE;

            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, memberOfTrunk, trunkId);

            /* 1.1.2. Call cpssDxChTrunkPortTrunkIdGet with non-NULL
               memberOfTrunkPtr, and non-NULL trunkIdPtr. Expected: GT_OK,
               memberOfTrunkPtr [GT_TRUE] and trunkIdPtr [2].   */

            st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunkWritten, &trunkIdWritten);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkPortTrunkIdGet: %d, %d", dev, port);

            /* Verifuing values */
            UTF_VERIFY_EQUAL2_STRING_MAC(memberOfTrunk, memberOfTrunkWritten,
                   "get another memberOfTrunkPtr than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(trunkId, trunkIdWritten,
                   "get another trunkIdPtr than was set: %d, %d", dev, port);

            /* 1.1.3. Call with memberOfTrunk [GT_FALSE], trunkid [2]
               (but trunkid will be set to 0). Expected: GT_OK. */

            trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            memberOfTrunk = GT_FALSE;

            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, memberOfTrunk, trunkId);

            trunkId = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            /* 1.1.4. Call cpssDxChTrunkPortTrunkIdGet with non-NULL
               memberOfTrunkPtr, and non-NULL trunkIdPtr. Expected: GT_OK,
               memberOfTrunkPtr [GT_FALSE] and trunkIdPtr [0].  */

            st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunkWritten, &trunkIdWritten);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkPortTrunkIdGet: %d, %d", dev, port);

            /* Verifuing values */
            UTF_VERIFY_EQUAL2_STRING_MAC(memberOfTrunk, memberOfTrunkWritten,
                   "get another memberOfTrunkPtr than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(trunkId, trunkIdWritten,
                   "get another trunkIdPtr than was set: %d, %d", dev, port);

            /* 1.1.5. Call with null trunk id [NULL_TRUNK_ID_CNS = 0] and
               memberOfTrunk [GT_TRUE]. Expected: NON GT_OK.    */

            trunkId = TRUNK_NULL_TRUNK_ID_CNS;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            memberOfTrunk = GT_TRUE;

            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, memberOfTrunk, trunkId);

            /* 1.1.6. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]
               memberOfTrunk [GT_TRUE]. Expected: NON GT_OK.    */

            trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, memberOfTrunk, trunkId);

            /* 1.1.7. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]
               (this value relevant only when memberOfTrunk = GT_TRUE)
                memberOfTrunk [GT_FALSE]. Expected: GT_OK.  */
            memberOfTrunk = GT_FALSE;

            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, memberOfTrunk, trunkId);
        }

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        memberOfTrunk = GT_TRUE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
        if(GT_FALSE == TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            /* cpu port like any other port */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    memberOfTrunk = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkPortTrunkIdSet(dev, port, memberOfTrunk, trunkId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
All Modes

GT_STATUS cpssDxChTrunkPortTrunkIdGet
(
    IN GT_U8            devNum,
    IN GT_U8            portNum,
    OUT GT_BOOL         *memberOfTrunkPtr,
    OUT GT_TRUNK_ID     *trunkIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkPortTrunkIdGet)
{
/*
ITERATE_DEVICES_VIRT_PORTS (DxChx)
1.1.1. Call with non-NULL memberOfTrunkPtr, and non-NULL trunkIdPtr. Expected: GT_OK.
1.1.2. Call with memberOfTrunkPtr [NULL], and non-NULL trunkIdPtr. Expected: GT_BAD_PTR.
1.1.3. Call with non-NULL memberOfTrunkPtr, and trunkIdPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_PHYSICAL_PORT_NUM     port;
    GT_BOOL         memberOfTrunk;
    GT_TRUNK_ID     trunkId;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-NULL memberOfTrunkPtr,
               and non-NULL trunkIdPtr. Expected: GT_OK.    */

            st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with memberOfTrunkPtr [NULL], and non-NULL
               trunkIdPtr. Expected: GT_BAD_PTR.    */

            st = cpssDxChTrunkPortTrunkIdGet(dev, port, NULL, &trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberOfTrunkPtr = NULL", dev, port);

            /* 1.1.3. Call with non-NULL memberOfTrunkPtr, and
               trunkIdPtr [NULL]. Expected: GT_BAD_PTR. */

            st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, trunkIdPtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
        if(GT_FALSE == TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            /* cpu port like any other port */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkPortTrunkIdGet(dev, port, &memberOfTrunk, &trunkId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode

GT_STATUS cpssDxChTrunkTableEntrySet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkTableEntrySet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with trunkId [2], numMembers [3], membersArray [0 / 1 / 2] {port [0 / 1 / 2], device [devNum]}. Expected: GT_OK.
1.2. Call with cpssDxChTrunkTableEntryGet with trunkId [2], non-NULL numMembersPtr, membersArray. Expected: GT_OK, numMembersPtr [3], and the same membersArray as was set.
1.3. Call with out of range numMembers [9]. Expected: NON GT_OK.
1.4. Call with null trunk id [NULL_TRUNK_ID_CNS = 0]. Expected: NON GT_OK.
1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]. Expected: NON GT_OK.
1.6. Call with trunkId [2], numMembers [3], membersArray [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_U32                  numMembers;
    static CPSS_TRUNK_MEMBER_STC   membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    GT_32                   res;
    GT_U32                  numMembersWritten;
    static CPSS_TRUNK_MEMBER_STC   membersArrayWritten[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], numMembers [3], membersArray [tested dev id, virtual port 0, 1, 2].      */
        /* Expected: GT_OK.                                                                                     */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        numMembers = 3;
        membersArray[0].hwDevice = hwDevNum;
        membersArray[0].port = 0;
        membersArray[1].hwDevice = hwDevNum;
        membersArray[1].port = 1;
        membersArray[2].hwDevice = hwDevNum;
        membersArray[2].port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev)-1;

        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, numMembers);

        /* 1.2. Call with cpssDxChTrunkTableEntryGet with trunkId [2], non-NULL numMembersPtr, membersArray.    */
        /* Expected: GT_OK, numMembersPtr [3], and the same membersArray as was set.                            */
        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembersWritten, membersArrayWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet: %d, %d", dev, trunkId);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(numMembers, numMembersWritten,
                   "get another numMembersPtr than was set: %d, %d", dev, trunkId);

        res = cpssOsMemCmp((const GT_VOID*)membersArray, (const GT_VOID*)membersArrayWritten,
                            numMembers*sizeof(membersArray[0]));
        UTF_VERIFY_EQUAL2_STRING_MAC(0, res,
                   "get another membersArray than was set: %d, %d", dev, trunkId);

        /* 1.3. Call with out of range numMembers [9].                                                          */
        /* Expected: NON GT_OK.                                                                                 */
        numMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS + 1;

        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, numMembers);

        /* 1.4. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].                                                */
        /* Expected: NON GT_OK.                                                                                 */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        numMembers = 3;

        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].                                         */
        /* Expected: NON GT_OK.                                                                                 */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        numMembers = 3;

        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.6. Call with runkId [2], numMembers [3], membersArray [NULL].
           Expected: GT_BAD_PTR. */

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, %d, membersArray = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    numMembers = 3;
    membersArray[0].hwDevice = dev;
    membersArray[0].port = 0;
    membersArray[1].hwDevice = dev;
    membersArray[1].port = 1;
    membersArray[2].hwDevice = dev;
    membersArray[2].port = 2;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
All Modes

GT_STATUS cpssDxChTrunkTableEntryGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT GT_U32                  *numMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   membersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkTableEntryGet)
{
/*
ITERATE_DEVICES
1.1. Call with trunkId [2], not-NULL numMembers, and allocated membersArray. Expected: GT_OK.
1.2. Call with numMembersPtr [NULL]. Expected: GT_BAD_PTR.
1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0]. Expected: NON GT_OK.
1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_U32                  numMembers;
    static CPSS_TRUNK_MEMBER_STC   membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], not-NULL numMembers, and allocated membersArray.         */
        /* Expected: GT_OK.                                                                     */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembers, membersArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.2. Call with numMembersPtr [NULL].                                                 */
        /* Expected: GT_BAD_PTR.                                                                */
        st = cpssDxChTrunkTableEntryGet(dev, trunkId, NULL, membersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, numMembersPtr = NULL", dev, trunkId);

        /* 1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].                                */
        /* Expected: NON GT_OK.                                                                 */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].                         */
        /* Expected: NON GT_OK.                                                                 */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembers, membersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembers, membersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode

GT_STATUS cpssDxChTrunkNonTrunkPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkNonTrunkPortsEntrySet)
{
/*
ITERATE_DEVICES
1.1. Call with trunkId [2], non-NULL nonTrunkPortsPtr [0xFF, 0x00]. Expected: GT_OK.
1.2. Call cpssDxChTrunkNonTrunkPortsEntryGet with trunkId [2], non-NULL nonTrunkPortsPtr. Expected: GT_OK and the same port bitmap as was written.
1.3. Check for NULL handling. Call with trunkId [2], nonTrunkPortsPtr [NULL]. Expected: GT_BAD_PTR.
1.4. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL nonTrunkPortsPtr [0xFF, 0x00]. Expected: GT_OK.
1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsPtr [0xFF, 0x00]. Expected: NON GT_OK.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_TRUNK_ID         trunkId;
    CPSS_PORTS_BMP_STC  nonTrunkPorts;
    GT_U8               port;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&nonTrunkPorts, sizeof(nonTrunkPorts));
    for (port = 0; port < CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS; port++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&nonTrunkPorts, port);
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        CPSS_PORTS_BMP_STC  nonTrunkPortsWritten;

        /* 1.1. Call with trunkId [2], non-NULL nonTrunkPortsPtr [0xFF, 0x00].                              */
        /* Expected: GT_OK.                                                                                 */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, trunkId, nonTrunkPorts.ports[0], nonTrunkPorts.ports[1]);

        /* 1.2. Call cpssDxChTrunkNonTrunkPortsEntryGet with trunkId [2], non-NULL nonTrunkPortsPtr.        */
        /* Expected: GT_OK and the same port bitmap as was written.                                         */
        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPortsWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkNonTrunkPortsEntryGet: %d, %d", dev, trunkId);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(1 , CPSS_PORTS_BMP_ARE_EQUAL_MAC(&nonTrunkPorts,&nonTrunkPortsWritten),
                   "get another nonTrunkPortsPtr->ports[x] than was set: %d, %d", dev, trunkId);

        /* 1.3. Check for NULL handling. Call with trunkId [2], nonTrunkPortsPtr [NULL].                    */
        /* Expected: GT_BAD_PTR.                                                                            */
        st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsPtr = NULL", dev, trunkId);

        /* 1.4. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0],              */
        /* non-NULL nonTrunkPortsPtr [0xFF, 0x00].                                                          */
        /* Expected: GT_OK.                                                                                 */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.5. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                                     */
        /* non-NULL nonTrunkPortsPtr [0xFF, 0x00].                                                          */
        /* Expected: NON GT_OK.                                                                             */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkNonTrunkPortsEntrySet(dev, trunkId, &nonTrunkPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
All Modes

GT_STATUS cpssDxChTrunkNonTrunkPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    OUT  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkNonTrunkPortsEntryGet)
{
/*
ITERATE_DEVICES
1.1. Call with trunkId [2], non-NULL nonTrunkPortsPtr. Expected: GT_OK.
1.2. Check for NULL handling. Call with trunk id [2], nonTrunkPortsPtr [NULL]. Expected: GT_BAD_PTR.
1.3. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0], non-NULL nonTrunkPortsPtr [0xFF, 0x00]. Expected: GT_OK.
1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128], non-NULL nonTrunkPortsPtr [0xFF, 0x00]. Expected: NON GT_OK.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_TRUNK_ID         trunkId;
    CPSS_PORTS_BMP_STC  nonTrunkPorts;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], non-NULL nonTrunkPortsPtr.                               */
        /* Expected: GT_OK.                                                                     */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.2. Check for NULL handling. Call with trunk id [2], nonTrunkPortsPtr [NULL].       */
        /* Expected: GT_BAD_PTR.                                                                */
        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, nonTrunkPortsPtr = NULL", st, dev);

        /* 1.3. This API can accept trunk id zero. Call with trunk id [NULL_TRUNK_ID_CNS = 0],  */
        /* non-NULL nonTrunkPortsPtr [0xFF, 0x00].                                              */
        /* Expected: GT_OK.                                                                     */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],                         */
        /* non-NULL nonTrunkPortsPtr [0xFF, 0x00].                                              */
        /* Expected: NON GT_OK.                                                                 */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkNonTrunkPortsEntryGet(dev, trunkId, &nonTrunkPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
Low-Level Mode

GT_STATUS cpssDxChTrunkDesignatedPortsEntrySet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDesignatedPortsEntrySet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with entryIndex [3], non-NULL designatedPortsPtr [0xFF, 0x00]. Expected: GT_OK.
1.2. Call cpssDxChTrunkDesignatedPortsEntryGet with entryIndex [3], non-NULL designatedPortsPtr. Expected: GT_OK and the same port bitmap as was written.
1.3. Call with out of range entryIndex [8], non-NULL designatedPortsPtr [0xFF, 0x00]. Expected: NON GT_OK.
1.4. Check for NULL handling. Call with entryIndex [3], designatedPortsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              entryIndex;
    CPSS_PORTS_BMP_STC  designatedPorts;
    GT_U8               port;
    CPSS_PORTS_BMP_STC  designatedPortsWritten;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&designatedPorts, sizeof(designatedPorts));
    for (port = 0; port < CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS; port++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&designatedPorts, port);
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with entryIndex [3], non-NULL designatedPortsPtr [0xFF, 0x00].                             */
        /* Expected: GT_OK.                                                                                     */
        entryIndex = 3;

        st = cpssDxChTrunkDesignatedPortsEntrySet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, entryIndex, designatedPorts.ports[0], designatedPorts.ports[1]);

        /* 1.2. Call cpssDxChTrunkDesignatedPortsEntryGet with entryIndex [3], non-NULL designatedPortsPtr.     */
        /* Expected: GT_OK and the same port bitmap as was written.                                             */
        st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, &designatedPortsWritten);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkDesignatedPortsEntryGet: %d, %d", dev, entryIndex);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(1 , CPSS_PORTS_BMP_ARE_EQUAL_MAC(&designatedPorts,&designatedPortsWritten),
                   "get another designatedPorts->ports[x] than was set: %d", dev);

        /* 1.3. Call with out of range entryIndex [8], non-NULL designatedPortsPtr [0xFF, 0x00].               */
        /* Expected: NON GT_OK.                                                                                 */
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvUtfDeviceFamilyGet: %d", dev);
            if(CPSS_PP_FAMILY_DXCH_XCAT3_E == devFamily)
            {
                entryIndex = 16;
            }
            else
            {
                entryIndex = 8;
            }
        }
        else if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            entryIndex = 128;/*split UC,MC table (each 64 entries)*/
        }
        else
        {
            entryIndex = 64;
        }

        st = cpssDxChTrunkDesignatedPortsEntrySet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        /* 1.4. Check for NULL handling. Call with entryIndex [3], designatedPortsPtr [NULL].                   */
        /* Expected: GT_BAD_PTR.                                                                                */
        st = cpssDxChTrunkDesignatedPortsEntrySet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, designatedPortsPtr = NULL", dev);
    }

    entryIndex = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDesignatedPortsEntrySet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDesignatedPortsEntrySet(dev, entryIndex, &designatedPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
All Modes

GT_STATUS cpssDxChTrunkDesignatedPortsEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDesignatedPortsEntryGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with entryIndex [3], non-NULL designatedPortsPtr. Expected: GT_OK.
1.2. Call with out of range entryIndex [8], non-NULL designatedPortsPtr. Expected: NON GT_OK.
1.3. Check for NULL handling. Call with entryIndex [3], designatedPortsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              entryIndex;
    CPSS_PORTS_BMP_STC  designatedPorts;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with entryIndex [3], non-NULL designatedPortsPtr.                          */
        /* Expected: GT_OK.                                                                     */
        entryIndex = 3;

        st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        /* 1.2. Call with out of range entryIndex [8], non-NULL designatedPortsPtr.            */
        /* Expected: NON GT_OK.                                                                 */
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
        {
            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvUtfDeviceFamilyGet: %d", dev);
            if(CPSS_PP_FAMILY_DXCH_XCAT3_E == devFamily)
            {
                entryIndex = 16;
            }
            else
            {
                entryIndex = 8;
            }
        }
        else if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            entryIndex = 128;/*split UC,MC table (each 64 entries)*/
        }
        else
        {
            entryIndex = 64;
        }

        st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, entryIndex);

        /* 1.3. Check for NULL handling. Call with entryIndex [3], designatedPortsPtr [NULL].   */
        /* Expected: GT_BAD_PTR.                                                                */
        st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, designatedPortsPtr = NULL", dev);
    }

    entryIndex = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, &designatedPorts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDesignatedPortsEntryGet(dev, entryIndex, &designatedPorts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbEnabledMembersGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with trunkId [2], non-NULL numOfEnabledMembers, and allocated enabledMembersArray for 8 members. Expected: GT_OK.
1.2. Check for NULL handling. Call with trunkId [2], numOfEnabledMembers [NULL], and allocated enabledMembersArray. Expected: GT_BAD_PTR.
1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0]. Expected: NON GT_OK.
1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_U32                  numOfEnabledMembers = 0;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)enabledMembersArray, sizeof(enabledMembersArray));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], non-NULL numOfEnabledMembers,                            */
        /* and allocated enabledMembersArray for 8 members.                                     */
        /* Expected: GT_OK.                                                                     */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        numOfEnabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.2. Check for NULL handling. Call with trunkId [2], numOfEnabledMembers [NULL],     */
        /* and allocated enabledMembersArray.                                                   */
        /* Expected: GT_BAD_PTR.                                                                */
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, NULL, enabledMembersArray);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfEnabledMembers = NULL", dev);

        /* 1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].                                */
        /* Expected: NON GT_OK.                                                                 */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].                         */
        /* Expected: NON GT_OK.                                                                 */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbDisabledMembersGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call with trunkId [2], non-NULL numOfDisabledMembersPtr, and allocated disabledMembersArray for 8 members. Expected: GT_OK.
1.2. Check for NULL handling. Call with trunkId [2], numOfDisabledMembersPtr [NULL], and allocated disabledMembersArray. Expected: GT_BAD_PTR.
1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0]. Expected: NON GT_OK.
1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128]. Expected: NON GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_U32                  numOfDisabledMembers;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];


    cpssOsBzero((GT_VOID*)disabledMembersArray, sizeof(disabledMembersArray));

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], non-NULL numOfDisabledMembersPtr,                            */
        /* and allocated disabledMembersArray for 8 members.                                        */
        /* Expected: GT_OK.                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        numOfDisabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.2. Check for NULL handling. Call with trunkId [2], numOfDisabledMembersPtr [NULL],     */
        /* and allocated disabledMembersArray.                                                      */
        /* Expected: GT_BAD_PTR.                                                                    */
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, NULL, disabledMembersArray);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, numOfDisabledMembersPtr = NULL", dev, trunkId);

        /* 1.3. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].                                    */
        /* Expected: NON GT_OK.                                                                     */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* 1.4. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].                             */
        /* Expected: NON GT_OK.                                                                     */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbIsMemberOfTrunk)
{
/*
ITERATE_DEVICES (DxChx)
1.1. At first add some member to trunk. Call cpssDxChTrunkMemberAdd with trunkId [2], and non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.2. Call with non-NULL memberPtr {port [8], device [devNum]}., non-NULL trunkIdPtr. Expected: GT_OK and trunkIdPtr [2].
1.3. Accordingly to header trunkIdPtr may be NULL. Call with non-NULL memberPtr {port [8], device [devNum]}., trunkIdPtr [NULL]. Expected: GT_OK.
1.4. Remove member from trunk. Call cpssDxChTrunkMemberRemove with trunkId [2], non-NULL memberPtr {port [8], device [devNum]}. Expected: GT_OK.
1.5. Try to find already removed member. Call with non-NULL memberPtr {port [8], device [devNum]}, non-NULL trunkIdPtr. Expected: NOT GT_OK.
1.6. Check for NULL handling. Call with memberPtr [NULL], non-NULL trunkIdPtr. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_TRUNK_ID             trunkId;
    GT_HW_DEV_NUM           hwDevNum;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GT_TRUNK_ID     trunkIdWritten;

        /* 1.1. At first add some member to trunk.                                                                  */
        /* Call cpssDxChTrunkMemberAdd with trunkId [2], and non-NULL memberPtr [tested dev id, virtual port 8].    */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.2. Call with non-NULL memberPtr [tested dev id, virtual port 8], non-NULL trunkIdPtr.                  */
        /* Expected: GT_OK and trunkIdPtr [2].                                                                      */
        trunkIdWritten = 0;

        st = cpssDxChTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdWritten);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, member.hwDevice, member.port);

        UTF_VERIFY_EQUAL1_STRING_MAC(trunkId, trunkIdWritten,
                   "get another trunkIdPtr than was set: %d", dev);

        /* 1.3. Accordingly to header trunkIdPtr may be NULL.                                                       */
        /* Call with non-NULL memberPtr [tested dev id, virtual port 8], trunkIdPtr [NULL].                         */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkDbIsMemberOfTrunk(dev, &member, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, trunkIdPtr = NULL", dev);

        /* 1.4. Remove member from trunk.                                                                           */
        /* Call cpssDxChTrunkMemberRemove with trunkId [2], non-NULL memberPtr [tested dev id, virtual port 8].     */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* 1.5. Try to find already removed member.                                                                 */
        /* Call with non-NULL memberPtr [tested dev id, virtual port 8], non-NULL trunkIdPtr.                       */
        /* Expected: NON GT_OK.                                                                                     */
        trunkIdWritten = 0;

        st = cpssDxChTrunkDbIsMemberOfTrunk(dev, &member, &trunkIdWritten);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, member.hwDevice, member.port);

        /* 1.6. Check for NULL handling. Call with memberPtr [NULL], non-NULL trunkIdPtr.                           */
        /* Expected: GT_BAD_PTR.                                                                                    */
        st = cpssDxChTrunkDbIsMemberOfTrunk(dev, NULL, &trunkIdWritten);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, memberPtr = NULL", dev);
    }

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbIsMemberOfTrunk(dev, &member, &trunkId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbIsMemberOfTrunk(dev, &member, &trunkId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
High-Level Mode

GT_STATUS cpssDxChTrunkDbTrunkTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT CPSS_TRUNK_TYPE_ENT     *typePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbTrunkTypeGet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_TRUNK_MEMBER_STC   member;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_TYPE_ENT     trunkType;
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_PORTS_BMP_STC      portsBmp;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* 1.1. At first add some member to trunk.                                                                  */
        /* Call cpssDxChTrunkMemberAdd with trunkId [2], and non-NULL memberPtr [tested dev id, virtual port 8].    */
        /* Expected: GT_OK.                                                                                         */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(trunkType == CPSS_TRUNK_TYPE_CASCADE_E)
        {
            st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsSet: %d, %d ,NULL ",
                                         dev, trunkId );
        }
        else
        {
            /* this API should cause the trunk to be empty */
            st = cpssDxChTrunkMembersSet(dev, trunkId, 0,NULL,0,NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet: %d, %d ,0,NULL,0,NULL ",
                                         dev, trunkId );
        }

        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* trunk must be 'FREE' now */
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_TRUNK_TYPE_FREE_E, trunkType);

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkMemberAdd(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberAdd: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* trunk must be now 'Regular trunk'*/
        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_TRUNK_TYPE_REGULAR_E, trunkType);

        /* 1.4. Remove member from trunk.                                                                           */
        /* Call cpssDxChTrunkMemberRemove with trunkId [2], non-NULL memberPtr [tested dev id, virtual port 8].     */
        /* Expected: GT_OK.                                                                                         */
        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* trunk must be now 'free' trunk */
        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_TRUNK_TYPE_FREE_E, trunkType);

        /* set cascade trunk */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,TRUNK_TESTED_VIRT_PORT_ID_CNS(dev));

        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsSet(dev,trunkId, &portsBmp);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* trunk must be 'cascade' now */
        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_TRUNK_TYPE_CASCADE_E, trunkType);

        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* trunk must be 'FREE' now */
        st = cpssDxChTrunkDbTrunkTypeGet(dev,trunkId,&trunkType);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_TRUNK_TYPE_FREE_E, trunkType);

        /* Call with trunkIdPtr [NULL].                         */
        /* Expected: GT_BAD_PTR.                                                                                         */
        st = cpssDxChTrunkDbTrunkTypeGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbTrunkTypeGet(dev, trunkId, &trunkType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbTrunkTypeGet(dev, trunkId, &trunkType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* --- different from ExMx */

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashDesignatedTableModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL useEntireTable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashDesignatedTableModeSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with mode [see CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT]. Expected: GT_OK.
1.2. Call cpssDxChTrunkHashDesignatedTableModeGet with non-NULL modePtr. Expected: GT_OK and same value as written.
*/
    GT_STATUS st              = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT     modes[] ={
        CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E,
        CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E,
        CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E};
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT     modeRet,tmpMode = 0;
    GT_U32  numOfModes = sizeof(modes)/sizeof(modes[0]);
    GT_U32  ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(ii = 0 ; ii < numOfModes ; ii++)
        {
            /* 1.1. Call function with valid mode
               Expected: GT_OK.   */

            st = cpssDxChTrunkHashDesignatedTableModeSet(dev, modes[ii]);
            if(modes[ii] == CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E)
            {
                if(0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(dev))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, modes[ii]);
                    continue;
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, modes[ii]);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, modes[ii]);
            }

            /* 1.2. Call cpssDxChTrunkHashDesignatedTableModeGet with non-NULL
               useEntireTablePtr. Expected: GT_OK and same value as written.    */

            st = cpssDxChTrunkHashDesignatedTableModeGet(dev, &modeRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashDesignatedTableModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL3_STRING_MAC(modeRet, modes[ii] ,
                       "in dev [%d] get another modeRet[%d] than was set[%d]",
                       dev ,modeRet, modes[ii]);
        }

        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashDesignatedTableModeSet(dev, tmpMode),
                                                    tmpMode);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    tmpMode = CPSS_DXCH_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashDesignatedTableModeSet(dev, tmpMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashDesignatedTableModeSet(dev, tmpMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashDesignatedTableModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *useEntireTablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashDesignatedTableModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL useEntireTablePtr. Expected: GT_OK.
1.2. Call function with useEntireTablePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_TRUNK_DESIGNATED_TABLE_MODE_ENT     mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL useEntireTablePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashDesignatedTableModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with useEntireTablePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashDesignatedTableModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, useEntireTablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashDesignatedTableModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashDesignatedTableModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashGlobalModeSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashGlobalModeSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with hashMode [CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E / CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E]. Expected: GT_OK.
1.2. Call cpssDxChTrunkHashGlobalModeGet with non-NULL hashModePtr. Expected: GT_OK and same value as written.
1.3. Call function with hashMode [wrong enum values]. Expected: GT_BAD_PARAM.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashModes[] = {
                        CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E,
                        CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E,
                        CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E};
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode = CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E;
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashModeRet = CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E;
    GT_U32  numOfModes = sizeof(hashModes) / sizeof(hashModes[0]);
    GT_U32  ii;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with hashMode [CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E
           / CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E]. Expected: GT_OK.  */

        for(ii = 0 ; ii < numOfModes; ii++)
        {
            hashMode = hashModes[ii];

            st = cpssDxChTrunkHashGlobalModeSet(dev, hashMode);

            if(hashMode == CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E)
            {
                if(PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.trunkCrcHashSupported == GT_FALSE)
                {
                    /* the device nit support this option */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, hashMode);
                    continue;
                }
            }

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashMode);

            /* 1.2. Call cpssDxChTrunkHashGlobalModeGet with non-NULL
               hashModePtr. Expected: GT_OK and same value as written.  */

            st = cpssDxChTrunkHashGlobalModeGet(dev, &hashModeRet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashGlobalModeGet: %d", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(hashMode, hashModeRet,
                       "get another hashMode than was set: %d", dev);
        }

        /*
           1.3. Call function with hashMode [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashGlobalModeSet
                            (dev, hashMode),
                            hashMode);
    }

    hashMode = CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashGlobalModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashGlobalModeSet(dev, hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashGlobalModeGet
(
    IN GT_U8    devNum,
    OUT CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashGlobalModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL hashModePtr. Expected: GT_OK.
1.2. Call function with hashModePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL hashModePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashGlobalModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with hashModePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashGlobalModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hashModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashGlobalModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashGlobalModeGet(dev, &hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpAddMacModeSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpAddMacModeSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with enable [GT_FALSE / GT_TRUE]. Expected: GT_OK.
1.2. Call cpssDxChTrunkHashIpAddMacModeGet with non-NULL enablePtr. Expected: GT_OK and same value as written.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableRet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable
           [GT_FALSE / GT_TRUE]. Expected: GT_OK.   */

        enable = GT_FALSE;

        st = cpssDxChTrunkHashIpAddMacModeSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enable = GT_TRUE;

        st = cpssDxChTrunkHashIpAddMacModeSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChTrunkHashIpAddMacModeGet with non-NULL
           enablePtr. Expected: GT_OK and same value as written.    */

        st = cpssDxChTrunkHashIpAddMacModeGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpAddMacModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpAddMacModeSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpAddMacModeSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpAddMacModeGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpAddMacModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL enablePtr. Expected: GT_OK.
1.2. Call function with enablePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashIpAddMacModeGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with enablePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashIpAddMacModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpAddMacModeGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpAddMacModeGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpModeSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpModeSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with enable [GT_FALSE / GT_TRUE]. Expected: GT_OK.
1.2. Call cpssDxChTrunkHashIpModeGet with non-NULL enablePtr. Expected: GT_OK and same value as written.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableRet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable
           [GT_FALSE / GT_TRUE]. Expected: GT_OK.   */

        enable = GT_FALSE;

        st = cpssDxChTrunkHashIpModeSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        enable = GT_TRUE;

        st = cpssDxChTrunkHashIpModeSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChTrunkHashIpModeGet with non-NULL
           enablePtr. Expected: GT_OK and same value as written.    */

        st = cpssDxChTrunkHashIpModeGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpModeSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpModeSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpModeGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL enablePtr. Expected: GT_OK.
1.2. Call function with enablePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashIpModeGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with enablePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashIpModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpModeGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpModeGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashL4ModeSet
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashL4ModeSet)
{
/*
ITERATE_DEVICES (DxChx)
    1.1. Call function with hashMode [CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E /
                                      CPSS_DXCH_TRUNK_L4_LBH_SHORT_E].
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkHashL4ModeGet with non-NULL hashModePtr.
    Expected: GT_OK and same value as written.
    1.3. Call function with hashMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT  hashMode = CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E;
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT  hashModeRet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with hashMode [CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E /
                                              CPSS_DXCH_TRUNK_L4_LBH_SHORT_E].
            Expected: GT_OK.
        */
        hashMode = CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E;

        st = cpssDxChTrunkHashL4ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashMode);

        /* 1.2. Call cpssDxChTrunkHashL4ModeGet with non-NULL
           hashModePtr. Expected: GT_OK and same value as written.  */

        st = cpssDxChTrunkHashL4ModeGet(dev, &hashModeRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashL4ModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(hashMode, hashModeRet,
                   "get another hashMode than was set: %d", dev);


        hashMode = CPSS_DXCH_TRUNK_L4_LBH_SHORT_E;

        st = cpssDxChTrunkHashL4ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashMode);

        /* 1.2. Call cpssDxChTrunkHashL4ModeGet with non-NULL
           hashModePtr. Expected: GT_OK and same value as written.  */

        st = cpssDxChTrunkHashL4ModeGet(dev, &hashModeRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashL4ModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(hashMode, hashModeRet,
                   "get another hashMode than was set: %d", dev);

        /*
           1.3. Call function with hashMode [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashL4ModeSet
                            (dev, hashMode),
                            hashMode);
    }

    hashMode = CPSS_DXCH_TRUNK_L4_LBH_SHORT_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashL4ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashL4ModeSet(dev, hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashL4ModeGet
(
    IN GT_U8   devNum,
    OUT CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT *hashModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashL4ModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL hashModePtr. Expected: GT_OK.
1.2. Call function with hashModePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT  hashMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL hashModePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashL4ModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with hashModePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashL4ModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hashModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashL4ModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashL4ModeGet(dev, &hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpv6ModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpv6ModeSet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with hashMode [CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E / CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E]. Expected: GT_OK.
1.2. Call cpssDxChTrunkHashIpv6ModeGet with non-NULL hashModePtr. Expected: GT_OK and same value as written.
1.3. Call function with hashMode [wrong enum values]. Expected: GT_BAD_PARAM.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT  hashMode;
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT  hashModeRet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with hashMode [CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E
           / CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E]. Expected: GT_OK.  */

        hashMode = CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E;

        st = cpssDxChTrunkHashIpv6ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashMode);

        hashMode = CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E;

        st = cpssDxChTrunkHashIpv6ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashMode);

        /* 1.2. Call cpssDxChTrunkHashIpv6ModeGet with non-NULL
           hashModePtr. Expected: GT_OK and same value as written.  */

        st = cpssDxChTrunkHashIpv6ModeGet(dev, &hashModeRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpv6ModeGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(hashMode, hashModeRet,
                   "get another hashMode than was set: %d", dev);

        /*
           1.3. Call function with hashMode [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashIpv6ModeSet
                            (dev, hashMode),
                            hashMode);
    }

    hashMode = CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpv6ModeSet(dev, hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpv6ModeSet(dev, hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpv6ModeGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   *hashModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpv6ModeGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL hashModePtr. Expected: GT_OK.
1.2. Call function with hashModePtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;

    GT_U8     dev;
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT  hashMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL hashModePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashIpv6ModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with hashModePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashIpv6ModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, hashModePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpv6ModeGet(dev, &hashMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpv6ModeGet(dev, &hashMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpShiftSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    IN GT_U32                       shiftValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpShiftSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                           CPSS_IP_PROTOCOL_IPV6_E /
                                           CPSS_IP_PROTOCOL_IPV4V6_E],
                                  isSrcIp [GT_TRUE / GT_FALSE]
                           and shiftValue [0 / 3 / 15].
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkHashIpShiftGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3. Call with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E],
                               isSrcIp [GT_TRUE],
                        and shiftValue [0].
    Expected: GT_BAD_PARAM.
    1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4_E],
                         isSrcIp [GT_TRUE],
            and wrong shiftValue [4].
    Expected: NOT GT_OK.
    1.5. Call protocolStack [CPSS_IP_PROTOCOL_IPV6_E],
                         isSrcIp [GT_TRUE],
            and wrong shiftValue [16].
    Expected: NOT GT_OK.
    1.6. Call with wrong enum values protocolStack ,
                               isSrcIp [GT_TRUE],
                        and shiftValue [0].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    isSrcIp       = GT_TRUE;
    GT_U32                     shiftValue    = 0;
    GT_U32                     shiftValueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                                   CPSS_IP_PROTOCOL_IPV6_E],
                                          isSrcIp [GT_TRUE / GT_FALSE],
                                   and shiftValue [0 / 3 / 15].
            Expected: GT_OK.
        */

        /* call with protocolStack [CPSS_IP_PROTOCOL_IPV4_E],
         * isSrcIp [GT_TRUE], shiftValue [0]*/

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        isSrcIp       = GT_TRUE;
        shiftValue    = 0;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashIpShiftGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpShiftGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(shiftValue, shiftValueGet,
                                     "get another shiftValue than was set: %d", dev);


        /* call with protocolStack [CPSS_IP_PROTOCOL_IPV6_E],
         * isSrcIp [GT_FALSE], shiftValue [3]*/
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        isSrcIp       = GT_FALSE;
        shiftValue    = 3;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashIpShiftGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpShiftGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(shiftValue, shiftValueGet,
                                     "get another shiftValue than was set: %d", dev);


        /* call with protocolStack [CPSS_IP_PROTOCOL_IPV6_E],
         * isSrcIp [GT_FALSE], shiftValue [15]*/
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        shiftValue    = 15;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashIpShiftGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashIpShiftGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(shiftValue, shiftValueGet,
                                     "get another shiftValue than was set: %d", dev);

        /*
            1.3. Call with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E],
                                       isSrcIp [GT_TRUE],
                                and shiftValue [0].
            Expected: GT_BAD_PARAM.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        shiftValue    = 0;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4_E],
                                 isSrcIp [GT_TRUE],
                    and wrong shiftValue [4].
            Expected: NOT GT_OK.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        shiftValue    = 4;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.5. Call with protocolStack [CPSS_IP_PROTOCOL_IPV6_E],
                                 isSrcIp [GT_TRUE],
                    and wrong shiftValue [16].
            Expected: NOT GT_OK.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        shiftValue    = 16;

        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.6. Call with wrong enum values protocolStack, isSrcIp [GT_TRUE],
                    and wrong shiftValue [0].
            Expected: GT_BAD_PARAM.
        */
        shiftValue = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashIpShiftSet
                            (dev, protocolStack, isSrcIp, shiftValue),
                            protocolStack);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpShiftSet(dev, protocolStack, isSrcIp, shiftValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIpShiftGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_BOOL                      isSrcIp,
    OUT GT_U32                      *shiftValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIpShiftGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                           CPSS_IP_PROTOCOL_IPV6_E /
                                           CPSS_IP_PROTOCOL_IPV4V6_E],
                                  isSrcIp [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E],
                                  isSrcIp [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum values protocolStack ,
                               isSrcIp [GT_TRUE].
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong shiftValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                      dev;
    CPSS_IP_PROTOCOL_STACK_ENT protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    GT_BOOL                    isSrcIp = GT_TRUE;
    GT_U32                     shiftValueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                                   CPSS_IP_PROTOCOL_IPV6_E],
                                          isSrcIp [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        /* call with protocolStack [CPSS_IP_PROTOCOL_IPV4_E], isSrcIp [GT_TRUE]*/

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        isSrcIp       = GT_TRUE;

        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with protocolStack [CPSS_IP_PROTOCOL_IPV6_E], isSrcIp [GT_FALSE]*/
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        isSrcIp       = GT_FALSE;

        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E],
                                       isSrcIp [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        isSrcIp       = GT_TRUE;

        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call with wrong enum values protocolStack, isSrcIp [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashIpShiftGet
                            (dev, protocolStack, isSrcIp, &shiftValueGet),
                            protocolStack);

        /*
            1.4. Call with wrong shiftValuePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIpShiftGet(dev, protocolStack, isSrcIp, &shiftValueGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    IN GT_U8                          maskValue
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E],
                              maskValue [0 / 0x2F /0x3F].
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values maskedField ,
                           maskValue [0].
    Expected: GT_BAD_PARAM.
    1.4. Call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E],
          and wrong  maskValue [0x4F].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st = GT_OK;

    GT_U8                          dev;
    CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;
    GT_U8                          maskValue = 0;
    GT_U8                          maskValueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E],
                                      maskValue [0 / 0x2F /0x3F].
            Expected: GT_OK.
        */

        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E], maskValue [0]*/

        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;
        maskValue   = 0;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(maskValue, maskValueGet,
                                     "get another maskValue than was set: %d", dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E], maskValue [0x2F]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E;
        maskValue   = 0x2F;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(maskValue, maskValueGet,
                                     "get another maskValue than was set: %d", dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E], maskValue [0x3F]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E;
        maskValue   = 0x3F;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(maskValue, maskValueGet,
                                     "get another maskValue than was set: %d", dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E], maskValue [0x3F]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(maskValue, maskValueGet,
                                     "get another maskValue than was set: %d", dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E], maskValue [0x3F]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChTrunkHashMaskGet with the same parameters.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(maskValue, maskValueGet,
                                     "get another maskValue than was set: %d", dev);

        /*
            1.3. Call with wrong enum values maskedField, maskValue [0].
            Expected: GT_BAD_PARAM.
        */
        maskValue   = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashMaskSet
                            (dev, maskedField, maskValue),
                            maskedField);

        /*
            1.4. Call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E],
                  and wrong  maskValue [0x4F].
            Expected: GT_OUT_OF_RANGE.
        */
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;
        maskValue   = 0x4F;

        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        maskValue   = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskSet(dev, maskedField, maskValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskGet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField,
    OUT GT_U8                         *maskValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E /
                                         CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E].
    Expected: GT_OK.
    1.2. Call with wrong enum values maskedField.
    Expected: GT_BAD_PARAM.
    1.4. Call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E],
         and wrong  maskValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                          dev;
    CPSS_DXCH_TRUNK_LBH_MASK_ENT   maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;
    GT_U8                          maskValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E /
                                                 CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E].
            Expected: GT_OK.
        */

        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E]*/

        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MPLS_LABEL0_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_IPV4_DIP_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_IPV6_FLOW_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /* call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E]*/
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_L4_SRC_PORT_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enum values maskedField.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashMaskGet
                            (dev, maskedField, &maskValue),
                            maskedField);

        /*
            1.3. Call with maskedField [CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E],
                  and wrong  maskValue [NULL].
            Expected: GT_BAD_PTR.
        */
        maskedField = CPSS_DXCH_TRUNK_LBH_MASK_MAC_DA_E;

        st = cpssDxChTrunkHashMaskGet(dev, maskedField, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskGet(dev, maskedField, &maskValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMplsModeEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMplsModeEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkHashMplsModeEnableGet with non-NULL enablePtr.
    Expected: GT_OK and same value as written.
*/
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with enable
           [GT_FALSE / GT_TRUE]. Expected: GT_OK.   */

        enable = GT_FALSE;

        st = cpssDxChTrunkHashMplsModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChTrunkHashMplsModeEnableGet with non-NULL
           enablePtr. Expected: GT_OK and same value as written.    */

        st = cpssDxChTrunkHashMplsModeEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMplsModeEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                   "get another enable than was set: %d", dev);


        enable = GT_TRUE;

        st = cpssDxChTrunkHashMplsModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* 1.2. Call cpssDxChTrunkHashMplsModeEnableGet with non-NULL
           enablePtr. Expected: GT_OK and same value as written.    */

        st = cpssDxChTrunkHashMplsModeEnableGet(dev, &enableRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkHashMplsModeEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableRet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMplsModeEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMplsModeEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMplsModeEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMplsModeEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_BOOL   enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashMplsModeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with enablePtr [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashMplsModeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMplsModeEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMplsModeEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U8       maxNumberOfTrunks,
    IN  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkInit)
{
/*
    NOTE: the trunk was already initialized during the cpssInitSystem.
        with the max number of trunks that the device supports.
        So the cpss API of cpssDxChTrunkInit should not allow to call the API
        again unless the maxNumberOfTrunks has same value as given during
        cpssInitSystem.

        the DXCH devices supports different number of trunks , so the test
        should loop on all number of trunks possible 0..127 and check that only
        one of then return GT_OK , while all others return GT_BAD_STATE


    ITERATE_DEVICES (DxChx)
    1.1. Call function with maxNumberOfTrunks [1],
        and trunkMembersMode [CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E /
                              CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E].
    Expected: see note above.
    1.2. Call function with wrong maxNumberOfTrunks [BIT_8].
    Expected: see note above.
    1.3. Call function with wrong enum values trunkMembersMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_TRUNK_ID  maxNumberOfTrunks = 1;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode = 0;
    GT_U32  numberOfOk;
    GT_STATUS lastSt;
    GT_U32      ii;
    GT_TRUNK_ID origInitNumberOfTrunks;/* the number of trunks that device was initialized with (during cpssInitSystem) */
    GT_U32      get_maxNumberOfTrunks;/* value returned by cpssDxChTrunkDbInitInfoGet */
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT get_trunkMembersMode;/* value returned by cpssDxChTrunkDbInitInfoGet */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with maxNumberOfTrunks [1],
                 and trunkMembersMode [CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E /
                                       CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E].
            Expected: GT_OK.
        */

        st = cpssDxChTrunkDbInitInfoGet(dev,&get_maxNumberOfTrunks,&get_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        numberOfOk = 0;
        origInitNumberOfTrunks = 0;

        for(ii = 0 ; ii < UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev); ii++)
        {
            maxNumberOfTrunks = (GT_TRUNK_ID)ii;

            trunkMembersMode = CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E;

            st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
            if(st == GT_OK)
            {
                numberOfOk ++;
                origInitNumberOfTrunks = maxNumberOfTrunks;

                /* check that those are the 'orig' init values */
                UTF_VERIFY_EQUAL0_PARAM_MAC(get_maxNumberOfTrunks, maxNumberOfTrunks);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);
            }

            lastSt = st;

            trunkMembersMode = CPSS_DXCH_TRUNK_MEMBERS_MODE_SOHO_EMULATION_E;

            st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(lastSt, st, dev);
        }

        if(numberOfOk != 1)
        {
            /* only single GT_OK should have been received */
            st = GT_BAD_STATE;
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* restore 'native' trunk init , for the sake of 'high level' trunk functionality
            for xcat A1 with 'trunk WA' */
        if((dev < PRV_CPSS_MAX_PP_DEVICES_CNS) &&
           ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)))
        {
            trunkMembersMode = CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E;
            st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /*
            1.2. Call function with wrong maxNumberOfTrunks [BIT_7].
            Expected: GT_BAD_STATE.
        */
        maxNumberOfTrunks = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);

        st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

        maxNumberOfTrunks = origInitNumberOfTrunks;

        /*
            1.3. Call with wrong enum values trunkMembersMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkInit
                            (dev, maxNumberOfTrunks, trunkMembersMode),
                            trunkMembersMode);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkInit(dev, maxNumberOfTrunks, trunkMembersMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDesignatedMemberSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDesignatedMemberSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunk id [2], non-NULL memberPtr {port [8], device [devNum]}.
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkDbDesignatedMemberGet with the same trunkId.
    Expected: GT_OK and the same values.
    1.3. Try to add the same trunk member again, accordingly
    to header the result must be OK. Call with trunk id [2],
    non-NULL memberPtr {port [8], device [devNum]}.
    Expected: GT_OK.
    1.4. Try to add already added trunk members to another trunk id.
    Call with trunk id [3], non-NULL memberPtr {port [8], device [devNum]}.
    Expected: NON GT_OK.
    1.5. Try to add invalid device id and legal port id
    as trunk entry member. Call with trunk id [2], non-NULL memberPtr {port [8],
    device [PRV_CPSS_MAX_PP_DEVICES_CNS]}.
    Expected: NON GT_OK.
    1.6. Try to add legal device id and invalid port id as trunk entry member.
    Call with trunk id [2], non-NULL memberPtr {port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
    device [devNum]}.
    Expected: NON GT_OK.
    1.7. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with null trunk id [NULL_TRUNK_ID_CNS = 0],
    non-NULL memberPtr [tested dev, virtual port 8].
    Expected: NON GT_OK.
    1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],
    non-NULL memberPtr [tested dev, virtual port 8].
    Expected: NON GT_OK.
    1.10. Call cpssDxChTrunkMemberRemove with trunkId [2],
    memberPtr {port [8], device [devNum]} to remove added member from the trunk.
    Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_BOOL                 enable = GT_FALSE;
    GT_BOOL                 enableGet = GT_TRUE;
    CPSS_TRUNK_MEMBER_STC   member;
    CPSS_TRUNK_MEMBER_STC   memberGet;
    GT_HW_DEV_NUM           hwDevNum;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        enable = GT_FALSE;

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);
        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChTrunkDbDesignatedMemberGet with the same trunkId.
                Expected: GT_OK and the same 'enable' value.
            */
            st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev, trunkId);
        }

        /*
            1.1. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        enable = GT_TRUE;

        member.hwDevice = hwDevNum;
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);
        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChTrunkDbDesignatedMemberGet with the same trunkId.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.hwDevice, memberGet.hwDevice,
                                   "get another member.hwDevice than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.port, memberGet.port,
                                   "get another member.port than was set: %d, %d", dev, trunkId);
        }

        /*
            1.3. Try to add the same trunk member again, accordingly to header the result must be OK.
            Call with trunk id [2], non-NULL memberPtr [tested dev, virtual port 8].
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                     dev, trunkId, member.hwDevice, member.port);
        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChTrunkDbDesignatedMemberGet with the same trunkId.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.hwDevice, memberGet.hwDevice,
                                   "get another member.hwDevice than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.port, memberGet.port,
                                   "get another member.port than was set: %d, %d", dev, trunkId);
        }

        /*
            1.4. Try to add already added trunk members to another trunk id.
            Call with trunk id [3], non-NULL memberPtr [tested dev id, virtual port 8].
            Expected: GT_OK.
        */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 1);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        enable = GT_TRUE;

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d, memberPtr->port = %d",
                                         dev, trunkId, member.hwDevice, member.port);
        if(GT_OK == st)
        {
            /*
                1.2. Call cpssDxChTrunkDbDesignatedMemberGet with the same trunkId.
                Expected: GT_OK and the same values.
            */
            st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "get another enable than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.hwDevice, memberGet.hwDevice,
                                   "get another member.hwDevice than was set: %d, %d", dev, trunkId);
            UTF_VERIFY_EQUAL2_STRING_MAC(member.port, memberGet.port,
                                   "get another member.port than was set: %d, %d", dev, trunkId);
        }

        /*
            1.5. Try to add invalid device id and legal port id as trunk entry member.
            Call with trunk id [2], non-NULL memberPtr [invalid dev id, virtual port 8].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        enable = GT_TRUE;

        member.hwDevice = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->device = %d",
                                         dev, trunkId, member.hwDevice);

        /*
            1.6. Try to add legal device id and invalid port id as trunk entry member.
            Call with trunk id [2], non-NULL memberPtr [tested dev id, invalid port].
            Expected: NON GT_OK.
        */
        member.hwDevice = hwDevNum;
        member.port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        enable = GT_TRUE;

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, memberPtr->port = %d",
                                         dev, trunkId, member.port);

        /*
            1.7. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        enable = GT_TRUE;

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);

        /*
            1.8. Call with null trunk id [NULL_TRUNK_ID_CNS = 0],
            non-NULL memberPtr [tested dev, virtual port 8].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.9. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],
            non-NULL memberPtr [tested dev, virtual port 8].
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.10. Call cpssDxChTrunkMemberRemove with trunkId [2],
            memberPtr [tested dev id, virtual port 8] to remove added member from the trunk.
            Expected: GT_OK
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMemberRemove(dev, trunkId, &member);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChTrunkMemberRemove: %d, %d, %d, %d",
                                     dev, trunkId, member.hwDevice, member.port);

        /* clear configuration to not impact for other tests.
           (when running cascade trunks , the 'designated port' can't be enabled !) */
        {
            trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            enable = GT_FALSE;
            st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 1);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            enable = GT_FALSE;
            st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
        }
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice = 0;
    member.port = TRUNK_TESTED_VIRT_PORT_ID_CNS(dev);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. Call function for non active device and valid parameters.   */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDesignatedMemberSet(dev, trunkId, enable, &member);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbDesignatedMemberGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunk id [2], non-NULL memberPtr and enablePtr.
    Expected: GT_OK.
    1.2. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],
    Expected: NON GT_OK.
    1.3. Check for NULL handling. Call with trunk id [2], enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_BOOL                 enableGet = GT_TRUE;
    CPSS_TRUNK_MEMBER_STC   memberGet;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunk id [2], non-NULL memberPtr [tested dev id, virtual port 8].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Check for NULL handling. Call with trunk id [2], enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, NULL, &memberGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, trunkId);

        /*
            1.4. Check for NULL handling. Call with trunk id [2], memberPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, memberPtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. Call function for non active device and valid parameters.   */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbDesignatedMemberGet(dev, trunkId, &enableGet, &memberGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
    Test function to Fill trunk table.
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkFillTrunkTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size. numEntries [PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1].
    1.2. Fill all entries in trunk table.
            Call cpssDxChTrunkTableEntrySet with trunkId [0... numEntries-1], numMembers [1], membersArray [0 / 1 / 2] {port [0 / 1 / 2], device [devNum]}.
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
            Call cpssDxChTrunkTableEntrySet with trunkId [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in trunk table and compare with original.
            Call with cpssDxChTrunkTableEntryGet with trunkId [0... numEntries-1], non-NULL numMembersPtr, membersArray.
    Expected: GT_OK, numMembersPtr [1], and the same membersArray as was set.
    1.5. Try to read entry with index out of range.
            Call with cpssDxChTrunkTableEntryGet with trunkId [numEntries], non-NULL numMembersPtr, membersArray.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U32      numEntries  = 0;
    GT_U32      iTemp       = 0;

    GT_U32                  numMembers = 0;
    static CPSS_TRUNK_MEMBER_STC   membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  numMembersWritten = 0;
    static CPSS_TRUNK_MEMBER_STC   membersArrayWritten[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    GT_TRUNK_ID             trunkId;
    GT_HW_DEV_NUM           hwDevNum;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        numEntries = PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1;

        if(prvUtfDxChTrunkWaNeeded == GT_TRUE)
        {
            /* only 30 trunks supported
                + 2 cascade trunks 126,127
                  if PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127
            */
            numEntries = 31;
        }

        hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

        /* Fill the entry for trunk table */
        numMembers = 1;
        membersArray[0].hwDevice = hwDevNum;
        membersArray[0].port = 0;
        membersArray[1].hwDevice = hwDevNum;
        membersArray[1].port = 1;
        membersArray[2].hwDevice = hwDevNum;
        membersArray[2].port = 2;

        /* 1.2. Fill all entries in trunk table */
        for(iTemp = 1; iTemp < numEntries; ++iTemp)
        {
            trunkId = (GT_TRUNK_ID)iTemp;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
            st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntrySet: %d, %d, %d", dev, iTemp, numMembers);
        }

        if(prvUtfDxChTrunkWaNeeded == GT_TRUE &&
           PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127)
        {
            /* only 30 trunks supported
                + 2 cascade trunks 126,127
                  if PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127
            */
            numEntries =
                PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1;
            /* 1.2. Fill all entries in trunk table */
            for(iTemp = 126; iTemp < numEntries; ++iTemp)
            {
                trunkId = (GT_TRUNK_ID)iTemp;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
                st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntrySet: %d, %d, %d", dev, iTemp, numMembers);
            }
        }

        numEntries =
           PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1;
        trunkId = (GT_TRUNK_ID)numEntries;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChTrunkTableEntrySet(dev, trunkId, numMembers, membersArray);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntrySet: %d, %d, %d", dev, numEntries, numMembers);

        if(prvUtfDxChTrunkWaNeeded == GT_TRUE)
        {
            /* only 30 trunks supported
                + 2 cascade trunks 126,127
                  if PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127
            */
            numEntries = 31;
        }

        /* 1.4. Read all entries in trunk table and compare with original */
        for(iTemp = 1; iTemp < numEntries; ++iTemp)
        {
            trunkId = (GT_TRUNK_ID)iTemp;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembersWritten, membersArrayWritten);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet: %d, %d", dev, iTemp);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(numMembers, numMembersWritten,
                       "get another numMembersPtr than was set: %d, %d", dev, iTemp);

            st = cpssOsMemCmp((const GT_VOID*)membersArray, (const GT_VOID*)membersArrayWritten,
                                numMembers*sizeof(membersArray[0]));
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "get another membersArray than was set: %d, %d", dev, iTemp);
        }

        if(prvUtfDxChTrunkWaNeeded == GT_TRUE &&
           PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127)
        {
            /* only 30 trunks supported
                + 2 cascade trunks 126,127
                  if PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw == 127
            */
            numEntries =
               PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1;

            /* 1.4. Read all entries in trunk table and compare with original */
            for(iTemp = 126; iTemp < numEntries; ++iTemp)
            {
                trunkId = (GT_TRUNK_ID)iTemp;
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

                st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembersWritten, membersArrayWritten);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet: %d, %d", dev, iTemp);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(numMembers, numMembersWritten,
                           "get another numMembersPtr than was set: %d, %d", dev, iTemp);

                st = cpssOsMemCmp((const GT_VOID*)membersArray, (const GT_VOID*)membersArrayWritten,
                                    numMembers*sizeof(membersArray[0]));
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "get another membersArray than was set: %d, %d", dev, iTemp);
            }
        }

        numEntries =
           PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numTrunksSupportedHw + 1;

        trunkId = (GT_TRUNK_ID)numEntries;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChTrunkTableEntryGet(dev, trunkId, &numMembersWritten, membersArrayWritten);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashCrcParametersSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    IN GT_U32                               crcSeed
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashCrcParametersSet)
{
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT  crcModes[] = {
                        CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E,
                        CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E,
                        CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E};
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT  crcMode = CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT  crcModeRet = CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E;
    GT_U32              crcSeed,crcSeedRet;
    GT_U32  numOfModes = sizeof(crcModes) / sizeof(crcModes[0]);
    GT_U32  ii;
    GT_BOOL supportCrc32;
    GT_BOOL useCrc32;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        supportCrc32 = UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev);

        for(ii = 0 ; ii < numOfModes; ii++)
        {
            crcMode = crcModes[ii];

            useCrc32 = GT_FALSE;
            if(crcMode == CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E)
            {
                crcSeed = BIT_6 - 5;
            }
            else
            if(crcMode == CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E)/* crc 16 */
            {
                crcSeed = BIT_16 - 5;
            }
            else/*CRC 32*/
            {
                crcSeed = 0xFFFFFFFF - BIT_14;
                useCrc32 = GT_TRUE;
            }

            st = cpssDxChTrunkHashCrcParametersSet(dev, crcMode , crcSeed);
            if(useCrc32 == GT_TRUE && supportCrc32 == GT_FALSE)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* 1.2. Call cpssDxChTrunkHashGlobalModeGet with non-NULL
                   hashModePtr. Expected: GT_OK and same value as written.  */

                st = cpssDxChTrunkHashCrcParametersGet(dev, &crcModeRet , &crcSeedRet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(crcModeRet, crcMode);
                UTF_VERIFY_EQUAL0_PARAM_MAC(crcSeedRet, crcSeed);
            }

            if(crcMode == CPSS_DXCH_TRUNK_LBH_CRC_6_MODE_E)
            {
                crcSeed = BIT_6;
            }
            else
            if(crcMode == CPSS_DXCH_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E)/* crc 16 */
            {
                crcSeed = BIT_16;
            }
            else/*CRC 32*/
            {
                crcSeed = 0xFFFFFFFF; /* no max value as all 32 bits are used ... */
            }

            st = cpssDxChTrunkHashCrcParametersSet(dev, crcMode , crcSeed);
            if(useCrc32 == GT_TRUE)
            {
                if(supportCrc32 == GT_FALSE)
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
                }
                else
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
            }
        }

        crcSeed = 5;/* make sure no errors on the crcSeed value */
        /*
           1.3. Call function with hashMode [wrong enum values].
           Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashCrcParametersSet(dev, crcMode , crcSeed),
                            crcMode);
    }

    crcMode = crcModes[0];
    crcSeed = 5;/* make sure no errors on the crcSeed value */

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashCrcParametersSet(dev, crcMode , crcSeed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashCrcParametersSet(dev, crcMode , crcSeed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashCrcParametersGet
(
    IN GT_U8                                devNum,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     *crcModePtr,
    OUT GT_U32                               *crcSeedPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashCrcParametersGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL enablePtr. Expected: GT_OK.
1.2. Call function with  [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;
    GT_U8     dev;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT   crcMode;
    GT_U32              crcSeed;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        st = cpssDxChTrunkHashCrcParametersGet(dev, &crcMode , &crcSeed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with [NULL].
           Expected: GT_BAD_PTR.    */

        st = cpssDxChTrunkHashCrcParametersGet(dev, NULL , &crcSeed);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChTrunkHashCrcParametersGet(dev, &crcMode , NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChTrunkHashCrcParametersGet(dev, NULL , NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashCrcParametersGet(dev, &crcMode , &crcSeed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashCrcParametersGet(dev, &crcMode , &crcSeed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkPortHashMaskInfoSet
(
    IN GT_U8     devNum,
    IN GT_U8     portNum,
    IN GT_BOOL   overrideEnable,
    IN GT_U32    index
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkPortHashMaskInfoSet)
{
    GT_STATUS  st    = GT_OK;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_PORT_NUM cpssPort;
    GT_BOOL   overrideEnable,overrideEnableRet;
    GT_U32    index,indexRet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            cpssPort = port;
            overrideEnable = GT_FALSE;
            index = 0xFFFFFFFF;/* should be ignored */

            st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnableRet , &indexRet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(overrideEnable, overrideEnableRet);

            overrideEnable = GT_TRUE;

            for(index = 0 ; index < BIT_4 ; index++)
            {
                st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnableRet , &indexRet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                UTF_VERIFY_EQUAL0_PARAM_MAC(overrideEnable, overrideEnableRet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(index, indexRet);
            }

            for(/* continue*/ ; index < BIT_6 ; index++)
            {
                st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
            }
        }

        overrideEnable = GT_FALSE;
        index = 0xFFFFFFFF;/* should be ignored */

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            cpssPort = port;
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, cpssPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        cpssPort = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, cpssPort);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        cpssPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
        if(TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port is in the 'valid range' */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    overrideEnable = GT_FALSE;
    index = 0xFFFFFFFF;/* should be ignored */
    cpssPort = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkPortHashMaskInfoSet(dev, cpssPort, overrideEnable , index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkPortHashMaskInfoGet
(
    IN GT_U8     devNum,
    IN GT_U8     portNum,
    OUT GT_BOOL  *overrideEnablePtr,
    OUT GT_U32   *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkPortHashMaskInfoGet)
{
    GT_STATUS  st    = GT_OK;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_PORT_NUM     cpssPort;
    GT_BOOL   overrideEnable;
    GT_U32    index;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL .
                Expected: GT_OK.
            */

            cpssPort = port;

            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cpssPort);

            /*
                1.1.2. Call with null [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, NULL , &index);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, NULL , NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            cpssPort = port;

            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, cpssPort);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        cpssPort = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, cpssPort);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        cpssPort = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
        if(TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port is in the 'valid range' */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    cpssPort = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkPortHashMaskInfoGet(dev, cpssPort, &overrideEnable , &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskCrcEntrySet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskCrcEntrySet)
{
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    /* the DB about the fields in the CRC hash mask entry.
        index to this table is according to fields in
        CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC */
    struct FIELD_INFO_STCT{
        GT_U32  length;
        GT_U32  offset;
    } hashMaskCrcFieldsInfo[]=
    {
        {2 ,0 }, /* l4DstPortMaskBmp;    */
        {2 ,0 }, /* l4SrcPortMaskBmp;    */
        {3 ,0 }, /* ipv6FlowMaskBmp;     */
        {16,0 }, /* ipDipMaskBmp;        */
        {16,0 }, /* ipSipMaskBmp;        */
        {6 ,0 }, /* macDaMaskBmp;        */
        {6 ,0 }, /* macSaMaskBmp;        */
        {3 ,0 }, /* mplsLabel0MaskBmp;   */
        {3 ,0 }, /* mplsLabel1MaskBmp;   */
        {3 ,0 }, /* mplsLabel2MaskBmp;   */
        {1 ,0 }, /* localSrcPortMaskBmp; */
        {9 ,14}, /* udbsMaskBmp;         */
    };
    GT_U32  numOfFields = sizeof(hashMaskCrcFieldsInfo) / sizeof(hashMaskCrcFieldsInfo[0]);
    GT_U32  index;
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC hashMaskEntry , hashMaskEntryRet;
    GT_U32  *fieldPtr;
    GT_U32  bit;/*bit index*/
    GT_U32  field;/*field index*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < TRUNK_HASH_MASK_CRC_NUM_ENTRIES_CNS(dev) ; index ++)
        {
            cpssOsMemSet(&hashMaskEntry,0,sizeof(hashMaskEntry));
            cpssOsMemSet(&hashMaskEntryRet,0,sizeof(hashMaskEntryRet));

            fieldPtr = (GT_U32*)(void*)(&hashMaskEntry);
            for(field = 0 ; field < numOfFields; field++,fieldPtr++)
            {
                *fieldPtr = 0;

                for(bit = 1 ; bit < (hashMaskCrcFieldsInfo[field].length - 1) ; bit+=2)
                {
                    /* set a bit */
                    U32_SET_FIELD_MAC((*fieldPtr) , bit , 1, 1);
                }

                /* set the MSB */
                U32_SET_FIELD_MAC((*fieldPtr) , (hashMaskCrcFieldsInfo[field].length - 1) , 1, 1);

                (*fieldPtr) <<= hashMaskCrcFieldsInfo[field].offset;

                if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                {
                    hashMaskEntry.symmetricMacAddrEnable    = ((index & 1) == 1) ? GT_TRUE  : GT_FALSE;
                    hashMaskEntry.symmetricIpv4AddrEnable   = ((index & 1) == 1) ? GT_FALSE : GT_TRUE ;
                    hashMaskEntry.symmetricMacAddrEnable    = ((index & 3) == 2) ? GT_TRUE  : GT_FALSE;
                    hashMaskEntry.symmetricL4PortEnable     = ((index & 3) == 3) ? GT_FALSE : GT_TRUE ;
                }

                st = cpssDxChTrunkHashMaskCrcEntrySet(dev, index , &hashMaskEntry);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* 1.2. Call cpssDxChTrunkHashGlobalModeGet with non-NULL
                   hashModePtr. Expected: GT_OK and same value as written.  */

                st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , &hashMaskEntryRet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if(cpssOsMemCmp(&hashMaskEntryRet,&hashMaskEntry,sizeof(hashMaskEntry)))
                {
                    UTF_VERIFY_EQUAL0_PARAM_MAC(1, 0);/* set and get has different values */
                }
            }

            fieldPtr = (GT_U32*)(void*)(&hashMaskEntry);
            for(field = 0 ; field < numOfFields; field++,fieldPtr++)
            {
                *fieldPtr = 1 + BIT_MASK_MAC(hashMaskCrcFieldsInfo[field].length);

                st = cpssDxChTrunkHashMaskCrcEntrySet(dev, index , &hashMaskEntry);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
            }

            st = cpssDxChTrunkHashMaskCrcEntrySet(dev, index , NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        }
    }

    cpssOsMemSet(&hashMaskEntry,0,sizeof(hashMaskEntry));
    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskCrcEntrySet(dev, index , &hashMaskEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskCrcEntrySet(dev, index , &hashMaskEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskCrcEntryGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskCrcEntryGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL enablePtr. Expected: GT_OK.
1.2. Call function with  [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;
    GT_U8     dev;
    GT_U32    index;
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC hashMaskEntry;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        for(index = 0 ; index < TRUNK_HASH_MASK_CRC_NUM_ENTRIES_CNS(dev) ; index ++)
        {
            st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , &hashMaskEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2. Call function with [NULL].
               Expected: GT_BAD_PTR.    */

            st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        /*index = TRUNK_HASH_MASK_CRC_NUM_ENTRIES_CNS;*/

        st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , &hashMaskEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , &hashMaskEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskCrcEntryGet(dev, index , &hashMaskEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashIndexCalculate
(
    IN GT_U8                                      devNum,
    IN CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC         *hashInputDataPtr,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT       hashEntityType,
    IN GT_U32                                     numberOfMembers,
    OUT GT_U32                                    *hashIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashIndexCalculate)
{
/*
    ITERATE_DEVICES (Bobcat2; Caelum; Bobcat3)
    1.1. Call function with hashInputData [ipDipArray/ipSipArray fit to ipv4/ipv6
                                           and other legal values]
             hashEntityType[CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E]
             numberOfMembers[10]

    Expected: GT_OK.
    1.2. Call function with hashInputData [legal values]
             hashEntityType[CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
                            CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E]
             numberOfMembers[legal values]

    Expected: GT_NOT_SUPPORTED.
    1.3. Call with wrong enum values hashEntityType.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong numberOfMembers[0,0xFFF+2].
    Expected: GT_BAD_PARAM.
    1.5. Call hashInputDataPtr [NULL], and hashIndexPtr[NULL]
    Expected: GT_BAD_PTR.
*/

    GT_STATUS st = GT_OK;

    GT_U8                                   dev;
    CPSS_DXCH_TRUNK_LBH_INPUT_DATA_STC      hashInputData;
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT    hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
    GT_U32                                  numberOfMembers=99;
    GT_U32                                  hashIndex = 1;

    GT_U32                                  l4DstPort;
    GT_U32                                  l4SrcPort;
    GT_U32                                  ipv6Flow;
    GT_U32                                  mplsLabel0;
    GT_U32                                  mplsLabel1;
    GT_U32                                  mplsLabel2;
    GT_U8                                   i=0;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT        crcModeGet;
    GT_U32                                  crcSeedGet;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT           packetType;

    l4DstPort       = 20;
    for(i=0; i<2; i++)
    {
        hashInputData.l4DstPortArray[i]= (GT_U8)((l4DstPort>>(8*i)) & 0xFF);

    }

    l4SrcPort       = 30;
    for(i=0; i<2; i++)
    {
        hashInputData.l4SrcPortArray[i]= (GT_U8)((l4SrcPort>>(8*i)) & 0xFF);
    }

    ipv6Flow   = 0x112233;
    for(i=0; i<3; i++)
    {
        hashInputData.ipv6FlowArray[i]= (GT_U8)((ipv6Flow>>(8*i)) & 0xFF);
    }

    for(i=0; i<6; i++)
    {
        hashInputData.macDaArray[i] = (GT_U8)(0x30 * (i+1));
    }

    for(i=0; i<6; i++)
    {
        hashInputData.macSaArray[i] =  (GT_U8)(0x40 * (i+1));
    }

    mplsLabel0      = 0xF33333;
    for(i=0; i<3; i++)
    {
        hashInputData.mplsLabel0Array[i]= (GT_U8)((mplsLabel0>>(8*i)) & 0xFF);
    }

    mplsLabel1      = 0xF44444;
    for(i=0; i<3; i++)
    {
        hashInputData.mplsLabel1Array[i]= (GT_U8)((mplsLabel1>>(8*i)) & 0xFF);
    }

    mplsLabel2      = 0xF55555;
    for(i=0; i<3; i++)
    {
        hashInputData.mplsLabel2Array[i]= (GT_U8)((mplsLabel2>>(8*i)) & 0xFF);
    }

    hashInputData.localSrcPortArray[0] = 0xAA;

    for(i=0; i<32; i++)
    {
        hashInputData.udbsArray[i] = i;
    }
    packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,UTF_CPSS_PP_E_ARCH_CNS);

   /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = cpssDxChTrunkHashCrcParametersGet(dev, &crcModeGet, &crcSeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkHashCrcParametersSet(dev,CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E,crcSeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call function with hashInputData [ipDipArray/ipSipArray fit to ipv4/ipv6
                                           and other legal values]
             hashEntityType[CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
                            CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
                            CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E]
             numberOfMembers[10]

            Expected: GT_OK.
        */

        /* Call with  hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E
           and ipv4 SIP and DIP */
        for(hashEntityType = 0 ;
            hashEntityType <= CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E;
            hashEntityType ++)
        {
            cpssOsMemSet(hashInputData.ipDipArray, 0, sizeof(hashInputData.ipDipArray));
            hashInputData.ipDipArray[0] = 0xff;
            hashInputData.ipDipArray[1] = 0x44;
            hashInputData.ipDipArray[2] = 0x55;
            hashInputData.ipDipArray[3] = 0x66;


            cpssOsMemSet(hashInputData.ipSipArray, 0, sizeof(hashInputData.ipSipArray));
            hashInputData.ipSipArray[0] = 0xff;
            hashInputData.ipSipArray[1] = 0x77;
            hashInputData.ipSipArray[2] = 0x88;
            hashInputData.ipSipArray[3] = 0x99;

            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

           /* Call with  hashEntityType = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E
              and ipv6 SIP and DIP */
            cpssOsMemSet(hashInputData.ipDipArray, 0, sizeof(hashInputData.ipDipArray));
            for(i=0; i<16; i++)
            {
                hashInputData.ipDipArray[i] = 0x10 * (i+1);
            }


            cpssOsMemSet(hashInputData.ipSipArray, 0, sizeof(hashInputData.ipSipArray));
            for(i=0; i<16; i++)
            {
                hashInputData.ipSipArray[i] =  0x20 * (i+1);
            }

            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.3. Call with wrong enum values packetTypes.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashIndexCalculate(dev,
                                                                100,
                                                                &hashInputData,
                                                                hashEntityType,
                                                                numberOfMembers,
                                                                &hashIndex),
                                                                packetType);

            /* 1.5. Call with wrong numberOfMembers[0,0xFFF+2].
               Expected: GT_BAD_PARAM
            */

            numberOfMembers = 0;
            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            numberOfMembers = 0xFFF+2;
            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            numberOfMembers = 0xFFF+1;
            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.6. Call hashInputDataPtr [NULL], and hashIndexPtr[NULL]
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 NULL,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 &hashIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChTrunkHashIndexCalculate(dev,
                                                 packetType,
                                                 &hashInputData,
                                                 hashEntityType,
                                                 numberOfMembers,
                                                 NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }

        /*
            1.4. Call with wrong enum values hashEntityType.
            Expected: GT_BAD_PARAM.
        */
                hashEntityType = 100;
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashIndexCalculate(dev,
                                                            packetType,
                                                            &hashInputData,
                                                            hashEntityType,
                                                            numberOfMembers,
                                                            &hashIndex),
                                                            hashEntityType);

        /* restore */
        st = cpssDxChTrunkHashCrcParametersSet(dev, crcModeGet, crcSeedGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev,UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashIndexCalculate(dev,
                                             packetType,
                                             &hashInputData,
                                             hashEntityType,
                                             numberOfMembers,
                                             &hashIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashIndexCalculate(dev,
                                         packetType,
                                         &hashInputData,
                                         hashEntityType,
                                         numberOfMembers,
                                         &hashIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashNumBitsSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               startBit,
    IN GT_U32                               numOfBits
)
*/
typedef struct {
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT client;
    GT_U32 start;
    GT_U32 num;
} PRV_CPSS_HASH_BIT_SET_CONFIG_STC;

UTF_TEST_CASE_MAC(cpssDxChTrunkHashNumBitsSet)
{
/*
    1. ITERATE_DEVICES
    1.1. Call function with all valid arguments and verify
    Expected: GT_OK.
    1.2. Call function with invalid hash client
    Expected: GT_BAD_PARAM.
    1.3. Call function with invalid start bit
    Expected: GT_OUT_OF_RANGE.
    1.4. Call function with invalid number of bits
    Expected: GT_OUT_OF_RANGE.
    1.5. Call function with sum of start bit and  number of bits >32
    (All except EGR OAM)
    Expected: GT_OUT_OF_RANGE.
    1.6. Call function with sum of start bit and  number of bits >12
    (EGR_OAM)
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS st            = GT_OK;
    GT_U8       dev;
    PRV_CPSS_HASH_BIT_SET_CONFIG_STC clientConfigs[] =
    {
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, 0, 16},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, 15, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E, 0, 16},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E, 15, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E, 0, 16},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E, 15, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E, 26, 6},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_DESIGNATED_TABLE_E, 31, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 26, 6},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 31, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 26, 6},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_INGRESS_OAM_E, 31, 1},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 0, 6},
        {CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 11, 1},
    };

    GT_U32      startBitGet;
    GT_U32      numOfBitsGet;
    GT_U32      ii;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with all valid arguments and verify
            Expected: GT_OK.
        */
        for(ii = 0; ii < NUM_OF_MEMBERS_MAC(clientConfigs); ii++)
        {
            st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[ii].client,
                clientConfigs[ii].start, clientConfigs[ii].num);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,
                clientConfigs[ii].client, clientConfigs[ii].start,
                clientConfigs[ii].num);

            st = cpssDxChTrunkHashNumBitsGet(dev, clientConfigs[ii].client,
                &startBitGet, &numOfBitsGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,
                clientConfigs[ii].client);

            UTF_VERIFY_EQUAL0_PARAM_MAC(clientConfigs[ii].start, startBitGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(clientConfigs[ii].num, numOfBitsGet);
        }

        /*
            1.2. Call function with invalid hash client
            Expected: GT_BAD_PARAM.
        */
        clientConfigs[0].client = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E;
        st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client,
            clientConfigs[0].start, clientConfigs[0].num);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev,
            clientConfigs[0].client, clientConfigs[0].start,
            clientConfigs[0].num);
        clientConfigs[0].client = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;

        /*
            1.3. Call function with invalid start bit
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client, 32,
            clientConfigs[0].num);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,
            clientConfigs[0].client, 32, clientConfigs[0].num);

        /*
            1.4. Call function with invalid number of bits
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client,
            clientConfigs[0].start, 17);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,
            clientConfigs[0].client, clientConfigs[0].start, 17);

        /*
            1.5. Call function with sum of start bit and  number of bits >32
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client, 17, 16);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,
            clientConfigs[0].client, 17, 16);

        /*  1.6. Call function with sum of start bit and  number of bits >12
            (EGR_OAM)
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChTrunkHashNumBitsSet(dev,
            CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 7, 6);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OUT_OF_RANGE, st, dev,
            CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_EGRESS_OAM_E, 7, 6);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
    UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
    UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client,
            clientConfigs[0].start, clientConfigs[0].num);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            clientConfigs[0].client, clientConfigs[0].start,
            clientConfigs[0].num);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashNumBitsSet(dev, clientConfigs[0].client,
        clientConfigs[0].start, clientConfigs[0].num);
    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, clientConfigs[0].client,
        clientConfigs[0].start, clientConfigs[0].num);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashNumBitsGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    IN GT_U32                               *startBitPtr,
    IN GT_U32                               *numOfBitsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashNumBitsGet)
{
/*
    1. ITERATE_DEVICES
    1.1. Call function with all valid arguments and verify
    Expected: GT_OK.
    1.2. Call function with invalid hash client
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st            = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient =
        CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;
    GT_U32      startBitGet;
    GT_U32      numOfBitsGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
        UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with all valid arguments and verify
            Expected: GT_OK.
        */
        for(hashClient = 0;
            hashClient < CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E;
            hashClient++)
        {
            st = cpssDxChTrunkHashNumBitsGet(dev, hashClient, &startBitGet,
                &numOfBitsGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, hashClient);
        }
        /*
            1.2. Call function with invalid hash client
            Expected: GT_BAD_PARAM.
        */
        hashClient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE___LAST___E;
        st = cpssDxChTrunkHashNumBitsGet(dev, hashClient, &startBitGet,
            &numOfBitsGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, hashClient);
        hashClient = CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E;

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
    UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E |
    UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashNumBitsGet(dev, hashClient, &startBitGet,
            &numOfBitsGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev,
            hashClient);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashNumBitsGet(dev, hashClient, &startBitGet,
        &numOfBitsGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, hashClient);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashPearsonValueSet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    IN GT_U32                         value
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashPearsonValueSet)
{
    GT_STATUS st              = GT_OK;

    GT_U8       dev;
    GT_U32      index    = 0;
    GT_U32      value    = 0;
    GT_U32      valueRet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0 ; index < 64; index++)
        {
            for(value = 0 ; value < 64; value++)
            {
                st = cpssDxChTrunkHashPearsonValueSet(dev, index , value);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* 1.2. Call cpssDxChTrunkHashPearsonValueGet with non-NULL
                   hashModePtr. Expected: GT_OK and same value as written.  */

                st = cpssDxChTrunkHashPearsonValueGet(dev, index , &valueRet);
                UTF_VERIFY_EQUAL0_PARAM_MAC(valueRet, value);
            }

            st = cpssDxChTrunkHashPearsonValueSet(dev, index , value);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
        }

        st = cpssDxChTrunkHashPearsonValueSet(dev, index , value);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
    }

    index = 0;
    value = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashPearsonValueSet(dev, index , value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashPearsonValueSet(dev, index , value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashPearsonValueGet
(
    IN GT_U8                          devNum,
    IN GT_U32                         index,
    OUT GT_U32                        *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashPearsonValueGet)
{
/*
ITERATE_DEVICES (DxChx)
1.1. Call function with non-NULL enablePtr. Expected: GT_OK.
1.2. Call function with  [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS st           = GT_OK;
    GT_U8     dev;
    GT_U32      index;
    GT_U32      value;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-NULL enablePtr.
           Expected: GT_OK. */

        for(index = 0 ; index < 64; index++)
        {
            st = cpssDxChTrunkHashPearsonValueGet(dev, index , &value);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2. Call function with [NULL].
               Expected: GT_BAD_PTR.    */

            st = cpssDxChTrunkHashPearsonValueGet(dev, index , NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashPearsonValueGet(dev, index , &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashPearsonValueGet(dev, index , &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashCrcSaltByteSet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    IN GT_U8    saltValue
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashCrcSaltByteSet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32   byteIndex;
    GT_U8    saltValue;
    GT_U8    getSaltValue;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* set all 70 value , and then check that can get all those values */

        /*set all values*/
        saltValue = 0xFF;
        for(byteIndex = 0; byteIndex < 70 ; byteIndex++,saltValue--)
        {
            st = cpssDxChTrunkHashCrcSaltByteSet(dev, byteIndex, saltValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*get all values*/
        saltValue = 0xFF;
        for(byteIndex = 0; byteIndex < 70 ; byteIndex++,saltValue--)
        {
            st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, &getSaltValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* compare set and get values */
            UTF_VERIFY_EQUAL0_PARAM_MAC(saltValue, getSaltValue);
        }

        /*check byte index*/
        byteIndex = 70;
        saltValue = 0;
        st = cpssDxChTrunkHashCrcSaltByteSet(dev, byteIndex, saltValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    byteIndex = 0;
    saltValue = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashCrcSaltByteSet(dev, byteIndex, saltValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashCrcSaltByteSet(dev, byteIndex, saltValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashCrcSaltByteGet
(
    IN GT_U8    devNum,
    IN GT_U32   byteIndex,
    OUT GT_U8    *saltValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashCrcSaltByteGet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32   byteIndex;
    GT_U8    getSaltValue;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        byteIndex = 50;

        st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, &getSaltValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        byteIndex = 70;

        st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, &getSaltValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        byteIndex = 30;
        st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    byteIndex = 25;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, &getSaltValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashCrcSaltByteGet(dev, byteIndex, &getSaltValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideSet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    IN GT_BOOL                                          override
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskCrcParamOverrideSet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT   fieldType;
    GT_BOOL    override;
    GT_BOOL    getOverride;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* set all values , and then check that can get all those values */
        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E ;
            fieldType <= CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E;
            fieldType++)
        {
            override = (fieldType & 1) ? GT_TRUE : GT_FALSE;

            st = cpssDxChTrunkHashMaskCrcParamOverrideSet(dev, fieldType, override);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E ;
            fieldType <= CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E;
            fieldType++)
        {
            override = (fieldType & 1) ? GT_TRUE : GT_FALSE;

            st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, &getOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* compare set and get values */
            UTF_VERIFY_EQUAL0_PARAM_MAC(override, getOverride);
        }


        /* now we opposite 'override' values */

        /* set all values , and then check that can get all those values */
        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E ;
            fieldType <= CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E;
            fieldType++)
        {
            override = (fieldType & 1) ? GT_FALSE : GT_TRUE;

            st = cpssDxChTrunkHashMaskCrcParamOverrideSet(dev, fieldType, override);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E ;
            fieldType <= CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E;
            fieldType++)
        {
            override = (fieldType & 1) ? GT_FALSE : GT_TRUE;

            st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, &getOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* compare set and get values */
            UTF_VERIFY_EQUAL0_PARAM_MAC(override, getOverride);
        }

        /* check unknown enum values */

        /* set all values , and then check that can get all those values */
        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E + 1;
            fieldType < 100;
            fieldType++)
        {
            override = (fieldType & 1) ? GT_FALSE : GT_TRUE;

            st = cpssDxChTrunkHashMaskCrcParamOverrideSet(dev, fieldType, override);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        override = GT_TRUE;
        /* check special enum values */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashMaskCrcParamOverrideSet
                            (dev, fieldType, override),
                            fieldType);
    }


    fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E;
    override = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskCrcParamOverrideSet(dev, fieldType, override);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskCrcParamOverrideSet(dev, fieldType, override);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkHashMaskCrcParamOverrideGet
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT fieldType,
    OUT GT_BOOL                                          *overridePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkHashMaskCrcParamOverrideGet)
{
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_ENT   fieldType;
    GT_BOOL    getOverride;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* cases of 'valid' ranges already tested in UT of cpssDxChTrunkHashMaskCrcParamOverrideSet */
        fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E;

        st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check unknown enum values */

        /* set all values , and then check that can get all those values */
        for(fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_IPV6_ADDR_24MSB_BY_24UDBS_E + 1 ;
            fieldType < 100;
            fieldType++)
        {
            st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, &getOverride);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* check special enum values */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkHashMaskCrcParamOverrideGet
                            (dev, fieldType, &getOverride),
                            fieldType);
    }

    fieldType = CPSS_DXCH_TRUNK_HASH_MASK_CRC_PARAM_OVERRIDE_UDB21_UDB22_BY_FLOW_ID_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, &getOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkHashMaskCrcParamOverrideGet(dev, fieldType, &getOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkCascadeTrunkPortsSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunkId [2],
                   valid portsMembersPtr.
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkCascadeTrunkPortsGet with the same trunkId.
    Expected: GT_OK and the same portsMembersPtr.
    1.3. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   and other valid parameters from 1.1.
    Expected: NON GT_OK.
    1.4. Call with out of range trunkId [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev)],
                   and other valid parameters from 1.1.
    Expected: NON GT_OK.
    1.5. Call with NULL portsMembersPtr,
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_TRUNK_ID         trunkId;
    GT_BOOL             isEqual         = GT_FALSE;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsMembersGet = {{0, 0}};
    GT_U32  ii,jj;
    CPSS_TRUNK_MEMBER_STC trunkMember;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&portsMembers,0,sizeof(portsMembers));
        /*
            1.1. Call with trunkId [2],
                           valid portsMembersPtr.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);

        /* fix coverity warning by check dev */
        if((dev) < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            if(0 == PRV_CPSS_DXCH_PP_HW_INFO_TXQ_REV_1_OR_ABOVE_MAC(dev))
            {
                /* since the device map table hold 8 entries , we can't set more then 8 port in the BMP !*/
                portsMembers.ports[0] = 0xAAAA & PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0];
            }
            else
            {
                /* since the device map table hold 64 entries , we can't set more then 64 ports in the BMP !*/
                ii = ((PRV_CPSS_PP_MAC(dev)->numOfPorts + 31) / 32) - 1;
                portsMembers.ports[ii] = PRV_CPSS_PP_MAC(dev)->existingPorts.ports[ii];
                if(ii) /* support device with less than 33 ports */
                {
                    ii--;
                    portsMembers.ports[ii] = PRV_CPSS_PP_MAC(dev)->existingPorts.ports[ii];
                }

                /* remove ports that already in pre-defined cascade trunk(s) */
                for(jj = 0 ; jj < CPSS_MAX_PORTS_NUM_CNS ; jj++)
                {
                    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsMembers,jj))
                    {
                        continue;
                    }

                    trunkMember.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(dev);
                    trunkMember.port = jj;
                    /* check if those ports are not members of some cascade trunk in the system */
                    st = cpssDxChTrunkDbIsMemberOfTrunk(dev,&trunkMember,NULL);
                    if(st == GT_OK)
                    {
                        /* port already in trunk ... remove it */
                        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembers,jj);
                    }
                }
            }
        }
        else
        {
            CPSS_PORTS_BMP_PORT_SET_ALL_MAC(&portsMembers);
        }

        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, &portsMembers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call cpssDxChTrunkCascadeTrunkPortsGet with the same trunkId.
            Expected: GT_OK and the same portsMembersPtr.
        */
        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsGet: %d", dev);

        /* Verifying values */
        isEqual = CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portsMembers,&portsMembersGet) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another value than was set: %d", dev);

        /*
            1.3. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           and other valid parameters from 1.1.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, &portsMembers);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.4. Call with out of range trunkId [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev)],
                           and other valid parameters from 1.1.
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, &portsMembers);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);
        /*
            1.5. Call with NULL portsMembersPtr,
                           and other valid parameters from 1.1.
            Expected: GT_OK.
        */
        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, portsMembersPtr = NULL", dev);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, &portsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkCascadeTrunkPortsSet(dev, trunkId, &portsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDbMcLocalSwitchingEnableGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbMcLocalSwitchingEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunk id [TRUNK_TESTED_TRUNK_ID_CNS(dev)].
    Expected: GT_OK.
    1.2. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].
    Expected: NON GT_OK.
    1.3. Check for NULL handling. Call with trunk id [2], enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId;
    GT_BOOL                 enableGet = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunk id [TRUNK_TESTED_TRUNK_ID_CNS(dev)].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbMcLocalSwitchingEnableGet(dev, trunkId, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128].
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkDbMcLocalSwitchingEnableGet(dev, trunkId, &enableGet);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Check for NULL handling. Call with trunk id [2], enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkDbMcLocalSwitchingEnableGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                            "%d, %d, enablePtr = NULL", dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. Call function for non active device and valid parameters.   */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbMcLocalSwitchingEnableGet(dev, trunkId, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbMcLocalSwitchingEnableGet(dev, trunkId, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkMcLocalSwitchingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMcLocalSwitchingEnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunk id [2] and enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].
    Expected: NON GT_OK.
    1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],
    Expected: NON GT_OK.
*/
    GT_STATUS     st = GT_OK;
    GT_U8         dev;
    GT_TRUNK_ID   trunkId;
    GT_BOOL       enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunk id [2] and enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /* call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with null trunk id [NULL_TRUNK_ID_CNS = 0].
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with out of range trunkId [GT_TRUNK_MAX_ID = 128],
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. Call function for non active device and valid parameters.   */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMcLocalSwitchingEnableSet(dev, trunkId, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDbMembersSortingEnableSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbMembersSortingEnableSet)
{
    GT_STATUS     st = GT_OK;
    GT_U8         dev;
    GT_BOOL       enable = GT_FALSE , enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChTrunkDbMembersSortingEnableSet(dev,  enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkDbMembersSortingEnableGet(dev,  &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(enable,enableGet);

        /* call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChTrunkDbMembersSortingEnableSet(dev,  enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkDbMembersSortingEnableGet(dev,  &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL0_PARAM_MAC(enable,enableGet);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. Call function for non active device and valid parameters.   */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkDbMembersSortingEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbMembersSortingEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDbMembersSortingEnableGet
(
    IN GT_U8                    devNum,
    OUT GT_BOOL                 *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbMembersSortingEnableGet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChTrunkDbMembersSortingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call with NULL portsMembersPtr,
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkDbMembersSortingEnableGet(dev,  NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsMembersPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkDbMembersSortingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbMembersSortingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* set the trunk mode to 'sorted' and then run next tests:
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMembersSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberDisable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberEnable)

*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)
{
    GT_STATUS     st = GT_OK;
    GT_U8         dev;
    GT_BOOL       enable , enableArr[] = {GT_TRUE , GT_FALSE , GT_TRUE , GT_FALSE};
    GT_U32        ii;

#ifdef _WIN32
    /* allow the GM under Linux run this test ! */

    /* set reduce time only for 'Members manipulations' */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC
#endif /*_WIN32*/


    for(ii = 0 ; ii < (sizeof(enableArr) / sizeof(enableArr[0])) ; ii++)
    {
        enable = enableArr[ii];

        sortMode = enable;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            st = cpssDxChTrunkDbMembersSortingEnableSet(dev,  enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        UTF_TEST_CALL_MAC(cpssDxChTrunkMembersSet);
        UTF_TEST_CALL_MAC(cpssDxChTrunkMemberAdd);
        UTF_TEST_CALL_MAC(cpssDxChTrunkMemberRemove);
        UTF_TEST_CALL_MAC(cpssDxChTrunkMemberDisable);
        UTF_TEST_CALL_MAC(cpssDxChTrunkMemberEnable);
    }

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkCascadeTrunkWithWeightedPortsSet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId = 7;
    GT_U32                  numOfMembers;
    CPSS_TRUNK_WEIGHTED_MEMBER_STC   membersArray[64];
    GT_U32                  membersNumGet;
    CPSS_TRUNK_WEIGHTED_MEMBER_STC   membersArrayGet[64];
    GT_U32                  ii,jj,kk;
    GT_U32               step = 1;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
    {
        step += 1;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], numOfEnabledMembers [5],                                         */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],                          */
        /* numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7].     */
        /* Expected: GT_OK.                                                                             */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        for(ii = 0 ; ii < sizeof(portsArrayWithSize)/sizeof(portsArrayWithSize[0]) ; ii+=step)
        {
            for(jj = 0 ; jj < portsArrayWithSize[ii].sizeofArray ;jj ++)
            {
                membersArray[jj].member.port = portsArrayWithSize[ii].portsArray[jj].port;
                membersArray[jj].member.hwDevice = TRUNK_HW_DEV_NUM_MAC(dev);
                membersArray[jj].weight = portsArrayWithSize[ii].portsArray[jj].weight;
            }
            numOfMembers = jj;

            if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
            {
                /* support ch3 XG : ports: 0,4,10,12,16,22, 24..27*/
                if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)
                {
                     membersArray[0].member.port = 0;
                     membersArray[1].member.port = 4;
                     membersArray[2].member.port = 10;
                     membersArray[3].member.port = 12;
                     membersArray[4].member.port = 16;
                     membersArray[5].member.port = 22;
                     membersArray[6].member.port = 24;
                     membersArray[7].member.port = 25;
                }
                else if(!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) && (PRV_CPSS_PP_MAC(dev)->numOfPorts <= 24))
                {
                    membersArray[0].member.port = 0;
                    membersArray[1].member.port = 2;
                    membersArray[2].member.port = 3;
                    membersArray[3].member.port = 5;
                    membersArray[4].member.port = 8;
                    membersArray[5].member.port = 10;
                    membersArray[6].member.port = 13;
                    membersArray[7].member.port = 15;
                }
            }

            /* replace ports that already in pre-defined cascade trunk(s) */
            st = mainUtTrunkReplaceUsedWeightedMembers(dev,trunkId,membersArray,numOfMembers);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, numOfMembers, membersArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            membersNumGet = sizeof(membersArray)/sizeof(membersArray[0]);

            st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, membersArrayGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

            /*check number of members */
            UTF_VERIFY_EQUAL2_STRING_MAC(numOfMembers, membersNumGet,
                       "get another numOfMembers than was set: %d, %d", dev, trunkId);

            if(numOfMembers == membersNumGet)
            {
                /* find the members */
                for(jj = 0 ; jj < membersNumGet; jj++)
                {
                    for(kk = 0 ; kk < numOfMembers; kk++)
                    {
                        if(membersArray[kk].member.port    == membersArrayGet[jj].member.port &&
                           membersArray[kk].member.hwDevice  == membersArrayGet[jj].member.hwDevice)
                        {
                            if(numOfMembers == 1)
                            {
                                /* for single member the weight is not relevant.*/
                            }
                            else
                            {
                                /* the member found - check the weight */
                                UTF_VERIFY_EQUAL0_PARAM_MAC(membersArray[kk].weight , membersArrayGet[jj].weight);
                            }
                            break;
                        }
                    }

                    if(kk == numOfMembers)
                    {
                        /* member was not found */
                        UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                    }
                }
            }/*if(numOfMembers == membersNumGet)*/
        }/*jj*/

        /* clear the trunk - for next tests */
        st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, 0, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    numOfMembers = 4;

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, numOfMembers, membersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, numOfMembers, membersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkCascadeTrunkWithWeightedPortsGet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_TRUNK_ID             trunkId = 7;
    GT_U32                  numOfMembers;
    CPSS_TRUNK_WEIGHTED_MEMBER_STC   membersArray[64];
    GT_U32                  membersNumGet;
    CPSS_TRUNK_WEIGHTED_MEMBER_STC   membersArrayGet[64];
    GT_U32                  ii,jj,kk;
    GT_U32               step = 1;
    GT_U32  numDesig;/* number of entries in the designated ports trunk table */
    GT_U32  factorIterator , factorArr[] = {1/*must be first*/ , 2 , 4 , 8  };

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
    {
        step += 1;
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with trunkId [2], numOfEnabledMembers [5],                                         */
        /* enabledMembersArray [tested dev id, virtual port id 0, 1, 2, 3, 4],                          */
        /* numOfDisabledMembers [3], disabledMembersArray [tested dev id, virtual port id 5, 6, 7].     */
        /* Expected: GT_OK.                                                                             */
        trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            numDesig = PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->numOfDesignatedTrunkEntriesHw;
        }
        else
            numDesig = 0;

        for(factorIterator = 0 ; factorIterator < (sizeof(factorArr)/sizeof(factorArr[0]));factorIterator++)
        {
            for(ii = 0 ; ii < sizeof(portsArrayWithSize)/sizeof(portsArrayWithSize[0]) ; ii+=step)
            {
                for(jj = 0 ; jj < portsArrayWithSize[ii].sizeofArray ;jj ++)
                {
                    membersArray[jj].member.port = portsArrayWithSize[ii].portsArray[jj].port;
                    membersArray[jj].member.hwDevice = TRUNK_HW_DEV_NUM_MAC(dev);
                    membersArray[jj].weight = portsArrayWithSize[ii].portsArray[jj].weight;

                    membersArray[jj].weight *= factorArr[factorIterator];
                }
                numOfMembers = jj;

                if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
                {
                   /* support ch3 XG : ports: 0,4,10,12,16,22, 24..27*/
                   if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)
                   {
                        membersArray[0].member.port = 0;
                        membersArray[1].member.port = 4;
                        membersArray[2].member.port = 10;
                        membersArray[3].member.port = 12;
                        membersArray[4].member.port = 16;
                        membersArray[5].member.port = 22;
                        membersArray[6].member.port = 24;
                        membersArray[7].member.port = 25;
                   }
                   else if(!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) && (PRV_CPSS_PP_MAC(dev)->numOfPorts <= 24))
                   {
                       membersArray[0].member.port = 0;
                       membersArray[1].member.port = 2;
                       membersArray[2].member.port = 3;
                       membersArray[3].member.port = 5;
                       membersArray[4].member.port = 8;
                       membersArray[5].member.port = 10;
                       membersArray[6].member.port = 13;
                       membersArray[7].member.port = 15;
                   }
                }

                /* replace ports that already in pre-defined cascade trunk(s) */
                st = mainUtTrunkReplaceUsedWeightedMembers(dev,trunkId,membersArray,numOfMembers);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

                st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, numOfMembers, membersArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

                membersNumGet = sizeof(membersArray)/sizeof(membersArray[0]);

                st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, membersArrayGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

                /*check number of members */
                UTF_VERIFY_EQUAL2_STRING_MAC(numOfMembers, membersNumGet,
                           "get another numOfMembers than was set: %d, %d", dev, trunkId);

                if(numOfMembers == membersNumGet)
                {
                    /* find the members */
                    for(jj = 0 ; jj < membersNumGet; jj++)
                    {
                        for(kk = 0 ; kk < numOfMembers; kk++)
                        {
                            if(membersArray[kk].member.port    == membersArrayGet[jj].member.port &&
                               membersArray[kk].member.hwDevice  == membersArrayGet[jj].member.hwDevice)
                            {
                                membersArray[kk].weight /= factorArr[factorIterator];

                                if(numOfMembers == 1)
                                {
                                    /* for single member the weight is not relevant.*/
                                }
                                else
                                {
                                    /* the member found - check the weight */
                                    UTF_VERIFY_EQUAL0_PARAM_MAC(membersArray[kk].weight , membersArrayGet[jj].weight);
                                }
                                break;
                            }
                        }

                        if(kk == numOfMembers)
                        {
                            /* member was not found */
                            UTF_VERIFY_EQUAL0_STRING_MAC(0 , 1,"member was not found");
                        }
                    }
                }/*if(numOfMembers == membersNumGet)*/

                if(numOfMembers > 3)
                {
                    membersNumGet = numOfMembers - 3;
                    st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, membersArrayGet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
                }

                st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, NULL, membersArrayGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, trunkId);

                st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, trunkId);

            }/*ii*/

            if(numDesig == 8 || numDesig == 16)
            {/*this device need to be checked only with single factor iterator , because only 8 entries */
                break;
            }
        }/*factorIterator*/

        /* clear the trunk - for next tests */
        st = cpssDxChTrunkCascadeTrunkWithWeightedPortsSet(dev, trunkId, 0, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    numOfMembers = 4;

    /*2. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* 2.1. <Call function for non active device and valid parameters>. */
        /* Expected: GT_BAD_PARAM.                                          */
        st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, membersArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of range device id.    */
    /* Expected: GT_BAD_PARAM.                          */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkCascadeTrunkWithWeightedPortsGet(dev, trunkId, &membersNumGet, membersArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkCascadeTrunkPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC       *portsMembersPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkCascadeTrunkPortsGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with trunkId [2],
                   valid portsMembersPtr.
    Expected: GT_OK.
    1.2. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                   and other valid parameters from 1.1.
    Expected: NON GT_OK.
    1.3. Call with out of range trunkId [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev)],
                   and other valid parameters from 1.1.
    Expected: NON GT_OK.
    1.4. Call with NULL portsMembersPtr,
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_TRUNK_ID         trunkId;
    CPSS_PORTS_BMP_STC  portsMembers = {{0, 0}};


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2],
                           valid portsMembersPtr.
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembers);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with null trunkId [NULL_TRUNK_ID_CNS = 0],
                           and other valid parameters from 1.1.
            Expected: NON GT_OK.
        */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembers);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.3. Call with out of range trunkId [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev)],
                           and other valid parameters from 1.1.
            Expected: NON GT_OK.
        */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        if(trunkId <= FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId))
        {
            trunkId = (GT_TRUNK_ID)(1 + FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId));
        }

        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembers);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
        trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);
        /*
            1.4. Call with NULL portsMembersPtr,
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, portsMembersPtr = NULL", dev);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    trunkId = FORCE_TRUNK_ID_FOR_CASCADE_TRUNK_MAC(trunkId);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkCascadeTrunkPortsGet(dev, trunkId, &portsMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkLearnPrioritySet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trunkId [2]
                   learnPriority [CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E /
                                  CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E]
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkLearnPriorityGet with same trunkId
                                                 not NULL learnPriorityPtr.
    Expected: GT_OK and the same learnPriority.
    1.3. Call with out of range trunkId [0 / 128],
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with learnPriority [wrong enum values].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_TRUNK_ID                         trunkId          = 0;
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriority    = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriorityGet = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2]
                           learnPriority [CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E /
                                          CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E]
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /* call with learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E */
        learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

        st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, learnPriority);

        /*
            1.2. Call cpssDxChTrunkLearnPriorityGet with same trunkId
                                                         not NULL learnPriorityPtr.
            Expected: GT_OK and the same learnPriority.
        */
        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriorityGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkLearnPriorityGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(learnPriority, learnPriorityGet,
                       "get another learnPriority than was set: %d", dev);

        /* call with learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E */
        learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E;

        st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, learnPriority);

        /*
            1.2. Call cpssDxChTrunkLearnPriorityGet with same trunkId
                                                         not NULL learnPriorityPtr.
            Expected: GT_OK and the same learnPriority.
        */
        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriorityGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkLearnPriorityGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(learnPriority, learnPriorityGet,
                       "get another learnPriority than was set: %d", dev);

        /*
            1.3. Call with out of range trunkId [0 / 128],
                           and other valid params.
            Expected: GT_OK.
        */

        /* call with trunkId = TRUNK_NULL_TRUNK_ID_CNS */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* call with trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* restore values */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /*
            1.4. Call with learnPriority [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkLearnPrioritySet
                            (dev, trunkId, learnPriority),
                            learnPriority);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkLearnPrioritySet(dev, trunkId, learnPriority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_TRUNK_ID                         trunkId,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkLearnPriorityGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trunkId [2]
                   non null learnPriorityPtr
    Expected: GT_OK.
    1.2. Call with out of range trunkId [0 / 128],
                   and other valid params.
    Expected: NON GT_OK.
    1.3. Call with learnPriorityPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_TRUNK_ID                         trunkId       = 0;
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2]
                           non null learnPriorityPtr
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with out of range trunkId [0 / 128],
                           and other valid params.
            Expected: GT_OK.
        */

        /* call with trunkId = TRUNK_NULL_TRUNK_ID_CNS */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* call with trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriority);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* restore values */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /*
            1.3. Call with learnPriorityPtr [NULL]
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, learnPriorityPtr = NULL", dev);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkLearnPriorityGet(dev, trunkId, &learnPriority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkMemberSelectionModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT  selectionMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberSelectionModeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with selectionMode [CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E = 0 /
                                  CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E /
                                  CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E]
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkMemberSelectionModeGet with not NULL selectionModePtr.
    Expected: GT_OK and the same selectionMode.
    1.3. Call with selectionMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   selectionMode;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   selectionModeGet;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with selectionMode [CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E = 0 /
                                          CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E /
                                          CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E]
            Expected: GT_OK.
        */
        /* selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;

        st = cpssDxChTrunkMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChTrunkMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /* selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E;

        st = cpssDxChTrunkMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChTrunkMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /* selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E */
        selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E;

        st = cpssDxChTrunkMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, selectionMode);

        /*
            1.2. Call cpssDxChTrunkMemberSelectionModeGet with not NULL selectionModePtr.
            Expected: GT_OK and the same selectionMode.
        */
        st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkMemberSelectionModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(selectionMode, selectionModeGet,
                       "get another selectionMode than was set: %d", dev);

        /*
            1.3. Call with selectionMode [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChTrunkMemberSelectionModeSet
                            (dev, selectionMode),
                            selectionMode);
    }

    selectionMode = CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkMemberSelectionModeSet(dev, selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberSelectionModeSet(dev, selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkMemberSelectionModeGet
(
    IN  GT_U8                                 devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT   *selectionModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberSelectionModeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non null selectionModePtr.
    Expected: GT_OK.
    1.2. Call with selectionModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT      selectionMode;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null selectionModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with selectionModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkMemberSelectionModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, selectionModePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkMemberSelectionModeGet(dev, &selectionMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           userGroup
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkUserGroupSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trunkId [2]
                   userGroup [0  / 7]
    Expected: GT_OK.
    1.2. Call cpssDxChTrunkUserGroupGet with same trunkId
                                             not NULL userGroupPtr.
    Expected: GT_OK and the same userGroup.
    1.3. Call with out of range trunkId [0 / 128],
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range userGroup [8],
                   and other valid params.
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_TRUNK_ID trunkId      = 0;
    GT_U32      userGroup    = 0;
    GT_U32      userGroupGet = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2]
                           userGroup [0  / 7]
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /* call with userGroup = 0 */
        userGroup = 0;

        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, userGroup);

        /*
            1.2. Call cpssDxChTrunkUserGroupGet with same trunkId
                                                     not NULL userGroupPtr.
            Expected: GT_OK and the same userGroup.
        */
        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroupGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkUserGroupGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(userGroup, userGroupGet,
                       "get another userGroup than was set: %d", dev);

        /* call with userGroup = 7 */
        userGroup = 7;

        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, userGroup);

        /*
            1.2. Call cpssDxChTrunkUserGroupGet with same trunkId
                                                     not NULL userGroupPtr.
            Expected: GT_OK and the same userGroup.
        */
        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroupGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChTrunkUserGroupGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(userGroup, userGroupGet,
                       "get another userGroup than was set: %d", dev);

        /*
            1.3. Call with out of range trunkId [0 / 128],
                           and other valid params.
            Expected: GT_OK.
        */

        /* call with trunkId = TRUNK_NULL_TRUNK_ID_CNS */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* call with trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* restore values */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /*
            1.4. Call with out of range userGroup [8],
                           and other valid params.
            Expected: NON GT_OK.
        */
        userGroup = 8;

        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, trunkId, userGroup);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    userGroup = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkUserGroupSet(dev, trunkId, userGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkUserGroupGet
(
    IN  GT_U8           devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *userGroupPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkUserGroupGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with trunkId [2]
                   non null userGroupPtr
    Expected: GT_OK.
    1.2. Call with out of range trunkId [0 / 128],
                   and other valid params.
    Expected: NON GT_OK.
    1.3. Call with userGroupPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_TRUNK_ID trunkId   = 0;
    GT_U32      userGroup = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with trunkId [2]
                           non null userGroupPtr
            Expected: GT_OK.
        */
        trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /*
            1.2. Call with out of range trunkId [0 / 128],
                           and other valid params.
            Expected: GT_OK.
        */

        /* call with trunkId = TRUNK_NULL_TRUNK_ID_CNS */
        trunkId = TRUNK_NULL_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* call with trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroup);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);

        /* restore values */
        trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        /*
            1.3. Call with userGroupPtr [NULL]
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTrunkUserGroupGet(dev, trunkId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, userGroupPtr = NULL", dev);
    }

    trunkId = TRUNK_TESTED_TRUNK_ID_CNS(dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkUserGroupGet(dev, trunkId, &userGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkPortMcEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkPortMcEnableSet)
{
/*
    ITERATE_DEVICES (xCat3)
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssDxChTrunkDbPortMcEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = TRUNK_PORT_VALID_PHY_PORT_CNS;
    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT     mode    = GT_FALSE;
    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT     modeGet = GT_FALSE;

    GT_U32  nonSupportedDevices;

    nonSupportedDevices = UTF_CH1_E | UTF_CH1_DIAMOND_E |
                          UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                          UTF_XCAT2_E | UTF_LION2_E;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, nonSupportedDevices);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with mode = CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_NATIVE_WEIGHTED_E] */
            for(mode = 0 ; mode < 2 ; mode++)
            {
                st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                /* 1.1.2. Call cpssDxChTrunkDbPortMcEnableGet.
                   Expected: GT_OK and the same enable.
                */
                st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &modeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "[cpssDxChTrunkDbPortMcEnableGet]: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                                             "get another enable value than was set: %d, %d", dev, port);
            }

            /* restore default mode*/
            mode = CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_NATIVE_WEIGHTED_E;

            st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        mode = CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORBID_ALL_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
        if (0 == TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORBID_ALL_E;
    port = TRUNK_PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, nonSupportedDevices);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChTrunkPortMcEnableSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTrunkDbPortMcEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbPortMcEnableGet)
{
    /*
    ITERATE_DEVICES (xCat3)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = TRUNK_PORT_VALID_PHY_PORT_CNS;
    CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT     mode = GT_FALSE;
    GT_U32  nonSupportedDevices;

    nonSupportedDevices = UTF_CH1_E | UTF_CH1_DIAMOND_E |
                          UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E |
                          UTF_XCAT2_E | UTF_LION2_E;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, nonSupportedDevices);


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssDxChTrunkDbPortMcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
        if (0 == TRUNK_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = TRUNK_PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, nonSupportedDevices );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbPortMcEnableGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static void  generateTrunk(
    IN GT_U8  dev ,
    IN GT_TRUNK_ID             trunkId
)
{
    GT_STATUS st;
    GT_HW_DEV_NUM           hwDevNum;
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  numOfDisabledMembers;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];

    hwDevNum = TRUNK_HW_DEV_NUM_MAC(dev);

    numOfEnabledMembers = 5;
    enabledMembersArray[0].hwDevice = hwDevNum;
    enabledMembersArray[0].port = 4;
    enabledMembersArray[1].hwDevice = hwDevNum;
    enabledMembersArray[1].port = 1;
    enabledMembersArray[2].hwDevice = hwDevNum;
    enabledMembersArray[2].port = 3;
    enabledMembersArray[3].hwDevice = hwDevNum;
    enabledMembersArray[3].port = 2;
    enabledMembersArray[4].hwDevice = hwDevNum;
    enabledMembersArray[4].port = 0;

    numOfDisabledMembers = 3;
    disabledMembersArray[0].hwDevice = hwDevNum;
    disabledMembersArray[0].port = 6;
    disabledMembersArray[1].hwDevice = hwDevNum;
    disabledMembersArray[1].port = 5;
    disabledMembersArray[2].hwDevice = hwDevNum;
    disabledMembersArray[2].port = 7;

    if(dev < PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* support for CH3 device with 10 XG ports */
        if(PRV_CPSS_PP_MAC(dev)->existingPorts.ports[0] == 0xF411411)/*ports: 0,4,10,12,16,22, 24..27*/
        {
            enabledMembersArray[1].port = 16/*1*/;
            enabledMembersArray[2].port = 10/*3*/;
            enabledMembersArray[3].port = 22/*2*/;

            disabledMembersArray[0].port = 26/*6*/;
            disabledMembersArray[1].port = 25/*5*/;
            disabledMembersArray[2].port = 27/*7*/;
        }
    }

    st = cpssDxChTrunkMembersSet(dev, trunkId, numOfEnabledMembers, enabledMembersArray,
                                 numOfDisabledMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkId);
}


static void check_trunk(
    IN GT_U8    dev,
    IN GT_TRUNK_ID  trunkId,
    IN GT_STATUS    expectedSt
)
{
    GT_STATUS st;
    GT_U32                  numOfEnabledMembers;
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32                  numOfDisabledMembers;
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    CPSS_TRUNK_MEMBER_STC   usedMember;

    numOfEnabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    /* check that the API is valid (or not) */
    st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId, &numOfEnabledMembers, enabledMembersArray);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

    if(st == GT_OK && numOfEnabledMembers)
    {
        usedMember = enabledMembersArray[0];
    }
    else
    {
        cpssOsMemSet(&usedMember,0,sizeof(usedMember));
    }

    numOfDisabledMembers = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    /* check that the API is valid (or not) */
    st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId, &numOfDisabledMembers, disabledMembersArray);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

    /* check that the API is valid (or not) */
    st = cpssDxChTrunkMemberRemove(dev, trunkId , &usedMember);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

    /* check that the API is valid (or not) */
    st = cpssDxChTrunkMemberAdd(dev, trunkId , &usedMember);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

    /* check that the API is valid (or not) */
    st = cpssDxChTrunkMemberDisable(dev, trunkId , &usedMember);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

    /* check that the API is valid (or not) */
    st = cpssDxChTrunkMemberEnable(dev, trunkId , &usedMember);
    UTF_VERIFY_EQUAL2_PARAM_MAC(expectedSt, st, dev, trunkId);

}

static void test_trunkDestroy(IN GT_U8   dev)
{
    GT_STATUS st;
    GT_TRUNK_ID  trunkId;

    /* get trunkId to work with */
    trunkId = (GT_TRUNK_ID)(TRUNK_TESTED_TRUNK_ID_CNS(dev) + 3 * dev);
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* generate trunk before destroy the trunk LIB */
    generateTrunk(dev,trunkId);

    /* check that the trunk is OK */
    check_trunk(dev,trunkId,GT_OK);

    /* destroy the current trunk LIB */
    st = cpssDxChTrunkDestroy(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* check that the 'init info' state 'GT_NOT_INITIALIZED' */
    st = cpssDxChTrunkDbInitInfoGet(dev,NULL,NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

    /* check that the trunk is not OK , return 'GT_NOT_INITIALIZED' */
    check_trunk(dev,trunkId,GT_NOT_INITIALIZED);
}

/*
GT_STATUS cpssDxChTrunkDestroy
(
    IN  GT_U8                                devNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDestroy)
{
/*
    after destroying the trunk LIB we need to reconstruct it.
    so get the orig values before destroy
    using cpssDxChTrunkDbInitInfoGet()
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    maxNumberOfTrunks;
    GT_U32    testedInitNum;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get original values so we can re-init the trunk LIB after destroy */
        st = cpssDxChTrunkDbInitInfoGet(dev,&maxNumberOfTrunks,&trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test the destroy */
        test_trunkDestroy(dev);

        testedInitNum = maxNumberOfTrunks / 2;
        if(PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(dev) &&
            testedInitNum < 68)
        {
            /* the trunkIds starts at 66 (represents 'trunkId 1') , 68 for 'trunkId 2' ..*/
            /* since trunkId '2' is used ... need to allow at least 68 trunks */
            testedInitNum = 68;
        }

        /* do initialization with 1/2 the number of trunks */
        st = cpssDxChTrunkInit(dev, testedInitNum, trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test the destroy */
        test_trunkDestroy(dev);

        testedInitNum = (maxNumberOfTrunks * 3) / 4;
        if(PRV_CPSS_DXCH_CHECK_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_MAC(dev) &&
            testedInitNum < 68)
        {
            /* the trunkIds starts at 66 (represents 'trunkId 1') , 68 for 'trunkId 2' ..*/
            /* since trunkId '2' is used ... need to allow at least 68 trunks */
            testedInitNum = 68;
        }

        /* do initialization with 3/4 the number of trunks */
        st = cpssDxChTrunkInit(dev, testedInitNum, trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test the destroy */
        test_trunkDestroy(dev);

        /* restore original init values */
        st = cpssDxChTrunkInit(dev, maxNumberOfTrunks , trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDestroy(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChTrunkDbInitInfoGet
(
    IN   GT_U8       devNum,
    OUT  GT_U32      *maxNumberOfTrunksPtr,
    OUT  CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT *trunkMembersModePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbInitInfoGet)
{
/*
    after destroying the trunk LIB we need to reconstruct it.
    so get the orig values before destroy
    using cpssDxChTrunkDbInitInfoGet()
*/
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    maxNumberOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT trunkMembersMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* ok with valid pointer */
        st = cpssDxChTrunkDbInitInfoGet(dev,&maxNumberOfTrunks,&trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* ok with NULL pointer on pointer 1*/
        st = cpssDxChTrunkDbInitInfoGet(dev,NULL,&trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* ok with NULL pointer on pointer 2*/
        st = cpssDxChTrunkDbInitInfoGet(dev,&maxNumberOfTrunks,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* ok with NULL pointer on pointer 1,2 */
        st = cpssDxChTrunkDbInitInfoGet(dev,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkDbInitInfoGet(dev,NULL,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbInitInfoGet(dev,NULL,NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChTrunkEcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    IN CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkEcmpLttTableSet)
{
    GT_STATUS st = GT_OK;
    GT_STATUS stExpected;

    GT_U8     dev;
    GT_TRUNK_ID                          trunkId;
    CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC   ecmpLttInfo;
    CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC   get_ecmpLttInfo;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      l2EcmpNumMembers;

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    cpssOsMemSet(&ecmpLttInfo,0,sizeof(ecmpLttInfo));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;

        trunkId = 1;
        /* check NULL pointer */
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        /* check boundary values */
        ecmpLttInfo.ecmpNumOfPaths = 0;/*bad param*/
        ecmpLttInfo.ecmpStartIndex = 0;
        ecmpLttInfo.ecmpEnable = GT_TRUE;
        ecmpLttInfo.ecmpRandomPathEnable = GT_FALSE;

        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ecmpLttInfo.ecmpNumOfPaths = _4K+1;/*bad param*/
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ecmpLttInfo.ecmpNumOfPaths = _4K;/*good*/
        if ((ecmpLttInfo.ecmpStartIndex + ecmpLttInfo.ecmpNumOfPaths) > l2EcmpNumMembers)
        {
            ecmpLttInfo.ecmpNumOfPaths = l2EcmpNumMembers - ecmpLttInfo.ecmpStartIndex;
        }
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        ecmpLttInfo.ecmpStartIndex = l2EcmpNumMembers - ecmpLttInfo.ecmpNumOfPaths;/*good*/
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*(ecmpStartIndex+ecmpNumOfPaths)>l2EcmpNumMembers*/
        ecmpLttInfo.ecmpStartIndex++;
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        ecmpLttInfo.ecmpNumOfPaths = _2K;/*good*/
        /*(ecmpStartIndex+ecmpNumOfPaths)>l2EcmpNumMembers*/
        if (ecmpLttInfo.ecmpNumOfPaths > l2EcmpNumMembers)
        {
            ecmpLttInfo.ecmpNumOfPaths = l2EcmpNumMembers;
        }
        ecmpLttInfo.ecmpStartIndex = 1 + (l2EcmpNumMembers - ecmpLttInfo.ecmpNumOfPaths);
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        for(trunkId = 0 ;
            trunkId <= (GT_TRUNK_ID)PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum ;
            trunkId ++)
        {
            ecmpLttInfo.ecmpStartIndex = cpssOsRand() % (l2EcmpNumMembers - 10);
            ecmpLttInfo.ecmpNumOfPaths = cpssOsRand() % (l2EcmpNumMembers - 10) % (_4K);

            if(ecmpLttInfo.ecmpNumOfPaths == 0)
            {
                stExpected = GT_BAD_PARAM;
            }
            else
            if((ecmpLttInfo.ecmpStartIndex + ecmpLttInfo.ecmpNumOfPaths) > l2EcmpNumMembers)
            {
                stExpected = GT_BAD_PARAM;
            }
            else
            {
                stExpected = GT_OK;
            }

            ecmpLttInfo.ecmpEnable           = (trunkId & 1)        ? GT_TRUE : GT_FALSE;
            ecmpLttInfo.ecmpRandomPathEnable = ((trunkId & 7) >= 6) ? GT_TRUE : GT_FALSE;

            st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(stExpected, st, dev);

            if(stExpected != GT_OK)
            {
                if(ecmpLttInfo.ecmpNumOfPaths == 0)
                {
                    ecmpLttInfo.ecmpNumOfPaths = 1;
                }
                else
                {
                    ecmpLttInfo.ecmpNumOfPaths =  l2EcmpNumMembers - ecmpLttInfo.ecmpStartIndex - 1;
                }

                /* should be OK now */
                st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /* check that the 'get' is OK and return good values */
            st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,&get_ecmpLttInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* expecting to get same values as we set */
            UTF_VERIFY_EQUAL0_PARAM_MAC(ecmpLttInfo.ecmpStartIndex      , get_ecmpLttInfo.ecmpStartIndex      );
            UTF_VERIFY_EQUAL0_PARAM_MAC(ecmpLttInfo.ecmpNumOfPaths      , get_ecmpLttInfo.ecmpNumOfPaths      );
            UTF_VERIFY_EQUAL0_PARAM_MAC(ecmpLttInfo.ecmpEnable          , get_ecmpLttInfo.ecmpEnable          );
            UTF_VERIFY_EQUAL0_PARAM_MAC(ecmpLttInfo.ecmpRandomPathEnable, get_ecmpLttInfo.ecmpRandomPathEnable);

            if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
            {
                /* decrement the number of iterations */
                trunkId += 10;
            }
        }

        /* this trunkId not supported */
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    trunkId = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkEcmpLttTableSet(dev,trunkId,&ecmpLttInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChTrunkEcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN GT_TRUNK_ID                          trunkId,
    OUT CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkEcmpLttTableGet)
{
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_TRUNK_ID                          trunkId;
    CPSS_DXCH_TRUNK_ECMP_LTT_ENTRY_STC   get_ecmpLttInfo;
    GT_U32      notAppFamilyBmp = 0;

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(trunkId = 0 ;
            trunkId <= (GT_TRUNK_ID)PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum ;
            trunkId ++)
        {
            /* check NULL pointer */
            st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /* check valid entry */
            st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,&get_ecmpLttInfo);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
            {
                /* decrement the number of iterations */
                trunkId += 12;
            }
        }

        /* this trunkId not supported */
        st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,&get_ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    trunkId = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,&get_ecmpLttInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkEcmpLttTableGet(dev,trunkId,&get_ecmpLttInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChTrunkFlexInfoSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_U32           l2EcmpStartIndex,
    IN GT_U32           maxNumOfMembers
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkFlexInfoSet)
{
    GT_STATUS st = GT_OK;
    GT_STATUS stExpected;

    GT_U8     dev;
    GT_TRUNK_ID                          trunkId;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      l2EcmpStartIndex,maxNumOfMembers;
    GT_U32      get_l2EcmpStartIndex,get_maxNumOfMembers;
    GT_U32      l2EcmpNumMembers;

    GT_U32    orig_maxNumberOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT orig_trunkMembersMode;

    GT_U32      maxTrunksInDev;
    GT_U32      maxNumberOfTrunks;

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;
        maxTrunksInDev   = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum;

        L2_ECMP_NULL_PORT_BOOKMARK
        l2EcmpNumMembers--;

        /* ok with valid pointer */
        st = cpssDxChTrunkDbInitInfoGet(dev,&orig_maxNumberOfTrunks,&orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(orig_trunkMembersMode != CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E)
        {
            for(trunkId = 1 ;
                trunkId <= (GT_TRUNK_ID)orig_maxNumberOfTrunks;
                trunkId += 50)
            {
                l2EcmpStartIndex = (trunkId * 3) % l2EcmpNumMembers;
                maxNumOfMembers  = trunkId % _4K;

                while((l2EcmpStartIndex + maxNumOfMembers) > l2EcmpNumMembers)
                {
                    if(l2EcmpStartIndex > 10)
                    {
                        l2EcmpStartIndex -= 10;
                    }

                    if(maxNumOfMembers > 33)
                    {
                        maxNumOfMembers -= 33;
                    }
                }

                /* we expect GT_BAD_STATE */
                st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

                /* we expect GT_NOT_INITIALIZED */
                st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
            }

            st = cpssDxChTrunkFlexInfoSet(dev,TRUNK_NULL_TRUNK_ID_CNS,1,1);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

            st = cpssDxChTrunkFlexInfoSet(dev,TRUNK_ID_ALL_CNS,0,4/*maxNumOfMembers*/);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, dev);

            /* now we can destroy the trunk LIB and generate new init with flex mode */
            st = cpssDxChTrunkDestroy(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChTrunkInit(dev , maxTrunksInDev , CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            maxNumberOfTrunks = maxTrunksInDev;
        }
        else
        {
            maxNumberOfTrunks = orig_maxNumberOfTrunks;
        }

        for(trunkId = 1 ;
            trunkId <= (GT_TRUNK_ID)maxNumberOfTrunks;
            trunkId ++)
        {
            l2EcmpStartIndex = cpssOsRand() % (l2EcmpNumMembers - 10);
            maxNumOfMembers  = cpssOsRand() % (l2EcmpNumMembers - 10) % _4K;

            if((l2EcmpStartIndex + maxNumOfMembers) > l2EcmpNumMembers)
            {
                stExpected = GT_OUT_OF_RANGE;
            }
            else
            {
                stExpected = GT_OK;
            }

            st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(stExpected, st, dev);

            if(stExpected != GT_OK)
            {
                maxNumOfMembers =  l2EcmpNumMembers - l2EcmpStartIndex - 1;

                /* should be OK now */
                st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /* check that the 'get' is OK and return good values */
            st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(l2EcmpStartIndex      , get_l2EcmpStartIndex );
            UTF_VERIFY_EQUAL0_PARAM_MAC(maxNumOfMembers       , get_maxNumOfMembers  );

            if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
            {
                /* decrement the number of iterations */
                trunkId += 10;
            }
        }

        l2EcmpStartIndex = 0;
        maxNumOfMembers = 1;
        /* this trunkId not supported */
        st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* NULL trunkId not supported */
        st = cpssDxChTrunkFlexInfoSet(dev,TRUNK_NULL_TRUNK_ID_CNS,1,1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /* now we can destroy the trunk LIB and generate new init with orig mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkInit(dev , orig_maxNumberOfTrunks,orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    trunkId = 1;
    l2EcmpStartIndex = 0;
    maxNumOfMembers = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChTrunkDbFlexInfoGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_U32          *l2EcmpStartIndexPtr,
    OUT GT_U32          *maxNumOfMembersPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkDbFlexInfoGet)
{
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_TRUNK_ID                          trunkId;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      l2EcmpStartIndex,maxNumOfMembers;
    GT_U32      get_l2EcmpStartIndex,get_maxNumOfMembers;
    GT_U32      l2EcmpNumMembers;

    GT_U32    orig_maxNumberOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT orig_trunkMembersMode;

    GT_U32      maxTrunksInDev;
    GT_U32      maxNumberOfTrunks;

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;
        maxTrunksInDev   = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum;

        L2_ECMP_NULL_PORT_BOOKMARK
        l2EcmpNumMembers--;

        st = cpssDxChTrunkDbInitInfoGet(dev,&orig_maxNumberOfTrunks,&orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(orig_trunkMembersMode != CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E)
        {
            for(trunkId = 1 ;
                trunkId <= (GT_TRUNK_ID)orig_maxNumberOfTrunks;
                trunkId += 50)
            {
                /* we expect GT_NOT_INITIALIZED */
                st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

                /* we expect GT_BAD_PTR */
                st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,NULL,&get_maxNumOfMembers);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

                /* we expect GT_BAD_PTR */
                st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,NULL);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
            }

            st = cpssDxChTrunkDbFlexInfoGet(dev,TRUNK_ID_ALL_CNS,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);

            /* now we can destroy the trunk LIB and generate new init with flex mode */
            st = cpssDxChTrunkDestroy(dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChTrunkInit(dev , maxTrunksInDev , CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            maxNumberOfTrunks = maxTrunksInDev;
        }
        else
        {
            maxNumberOfTrunks = orig_maxNumberOfTrunks;
        }

        for(trunkId = 1 ;
            trunkId <= (GT_TRUNK_ID)maxNumberOfTrunks;
            trunkId ++)
        {
            l2EcmpStartIndex = (trunkId * 3) % l2EcmpNumMembers;
            maxNumOfMembers  = trunkId % _4K;

            while((l2EcmpStartIndex + maxNumOfMembers) > l2EcmpNumMembers)
            {
                if(l2EcmpStartIndex > 10)
                {
                    l2EcmpStartIndex -= 10;
                }

                if(maxNumOfMembers > 33)
                {
                    maxNumOfMembers -= 33;
                }
            }

            st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* check that the 'get' is OK and return good values */
            st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL0_PARAM_MAC(l2EcmpStartIndex      , get_l2EcmpStartIndex );
            UTF_VERIFY_EQUAL0_PARAM_MAC(maxNumOfMembers       , get_maxNumOfMembers  );

            if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
            {
                /* decrement the number of iterations */
                trunkId += 10;
            }
        }

        /* this trunkId not supported */
        st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* now we can destroy the trunk LIB and generate new init with orig mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkInit(dev , orig_maxNumberOfTrunks,orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    trunkId = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTrunkDbFlexInfoGet(dev,trunkId,&get_l2EcmpStartIndex,&get_maxNumOfMembers);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* 'orig' array to set members : initialized once */
/*not static*/CPSS_TRUNK_MEMBER_STC   membersArray_orig_4K_tests[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
/* array to set members : may copy from 'orig' and do modifications */
/*not static*/CPSS_TRUNK_MEMBER_STC   membersArray_set_4K_tests[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
/* array to get members */
static CPSS_TRUNK_MEMBER_STC   membersArray_get_4K_tests[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];

static CPSS_TRUNK_MEMBER_STC   membersArray_set1_4K_tests[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];

static CPSS_TRUNK_MEMBER_STC   membersArray_expected_enabled_tests [CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
static CPSS_TRUNK_MEMBER_STC   membersArray_expected_disabled_tests[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];



/*not static*/void init_4k_members(
    IN GT_U8   dev
)
{
    static GT_BOOL  alreadyInit = GT_FALSE;
    GT_U32  ii;
    CPSS_TRUNK_MEMBER_STC   *memberPtr;
    GT_U32  devMask;
    GT_U32  portMask;
    GT_U32  myDevPortMask;
    GT_U32  section;
    GT_U32  ownHwDevNum;
    GT_U32  currentPortMask;

    if(alreadyInit == GT_TRUE)
    {
        /* already initialized */
        return;
    }

    portMask = PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->validityMask.port;
    devMask  = PRV_CPSS_DEV_TRUNK_INFO_MAC(dev)->validityMask.hwDevice;

    /* limitation on 'own devNum' because trunk members are set into
       'per physical port' (TTI,HA) */
    myDevPortMask = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev) - 1;
    ownHwDevNum   = PRV_CPSS_HW_DEV_NUM_MAC(dev);


    memberPtr = &membersArray_orig_4K_tests[0];
    /* fill the orig array with members */
    for(ii = 0; ii < _4K ; ii++ , memberPtr++)
    {
        memberPtr->hwDevice = (devMask - ii) & (devMask);

        section = (ii / devMask);

        if(ownHwDevNum == memberPtr->hwDevice)
        {
            currentPortMask = myDevPortMask;
        }
        else
        {
            currentPortMask = portMask;
        }

        memberPtr->port     = (ii + section) & currentPortMask;
        if(PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(memberPtr->hwDevice))
        {
            /* support the existence of Lion2 in the system */
            /* like 2Lion2 + Bobcat2_B0 */
            if(memberPtr->hwDevice & 1)
            {
                memberPtr->hwDevice --;

                memberPtr->port = (memberPtr->port*2 + 1) & portMask;
            }
        }

        if(memberPtr->port == CPSS_CPU_PORT_NUM_CNS ||
           memberPtr->port == CPSS_NULL_PORT_NUM_CNS)
        {
            /*
                those are special port numbers that ignores the 'hwDevice' and
                may cause 'trap' or 'drop' on the local device .

                so need to use different value for those ports
            */

            /* jumping the port to higher section */
            memberPtr->port += (currentPortMask / 2) + 1;
        }

    }
}


static void main_cpssDxChTrunkMembersSet_1K_members(
    IN GT_U8    dev ,
    IN GT_TRUNK_ID trunkId,
    IN GT_U32      l2EcmpNumMembers
    )
{
    GT_STATUS st = GT_OK;
    GT_U32      ii,jj;
    GT_U32      get_numOfMembers;
    GT_U32      numOfEnabledMembers;
    GT_U32      numOfDisabledMembers;
    GT_U32  secondsEnd,nanoSecondsEnd,secondsStart,nanoSecondsStart;
    GT_U32  set_index;
    GT_U32  maxMembers = _1K; /* use 1K instead of 4K to reduce time by factor 4 */

    if (maxMembers > l2EcmpNumMembers)
    {
        maxMembers = l2EcmpNumMembers;
    }
    for(ii = 0 ; ii < maxMembers ; ii+= ((maxMembers/4) + (maxMembers/4)/2 - 1))
    {
        numOfEnabledMembers  = maxMembers - ii;
        numOfDisabledMembers = ii ;

        /* keep alive */
        cpssOsPrintf("Going to : cpssDxChTrunkMembersSet with : [%d] members [%d] enabled , [%d] disabled\n",
            numOfEnabledMembers + numOfDisabledMembers,
            numOfEnabledMembers,
            numOfDisabledMembers);

        cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

        /* start manipulation on the trunk */
        st = cpssDxChTrunkMembersSet(dev,trunkId,
                    numOfEnabledMembers,
                    &membersArray_set_4K_tests[0],/*enabled members in the start*/
                    numOfDisabledMembers,
                    &membersArray_set_4K_tests[numOfEnabledMembers]);/* disabled members at the end */
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
        cpssOsPrintf("ended iteration : took [%d] seconds \n",
            secondsEnd-secondsStart);

        /* check with the DB the enabled members */
        get_numOfMembers = _4K;
        st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId,
            &get_numOfMembers,
            &membersArray_get_4K_tests[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* compare the number */
        UTF_VERIFY_EQUAL0_PARAM_MAC(numOfEnabledMembers , get_numOfMembers);

        /* compare the members */
        for(jj = 0 ; jj < get_numOfMembers; jj++)
        {
            set_index = jj;

            UTF_VERIFY_EQUAL1_PARAM_MAC(membersArray_set_4K_tests[set_index].port     ,
                                        membersArray_get_4K_tests[jj].port,
                                        jj);
            UTF_VERIFY_EQUAL1_PARAM_MAC(membersArray_set_4K_tests[set_index].hwDevice ,
                                        membersArray_get_4K_tests[jj].hwDevice,
                                        jj);
        }

        /* check with the DB the disabled members */
        get_numOfMembers = _4K;
        st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId,
            &get_numOfMembers,
            &membersArray_get_4K_tests[0]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* compare the number */
        UTF_VERIFY_EQUAL0_PARAM_MAC(numOfDisabledMembers , get_numOfMembers);

        /* compare the members */
        for(jj = 0 ; jj < get_numOfMembers; jj++)
        {
            set_index = jj + numOfEnabledMembers;

            UTF_VERIFY_EQUAL1_PARAM_MAC(membersArray_set_4K_tests[set_index].port     ,
                                        membersArray_get_4K_tests[jj].port,
                                        jj);
            UTF_VERIFY_EQUAL1_PARAM_MAC(membersArray_set_4K_tests[set_index].hwDevice ,
                                        membersArray_get_4K_tests[jj].hwDevice,
                                        jj);
        }

        if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
        {
            /* reduce number of iterations */
            break;
        }

    }
}

/*
    test for : cpssDxChTrunkMembersSet with 4K members
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMembersSet_1K_members)
{
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    numOfApplicableDevices = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      l2EcmpNumMembers;
    GT_U32      maxTrunksInDev;
    GT_U32      l2EcmpStartIndex,maxNumOfMembers;

    GT_U32    orig_maxNumberOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT orig_trunkMembersMode;
    GT_TRUNK_ID trunkId;
#ifdef CPSS_LOG_ENABLE
    GT_BOOL logWasOpen;
#endif

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* skip test for EMULATOR when running 'skip long tests' */
    PRV_TGF_SKIP_LONG_TEST_EMULATOR_MAC(UTF_ALL_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfApplicableDevices++;

        init_4k_members(dev);

        l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;
        L2_ECMP_NULL_PORT_BOOKMARK
        l2EcmpNumMembers--;

        maxTrunksInDev   = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum;

        st = cpssDxChTrunkDbInitInfoGet(dev,&orig_maxNumberOfTrunks,&orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* destroy the trunk LIB so we can set flex mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*set (4K-1) trunks in flex mode */
        st = cpssDxChTrunkInit(dev , maxTrunksInDev,CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        trunkId = (GT_TRUNK_ID)maxTrunksInDev;

        /* set to use the end of the 'L2 ECMP' */
        maxNumOfMembers  = (l2EcmpNumMembers > _4K) ? _4K : l2EcmpNumMembers;
        l2EcmpStartIndex = l2EcmpNumMembers - maxNumOfMembers;

        /* set this trunk to be operational */
        st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* copy to 'set' array from the 'orig' array */
        cpssOsMemCpy(membersArray_set_4K_tests,
                     membersArray_orig_4K_tests,
                     sizeof(membersArray_orig_4K_tests));

#ifdef CPSS_LOG_ENABLE
        logWasOpen = cpssLogEnableGet();
        if(GT_FALSE == logWasOpen)
        {
            logWasOpen = GT_FALSE;
            cpssLogEnableSet(GT_TRUE);
            cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,GT_TRUE);
        }
#endif

        /* run the 'main' of the test . it is in separate function to allow
           'restore' of 'destroy+init' even if fail at the 'main' function */
        main_cpssDxChTrunkMembersSet_1K_members(dev,trunkId,l2EcmpNumMembers);

#ifdef CPSS_LOG_ENABLE
        if(logWasOpen == GT_FALSE)
        {
            cpssLogEnableSet(GT_FALSE);
        }
#endif

        /* now we can destroy the trunk LIB and generate new init with orig mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkInit(dev , orig_maxNumberOfTrunks,orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    if(numOfApplicableDevices == 0)
    {
        /* state that this test is not relevant to this system */
        SKIP_TEST_MAC
    }

    return;
}

/* search for member in array */
static GT_BOOL isMemberExistsInArr(
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr,
    IN CPSS_TRUNK_MEMBER_STC    membersArr[],
    IN GT_U32                   numOfMembers
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numOfMembers; ii++)
    {
        if(membersArr[ii].port     == memberPtr->port &&
           membersArr[ii].hwDevice == memberPtr->hwDevice )
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/* remove the for member from array , and update last index */
/* remove done by moving last index to replace the removed one */
static GT_BOOL removeMemberExistsInArr(
    IN    CPSS_TRUNK_MEMBER_STC    *memberPtr,
    INOUT CPSS_TRUNK_MEMBER_STC    membersArr[],
    INOUT GT_U32                   *numOfMembersPtr
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < (*numOfMembersPtr); ii++)
    {
        if(membersArr[ii].port     == memberPtr->port &&
           membersArr[ii].hwDevice == memberPtr->hwDevice )
        {
            if((*numOfMembersPtr) > 0)
            {
                (*numOfMembersPtr)--;
            }
            /* move last member to replace the removed one */
            membersArr[ii] = membersArr[(*numOfMembersPtr)];

            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/* check that DB members hold the enabled members and the disabled members */
/* check that low level HW members hold the enabled members and not the disabled members */
static void checkExpectedAndActualMembers(
    IN  GT_U8   dev,
    IN  GT_TRUNK_ID trunkId,
    IN  GT_U32  expected_db_numOfEnabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   expected_db_EnabledMembersArr[],
    IN  GT_U32  expected_db_numOfDisabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   expected_db_DisabledMembersArr[]
)
{
    GT_STATUS st = GT_OK;

    GT_U32      get_numOfMembers;
    GT_U32  set_index;
    GT_U32  jj;

    /* check with the DB the enabled members */
    get_numOfMembers = _4K;
    st = cpssDxChTrunkDbEnabledMembersGet(dev, trunkId,
        &get_numOfMembers,
        &membersArray_get_4K_tests[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* compare the number */
    UTF_VERIFY_EQUAL0_PARAM_MAC(expected_db_numOfEnabledMembers , get_numOfMembers);

    /* compare the enabled : members */
    for(jj = 0 ; jj < expected_db_numOfEnabledMembers; jj++)
    {
        set_index = jj;

        /* check that the member found in the 'get array' */
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            GT_TRUE,
            isMemberExistsInArr(&expected_db_EnabledMembersArr[set_index],
                                membersArray_get_4K_tests,
                                get_numOfMembers),
            jj);
    }

    /* check with the DB the disabled members */
    get_numOfMembers = _4K;
    st = cpssDxChTrunkDbDisabledMembersGet(dev, trunkId,
        &get_numOfMembers,
        &membersArray_get_4K_tests[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* compare the number */
    UTF_VERIFY_EQUAL0_PARAM_MAC(expected_db_numOfDisabledMembers , get_numOfMembers);


    /* compare the disabled : members */
    for(jj = 0 ; jj < expected_db_numOfDisabledMembers; jj++)
    {
        set_index = jj;

        /* check that the member found in the 'get array' */
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            GT_TRUE,
            isMemberExistsInArr(&expected_db_DisabledMembersArr[set_index],
                                membersArray_get_4K_tests,
                                get_numOfMembers),
            jj);
    }

    /* check that low level HW members hold the enabled members and not the disabled members */
    checkLowLevelMembers(dev,trunkId,
        expected_db_numOfEnabledMembers,
        expected_db_EnabledMembersArr,
        expected_db_numOfDisabledMembers,
        expected_db_DisabledMembersArr);
}


/* check that low level HW members hold the enabled members and not the disabled members */
static void checkLowLevelMembers(
    IN  GT_U8   dev,
    IN  GT_TRUNK_ID trunkId,
    IN  GT_U32  db_numOfEnabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   db_EnabledMembersArr[],
    IN  GT_U32  db_numOfDisabledMembers,
    IN  CPSS_TRUNK_MEMBER_STC   db_DisabledMembersArr[]
)
{
    GT_STATUS st = GT_OK;

    GT_U32      get_numOfMembers;
    GT_U32  set_index;
    GT_U32  jj;
    /* get the low level trunk members and check that all 'needed' members exists */
    st = cpssDxChTrunkTableEntryGet(dev,trunkId,
        &get_numOfMembers,
        &membersArray_get_4K_tests[0]);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC((get_numOfMembers <= _4K)?1:0, 1, dev);

    if(db_numOfEnabledMembers == 0 || db_EnabledMembersArr == NULL)
    {
        /* the CPSS force the HW to hold single member -- the 'NULL port' */
        UTF_VERIFY_EQUAL1_PARAM_MAC(get_numOfMembers , 1/*NULL port*/, dev);

        return;
    }


    /* compare the enabled : members */
    for(jj = 0 ; jj < db_numOfEnabledMembers; jj++)
    {
        set_index = jj;

        /* check that the member found in the 'get array' */
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            GT_TRUE,
            isMemberExistsInArr(&db_EnabledMembersArr[set_index],
                                membersArray_get_4K_tests,
                                get_numOfMembers),
            jj);
    }

    /* check that no disabled member exists */
    for(jj = 0 ; jj < db_numOfDisabledMembers; jj++)
    {
        set_index = jj;

        /* check that the member NOT found in the 'get array' */
        UTF_VERIFY_EQUAL1_PARAM_MAC(
            GT_FALSE, /* must not exist ! */
            isMemberExistsInArr(&db_DisabledMembersArr[set_index],
                                membersArray_get_4K_tests,
                                get_numOfMembers),
            jj);
    }

}



/* remove the member from the test DB of expected enabled or disabled */
static void removeMemberTestDb(
    IN      CPSS_TRUNK_MEMBER_STC   *memberToRemovePtr,
    INOUT   GT_U32                  *db_numOfEnabledMembers_Ptr,
    IN      CPSS_TRUNK_MEMBER_STC   db_EnabledMembersArr[],
    INOUT   GT_U32                  *db_numOfDisabledMembers_Ptr,
    IN      CPSS_TRUNK_MEMBER_STC   db_DisabledMembersArr[]
)
{
    GT_BOOL isRemoved = GT_FALSE;

    if(db_EnabledMembersArr && db_numOfEnabledMembers_Ptr)
    {
        isRemoved = removeMemberExistsInArr(
            memberToRemovePtr,
            db_EnabledMembersArr,
            db_numOfEnabledMembers_Ptr);

        if(isRemoved == GT_TRUE)
        {
            return;
        }
    }

    if(db_numOfDisabledMembers_Ptr && db_DisabledMembersArr)
    {
        isRemoved = removeMemberExistsInArr(
            memberToRemovePtr,
            db_DisabledMembersArr,
            db_numOfDisabledMembers_Ptr);
    }

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE, isRemoved);

}


static void main_cpssDxChTrunkMemberManipulation_4K_members(
    IN GT_U8    dev ,
    IN GT_TRUNK_ID trunkId,
    IN GT_U32   maxNumOfMembers)
{
    GT_STATUS st = GT_OK;
    GT_U32      ii,jj;
    GT_U32      numOfEnabledMembers;
    GT_U32      numOfDisabledMembers;
    GT_U32  secondsEnd,nanoSecondsEnd,secondsStart,nanoSecondsStart;
    GT_U32  set_index,iterations;
    GT_U32  maxMembers = maxNumOfMembers;
    GT_U32  maxIterations;

    if(IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC)
    {
        /* reduce number of iterations */
        maxMembers = maxNumOfMembers > _1K ? _1K : maxNumOfMembers;
    }

    numOfEnabledMembers  = (maxMembers/2) - 235;
    numOfDisabledMembers = (maxMembers/2) - numOfEnabledMembers;

    /*****************************************/
    /*****************************************/
    /*******   part-1                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/
    cpssOsPrintf("cpssDxChTrunkMembersSet with : [%d] members [%d] enabled , [%d] disabled\n",
        numOfEnabledMembers + numOfDisabledMembers ,
        numOfEnabledMembers,
        numOfDisabledMembers);

    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

    /* start manipulation on the trunk */
    st = cpssDxChTrunkMembersSet(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_set_4K_tests[0],/*enabled members in the start*/
                numOfDisabledMembers,
                &membersArray_set_4K_tests[numOfEnabledMembers]);/* disabled members at the end */
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);

    for(jj = 0 ; jj < numOfEnabledMembers ; jj++)
    {
        set_index = jj;
        membersArray_expected_enabled_tests[jj] = membersArray_set_4K_tests[set_index];
    }
    for(jj = 0 ; jj < numOfDisabledMembers ; jj++)
    {
        set_index = jj+numOfEnabledMembers;
        membersArray_expected_disabled_tests[jj] = membersArray_set_4K_tests[set_index];
    }


    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_expected_enabled_tests[0],
                numOfDisabledMembers,
                &membersArray_expected_disabled_tests[0]);

    /*****************************************/
    /*****************************************/
    /*******   part-2                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/
    secondsStart = secondsEnd;

    cpssOsPrintf("Going to : cpssDxChTrunkMemberAdd : (%d) members to trunk \n",
        maxMembers - (numOfEnabledMembers + numOfDisabledMembers));

    iterations = 0;
    for(ii = numOfEnabledMembers + numOfDisabledMembers ; ii < maxMembers ; ii++,iterations++)
    {
        /* keep alive */
        /* start ADDING members to the trunk */
        st = cpssDxChTrunkMemberAdd(dev,trunkId,
                    &membersArray_set_4K_tests[ii]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);

    for(jj = numOfEnabledMembers + numOfDisabledMembers ; jj < maxMembers ; jj++)
    {
        set_index = jj;
        membersArray_expected_enabled_tests[jj-numOfDisabledMembers] = membersArray_set_4K_tests[set_index];
    }

    numOfEnabledMembers += iterations;

    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_expected_enabled_tests[0],
                numOfDisabledMembers,
                &membersArray_expected_disabled_tests[0]);

    /*****************************************/
    /*****************************************/
    /*******   part-3                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/

    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

    iterations = 0;
    for(ii = 0; ii < numOfEnabledMembers ; ii += 5 , iterations++)
    {
        /* just counting */
    }

    cpssOsPrintf("Going to : cpssDxChTrunkMemberDisable : [%d] members \n",
        iterations);
    /* start disabling members (one every 5 enabled members)*/
    iterations = 0;
    for(ii = 0; ii < numOfEnabledMembers ; ii += 5 , iterations++)
    {
        /* start disabling members in the trunk */
        st = cpssDxChTrunkMemberDisable(dev,trunkId,
                    &membersArray_expected_enabled_tests[ii]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);

    for(ii = 0; ii < numOfEnabledMembers ; ii++)
    {
        /* copy array */
        membersArray_set1_4K_tests[ii] = membersArray_expected_enabled_tests[ii];
    }

    set_index = numOfDisabledMembers;
    maxIterations = numOfEnabledMembers;
    for(ii = 0; ii < maxIterations ; ii += 5)
    {
        /* remove the member from the test DB of expected enabled */
        removeMemberTestDb(&membersArray_set1_4K_tests[ii] ,
            &numOfEnabledMembers,
            membersArray_expected_enabled_tests,
            NULL,
            NULL);

        /* set the expected to have another member */
        membersArray_expected_disabled_tests[set_index++] =
            membersArray_set1_4K_tests[ii];
    }

    /*numOfEnabledMembers  -= iterations; -- was updated by removeMemberTestDb(...)*/
    numOfDisabledMembers += iterations;

    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_expected_enabled_tests[0],
                numOfDisabledMembers,
                &membersArray_expected_disabled_tests[0]);


    /*****************************************/
    /*****************************************/
    /*******   part-4                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/
    iterations = 0;
    for(ii = 0 ; ii < (numOfEnabledMembers + numOfDisabledMembers) ; ii += 3,iterations++)
    {
        /* just counting */
    }



    secondsStart = secondsEnd;

    cpssOsPrintf("Going to : cpssDxChTrunkMemberRemove : (%d) members from the trunk \n",
        iterations);

    iterations = 0;
    for(ii = 0 ; ii < (numOfEnabledMembers + numOfDisabledMembers) ; ii += 3,iterations++)
    {
        /* start REMOVING members from the trunk */
        st = cpssDxChTrunkMemberRemove(dev,trunkId,
                    &membersArray_set_4K_tests[ii]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);

    maxIterations = (numOfEnabledMembers + numOfDisabledMembers);
    for(ii = 0 ; ii < maxIterations ; ii += 3,iterations++)
    {
        /* remove the member from the test DB of expected enabled or disabled */
        removeMemberTestDb(&membersArray_set_4K_tests[ii] ,
            &numOfEnabledMembers,
            membersArray_expected_enabled_tests,
            &numOfDisabledMembers,
            membersArray_expected_disabled_tests);
    }

    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_expected_enabled_tests[0],
                numOfDisabledMembers,
                &membersArray_expected_disabled_tests[0]);

    /*****************************************/
    /*****************************************/
    /*******   part-5                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/
    iterations = 0;
    for(ii = 0; ii < numOfDisabledMembers ; ii += 2 , iterations++)
    {
        /* just counting */
    }

    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

    cpssOsPrintf("Going to : cpssDxChTrunkMemberEnable : [%d] members \n" ,
        iterations);
    /* start enabling members (one every 2 disabled members)*/
    iterations = 0;
    for(ii = 0; ii < numOfDisabledMembers ; ii += 2 , iterations++)
    {
        /* keep alive */
        /* start ENABLING members in the trunk */
        st = cpssDxChTrunkMemberEnable(dev,trunkId,
                    &membersArray_expected_disabled_tests[ii]);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);

    for(ii = 0; ii < numOfDisabledMembers ; ii++)
    {
        /* copy array */
        membersArray_set1_4K_tests[ii] = membersArray_expected_disabled_tests[ii];
    }

    maxIterations = numOfDisabledMembers;
    set_index = numOfEnabledMembers;
    for(ii = 0; ii < maxIterations ; ii += 2)
    {
        /* remove the member from the test DB of expected disabled */
        removeMemberTestDb(&membersArray_set1_4K_tests[ii] ,
            NULL,
            NULL,
            &numOfDisabledMembers,
            membersArray_expected_disabled_tests);

        /* set the expected to have another member */
        membersArray_expected_enabled_tests[set_index++] =
            membersArray_set1_4K_tests[ii];
    }

    numOfEnabledMembers += iterations;
    /*numOfDisabledMembers-= iterations; -- updated by removeMemberTestDb() */

    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                numOfEnabledMembers,
                &membersArray_expected_enabled_tests[0],
                numOfDisabledMembers,
                &membersArray_expected_disabled_tests[0]);

    /*****************************************/
    /*****************************************/
    /*******   part-6                    *****/
    /*****************************************/
    /*****************************************/
    /*****************************************/
    cpssOsPrintf("Going to : cpssDxChTrunkMembersSet with : 0 members (from [%d] members)\n",
        numOfEnabledMembers+numOfDisabledMembers);

    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

    /* start manipulation on the trunk */
    st = cpssDxChTrunkMembersSet(dev,trunkId,0,NULL,0,NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    cpssOsPrintf("ended iteration : took [%d] seconds \n",
        secondsEnd-secondsStart);


    /* check that DB members hold the enabled members and the disabled members */
    /* check that low level HW members hold the enabled members and not the disabled members */
    checkExpectedAndActualMembers(dev,trunkId,
                0,
                NULL,
                0,
                NULL);
}
/*
    test for : cpssDxChTrunkMemberAdd with 4K members
*/
UTF_TEST_CASE_MAC(cpssDxChTrunkMemberManipulation_4K_members)
{
    GT_STATUS st = GT_OK;

    GT_U8     dev;
    GT_U32    numOfApplicableDevices = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      l2EcmpNumMembers;
    GT_U32      maxTrunksInDev;
    GT_U32      l2EcmpStartIndex,maxNumOfMembers;

    GT_U32    orig_maxNumberOfTrunks;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT orig_trunkMembersMode;
    GT_TRUNK_ID trunkId;
#ifdef CPSS_LOG_ENABLE
    GT_BOOL logWasOpen;
#endif

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_ALDRIN_E, "JIRA-6748");

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* skip test for EMULATOR when running 'skip long tests' */
    PRV_TGF_SKIP_LONG_TEST_EMULATOR_MAC(UTF_ALL_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        numOfApplicableDevices++;

        init_4k_members(dev);

        l2EcmpNumMembers = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.l2EcmpNumMembers;
        maxTrunksInDev   = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum;

        L2_ECMP_NULL_PORT_BOOKMARK
        l2EcmpNumMembers--;

        st = cpssDxChTrunkDbInitInfoGet(dev,&orig_maxNumberOfTrunks,&orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* destroy the trunk LIB so we can set flex mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*set (4K-1) trunks in flex mode */
        st = cpssDxChTrunkInit(dev , maxTrunksInDev,CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        trunkId = (GT_TRUNK_ID)maxTrunksInDev;

        /* set to use the end of the 'L2 ECMP' */
        maxNumOfMembers  = (l2EcmpNumMembers > _4K) ? _4K : l2EcmpNumMembers;
        l2EcmpStartIndex = l2EcmpNumMembers - maxNumOfMembers;

        /* set this trunk to be operational */
        st = cpssDxChTrunkFlexInfoSet(dev,trunkId,l2EcmpStartIndex,maxNumOfMembers);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* copy to 'set' array from the 'orig' array */
        cpssOsMemCpy(membersArray_set_4K_tests,
                     membersArray_orig_4K_tests,
                     sizeof(membersArray_orig_4K_tests));

#ifdef CPSS_LOG_ENABLE
        logWasOpen = cpssLogEnableGet();
        if(GT_FALSE == logWasOpen)
        {
            logWasOpen = GT_FALSE;
            cpssLogEnableSet(GT_TRUE);
            cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,GT_TRUE);
        }
#endif

        /* run the 'main' of the test . it is in separate function to allow
           'resore' of 'destroy+init' even if fail at the 'main' function */
        main_cpssDxChTrunkMemberManipulation_4K_members(dev,trunkId,maxNumOfMembers);

#ifdef CPSS_LOG_ENABLE
        if(logWasOpen == GT_FALSE)
        {
            cpssLogEnableSet(GT_FALSE);
        }
#endif

        /* now we can destroy the trunk LIB and generate new init with orig mode */
        st = cpssDxChTrunkDestroy(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChTrunkInit(dev , orig_maxNumberOfTrunks,orig_trunkMembersMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    if(numOfApplicableDevices == 0)
    {
        /* state that this test is not relevant to this system */
        SKIP_TEST_MAC
    }

    return;
}

#define  TRUNK_HIGH_LEVEL_LIST  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMembersSet)                   \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberAdd)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberRemove)                 \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberDisable)                \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberEnable)                 \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbEnabledMembersGet)          \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbDisabledMembersGet)         \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbIsMemberOfTrunk)            \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbTrunkTypeGet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDesignatedMemberSet)          \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbDesignatedMemberGet)        \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkCascadeTrunkPortsSet)         \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkCascadeTrunkPortsGet)         \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbMcLocalSwitchingEnableGet)  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMcLocalSwitchingEnableSet)    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbMembersSortingEnableSet)    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbMembersSortingEnableGet)    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkCascadeTrunkWithWeightedPortsSet) \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkCascadeTrunkWithWeightedPortsGet) \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkPortMcEnableSet)                  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbPortMcEnableGet)                \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDestroy)                          \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbInitInfoGet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkFlexInfoSet)                      \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDbFlexInfoGet)

/* list 1 hold tests than need to run only once (take too much time) */
#define  TRUNK_HIGH_LEVEL_LIST_1  \
    TRUNK_HIGH_LEVEL_LIST         \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMembersSet_1K_members)           \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberManipulation_4K_members)


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTrunk high-level suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTrunkHighLevel)
    TRUNK_HIGH_LEVEL_LIST
UTF_SUIT_END_TESTS_MAC(cpssDxChTrunkHighLevel)

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTrunkHighLevel_1)
    TRUNK_HIGH_LEVEL_LIST_1
UTF_SUIT_END_TESTS_MAC(cpssDxChTrunkHighLevel_1)

#define  TRUNK_LOW_LEVEL_LIST  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkNonTrunkPortsAdd)                  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkNonTrunkPortsRemove)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkPortTrunkIdSet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkPortTrunkIdGet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkTableEntrySet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkTableEntryGet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkNonTrunkPortsEntrySet)             \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkNonTrunkPortsEntryGet)             \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDesignatedPortsEntrySet)           \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkDesignatedPortsEntryGet)           \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashDesignatedTableModeSet)        \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashDesignatedTableModeGet)        \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashGlobalModeSet)                 \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashGlobalModeGet)                 \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpAddMacModeSet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpAddMacModeGet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpModeSet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpModeGet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashL4ModeSet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashL4ModeGet)                     \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpv6ModeSet)                   \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpv6ModeGet)                   \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpShiftSet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIpShiftGet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskSet)                       \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskGet)                       \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMplsModeEnableSet)             \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMplsModeEnableGet)             \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkInit)                              \
                                                                              \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashCrcParametersSet)              \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashCrcParametersGet)              \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkPortHashMaskInfoSet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkPortHashMaskInfoGet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskCrcEntrySet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskCrcEntryGet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashIndexCalculate)                \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashNumBitsSet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashNumBitsGet)                    \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashPearsonValueSet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashPearsonValueGet)               \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashCrcSaltByteGet)                \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashCrcSaltByteSet)                \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskCrcParamOverrideSet)       \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkHashMaskCrcParamOverrideGet)       \
                                                                              \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkLearnPrioritySet)                  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkLearnPriorityGet)                  \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberSelectionModeSet)            \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkMemberSelectionModeGet)            \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkUserGroupSet)                      \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkUserGroupGet)                      \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkEcmpLttTableSet)                   \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkEcmpLttTableGet)                   \
                                                                              \
    /* Test for Table */                                                      \
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTrunkFillTrunkTable)
/*
 * Configuration of cpssDxChTrunk low-level suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTrunkLowLevel)
    TRUNK_LOW_LEVEL_LIST
UTF_SUIT_END_TESTS_MAC(cpssDxChTrunkLowLevel)

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTrunkLowLevel_1)
    TRUNK_LOW_LEVEL_LIST
UTF_SUIT_END_TESTS_MAC(cpssDxChTrunkLowLevel_1)

