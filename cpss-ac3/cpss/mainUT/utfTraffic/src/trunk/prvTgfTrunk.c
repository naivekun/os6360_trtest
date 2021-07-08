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
* @file prvTgfTrunk.c
*
* @brief CPSS trunk testing implementation.
*
* @version   67
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <common/tgfPortGen.h>
#include <common/tgfPclGen.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfCncGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>

#if (defined CHX_FAMILY)
/* indication for Galtis wrappers call the WA 'B' APIs */
extern GT_U32   appDemo_trunk_phy1690_WA_B;
extern void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
);
extern GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);
extern GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
);
#else
static GT_U32   appDemo_trunk_phy1690_WA_B = 0;
static void prvTgfCaptureForceTtiDisableModeSet(
    IN GT_BOOL                  forceTtiLookupUnchanged
)
{
    return;
}
static GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
)
{
    *globalEPortPtr = 0;
    return GT_OK
}
static GT_STATUS prvTgfDxChPclCaptureSet
(
    IN GT_U8    devNum,
    IN GT_PORT_NUM   portNum,
    IN GT_BOOL  start
)
{
    return GT_OK
}
#endif


/* MPLS labels considered 'reserved' for values 0..15. so use larger value .*/
/* Bobk and above not doing LBH on such 0..15 labels ! */
#define BASE_MPLS_LABEL_VALUE_CNS   0x1234

/* indication that generic ports restore was done */
static GT_BOOL prvTgfTrunkGenericRestorePorts_done = GT_TRUE;

/********remove all printings to the log from this file **************/
/********  due to large printings ************************************/
static GT_BOOL  trunkOpenLog = 0;
#define TRUNK_PRV_UTF_LOG0_MAC \
    if(trunkOpenLog) PRV_UTF_LOG0_MAC

#define TRUNK_PRV_UTF_LOG1_MAC \
    if(trunkOpenLog) PRV_UTF_LOG1_MAC

#define TRUNK_PRV_UTF_LOG2_MAC \
    if(trunkOpenLog) PRV_UTF_LOG2_MAC

#define TRUNK_PRV_UTF_LOG3_MAC \
    if(trunkOpenLog) PRV_UTF_LOG3_MAC

#define TRUNK_PRV_UTF_LOG4_MAC \
    if(trunkOpenLog) PRV_UTF_LOG4_MAC

void tgfTrunkOpenLog(GT_U32 openLog)
{
    trunkOpenLog = openLog;
}


/*********************************************************************/

#define REDUCE_RUNTIME_BY_FACTOR_MAC(factor)                                \
        {                                                                   \
            static GT_U32 _globalIterator = 0;                              \
            static GT_U32 _isFirst = 1;                                     \
                                                                            \
            if(_isFirst == 1)                                               \
            {   /*allow each baseline to run different cases*/              \
                _globalIterator = prvUtfSeedFromStreamNameGet();            \
                /* use factor 11 to not allow consecutive baseline to run consecutive permutations */ \
                _globalIterator *= 11;                                      \
                _isFirst = 0;                                               \
            }                                                               \
            _globalIterator++;                                              \
                                                                            \
            if(0 != (_globalIterator % (factor)))                           \
            {                                                               \
                /* only 1:factor sending is allowed */                      \
                continue;                                                   \
            }                                                               \
        }

#define REDUCE_RUNTIME_BY_FACTOR_WITH_RETURN_GT_OK_MAC(factor)              \
    {                                                                       \
        GT_U32 _tmpIter = 2;                                                \
        do{                                                                 \
            _tmpIter--;                                                     \
            if(_tmpIter == 0)                                               \
            {                                                               \
                /* if we got here , the REDUCE_RUNTIME_BY_FACTOR_MAC did 'continue' */ \
                return GT_OK;                                               \
            }                                                               \
                                                                            \
            /*this macro will 'continue' if we should skip the iteration */ \
            REDUCE_RUNTIME_BY_FACTOR_MAC(factor);                           \
                                                                            \
            /* so if we got here , we need to do this iteration */          \
            break;                                                          \
        }while(1);                                                          \
    }
#if 0 /* functions to debug the above macros */
static GT_BOOL reduceRuntimeByFactor(GT_U32 factor)
{
    static GT_U32 _globalIterator = 0;
    static GT_U32 _isFirst = 1;

    if(_isFirst == 1)
    {   /*allow each baseline to run different cases*/
        _globalIterator = prvUtfSeedFromStreamNameGet();
        /* use factor 11 to not allow consecutive baseline to run consecutive permutations */
        _globalIterator *= 11;
        _isFirst = 0;
    }
    _globalIterator++;

    if(0 != (_globalIterator % (factor)))
    {
        /* only 1:factor sending is allowed */
        return GT_FALSE;
    }
    return GT_TRUE;
}

static GT_BOOL reduceRuntimeByFactorWithReturn(GT_U32 factor)
{
    GT_U32 _tmpIter = 2;
    do{
        _tmpIter --;/*1 , 0*/
        if(_tmpIter == 0)
        {
            /* if we got here , the REDUCE_RUNTIME_BY_FACTOR_MAC did 'continue' */
            return GT_FALSE;
        }

        /*this macro will 'continue' if we should skip the iteration */
        if(GT_FALSE == reduceRuntimeByFactor(factor))
        {
            continue;
        }

        /* so if we got here , we need to do this iteration */
        break;
    }while(1);

    /* we can continue the iteration */
    return GT_TRUE;
}
#endif /*0*/

#ifdef CHX_FAMILY
#define    ECMP_TABLE_SIZE_CNS(_dev) (PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers - \
    PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers_usedByTrunk) /*actual is _8K but high part used for the trunk entries*/

#define GET_MAX_ECMP_INDEX_MAC(/*IN*/_devNum,/*OUT*/_maxIndex)              \
    _maxIndex = 0;                                                          \
    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(_devNum))                   \
    {                                                                       \
        _maxIndex = ECMP_TABLE_SIZE_CNS(dev)-1;                             \
    }
#else
#define GET_MAX_ECMP_INDEX_MAC(/*IN*/_devNum,/*OUT*/_maxIndex)              \
    _maxIndex = 0/* just for compilation */
#endif

/* number of entries in the hash mask table */
#define PRV_TGF_TRUNK_HASH_MASK_TABLE_SIZE_CNS  (PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS + \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) ? 12/*TDB-support 16*/ : 12))

/* the start index of 'Traffic type' entries in the hash mask table */
#define PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS  16


static GT_U32   forceCrcRunAllPermutations = 0;
GT_STATUS tgfTrunkForceCrcRunAllPermutations(IN GT_U32 forceAll)
{
    forceCrcRunAllPermutations = forceAll;
    return GT_OK;
}


#define MAX_TRUNKS_CNS(devNum)  \
    PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numTrunksSupportedHw

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    /* default Trunk Id */
    /* Tested trunk id. All tests uses this trunk id for testing. It means that     */
    /* test must clear this trunk id after testing.is completed.                    */
    /* NOTE: in XCAT use trunk 2 because issues of 'trunk WA' that the appdemo enable for it
        so need to use range of 0..31 (that is converted to even numbers > 64 ...) */
    #define _PRV_TGF_TRUNK_ID_CNS                                                               \
            (GT_TRUNK_ID)((((prvTgfDevNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) &&                    \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&     \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&    \
                (MAX_TRUNKS_CNS(prvTgfDevNum) > 10)) ?    \
                    (MAX_TRUNKS_CNS(prvTgfDevNum) - 10) : \
                    25)

    /* secondary Trunk Id */
    #define _PRV_TGF_TRUNK_ID_2_CNS                                                             \
            (GT_TRUNK_ID)((((prvTgfDevNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) &&                    \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&     \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&    \
                (MAX_TRUNKS_CNS(prvTgfDevNum) > 20)) ?    \
                    (MAX_TRUNKS_CNS(prvTgfDevNum) - 20) : \
                    13)
    #define _PRV_TGF_TRUNK_ID_3_CNS                                                             \
            (GT_TRUNK_ID)((((prvTgfDevNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) &&                    \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E) &&     \
                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT3_E) &&    \
                (MAX_TRUNKS_CNS(prvTgfDevNum) > 50)) ?    \
                    (MAX_TRUNKS_CNS(prvTgfDevNum) - 50) : \
                    20)
#elif defined EXMXPM_FAMILY
    #define _PRV_TGF_TRUNK_ID_CNS            239
    #define _PRV_TGF_TRUNK_ID_2_CNS          111 /* 239 - 128 = 111 --> check MSB issue */
    #define _PRV_TGF_TRUNK_ID_3_CNS          105
#else /*exmx ?*/
    #define _PRV_TGF_TRUNK_ID_CNS            25
    #define _PRV_TGF_TRUNK_ID_2_CNS          13
    #define _PRV_TGF_TRUNK_ID_3_CNS          20
#endif

static GT_TRUNK_ID      internalTrunkId;
static GT_TRUNK_ID      internalTrunkId2;
static GT_TRUNK_ID      internalTrunkId3;

#define PRV_TGF_TRUNK_ID_CNS internalTrunkId
#define PRV_TGF_TRUNK_ID_2_CNS internalTrunkId2
#define PRV_TGF_TRUNK_ID_3_CNS internalTrunkId3

static void initTrunk1Ids(GT_BOOL isCascade)
{
    internalTrunkId  = _PRV_TGF_TRUNK_ID_CNS  ;
    if(isCascade)
    {
        /* limited to 256 */
        internalTrunkId  &= 0xFF;
    }

    if(appDemo_trunk_phy1690_WA_B)
    {
        internalTrunkId  &= 0x3F;
    }
}
static void initTrunk2Ids(GT_BOOL isCascade)
{
    internalTrunkId2  = _PRV_TGF_TRUNK_ID_2_CNS  ;
    if(isCascade)
    {
        /* limited to 256 */
        internalTrunkId2  &= 0xFF;
    }

    if(appDemo_trunk_phy1690_WA_B)
    {
        internalTrunkId2  &= 0x3F;
    }
}
static void initTrunk3Ids(GT_BOOL isCascade)
{
    internalTrunkId3  = _PRV_TGF_TRUNK_ID_3_CNS  ;
    if(isCascade)
    {
        /* limited to 256 */
        internalTrunkId3  &= 0xFF;
    }

    if(appDemo_trunk_phy1690_WA_B)
    {
        internalTrunkId3  &= 0x3F;
    }
}

static void initTrunkIds(GT_BOOL isCascade)
{
    initTrunk1Ids(isCascade);
    initTrunk2Ids(isCascade);
    initTrunk3Ids(isCascade);
}

/* support for CRC hash calculation of expected egress port */
static GT_BOOL  use_calculatedHash_expectedEgressPortNum = GT_FALSE;
void prvTgfTrunkCrcHash_cpssPredictEgressPortEnableSet(IN GT_BOOL enable)
{
    use_calculatedHash_expectedEgressPortNum = enable;
}

/*
 *     symmetricMacAddrEnable  - Enable/Disable 'symmetric mac addr' in hash calculations.
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricIpv4AddrEnable - Enable/Disable 'symmetric IPv4 addr' in hash calculations.(NOTE: 4  LSBytes in 16 bytes IP)
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricIpv6AddrEnable - Enable/Disable 'symmetric IPv6 addr' in hash calculations.(NOTE: 12 MSBytes in 16 bytes IP)
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *     symmetricL4PortEnable   - Enable/Disable 'symmetric L4 port' in hash calculations.
 *           (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*/
static GT_BOOL     symmetricMacAddrEnable   = GT_FALSE;
static GT_BOOL     symmetricIpv4AddrEnable  = GT_FALSE;
static GT_BOOL     symmetricIpv6AddrEnable  = GT_FALSE;
static GT_BOOL     symmetricL4PortEnable    = GT_FALSE;

void prvTgfTrunkCrcHash_symmetricModeEnable(IN GT_BOOL enable)
{
    symmetricMacAddrEnable   = enable;
    symmetricIpv4AddrEnable  = enable;
    symmetricIpv6AddrEnable  = enable;
    symmetricL4PortEnable    = enable;
}
static GT_U32   all_trunks_fixed_size = 0;
static GT_U32   calculatedHash_expectedTrunkMemberIndex = 0;
/* the CPSS do 'optimization for better' LBH with 1,2,4,8 members */
/* and sets the device with 8 members on those case */
static GT_U32   calculatedHash_numOfmembers = 8;
extern void tgfTrafficGeneratorSendOnlyLastPacketSet(IN GT_BOOL allowOnlyLastPacket);
#ifdef ASIC_SIMULATION
#include <asicSimulation/SLog/simLog.h>
/* modify this flag under debugger to open simulation log on specific case */
static GT_U32  debug_withSimulationLog = 0;
#endif /* ASIC_SIMULATION */

/* index of port in port-array for trunk member 0 */
#define PRV_TGF_TRUNK_PORT0_INDEX_CNS     0

/* index of port in port-array for trunk member 1 */
#define PRV_TGF_TRUNK_PORT1_INDEX_CNS     2

/* index of port in port-array for trunk member 2 */
#define PRV_TGF_TRUNK_PORT2_INDEX_CNS     1

/* index of port in port-array for trunk member 3 */
#define PRV_TGF_TRUNK_PORT3_INDEX_CNS     3

/* index of port in port-array to send traffic to */
#define PRV_TGF_SEND_PORT_INDEX_CNS     1

/* 8 ports in trunk tests */
#define TRUNK_8_PORTS_IN_TEST_CNS       8

#ifdef EXMXPM_FAMILY
    #define TRUNK_MAX_NUM_OF_MEMBERS_CNS    PRV_CPSS_TRUNK_12_MAX_NUM_OF_MEMBERS_CNS
#else/*!EXMXPM_FAMILY*/
    #define TRUNK_MAX_NUM_OF_MEMBERS_CNS    PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS
#endif /*!EXMXPM_FAMILY*/

/* we have at least 4 non-trunk members , and up to 'max' trunk members */
#define TRUNK_MAX_PORTS_IN_TEST_CNS     (NOT_IN_TRUNK_PORTS_NUM_CNS + PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS)
/* 4 ports of test are not in trunk */
#define NOT_IN_TRUNK_PORTS_NUM_CNS      4

/* to avoid running test of about 40 minutes , run only subset of it in windows */
/* it means that in Linux_sim and on HW it will run full test */
#define FORCE_REDUCED_TEST_MAC

/* the test need 4 ports not in trunk + 8 ports in trunk + extra 5 ports for additional manipulations */
#define TGF_NUM_NON_USED_PORTS_CNS  5
static GT_U32  tgfTrunkNonUsedPorts[TGF_NUM_NON_USED_PORTS_CNS];

enum {
    TGF_CASCADE_TRUNK_B_MEMBER_0_INDEX_E = (NOT_IN_TRUNK_PORTS_NUM_CNS+TRUNK_8_PORTS_IN_TEST_CNS),
    TGF_CASCADE_TRUNK_B_MEMBER_1_INDEX_E ,
    TGF_CASCADE_TRUNK_C_MEMBER_0_INDEX_E ,
    TGF_CASCADE_TRUNK_C_MEMBER_1_INDEX_E ,
    TGF_CASCADE_PORT_INDEX_E
};
enum{
    devNumForTrunkB = 2,
    devNumForTrunkC = 4,
    devNumForCascadePort = 6
};

static CPSS_TRUNK_WEIGHTED_MEMBER_STC  trunkB_weightedMembers[64];
static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {10     ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      {10     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      {10     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {24     ,0},/* local member */
  /*5*/      {15     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {31     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {59     ,devNumForTrunkC} /* remote member - dev 4 */
        };

/* trunk members for XLG ports */
static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_XLG[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      { 8     ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      { 8     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      { 8     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {24     ,0},/* local member */
  /*5*/      {20     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {25     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };

/* trunk members for bobk-cetus ports */
/* first 4 port not in the trunk are :{56,57,58,59}
    we need to select 6 ports from range 64..71
*/

static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_bobk_cetus[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {65     ,0},/* local member */
  /*1*/      {70     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      {65     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      {65     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {71     ,0},/* local member */
  /*5*/      {69     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {68     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {67     ,devNumForTrunkC} /* remote member - dev 4 */
        };

/* trunk members for bobcat3 ports */
/*  the next ports are not in trunk : 0,1,2,3
    use 59,64
*/

static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_bobcat3[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {77      ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      { 8     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      { 8     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {64     ,0},/* local member */
  /*5*/      {20     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {25     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };

/* trunk members for Armstron2 ports */
/*  the next ports are not in trunk : 0,1,2,3
    use 59,64
*/

static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_aldrin2[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {77      ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      { 8     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      { 8     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {64     ,0},/* local member */
  /*5*/      {20     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {25     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };


static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_falcon[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {68      ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      { 8     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      { 8     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {64     ,0},/* local member */
  /*5*/      {20     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {25     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };

/* trunk members for Aldrin-Z0 ports */
/*  the next ports are not in trunk : 0,1,2,3
*/

static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_aldrinZ0[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      {59     ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      {32      ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      {34      ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      {64     ,0},/* local member */
  /*5*/      {18     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {67     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };

static CPSS_TRUNK_MEMBER_STC   trunkA_MembersArray_xCat3x[TRUNK_8_PORTS_IN_TEST_CNS] = {
  /*index*/ /*port,device*/
  /*0*/      { 4     ,0},/* local member */
  /*1*/      {16     ,devNumForTrunkB},/* remote member - dev 2 */
  /*2*/      { 8     ,devNumForTrunkB},/* remote member - dev 2 */
  /*3*/      { 8     ,devNumForTrunkC},/* remote member - dev 4 */
  /*4*/      { 0     ,0},/* local member */
  /*5*/      {20     ,devNumForTrunkB},/* remote member - dev 2 */
  /*6*/      {25     ,devNumForCascadePort},/* remote member - dev 6 */
  /*7*/      {56     ,devNumForTrunkC} /* remote member - dev 4 */
        };

typedef enum{
    TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E = 30,/*need to be value >= 2*/ /* remove port from Cascade trunk B */
    TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_CASCADE_TRUNK_B_E      ,/* restore port to  Cascade trunk B */
    TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E             ,/* remove port from trunk A */
    TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_TRUNK_A_E               /* restore port to trunk B */
}TRUNK_WITH_REMOTE_MEMBERS_ACTIONS_ENT;

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5


/* indication that we check SA learning */
static GT_U32 testSaLearning = 0;

/* indication that test that was written for 'trunk' will run with 'global eport that represents the trunk' */
GT_U32   tgfTrunkGlobalEPortRepresentTrunk = GT_FALSE;


#define GLOBAL_E_PORT_PER_DEVICE_MAC tgfTrunkEportBase()

/* trunks and L2ECMP should hold the same mask for global eports in the bridge
   but still need 2 different ranges ... so the pattern of those 2 should be
   differ only by single bit

   _6K..(7K-1) - global eports represent trunks
   _7K..(8K-1) - global eports represent L2ECMP
*/
#define MASK_FOR_GLOBAL_EPORTS_CNS                          0x1F00
/* the range for the global eports represent trunks is _6K..(7K-1) */
#define PATTERN_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS      GLOBAL_E_PORT_PER_DEVICE_MAC
/* the range for the global eports represent L2ECMP is _7K..(8K-1) */
#define PATTERN_FOR_GLOBAL_EPORT_REPRESENT_L2ECMP_CNS       (GLOBAL_E_PORT_PER_DEVICE_MAC + 256)

/* base eport for global eports that represents trunkId */
#define BASE_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS     PATTERN_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS
/* set pattern,mask to have all the eports  */
#define MASK_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS     MASK_FOR_GLOBAL_EPORTS_CNS

/* local target eport */
#define PRV_TGF_TARGET_EPORT_CNS (GLOBAL_E_PORT_PER_DEVICE_MAC - 1)

/* convert trunkId to the global ePort that will represent it */
/* note : use default eport with value greater than the number of entries in the 'default eport table'
   only LSBits will be used to access the 'default eport table' but still the
   default eport is set to high value in the range of global eports */
#define DEFAULT_SOURCE_EPORT_FROM_TRUNK_ID_MAC(_trunkId) \
    ((((_trunkId) - 15) & 0xFF) + (BASE_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS))

static GT_BOOL globalEPortRepresentTrunk_sortingEnabled = GT_FALSE;

/* indication that the E2Phy was set for the Global EPort Represent Trunk */
static GT_BOOL globalEPortRepresentTrunk_e2phyReady = GT_FALSE;

static GT_PORT_NUM defaultEPort[PRV_TGF_MAX_PORTS_NUM_CNS];
static GT_PORT_NUM defaultEPortWasChanged[PRV_TGF_MAX_PORTS_NUM_CNS] = {0};
static PRV_TGF_CFG_GLOBAL_EPORT_STC origGlobalRange = {0};
static PRV_TGF_CFG_GLOBAL_EPORT_STC origEcmpRange = {0};

static TGF_DSA_DSA_FORWARD_STC  dsaFromRemotePortOfTrunkPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/ /* set in runtime */
    },/*source;*/

    13,/*srcId*/

    GT_TRUE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    3,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum*/    /* set in runtime */
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};
static TGF_PACKET_DSA_TAG_STC  dsaFromRemotePortOfTrunkPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_CNS,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/       /* set in runtime */
};



/* burst size of the CRC hash tests .
NOTE: bad LBH achieved when burst size was 16 or 32 !

Most fields where OK with 64 packets but some of them needed 128 for good LBH !
*/
#define CRC_HASH_BURST_COUNT_NEW_CNS             128
/* number of mac addresses for incremental DA */
#define FDB_MAC_COUNT_CNS             64

/* trunk '1' members */
static CPSS_TRUNK_MEMBER_STC    trunk_1_Members[2];
/* trunk '2' members */
static CPSS_TRUNK_MEMBER_STC    trunk_2_Members[2];

static CPSS_TRUNK_MEMBER_STC    designatedMember3;
static CPSS_TRUNK_MEMBER_STC    designatedMember4;

#define BURST_COUNT_CNS                 30
#define NO_TRAFFIC_EXPECTED_CNS         0
#define FULL_TRAFFIC_EXPECTED_CNS       (BURST_COUNT_CNS * 4)
#define HALF_TRAFFIC_EXPECTED_CNS       (FULL_TRAFFIC_EXPECTED_CNS / 2)

/* the specific members of a trunk that will get traffic when the LBH
   expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E.
   bmp of indexes in the arrays of :
       prvTgfDevsArray[] , prvTgfPortsArray[]
           the traffic should do LBH in those ports and not in other ports of
           the trunk.

       WARNING : this is NOT a good practice to set the system to get this behavior
*/
#define TRUNK_LBH_MEMBERS_INDEXES_ALL_BMP_CNS 0xFFFFFFFF
static GT_U32   trunkLbhMembersIndexesBmp = TRUNK_LBH_MEMBERS_INDEXES_ALL_BMP_CNS;
/* the limited number of members of a trunk that will get traffic when the LBH
   expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E.

       WARNING : this is NOT a good practice to set the system to get this behavior
*/
static GT_U32   trunkLbhNumOfLimitedMembers = 0;

/* hash mask table size (lion and above) */
static GT_U32   hashMaskTableSize = 0xFFFFFFFF;

#define SECOND_HEMISPHERE_CNS   64

/* Bobcat2 - cpssInitSystem 29,1,0 : ports 0..51 , 56,58,80,82 , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPortsFor72Ports[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,       17,         34,         51 , /* ports not in trunk */
     2 ,      19,         41,         82 , /* first 4 members of trunk */
     15,      80,         58,         50 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6,        7   /* next 4 members of trunk */};

/* Bobk-caelum : ports 0.47 , 57,58 , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPortsFor72Ports_bobk[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,       17,         34,         43 , /* ports not in trunk */
     2 ,      11,         19,         41 , /* first 4 members of trunk */
     15,      57,         58,         33 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6,        7   /* next 4 members of trunk */};

/* Bobk-cetus : ports 56..59 , 64..71 , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPortsFor72Ports_bobk_cetus[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {56,      57,         58,         59 , /* ports not in trunk */
     64,      65,         66,         67 , /* first 4 members of trunk */
     68,      69,         70,         71 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};


#ifdef GM_USED
/* according to current appDemo settings of 12 ports only  */
/* Bobcat3 : ports  , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPorts_bobcat3[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         1,          2,          3, /* ports not in trunk */
     18,        36,         54,         58,/* first 4 members of trunk */
     79,        64,         65,         80,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};
/* according to current appDemo settings of 12 ports only  */
/* Aldrin2 : ports  , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPorts_aldrin2[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         1,          2,          3, /* ports not in trunk */
     18,        36,         54,         58,/* first 4 members of trunk */
     79,        64,         65,         80,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};

/* falcon GM have only 8 ports: 0, 18, 36, 58, 54, 64, 79, 80*/
static GT_PHYSICAL_PORT_NUM    testPorts_falcon[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         64,         79,         80, /* ports not in trunk */
     18,        36,         54,         58,/* first 4 members of trunk */
     0xffffffff,0xffffffff, 0xffffffff, 0xffffffff,/* next 4 members of trunk - invalid values because gm falcon have only 8 ports*/
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};

#else /* not GM_USED */
static GT_PHYSICAL_PORT_NUM    testPorts_bobcat3[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         12,         27,         33, /* ports not in trunk */
     16,        36,         54,         58,/* first 4 members of trunk */
     64,        69,         70,         65,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};
static GT_PHYSICAL_PORT_NUM    testPorts_aldrin2[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         12,         27,         33, /* ports not in trunk */
     16,        36,         54,         58,/* first 4 members of trunk */
     64,        69,         70,         65,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};
static GT_PHYSICAL_PORT_NUM    testPorts_falcon[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         12,         27,         33, /* ports not in trunk */
     16,        36,         54,         58,/* first 4 members of trunk */
     64,        67,         68,         65,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};
#endif /* not GM_USED */


/* Aldrin : ports 0..31  , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPorts_aldrin[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         1,          2,          3, /* ports not in trunk */
     4,        10,         16,         20,/* first 4 members of trunk */
     22,       24,         25,         30,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};

/* Aldrin Z0: ports 0..4, 16..19, 32..35, 56..59, 64..71, 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPorts_aldrinZ0[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,         1,          2,          3, /* ports not in trunk */
     16,       32,         64,         68,/* first 4 members of trunk */
     17,       19,         33,         35,/* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};

/* AC3X : ports 0,4, 8-11 , 12..59 , 63(CPU SDMA) */
static GT_PHYSICAL_PORT_NUM    testPorts_xCat3x[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {8             ,9              , 10            ,  11 , /* ports not in trunk */
     0 , 12+4+8*0/*16*/, 12+4+8*3/*40*/, 12+4+8*5/*56*/,   /* first 4 members of trunk */
     12+7+8*2/*35*/, 12+5+8*1/*25*/, 12+5+8*4/*49*/,  4 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     0 ,      0,          0,        0   /* next 4 members of trunk */};

/* Lion */
static GT_PHYSICAL_PORT_NUM    testPortsFor60Ports[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+1,       32+2,       48+3 , /* ports not in trunk */
     2 ,      19,         41,         57 , /* first 4 members of trunk */
     4 ,      27,         37,         50 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6,        7   /* next 4 members of trunk */};

/* Lion2 */
static GT_PHYSICAL_PORT_NUM    testPortsFor124Ports[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+1,       32+2 +SECOND_HEMISPHERE_CNS,       48+3 +SECOND_HEMISPHERE_CNS, /* ports not in trunk */
     2 ,      19,         41 +SECOND_HEMISPHERE_CNS,         57 +SECOND_HEMISPHERE_CNS, /* first 4 members of trunk */
     37 ,     50,         37 +SECOND_HEMISPHERE_CNS,         50 +SECOND_HEMISPHERE_CNS, /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6 +SECOND_HEMISPHERE_CNS,        7 +SECOND_HEMISPHERE_CNS   /* next 4 members of trunk */};

/* Lion2 XLG mode, only local port 0,4,8,9 are used */
static GT_PHYSICAL_PORT_NUM    testPortsFor124Ports_XLG[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+4,       32+8 +SECOND_HEMISPHERE_CNS,       48+9 +SECOND_HEMISPHERE_CNS, /* ports not in trunk */
     4 ,    16+8,       32+9 +SECOND_HEMISPHERE_CNS,       48+8 +SECOND_HEMISPHERE_CNS, /* first 4 members of trunk */
     32+4 , 48+4,       32+4 +SECOND_HEMISPHERE_CNS,       48+4 +SECOND_HEMISPHERE_CNS, /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6 +SECOND_HEMISPHERE_CNS,        7 +SECOND_HEMISPHERE_CNS   /* next 4 members of trunk */};

/* Lion - ports that used with force configuration */
static GT_PHYSICAL_PORT_NUM    testPortsFor60PortsConfigForced[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16,       34,       48 , /* ports not in trunk */
     2 ,    18,       40,       58 , /* first 4 members of trunk */
     4 ,    26,       38,       50 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     20 ,   10,        6,       54   /* next 4 members of trunk */};

/* Lion2 - ports that used with force configuration */
static GT_PHYSICAL_PORT_NUM    testPortsFor124PortsConfigForced[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16,       34 +SECOND_HEMISPHERE_CNS,       48 +SECOND_HEMISPHERE_CNS , /* ports not in trunk */
     2 ,    18,       40 +SECOND_HEMISPHERE_CNS,       58 +SECOND_HEMISPHERE_CNS , /* first 4 members of trunk */
     38,    50,       38 +SECOND_HEMISPHERE_CNS,       50 +SECOND_HEMISPHERE_CNS , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     20 ,   10,        6 +SECOND_HEMISPHERE_CNS,       54 +SECOND_HEMISPHERE_CNS  /* next 4 members of trunk */};

/* cascade trunk in Lion A must be with Id that match to the port group of the cascade ports */
#define PORT_GROUP_OF_CASCADE_TRUNK_LION_A_CNS  (U32_GET_FIELD_MAC(PRV_TGF_TRUNK_ID_CNS,4,2))

#define PORT_OF_CASCADE_TRUNK_LION_A_WITH_OFFSET_MAC(_offset)    \
    (PORT_GROUP_OF_CASCADE_TRUNK_LION_A_CNS << 4) + (_offset)

#define FILLED_IN_RUN_TIME_CNS  0xFFFFFFFF

/*ports of cascade trunk - in the same port group -- limitation for Lion A */
static GT_PHYSICAL_PORT_NUM    testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+1,       32+2,       48+3 , /* ports not in trunk */
     FILLED_IN_RUN_TIME_CNS, /* cascade trunk in Lion A must be with Id that match to the port group of the cascade ports */
     FILLED_IN_RUN_TIME_CNS,
     FILLED_IN_RUN_TIME_CNS,
     FILLED_IN_RUN_TIME_CNS};/* ports of cascade trunk - in the same port group -- limitation for Lion A */

/*ports of cascade trunk - in the same hemisphere -- limitation for Lion2 and Bobcat2; Caelum; Bobcat3 */
static GT_PHYSICAL_PORT_NUM    testPortsFor124Ports_cascadeTrunkPortsSameHemisphere[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+1,       32+2 +SECOND_HEMISPHERE_CNS,       48+3 +SECOND_HEMISPHERE_CNS, /* ports not in trunk */
     (2  + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (19 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (41 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (57 + SECOND_HEMISPHERE_CNS)};/* cascade trunk in Lion2 have all ports in the same hemisphere */


/*ports of cascade trunk for XLG ports - in the same hemisphere -- limitation for Lion2 and Bobcat2; Caelum; Bobcat3 */
static GT_PHYSICAL_PORT_NUM    testPortsFor124Ports_XLG_cascadeTrunkPortsSameHemisphere[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16+4,       32+8 +SECOND_HEMISPHERE_CNS,       48+9 +SECOND_HEMISPHERE_CNS, /* ports not in trunk */
     (0    + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (16+4 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (32+0 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (48+8 + SECOND_HEMISPHERE_CNS)};/* cascade trunk in Lion2 have all ports in the same hemisphere */


/* ports that used with force configuration */
static GT_PHYSICAL_PORT_NUM    testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16,       32,       48 , /* ports not in trunk */
     FILLED_IN_RUN_TIME_CNS, /* cascade trunk in Lion A must be with Id that match to the port group of the cascade ports */
     FILLED_IN_RUN_TIME_CNS,
     FILLED_IN_RUN_TIME_CNS,
     FILLED_IN_RUN_TIME_CNS};/* ports of cascade trunk - in the same port group -- limitation for Lion A */

/*ports of cascade trunk - in the same hemisphere -- limitation for Lion2,3 */
static GT_PHYSICAL_PORT_NUM    testPortsFor124PortsConfigForced_cascadeTrunkPortsSameHemisphere[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,     16,       32 +SECOND_HEMISPHERE_CNS,       48 +SECOND_HEMISPHERE_CNS, /* ports not in trunk */
     (2  + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (20 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (42 + SECOND_HEMISPHERE_CNS), /* cascade trunk in Lion2 have all ports in the same hemisphere */
     (58 + SECOND_HEMISPHERE_CNS)};/* cascade trunk in Lion2 have all ports in the same hemisphere */

/*
NOTE :
in Puma2 :
    24 --> 0 (%24)
    25 --> 1 (%24)
    26 --> 2 (%24)
    27 --> 3 (%24)
--> so must not use :
    0 and 24
    1 and 25
    2 and 26
    3 and 27

in Puma3 : while our tests not supporting virtual ports , we need to use
        physical ports , so the trunk members should be network ports :
        0..11 , 16..27

        --> so make sure that ports 12,13,14,15 are not in this array.
*/

static GT_PHYSICAL_PORT_NUM    testPortsFor28Ports[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,        8,         18,       23 , /* ports not in trunk */
     2 ,      19,         21,       27 , /* first 4 members of trunk */
     4 ,      11,         22,       25 , /* next 4 members of trunk */
     /* last 4 ports are for Puma families only which supports 12 members in trunk*/
     16 ,      9,          6,        7}; /* next 4 members of trunk */

static GT_PHYSICAL_PORT_NUM    crcTestPortsFor28Ports[TRUNK_MAX_PORTS_IN_TEST_CNS] =
    {0,        8,         17 /*18*/,       23 , /* ports not in trunk, don't use port 18 because it get same (CRC_hash % 4) as port 0 */
     2 ,      19,         21,       27 };/* ports of trunk */

static GT_PHYSICAL_PORT_NUM     testDbPortsOrig[PRV_TGF_MAX_PORTS_NUM_CNS];
static GT_U8                    testDbDevsOrig[PRV_TGF_MAX_PORTS_NUM_CNS];
/*
enum for type of tests
*/
typedef enum{
    TRUNK_TEST_TYPE_CRC_HASH_E,
    TRUNK_TEST_TYPE_CASCADE_TRUNK_E,
    TRUNK_TEST_TYPE_DESIGNATED_TABLE_MODES_E,
    TRUNK_TEST_TYPE_MC_LOCAL_SWITCHING_E,
    TRUNK_TEST_TYPE_MPLS_SIMPLE_HASH_E,/* L3L4parsing over MPLS - simple hash */
    TRUNK_TEST_TYPE_MPLS_CRC_HASH_E,   /* L3L4parsing over MPLS - CRC    hash */
    TRUNK_TEST_TYPE_SIMPLE_HASH_E,

    /* keep this as last one */
    TRUNK_TEST_TYPE_MAX_E

}TRUNK_TEST_TYPE_ENT;

/* sorting mode - by default = GT_FALSE */
static GT_BOOL  sortMode = GT_FALSE;

/* array of weights for 1 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_1_members_total_weight_8[] = {500/*don't care*/};
/* array of weights for 2 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_2_members_total_weight_8[] = {3,5};
/* array of weights for 3 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_3_members_total_weight_8[] = {1,4,3};
/* array of weights for 4 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_4_members_total_weight_8[] = {1,2,3,2};
/* array of weights for 5 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_5_members_total_weight_8[] = {1,2,2,2,1};
/* array of weights for 6 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_6_members_total_weight_8[] = {1,2,1,2,1,1};
/* array of weights for 7 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_7_members_total_weight_8[] = {1,1,1,1,1,2,1};
/* array of weights for 8 members , where total weight is 8 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_8_members_total_weight_8[] = {1,1,1,1,1,1,1,1};

typedef struct{
    GT_U32  *weightsArray;
    GT_U32  sizeofArray;
}PORTS_ARRAY_WITH_SIZE_STC;
#define PORTS_ARRAY_WITH_SIZE_MAC(weightsArray) {weightsArray,sizeof(weightsArray)/sizeof(weightsArray[0])}
#define NULL_ARRAY_WITH_SIZE_MAC {NULL,0}

static PORTS_ARRAY_WITH_SIZE_STC  portsArray_total_weight_8[]=
{
/*0*/    NULL_ARRAY_WITH_SIZE_MAC,
/*1*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_1_members_total_weight_8),
/*2*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_2_members_total_weight_8),
/*3*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_3_members_total_weight_8),
/*4*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_4_members_total_weight_8),
/*5*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_5_members_total_weight_8),
/*6*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_6_members_total_weight_8),
/*7*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_7_members_total_weight_8),
/*8*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_8_members_total_weight_8)
};

/* array of weights for 1 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_1_members_total_weight_64[] = {500/*don't care*/};
/* array of weights for 2 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_2_members_total_weight_64[] = {25,39};
/* array of weights for 3 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_3_members_total_weight_64[] = {15,30,19};
/* array of weights for 4 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_4_members_total_weight_64[] = {17,6,23,18};
/* array of weights for 5 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_5_members_total_weight_64[] = {21,7,15,12,9};
/* array of weights for 6 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_6_members_total_weight_64[] = {10,5,20,1,18,10};
/* array of weights for 7 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_7_members_total_weight_64[] = {2,4,8,16,20,5,9};
/* array of weights for 8 members , where total weight is 64 .
   good for all DX for regular,cascade trunk
*/
static GT_U32   weightsInTrunk_8_members_total_weight_64[] = {5,10,5,10,5,10,15,4};

static PORTS_ARRAY_WITH_SIZE_STC  portsArray_total_weight_64[]=
{
/*0*/    NULL_ARRAY_WITH_SIZE_MAC,
/*1*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_1_members_total_weight_64),
/*2*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_2_members_total_weight_64),
/*3*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_3_members_total_weight_64),
/*4*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_4_members_total_weight_64),
/*5*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_5_members_total_weight_64),
/*6*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_6_members_total_weight_64),
/*7*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_7_members_total_weight_64),
/*8*/    PORTS_ARRAY_WITH_SIZE_MAC(weightsInTrunk_8_members_total_weight_64)
};

/* indication if the trunk using weighted trunk members */
static GT_BOOL     trunkUseWeightedPorts = GT_FALSE;
/* pointer to one of:
portsArray_total_weight_8
or
portsArray_total_weight_64
*/
static PORTS_ARRAY_WITH_SIZE_STC  *portsArray_total_weight_Ptr = portsArray_total_weight_8;


/* number of seeds to test with CRC-16 */
#define SEED_CRC_16_NUM_CNS   6
/* number of seeds to test with CRC-6 */
#define SEED_CRC_6_NUM_CNS    4
/* seeds to test with CRC 6/16/32 , the last 2 seeds not needed for CRC-6
    since (seed & 0x3f) on those values will give same value as on the previous
    2 seeds */
static GT_U32 crcSeeds[SEED_CRC_16_NUM_CNS] = {0xFFFFFFFF , 0xAAAAAAAA , 0x55555555 , 0 ,0xA5A5A5A5 , 0x5A5A5A5A};
/******************************* Test packet **********************************/

/******************************* common payload *******************************/

/* Data of packet (long version) */
static GT_U8 prvTgfPacketPayloadLongDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* size of prvTgfPacketPayloadLongDataArr */
#define  PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS  \
    sizeof(prvTgfPacketPayloadLongDataArr)

/* Payload part (long version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketLongPayloadPart =
{
    PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS, /* dataLength */
    prvTgfPacketPayloadLongDataArr          /* dataPtr */
};


/* copy of prvTgfPacketL2Part for restore purposes */
static TGF_PACKET_L2_STC prvTgfPacketL2PartOrig;


/* common L2 traffic for all packets of prvTgfTrunkTrafficTypes[] */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee},                /* daMac */
    {0x00, 0xbb, 0xcc, 0xdd, 0xee, 0xff}                 /* saMac */
};

/* 'Remote device' number */
#define REMOTE_DEVICE_NUMBER_CNS       7

#define MAX_VLANS_CNS    5

/* vlans array */
static GT_U16   prvTgfVlansArr[MAX_VLANS_CNS]={1 , 3, 555, 1234, 4094};
static GT_U32   prvTgfVlansNum = 2;/* number of vlan used in test */

/* Selected port numbers */
static GT_U32   prvTgfLbhSelectedPortsNum = 1;

static GT_BOOL  prvTgfPacketVlanTagPartOrigValid = GT_FALSE;
/* copy of prvTgfPacketVlanTagPart for restore purposes */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPartOrig;

/* First VLAN_TAG part */
TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* size of L2 (mac SA,Da) + vlan TAG + ethertype */
#define L2_VLAN_TAG_ETHERETYPE_SIZE_CNS \
                    (TGF_L2_HEADER_SIZE_CNS +/*l2*/         \
                     TGF_VLAN_TAG_SIZE_CNS  +/*vlan tag*/   \
                     TGF_ETHERTYPE_SIZE_CNS )/*ethertype*/



/******************************* MPLS packet **********************************/

/* MAC_DA[5:0] == MAC_SA[5:0] to cause         */
/* MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0] == 0 */
static TGF_PACKET_L2_STC prvTgfPacketMplsL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart =
{TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsPart =
{
    BASE_MPLS_LABEL_VALUE_CNS + 1,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x40                /* timeToLive */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketMplsL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLable1Part =
{
    BASE_MPLS_LABEL_VALUE_CNS + 1,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> NOT end of stack (last bit = 0) */
    0x15                /* timeToLive */
};
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLable2Part =
{
    BASE_MPLS_LABEL_VALUE_CNS + 2,                  /* label */
    0,                  /* experimental use */
    0,                  /* stack --> NOT end of stack (last bit = 0) */
    0x16                /* timeToLive */
};
/* packet's MPLS */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsLable3Part =
{
    BASE_MPLS_LABEL_VALUE_CNS + 3,                  /* label */
    0,                  /* experimental use */
    1,                  /* stack --> end of stack (last bit) */
    0x17                /* timeToLive */
};

/* PARTS of packet MPLS -- 3 labels */
static TGF_PACKET_PART_STC prvTgfMpls3LabelsPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable1Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable3Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* size of bytes till start of passenger on MPLS packet */
#define PRV_TGF_MPLS_PACKET_PASSENGER_OFFSET_CNS(numLables) \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + (numLables*TGF_MPLS_HEADER_SIZE_CNS)

/* Length of MPLS packet */
#define PRV_TGF_MPLS_PACKET_LEN_CNS(numLables) \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + (numLables*TGF_MPLS_HEADER_SIZE_CNS) + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS + TGF_CRC_LEN_CNS

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsPacketInfo =
{
    PRV_TGF_MPLS_PACKET_LEN_CNS(1),                 /* totalLen */
    (sizeof(prvTgfMplsPacketPartArray)
        / sizeof(prvTgfMplsPacketPartArray[0])), /* numOfParts */
    prvTgfMplsPacketPartArray                    /* partsArray */
};

/* MPLS packet to send - 3 labels */
static TGF_PACKET_STC prvTgfMpls3LablesPacketInfo =
{
    PRV_TGF_MPLS_PACKET_LEN_CNS(3) + TGF_VLAN_TAG_SIZE_CNS,                 /* totalLen */
    (sizeof(prvTgfMpls3LabelsPacketPartArray)
        / sizeof(prvTgfMpls3LabelsPacketPartArray[0])), /* numOfParts */
    prvTgfMpls3LabelsPacketPartArray                    /* partsArray */
};

/******************************* IPv4 packet **********************************/

/* MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0] == 0 */
static TGF_PACKET_L2_STC prvTgfPacketIpShiftL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x34, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x34, 0x00}                 /* saMac */
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
/* SIP[5:0]==1, DIP[5:1]==0 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 7,  0,  8,   0},    /* srcAddr */
    { 1,  1,  1,   0}    /* dstAddr */
};

/* PARTS of packet IPv4 - IPV4 OTHER */
static TGF_PACKET_PART_STC prvTgfIpv4OtherPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpShiftL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS+ TGF_CRC_LEN_CNS

/* IPv4 Other packet to send */
static TGF_PACKET_STC prvTgfIpv4OtherPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                      /* totalLen */
    (sizeof(prvTgfIpv4OtherPacketPartArray)
        / sizeof(prvTgfIpv4OtherPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4OtherPacketPartArray                    /* partsArray */
};

/******************************* ETH_OTHER packet *****************************/
/* MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0] == 1 */
static TGF_PACKET_L2_STC prvTgfPacketMaskL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x34, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x34, 0x01}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOtherEtherTypePart = {0x1234};

/* PARTS of packet ETHERNET OTHER */
static TGF_PACKET_PART_STC prvTgfEthOtherPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketMaskL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOtherEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_ETH_OTHER_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS+ TGF_CRC_LEN_CNS

/* packet to send */
static TGF_PACKET_STC prvTgfPacketEthOtherInfo =
{
    PRV_TGF_PACKET_ETH_OTHER_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfEthOtherPacketPartArray) / sizeof(prvTgfEthOtherPacketPartArray[0]), /* numOfParts */
    prvTgfEthOtherPacketPartArray                                                     /* partsArray */
};

/*c.Send unknown unicast (00:09:99:99:99:99) with 100 incremental DA and static SA */
/*d.Send unknown unicast (00:09:99:99:99:99) with 100 static DA and incremental SA*/
static TGF_PACKET_L2_STC multiDestination_prvTgfPacketL2Part_unk_UC =
{
    {0x00, 0x09, 0x99, 0x99, 0x99, 0x99},                /* daMac */
    {0x00, 0x01, 0x11, 0x11, 0x11, 0x11}                 /* saMac */
};

/******************************* simple vlan tagged packet **********************************/
/* port bitmap VLAN members */
static CPSS_PORTS_BMP_STC localPortsVlanMembers = {{0, 0}};

/* PARTS of packet */
TGF_PACKET_PART_STC prvTgfPacketVlanTagPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &multiDestination_prvTgfPacketL2Part_unk_UC},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketLongPayloadPart}
};
/* Length of MPLS packet */
#define PRV_TGF_VLAN_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
    PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS+ TGF_CRC_LEN_CNS

/* PACKET to send */
TGF_PACKET_STC prvTgfPacketVlanTagInfo = {
    PRV_TGF_VLAN_TAGGED_PACKET_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfPacketVlanTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketVlanTagPartArray                                        /* partsArray */
};


/***********************************************/
/* traffic types */

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv6 = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

#define IPV6_DUMMY_PROTOCOL_CNS  0x3b

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    IPV6_DUMMY_PROTOCOL_CNS, /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff02, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* LENGTH of ipv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS+ TGF_CRC_LEN_CNS

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketIpv6Info = {
    PRV_TGF_IPV6_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]), /* numOfParts */
    prvTgfIpv6PacketArray                                        /* partsArray */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    0                   /* urgent pointer */
};


/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4TcpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_TCP_E, /* TCP -- protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum - need calc */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv4 - TCP */
static TGF_PACKET_PART_STC prvTgfIpv4TcpPacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4TcpPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* LENGTH of ipv4 TCP packet */
#define PRV_TGF_IPV4_TCP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS

/* ipv4 TCP to send */
static TGF_PACKET_STC prvTgfIpv4TcpPacketInfo = {
    PRV_TGF_IPV4_TCP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS ,                      /* totalLen */
    sizeof(prvTgfIpv4TcpPacketArray) / sizeof(prvTgfIpv4TcpPacketArray[0]), /* numOfParts */
    prvTgfIpv4TcpPacketArray                                        /* partsArray */
};


/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4UdpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_UDP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfIpv4UdpPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* LENGTH of ipv4 UDP packet */
#define PRV_TGF_IPV4_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS

/* ipv4 UDP to send */
static TGF_PACKET_STC prvTgfIpv4UdpPacketInfo = {
    PRV_TGF_IPV4_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS ,                      /* totalLen */
    sizeof(prvTgfIpv4UdpPacketArray) / sizeof(prvTgfIpv4UdpPacketArray[0]), /* numOfParts */
    prvTgfIpv4UdpPacketArray                                        /* partsArray */
};

#define IPV4_FRAGMENT_DUMMY_PROTOCOL_CNS    4

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4FragmentPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */    /* more fragments     */
    0x200,              /* offset */   /* not first fragment */
    0x40,               /* timeToLive */
    IPV4_FRAGMENT_DUMMY_PROTOCOL_CNS,  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* PARTS of packet IPv4 - IPV4 FRAGMENT */
static TGF_PACKET_PART_STC prvTgfIpv4FragmentPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4FragmentPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* IPv4 Fragment packet to send */
static TGF_PACKET_STC prvTgfIpv4FragmentPacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS,                         /* totalLen */
    (sizeof(prvTgfIpv4FragmentPacketPartArray)
        / sizeof(prvTgfIpv4FragmentPacketPartArray[0])), /* numOfParts */
    prvTgfIpv4FragmentPacketPartArray                    /* partsArray */
};


/* PARTS of packet IPv4 - IPV4 OTHER */
static TGF_PACKET_PART_STC prvTgfIpv4OtherPacketPart1Array[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* IPv4 Other packet to send */
static TGF_PACKET_STC prvTgfIpv4OtherPacket1Info =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS,                      /* totalLen */
    (sizeof(prvTgfIpv4OtherPacketPart1Array)
        / sizeof(prvTgfIpv4OtherPacketPart1Array[0])), /* numOfParts */
    prvTgfIpv4OtherPacketPart1Array                    /* partsArray */
};




/* PARTS of packet ETHERNET OTHER */
static TGF_PACKET_PART_STC prvTgfEthOtherPacketPart1Array[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOtherEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketEthOther1Info =
{
    PRV_TGF_PACKET_ETH_OTHER_LEN_CNS + TGF_VLAN_TAG_SIZE_CNS,                                             /* totalLen */
    sizeof(prvTgfEthOtherPacketPart1Array) / sizeof(prvTgfEthOtherPacketPart1Array[0]), /* numOfParts */
    prvTgfEthOtherPacketPart1Array                                                     /* partsArray */
};

/* number of UDEs */
#define NUM_UDE_CNS     5
/* use this ether type as base value for the 5 UDEs */
#define UDE_BASE_CNS    0x5678

/* ether type part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketUdeTypePart = {UDE_BASE_CNS};

/* PARTS of packet ETHERNET OTHER */
static TGF_PACKET_PART_STC prvTgfEthUdePartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketUdeTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_UDE_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS \
    + PRV_TGF_PACKET_PAYLOAD_LONG_DATA_ARR_LEN_CNS

/* packet to send */
static TGF_PACKET_STC prvTgfPacketUdeInfo =
{
    PRV_TGF_PACKET_UDE_LEN_CNS+ TGF_CRC_LEN_CNS ,                                             /* totalLen */
    sizeof(prvTgfEthUdePartArray) / sizeof(prvTgfEthUdePartArray[0]), /* numOfParts */
    prvTgfEthUdePartArray                                                     /* partsArray */
};

/* array of traffic types */
static TGF_PACKET_STC *prvTgfTrunkTrafficTypes[PRV_TGF_PCL_PACKET_TYPE_LAST_E] =
{
     &prvTgfIpv4TcpPacketInfo/* ipv4 - TCP */
    ,&prvTgfIpv4UdpPacketInfo/* ipv4 - UDP */
    ,&prvTgfMpls3LablesPacketInfo/* mpls */
    ,&prvTgfIpv4FragmentPacketInfo/* ipv4 - fragment */
    ,&prvTgfIpv4OtherPacket1Info/* ipv4 - other */
    ,&prvTgfPacketEthOther1Info/* ethernet */
    ,&prvTgfPacketUdeInfo/* ude 0 */
    ,&prvTgfPacketIpv6Info/* ipv6 */
    ,&prvTgfPacketUdeInfo/* ude 1 */
    ,&prvTgfPacketUdeInfo/* ude 2 */
    ,&prvTgfPacketUdeInfo/* ude 3 */
    ,&prvTgfPacketUdeInfo/* ude 4 */
};

TGF_PACKET_STC * prvTgfTrunkPacketGet(IN PRV_TGF_PCL_PACKET_TYPE_ENT packetType)
{
    if(packetType < PRV_TGF_PCL_PACKET_TYPE_LAST_E)
    {
        return prvTgfTrunkTrafficTypes[packetType];
    }

    return NULL;
}


/* we have 4 ports out side the trunk that each port will use different type of traffic .
   make sure to add 'add Mac info' for IP packets for the LBH

NOTE: all those packets uses prvTgfPacketL2Part as L2 part !
                    and uses prvTgfPacketVlanTagPart as vlan tag part !
*/
static TGF_PACKET_STC* packetsForSimpleHashMacSaDaArr[NOT_IN_TRUNK_PORTS_NUM_CNS]={
    &prvTgfMpls3LablesPacketInfo,
    &prvTgfPacketEthOther1Info,
    &prvTgfPacketIpv6Info,
    &prvTgfIpv4UdpPacketInfo
};


/* Pearson hash modes for filling the table with values */
typedef enum{
    PEARSON_TEST_MODE_INDEX_E,          /*value = index (0..63)*/
    PEARSON_TEST_MODE_REVERSE_INDEX_E,  /*value = 63 - index (63..0)*/
    PEARSON_TEST_MODE_START_MID_INDEX_E,/*value = (32 + index) % 64 (32..63,0..31)*/
    PEARSON_TEST_MODE_EVEN_INDEX_E,     /*value = index & 0x3e (0,0,2,2..62,62)*/
    PEARSON_TEST_MODE_MODE_8_INDEX_E,   /*value = index & 0x7 (0..7,0..7,0..7,...,0..7)*/

    PEARSON_TEST_MODE_0_E,              /*value = 0 (all indexes) */

    PEARSON_TEST_MODE_LAST_E

}PEARSON_TEST_MODE_ENT;

/* get string for Pearson mode */
#define TO_STRING_PEARSONE_MODE_MAC(_pearsonMode)    \
        (                                        \
        _pearsonMode ==    PEARSON_TEST_MODE_INDEX_E               ? "INDEX"           :  \
        _pearsonMode ==    PEARSON_TEST_MODE_REVERSE_INDEX_E       ? "REVERSE_INDEX"   :  \
        _pearsonMode ==    PEARSON_TEST_MODE_START_MID_INDEX_E     ? "START_MID_INDEX" :  \
        _pearsonMode ==    PEARSON_TEST_MODE_EVEN_INDEX_E          ? "EVEN_INDEX"      :  \
        _pearsonMode ==    PEARSON_TEST_MODE_MODE_8_INDEX_E        ? "MODE_8_INDEX"    :  \
        _pearsonMode ==    PEARSON_TEST_MODE_0_E                   ? "MODE_0"          :  \
                                                                              "???")


/* get string for packet type  */
#define TO_STRING_PACKET_TYPE_MAC(_trafficType)    \
        (                                        \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E       ? "IPV4_TCP       " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E       ? "IPV4_UDP       " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_MPLS_E           ? "MPLS           " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E  ? "IPV4_FRAGMENT  " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E     ? "IPV4_OTHER     " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E ? "ETHERNET_OTHER " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_UDE_E            ? "UDE_0          " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_IPV6_E           ? "IPV6           " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_UDE_1_E          ? "UDE_1          " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_UDE_2_E          ? "UDE_2          " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_UDE_3_E          ? "UDE_3          " : \
    _trafficType == PRV_TGF_PCL_PACKET_TYPE_UDE_4_E          ? "UDE_4          " : \
                                                                              "???")



/* number of traffic types sent by prvTgfCommonMultiDestinationTrafficTypeSend(...) */
#define MULTI_DEST_COUNT_MAC    4
/**
* @enum PRV_TGF_TRUNK_HASH_TEST_PACKET_ENT
 *
 * @brief enumerator packets used in trunk hash tests
*/
typedef enum{

    /** @brief ETH_OTHER packet,
     *  MAC_DA(5:0]==0, MAC_SA[5:0]==1
     *  incremental SA LBH check
     */
    PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E,

    /** @brief ETH_OTHER packet,
     *  incremental DA LBH check
     */
    PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E,

    /** @brief IPV4 packet,
     *  DIP[5:0]==0, SIP[5:0]==1
     */
    PRV_TGF_TRUNK_HASH_TEST_PACKET_IPV4_SIP_E,

    /** @brief MPLS packet
     *  MAC_DA(5:0]==0, MAC_SA[5:0]==0, LABEL0[5:0]==1
     */
    PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_LAB0_E,

    /** IPV4/6 over MPLS packet */
    PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_IPvX_PASSENGER_E,

    PRV_TGF_TRUNK_HASH_TEST_PACKET_LAST_E

} PRV_TGF_TRUNK_HASH_TEST_PACKET_ENT;

/* if flag don't match the tested field then we don't need to run traffic ... not relevant */
#define RETURN_IF_FIELD_NOT_RELEVANT_MAC(flag) \
             if(flag == GT_FALSE)              \
             {                                 \
                if(debugMode)                  \
                {                              \
                    PRV_UTF_LOG1_DEBUG_MAC(" debug ERROR ---> value is 0 for [%s] --> field not relevant \n",#flag);\
                }                              \
                return GT_OK;                  \
             }

/* the ipv4 is 'Shifted 12 bytes in the Ipv6 field' */
#define IPV4_OFFSET_FROM_BYTE_12_CNS     12

/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* stored default hash mode */
static PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT prvTgfDefHashMode = PRV_TGF_TRUNK_LBH_PACKETS_INFO_E;

/* stored default hash mode - ipv6 */
static PRV_TGF_TRUNK_IPV6_HASH_MODE_ENT  prvTgfDefHashModeIpv6 = 0;

/* stored default hash mode - L4 */
static PRV_TGF_TRUNK_L4_LBH_MODE_ENT prvTgfDefHashModeL4 = 0;


typedef enum{
    PRV_TGF_TRUNK_DESIGNATED_PORTS_BASIC_E,
    PRV_TGF_TRUNK_DESIGNATED_PORTS_SET_DESIGNATED_AFTER_TRUNK_SETTING_ENT,
    PRV_TGF_TRUNK_DESIGNATED_PORTS_ADD_PORTS_INSTEAD_OF_SET_ENT
}PRV_TGF_TRUNK_DESIGNATED_PORTS_MUTATION_ENT;
static PRV_TGF_TRUNK_DESIGNATED_PORTS_MUTATION_ENT designatedPortsMutation = PRV_TGF_TRUNK_DESIGNATED_PORTS_BASIC_E;
static GT_BOOL designatedPort4AsDisabled = GT_FALSE;

static GT_U32   debug_burstNum = 0;

static GT_U32   debugMode = 0;
static GT_U32   debug_hashMaskTableIndex        = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;
static GT_U32   debug_hashMaskFieldType         = PRV_TGF_TRUNK_FIELD_MAC_ADDR_E;
static GT_U32   debug_hashMaskFieldSubType      = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
static GT_U32   debug_hashMaskFieldByteIndex    = 5;
static GT_U32   debug_pearsonMode               = PEARSON_TEST_MODE_INDEX_E;
static GT_U32   debug_crcMode                   = PRV_TGF_TRUNK_LBH_CRC_6_MODE_E;
static GT_U32   debug_seedIndex                 = 0;

#ifdef CHX_FAMILY
    /*default for DXCH*/
    #define PRV_TGF_TRUNK_DESIGNATED_TABLE_DEFAULT_CNS  PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E
#else
    /*default for Puma*/
    #define PRV_TGF_TRUNK_DESIGNATED_TABLE_DEFAULT_CNS  PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E
#endif

static GT_U32   debug_desigMode                 = PRV_TGF_TRUNK_DESIGNATED_TABLE_DEFAULT_CNS;
static GT_U32   debug_lbhGenerated              = 0;


/* if debug mode break */
#define IF_DEBUG_MODE_BREAK_MAC(debugMode) \
    if(debugMode) break

static void mcLocalSwitchingSendAndCheck
(
    IN GT_U32  lastPortIndex,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode
);


/* mode of adding MAC SA+DA to simple hash mode of the IP packets */
static GT_BOOL  prvTgfTrunkHashIpAddMacMode = GT_FALSE;





/* PARTS of packet MPLS -- 3 labels */
static TGF_PACKET_PART_STC prvTgfMplsIpvxPassengerPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable1Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable2Part},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsLable3Part},

    {TGF_PACKET_PART_SKIP_E,        NULL},/* L3 header - IPv4/6  --> see L3L4PassengerMatrixArr[].l3HeaderPointer*/
    {TGF_PACKET_PART_SKIP_E,        NULL},/* L4 header - TCP/UDP --> see L4PassengerHeader[].l4HeaderPointer */

    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketLongPayloadPart}
};

/* MPLS packet to send */
static TGF_PACKET_STC prvTgfMplsIpvxPassengerPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS /* calc in this file -- not by engine */, /* totalLen */
    (sizeof(prvTgfMplsIpvxPassengerPacketArray)
        / sizeof(prvTgfMplsIpvxPassengerPacketArray[0])), /* numOfParts */
    prvTgfMplsIpvxPassengerPacketArray                    /* partsArray */
};


/* dummy protocol -- should be set according to test in runtime */
#define IPV4_DUMMY_PROTOCOL_CNS   0xDD

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    IPV4_DUMMY_PROTOCOL_CNS,  /* protocol -- set in runtime */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum - need calc */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* IPv4 header */
static TGF_PACKET_PART_STC   prvTgfPacketIpv4Header =
    {TGF_PACKET_PART_IPV4_E,       &prvTgfPacketIpv4Part};

/* IPv4 fragment header */
static TGF_PACKET_PART_STC   prvTgfPacketIpv4FragmentHeader =
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4FragmentPart};

/* IPv4 other header */
static TGF_PACKET_PART_STC   prvTgfPacketIpv4OtherHeader =
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart};

    /* IPv6 header */
static TGF_PACKET_PART_STC   prvTgfPacketIpv6Header =
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part};

typedef struct{
    TGF_PACKET_PART_STC     *l3HeaderPointer;
    char*                   l3HeaderName;
    GT_BOOL                 isL4ModificationModifyLbh;/* should changes in L4 header cause change of LBH */
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;
}L3_L4_PASSENGER_MATRIX_STC;
/* array of L3+L4 connections and expected behavior when LBH according to L3+L4 enabled */
static L3_L4_PASSENGER_MATRIX_STC L3L4PassengerMatrixArr[]=
{
    {&prvTgfPacketIpv4OtherHeader,"Ipv4 other", GT_FALSE , PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E}, /* Ipv4 - other -- L4 protocol not counted for hashing */
    {&prvTgfPacketIpv4FragmentHeader,"Ipv4 fragment", GT_FALSE , PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E},/* Ipv4 - fragment -- NO L4 protocol !
                                                    but used with TCP/UDP header to check
                                                    that device should ignore changes in it
                                                    for trunk hashing ! */
    {&prvTgfPacketIpv4Header,"Ipv4 for TCP/UDP", GT_TRUE , PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E},         /* ipv4 */
    {&prvTgfPacketIpv6Header,"Ipv6 for TCP/UDP", GT_TRUE , PRV_TGF_PCL_PACKET_TYPE_IPV6_E},         /* ipv6 */
    {NULL                   ,"", GT_FALSE  }/* NULL - indication for 'last'*/
};

/* TCP header */
static TGF_PACKET_PART_STC   prvTgfPacketTcpHeader =
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart};

/* UDP header */
static TGF_PACKET_PART_STC   prvTgfPacketUdpHeader =
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart};

typedef struct{
    TGF_PACKET_PART_STC     *l4HeaderPointer;
    GT_BOOL                 protocol;/* the protocol of this L4 traffic (in the IP header) */
}L4_PROTOCOL_INFO_STC;

/* array of L3 as passengers */
static L4_PROTOCOL_INFO_STC   L4PassengerHeader[] =
{
    {&prvTgfPacketTcpHeader  , TGF_PROTOCOL_TCP_E  },
    {&prvTgfPacketUdpHeader  , TGF_PROTOCOL_UDP_E  },
    {NULL                    , TGF_PROTOCOL_MAX_E  }/* NULL - indication for 'last'*/
};



static GT_U32   getSendingInfo = 0;
static struct{
    TGF_VFD_INFO_STC     vfdArray[1];
}sendingInfo;

#define PRV_TGF_CNC_BLOCK_IPCL1 (0)
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
static GT_STATUS trunkHashCrcLbhTrafficSend
(
    IN TGF_PACKET_STC      *trafficPtr,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType,
    IN PRV_TGF_TRUNK_FIELD_TYPE_ENT fieldType,
    IN GT_U32              fieldSubType,
    IN GT_U32              fieldByteIndex,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN GT_U32               L3Offset,
    IN GT_BOOL              checkSimpleHash,
    IN GT_U32               reduceTimeFactor,
    OUT GT_BOOL             *wasIgnoredDueToReduceTimePtr
);

static void prvTgfTrunkGenericRestore
(
    IN GT_BOOL cascadeTrunk
);

static void prvTgfTrunkGenericInit
(
    IN TRUNK_TEST_TYPE_ENT testType,
    IN GT_BOOL cascadeTrunk,
    IN GT_U32   numOfPortsInTest
);

static GT_STATUS trunkHashMaskCrcTableEntryClear
(
    IN GT_U32              index
);

static GT_STATUS trunkHashPearsonFill
(
    IN PEARSON_TEST_MODE_ENT   pearsonMode
);

static GT_STATUS trunkHashMaskCrcFieldSet
(
    IN GT_U32              index,
    IN PRV_TGF_TRUNK_FIELD_TYPE_ENT fieldType,
    IN GT_U32              fieldSubType,
    IN GT_U32              fieldByteIndex
);

/* TTI0 and TTI1 Rule indexes */
static GT_U32 prvTgfTtiRuleLookup0Index;
static GT_U32 prvTgfTtiRuleLookup1Index;

static GT_U32 PRV_TGF_SEND_PORT_IDX_CNS = 0;

/**
* @internal tgfTrunkEportBase function
* @endinternal
*
* @brief   Get base ePort for the device.
*/
GT_U32 tgfTrunkEportBase
(
    GT_VOID
)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
    if(CPSS_PP_FAMILY_DXCH_AC3X_E == devFamily)
    {
        return 512;
    }
    return UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum) > _6K ? _6K : _4K;
}

/**
* @internal tgfTrunkDebugCrcHash function
* @endinternal
*
* @brief   debug function to run test of CRC hash for specific
*/
GT_STATUS tgfTrunkDebugCrcHash
(
    IN GT_U32   hashMaskTableIndex,
    IN GT_U32   hashMaskFieldType,
    IN GT_U32   hashMaskFieldSubType,
    IN GT_U32   hashMaskFieldByteIndex,
    IN GT_U32   pearsonMode,  /* also numLables --> see mplsWithIpPassengerLbhSendAndCheck*/
    IN GT_U32   crcMode,      /* also matrixIndex --> see mplsWithIpPassengerLbhSendAndCheck*/
    IN GT_U32   seedIndex     /* also isIpv4 --> see mplsWithIpPassengerLbhSendAndCheck*/
)
{
    debugMode = 1;
    debug_hashMaskTableIndex     = hashMaskTableIndex      ;
    debug_hashMaskFieldType      = hashMaskFieldType       ;
    debug_hashMaskFieldSubType   = hashMaskFieldSubType    ;
    debug_hashMaskFieldByteIndex = hashMaskFieldByteIndex  ;
    debug_pearsonMode            = pearsonMode             ;
    debug_crcMode                = crcMode                 ;
    debug_seedIndex              = seedIndex               ;

    return GT_OK;
}

/* clear all the counter of the test */
void clearAllTestedCounters
(
    void
)
{
    GT_U8   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;/*device number to send packets , and port number */
    GT_U32  portIter;/* port iterator */

    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        devNum  = prvTgfDevsArray [portIter];
        portNum = prvTgfPortsArray[portIter];

        if(devNum != prvTgfDevNum)
        {
            continue;
        }

        TRUNK_PRV_UTF_LOG2_MAC("=======  : dev[%d]port[%d] reset counters  =======\n",
            devNum, portNum);
        /* reset counters */
        prvTgfResetCountersEth(devNum, portNum);
    }
}
/* force link up on all tested ports */
void forceLinkUpOnAllTestedPorts
(
    void
)
{
    GT_STATUS    rc;
    GT_U32  portIter;/* port iterator */
    CPSS_INTERFACE_INFO_STC portInterface;

    TRUNK_PRV_UTF_LOG0_MAC("======= force links up =======\n");

    portInterface.type            = CPSS_INTERFACE_PORT_E;

    /* force linkup on all ports involved */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        portInterface.devPort.hwDevNum  = prvTgfDevsArray[portIter];
        portInterface.devPort.portNum = prvTgfPortsArray[portIter];

        if(portInterface.devPort.hwDevNum != prvTgfDevNum)
        {
            continue;
        }

        rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface,GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet: port %d/%d",
                                                portInterface.devPort.hwDevNum,
                                                portInterface.devPort.portNum);
    }
}


/**
* @internal prvTgfTrunkForHashTestConfigure function
* @endinternal
*
* @brief   This function configures trunk of two ports
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkForHashTestConfigure
(
    GT_VOID
)
{
    static CPSS_TRUNK_MEMBER_STC          enabledMembersArray[2];
    GT_STATUS                      rc;
    GT_TRUNK_ID             trunkId;

    prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_SIMPLE_HASH_E,GT_FALSE,2);/*allow test to be single with out others before it*/

    /* AUTODOC: SETUP CONFIGURATION: */
    enabledMembersArray[0].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT0_INDEX_CNS];
    enabledMembersArray[0].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT0_INDEX_CNS];
    enabledMembersArray[1].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];
    enabledMembersArray[1].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: create trunk 25 with ports [0,2] */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        2 /*numOfEnabledMembers*/,
        enabledMembersArray,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* save default hashMode */
    rc = prvTgfTrunkHashGlobalModeGet(prvTgfDevNum, &prvTgfDefHashMode);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashGlobalModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: set general hashing mode - hash is based on the packets data */
    rc = prvTgfTrunkHashGlobalModeSet(
        PRV_TGF_TRUNK_LBH_PACKETS_INFO_E);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* save default Ipv6 mode */
    rc = prvTgfTrunkHashIpv6ModeGet(prvTgfDevNum, &prvTgfDefHashModeIpv6);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashIpv6ModeGet FAILED, rc = [%d]", rc);
        return rc;
    }


    /* save default L4 mode */
    rc = prvTgfTrunkHashL4ModeGet(prvTgfDevNum, &prvTgfDefHashModeL4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashL4ModeGet FAILED, rc = [%d]", rc);
        return rc;
    }



    return GT_OK;
}

/**
* @internal prvTgfTrunkForHashTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkForHashTestConfigurationReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID             trunkId;

    /* AUTODOC: RESTORE CONFIGURATION: */

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: clear TRUNK 25 */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: restore default general hashing mode */
    rc = prvTgfTrunkHashGlobalModeSet(prvTgfDefHashMode);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: restore default IPv6 hash generation mode */
    rc = prvTgfTrunkHashIpv6ModeSet(prvTgfDefHashModeIpv6);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashIpv6ModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: restore default L4 hash generation mode */
    rc = prvTgfTrunkHashL4ModeSet(prvTgfDefHashModeL4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashL4ModeSet FAILED, rc = [%d]", rc);
        return rc;
    }

    prvTgfTrunkGenericRestore(GT_FALSE);


    return GT_OK;
}

/**
* @internal prvTgfTrunkForHashTestSendPacketAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               -  burst size
* @param[in] expectedEgressFromPort0  - amount of packets expected to egress from
*                                      thunk member 0
* @param[in] expectedEgressFromPort1  - amount of packets expected to egress from
*                                      thunk member 1
*                                       None
*/
static GT_VOID prvTgfTrunkForHashTestSendPacketAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,
    IN GT_U32            burstCount,
    IN GT_U32            expectedEgressFromPort0,
    IN GT_U32            expectedEgressFromPort1
)
{
    GT_STATUS                       rc           = GT_OK;
    GT_U32                          portIter     = 0;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    static GT_U32                          portNums[3] =
        {
            PRV_TGF_SEND_PORT_INDEX_CNS,
            PRV_TGF_TRUNK_PORT0_INDEX_CNS,
            PRV_TGF_TRUNK_PORT1_INDEX_CNS
        };
    GT_U32                          expectedPackets[3];
    GT_U32  isSinglePort;/*is single port from the trunk should get the traffic */
    GT_U32  sumPacketsOut = 0;/* sum the number of packets egress the trunk */

    TRUNK_PRV_UTF_LOG0_MAC("======= Reset Counters =======\n");

    expectedPackets[0] = burstCount;
    expectedPackets[1] = expectedEgressFromPort0;
    expectedPackets[2] = expectedEgressFromPort1;

    if(burstCount == expectedEgressFromPort0 ||
       burstCount == expectedEgressFromPort1 )
    {
        isSinglePort = 1;
    }
    else
    {
        isSinglePort = 0;
    }

    for (portIter = 0; (portIter < 3); portIter++)
    {
        /* reset counters */
        rc = prvTgfResetCountersEth(
            prvTgfDevsArray[portNums[portIter]],
            prvTgfPortsArray[portNums[portIter]]);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfResetCountersEth: %d, %d\n",
            prvTgfDevsArray[portNums[portIter]],
            prvTgfPortsArray[portNums[portIter]]);
    }

    TRUNK_PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_INDEX_CNS],
        packetInfoPtr, burstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevsArray[PRV_TGF_SEND_PORT_INDEX_CNS], burstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* send Packet from port 8 */
    rc = prvTgfStartTransmitingEth(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth: %d %d",
        prvTgfDevsArray[PRV_TGF_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);

    /* Delay 100 milliseconds - give to sent packet to pass all pilelines in PP */
    cpssOsTimerWkAfter(100);


    /* read and check ethernet counters */
    for (portIter = 0; (portIter < 3); portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portNums[portIter]],
            prvTgfPortsArray[portNums[portIter]],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
            prvTgfDevsArray[portNums[portIter]],
            prvTgfPortsArray[portNums[portIter]]);

        /* copy read counters to  expected counters */
        cpssOsMemCpy(&expectedCntrs, &portCntrs, sizeof(expectedCntrs));

        /* calculate expected Tx counters */
        if (PRV_TGF_SEND_PORT_INDEX_CNS == portNums[portIter])
        {
            expectedCntrs.goodPktsRcv.l[0]   = expectedPackets[portIter];
        }
        else
        {
            /* calculate expected Rx counters */
            expectedCntrs.goodPktsSent.l[0]   = expectedPackets[portIter];
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);

        if((portNums[portIter] == PRV_TGF_TRUNK_PORT0_INDEX_CNS) ||
           (portNums[portIter] == PRV_TGF_TRUNK_PORT1_INDEX_CNS))
        {
            sumPacketsOut += expectedCntrs.goodPktsSent.l[0];
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
            {
                if(isEqualCntrs == GT_FALSE && isSinglePort &&
                    ((expectedCntrs.goodPktsSent.l[0] == 0) || portCntrs.goodPktsSent.l[0] == 0))
                {
                    /* let the sum to be checked */
                    continue;
                }
            }
        }

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqualCntrs, "get another counters values.");

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs,expectedCntrs,portCntrs);
    }

    UTF_VERIFY_EQUAL0_STRING_MAC((expectedEgressFromPort0 + expectedEgressFromPort1), sumPacketsOut,
        "get another sum of packets egress the trunk. \n");
}

/**
* @internal mplsWithIpPassengerLbhSendAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] L3L4ParsingOverMplsEnabled - L3 L4 Parsing Over Mpls Enabled.
*                                      GT_TRUE - enabled
*                                      GT_FALSE - disabled
*                                       None
*/
static GT_VOID mplsWithIpPassengerLbhSendAndCheck
(
    IN TGF_PACKET_STC   *packetInfoPtr,/*prvTgfMplsIpvxPassengerPacketInfo*/
    IN GT_BOOL          L3L4ParsingOverMplsEnabled
)
{
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;
    PRV_TGF_TRUNK_FIELD_TYPE_ENT    hashMaskFieldType;
    GT_U32                          hashMaskFieldSubType,hashMaskFieldSubTypeStart,hashMaskFieldSubTypeEnd;
    GT_U32                          hashMaskFieldByteIndex,hashMaskFieldByteIndexStart,hashMaskFieldByteIndexEnd;
    GT_U32                          numLables;
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT lbhMode;
    GT_U32                          mplsLableIndex;
    GT_U32                          ii;
    GT_U32                          isIpv4;
    GT_U32                          L3Offset;/* for fields in the L3/L4 headers , define the number of bytes
                                                from start of the packet*/
    GT_U32                          maxMplsLables = 3;
    GT_U32                          rc = GT_OK;
    L3_L4_PASSENGER_MATRIX_STC      *currL3L4InfoPtr;
    L4_PROTOCOL_INFO_STC            *currL4InfoPtr;
    GT_BOOL          checkSimpleHash; /* check simple hash or CRC hash */
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode;/* current hash mode */
    GT_U32              hashMaskTableIndex;/* index of entry in hash mask table - 0..27 */
    GT_U32              hashMaskTableIndexFirst;/* first,last index for hashMaskTableIndex */
    GT_BOOL             l4Ipv4OtherErratum; /* CRC Trunk hash L4 info use for IPv4 Other packets erratum */

    rc = prvTgfTrunkHashGlobalModeGet(prvTgfDevNum, &hashMode);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return ;
    }

    switch(hashMode)
    {
        case PRV_TGF_TRUNK_LBH_PACKETS_INFO_E:
            checkSimpleHash = GT_TRUE;
            break;
        case PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E:
            checkSimpleHash = GT_FALSE;
            break;
        case PRV_TGF_TRUNK_LBH_INGRESS_PORT_E:
        case PRV_TGF_TRUNK_LBH_PERFECT_E:
        default:
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(PRV_TGF_TRUNK_LBH_PACKETS_INFO_E, hashMode);
            return;
    }


    hashMaskTableIndexFirst = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;

    if(checkSimpleHash == GT_FALSE)
    {
        /* set new mode and seed */
        TRUNK_PRV_UTF_LOG0_MAC("=======  : set CRC sub-mode and seed =======\n");
        rc = prvTgfTrunkHashCrcParametersSet(PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E,crcSeeds[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        TRUNK_PRV_UTF_LOG0_MAC("=======  : clear mask hash CRC table =======\n");
        /* clear full table */
        rc = trunkHashMaskCrcTableEntryClear(PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* set Pearson table */
        rc = trunkHashPearsonFill(PEARSON_TEST_MODE_INDEX_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_XCAT2_E | UTF_LION2_E | UTF_LION_E))
    {
        /* xCat2, Lion and Lion2 have CRC L4 erratum  */
        l4Ipv4OtherErratum = GT_TRUE;
    }
    else
    {
        l4Ipv4OtherErratum = GT_FALSE;
    }


    utfGeneralStateMessageSave(0,"checkSimpleHash = %d",checkSimpleHash);
    utfGeneralStateMessageSave(1,"L3L4ParsingOverMplsEnabled = %d ",L3L4ParsingOverMplsEnabled);

    mplsLableIndex = 0;
    for(ii = 0 ; ii < packetInfoPtr->numOfParts ; ii++)
    {
        if(packetInfoPtr->partsArray[ii].type == TGF_PACKET_PART_ETHERTYPE_E)
        {
            mplsLableIndex = ii+1;/* next after the ethertype is MPLS label(s)*/
            break;
        }
    }

    if(mplsLableIndex == 0)
    {
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, mplsLableIndex);
        return;
    }


    for(isIpv4 = 0 ;isIpv4 < 2 ; isIpv4++)
    {
        if(debugMode)
        {
            isIpv4 = debug_seedIndex;/*also isIpv4*/
        }

        utfGeneralStateMessageSave(9,"isIpv4 = %d",isIpv4);
        for(hashMaskFieldType = 0 ; hashMaskFieldType < PRV_TGF_TRUNK_FIELD_LAST_E ; hashMaskFieldType++)
        {
            if(debugMode)
            {
                hashMaskFieldType       = debug_hashMaskFieldType;
            }

            utfGeneralStateMessageSave(2,"hashMaskFieldType = %d %s ",hashMaskFieldType,
                TO_STRING_HASH_MASK_FIELD_TYPE_MAC(hashMaskFieldType));

            switch(hashMaskFieldType)
            {
                case PRV_TGF_TRUNK_FIELD_L4_PORT_E:   /*src/dst*/
                    hashMaskFieldSubTypeStart = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
                    hashMaskFieldSubTypeEnd = PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E;
                    hashMaskFieldByteIndexStart = 0;
                    hashMaskFieldByteIndexEnd = 2; /* assume on 'Simple hash' - use 'long L4' mode */
                    break;
                case PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E:
                    if(isIpv4 == 1)
                    {
                        continue;/* not relevant to ipv4 */
                    }
                    hashMaskFieldSubTypeStart = 0;
                    hashMaskFieldSubTypeEnd = 0;
                    if(checkSimpleHash == GT_TRUE)
                    {
                        hashMaskFieldByteIndexStart = 1;/* assume on 'Simple hash' - use 'MSB_LSB_SIP_DIP_FLOW' ipv6 mode */
                    }
                    else
                    {
                        hashMaskFieldByteIndexStart = 0;
                    }
                    hashMaskFieldByteIndexEnd = 3;
                    break;
                case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:   /*src/dst*/
                    hashMaskFieldSubTypeStart = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
                    hashMaskFieldSubTypeEnd = PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E;
                    if(isIpv4 == 1)
                    {
                        hashMaskFieldByteIndexStart = 12;
                        hashMaskFieldByteIndexEnd = 16;
                    }
                    else
                    {
                        hashMaskFieldByteIndexStart = 0;
                        hashMaskFieldByteIndexEnd = 16;
                    }
                    break;
                default:
                    continue;
            }

            for(hashMaskFieldSubType = hashMaskFieldSubTypeStart ;
                hashMaskFieldSubType <= hashMaskFieldSubTypeEnd ;
                hashMaskFieldSubType++)
            {
                if(debugMode)
                {
                    hashMaskFieldSubType    = debug_hashMaskFieldSubType;
                }

                if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
                {
                    /* reduce number of iterations */
                    hashMaskFieldSubType = hashMaskFieldSubTypeEnd;
                }

                switch(hashMaskFieldType)
                {
                    case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
                    case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
                        utfGeneralStateMessageSave(3,"hashMaskFieldSubType = %d %s ",hashMaskFieldSubType,
                            TO_STRING_HASH_MASK_FIELD_SUB_TYPE_MAC(hashMaskFieldSubType));
                    break;
                    default:
                        utfGeneralStateMessageSave(3,"hashMaskFieldSubType = %d ",hashMaskFieldSubType);
                    break;
                }

                for(hashMaskFieldByteIndex = hashMaskFieldByteIndexStart;
                    hashMaskFieldByteIndex < hashMaskFieldByteIndexEnd;
                    hashMaskFieldByteIndex++)
                {
                    if(debugMode)
                    {
                        hashMaskFieldByteIndex  = debug_hashMaskFieldByteIndex;
                    }

                    if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
                    {
                        /* reduce number of iterations */
                        hashMaskFieldByteIndex = hashMaskFieldByteIndexEnd - 1;
                    }

                    utfGeneralStateMessageSave(4,"hashMaskFieldByteIndex = %d ",hashMaskFieldByteIndex);

                    /* limitations on ipv4/6 in 'simple hash mode'*/
                    if(checkSimpleHash == GT_TRUE &&
                       hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IP_ADDR_E)
                    {
                        if(isIpv4 == 1)
                        {
                            switch(hashMaskFieldByteIndex)
                            {
                                case 15: /* ip[5:0]*/
                                case 13: /* ip[21:16]*/
                                    break;
                                default:
                                    continue;/* skip this byte -- simple hash not support it */
                            }
                        }
                        else /* ipv6 */
                        {
                            switch(hashMaskFieldByteIndex)/* assume on 'Simple hash' - use 'MSB_LSB_SIP_DIP_FLOW' ipv6 mode */
                            {
                                case 15: /* ip[5:0]*/
                                case 13: /* ip[21:16]*/
                                case  7: /* ip[69:64]*/
                                case  0: /* ip[125:120]*/
                                    break;
                                default:
                                    continue;/* skip this byte -- simple hash not support it */
                            }
                        }
                    }
                    else if(checkSimpleHash == GT_FALSE &&  /*CRC hash*/
                            hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IP_ADDR_E &&
                            isIpv4 == 0)
                    {
                        /* limit the number of bytes checked --> reduce the time of the test */
#ifdef FORCE_REDUCED_TEST_MAC
                        {
                            /* for debug */
                            switch(hashMaskFieldByteIndex)/* assume on 'Simple hash' - use 'MSB_LSB_SIP_DIP_FLOW' ipv6 mode */
                            {
                                case 0:
                                case 4:
                                case 6:
                                case 11:
                                case 13:
                                case 15:
                                    break;
                                default:
                                    continue;/* limit the number of bytes checked --> reduce the time of the test */
                            }
                        }
#endif /*FORCE_REDUCED_TEST_MAC*/
                    }


                    for(numLables = 1 ; numLables <= maxMplsLables ; numLables++)
                    {
                        if(debugMode)
                        {
                            numLables = debug_pearsonMode;/*also numLables */
                        }

                        if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
                        {
                            /* reduce number of iterations */
                            numLables = maxMplsLables;
                        }

                        utfGeneralStateMessageSave(5,"numLables = %d ",numLables);

                        /*check of number of MPLS labels */
                        L3Offset = PRV_TGF_MPLS_PACKET_PASSENGER_OFFSET_CNS(numLables);
                        for(ii = 0 ; ii < maxMplsLables ; ii++)
                        {
                            if(ii < numLables)
                            {
                                TGF_PACKET_MPLS_STC *mplsPartPtr;

                                packetInfoPtr->partsArray[mplsLableIndex + ii].type = TGF_PACKET_PART_MPLS_E;
                                mplsPartPtr = packetInfoPtr->partsArray[mplsLableIndex + ii].partPtr;
                                mplsPartPtr->stack = (TGF_MPLS_STACK)((ii == (numLables - 1)) ? 1 : 0);
                            }
                            else
                            {
                                packetInfoPtr->partsArray[mplsLableIndex + ii].type = TGF_PACKET_PART_SKIP_E;
                            }
                        }

                        currL3L4InfoPtr = &L3L4PassengerMatrixArr[0];
                        ii = 0;
                        for( ; (currL3L4InfoPtr->l3HeaderPointer != NULL) ; currL3L4InfoPtr++,ii++)
                        {
                            if(debugMode)
                            {
                                ii = debug_crcMode;/*also matrix index*/
                                currL3L4InfoPtr = &L3L4PassengerMatrixArr[ii];
                            }
                            else
                            if(isIpv4 == 1)
                            {
                                if(currL3L4InfoPtr->l3HeaderPointer == (&prvTgfPacketIpv6Header))
                                {
                                    /* not relevant traffic for Ipv4 fields */
                                    continue;
                                }
                            }
                            else
                            {
                                if(currL3L4InfoPtr->l3HeaderPointer != (&prvTgfPacketIpv6Header))
                                {
                                    /* not relevant traffic for Ipv6 fields */
                                    continue;
                                }
                            }

                            trafficType = currL3L4InfoPtr->trafficType;


                            utfGeneralStateMessageSave(6,"l3HeaderName = %s ",currL3L4InfoPtr->l3HeaderName);
                            utfGeneralStateMessageSave(7,"matrixIndex = %d ",ii);

                            /* put the relevant L3 part of the passenger */
                            packetInfoPtr->partsArray[mplsLableIndex + maxMplsLables] =
                                *(currL3L4InfoPtr->l3HeaderPointer);

                            if(L3L4ParsingOverMplsEnabled == GT_TRUE)
                            {
                                if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_L4_PORT_E)
                                {
                                    lbhMode = currL3L4InfoPtr->isL4ModificationModifyLbh == GT_TRUE ?
                                        PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E :      /* do LBH */
                                        PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;/* no LBH */

                                    /* special treatment for IPV4 Other L4 info */
                                    if ((GT_TRUE == l4Ipv4OtherErratum) &&
                                        (currL3L4InfoPtr == &L3L4PassengerMatrixArr[0]))
                                    {
                                        /* CRC hash use L4 info for hash calculation */
                                        if (checkSimpleHash == GT_FALSE)
                                        {
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
                                        }
                                    }
                                }
                                else
                                {
                                    /* we do LBH on fields of the L3 */
                                    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
                                }

                                if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E &&
                                   hashMaskFieldByteIndex == 2 && checkSimpleHash == GT_FALSE &&
                                   lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E)
                                {
                                    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                                    {
                                        /* new LBH in SIP5 so this issue is gone ... */
                                    }
                                    else
                                    {
                                        /* the 4 LSB (in third word) don't give good LBH -->
                                            only 2 ports from 4 do LBH
                                            in simulation and in HW.
                                        */
                                        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                        prvTgfTrunkLoadBalanceLimitedNumSet(2);
                                    }
                                }
                            }
                            else
                            {
                                /* no LBH since the L3L4 changes not taken to hash by PP */
                                lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
                            }

                            utfGeneralStateMessageSave(8,"lbhMode = %d %s",lbhMode,
                                TO_STRING_LBH_MODE_MAC(lbhMode)
                                );

                            if(isIpv4 == 1)
                            {
                                /* check only TCP --> the UDP checked by ipv6 --> reduce time */
                                currL4InfoPtr = &L4PassengerHeader[0];
                            }
                            else
                            {
                                /* check only UDP --> the TCP checked by ipv4 --> reduce time */
                                currL4InfoPtr = &L4PassengerHeader[1];
                            }

                            /* reduce time .
                            for(  ; (currL4InfoPtr->l4HeaderPointer != NULL); currL4InfoPtr++)*/
                            {
                                /* put the relevant L4 part of the passenger */
                                packetInfoPtr->partsArray[mplsLableIndex + (maxMplsLables + 1)] =
                                    *(currL4InfoPtr->l4HeaderPointer);

                                if(currL3L4InfoPtr->l3HeaderPointer != (&prvTgfPacketIpv4OtherHeader))
                                {
                                    /* don't update the protocol of the 'ipv4 other' ,
                                       because it will not be 'other' any more ! */

                                    /* set the IPv4/6 protocol to match the 'L4 info' */

                                    if(isIpv4 == 1)
                                    {
                                        TGF_PACKET_IPV4_STC *ipv4PartPtr;

                                        /* update the protocol */
                                        ipv4PartPtr = packetInfoPtr->partsArray[mplsLableIndex + maxMplsLables].partPtr;
                                        ipv4PartPtr->protocol = (TGF_PROT)currL4InfoPtr->protocol;
                                    }
                                    else
                                    {
                                        TGF_PACKET_IPV6_STC *ipv6PartPtr;
                                        /* update the protocol */
                                        ipv6PartPtr = packetInfoPtr->partsArray[mplsLableIndex + maxMplsLables].partPtr;
                                        ipv6PartPtr->nextHeader = (TGF_NEXT_HEADER)currL4InfoPtr->protocol;
                                    }
                                }

                                hashMaskTableIndex = hashMaskTableIndexFirst + trafficType;

                                rc = trunkHashMaskCrcFieldSet(
                                        hashMaskTableIndex,
                                        hashMaskFieldType,
                                        hashMaskFieldSubType,
                                        hashMaskFieldByteIndex);
                                if(rc != GT_OK)
                                {
                                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                    break;
                                }

                                /* send traffic according to field checking */
                                rc = trunkHashCrcLbhTrafficSend(
                                        packetInfoPtr,/* this is MPLS packet , with IP passenger , with L4 info */
                                        trafficType,
                                        hashMaskFieldType,
                                        hashMaskFieldSubType,
                                        hashMaskFieldByteIndex,
                                        lbhMode,
                                        L3Offset,
                                        checkSimpleHash,
                                        10,NULL);

                                if(rc != GT_OK)
                                {
                                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                    break;
                                }
                            }/* currL4InfoPtr */
                            IF_DEBUG_MODE_BREAK_MAC(debugMode);
                        }/* currL3L4InfoPtr */
                        IF_DEBUG_MODE_BREAK_MAC(debugMode);
                    }/*numLables*/
                    IF_DEBUG_MODE_BREAK_MAC(debugMode);
                }/*hashMaskFieldByteIndex*/
                IF_DEBUG_MODE_BREAK_MAC(debugMode);
            }/*hashMaskFieldSubType*/
            IF_DEBUG_MODE_BREAK_MAC(debugMode);
        }/*hashMaskFieldType*/
        IF_DEBUG_MODE_BREAK_MAC(debugMode);
    }/*trafficType*/

    /* restore protocols for the modified parts */
    prvTgfPacketIpv4FragmentPart.protocol = IPV4_FRAGMENT_DUMMY_PROTOCOL_CNS;
    prvTgfPacketIpv4Part.protocol   = IPV4_DUMMY_PROTOCOL_CNS;
    prvTgfPacketIpv6Part.nextHeader = IPV6_DUMMY_PROTOCOL_CNS;

}


/**
* @internal prvTgfTrunkForHashTestTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
* @param[in] testPacketType           - the packet type
* @param[in] burstCount               -  burst size
* @param[in] expectedEgressFromPort0  - amount of packets expected to egress from
*                                      thunk member 0
* @param[in] expectedEgressFromPort1  - amount of packets expected to egress from
*                                      thunk member 1
*                                       None
*/
static GT_VOID prvTgfTrunkForHashTestTrafficGenerateAndCheck
(
    IN PRV_TGF_TRUNK_HASH_TEST_PACKET_ENT testPacketType,
    IN GT_U32                             burstCount,
    IN GT_U32                             expectedEgressFromPort0,
    IN GT_U32                             expectedEgressFromPort1
)
{
    TGF_PACKET_STC            *packetInfoPtr;
    GT_STATUS                 rc;
    PRV_TGF_BRG_MAC_ENTRY_STC brgMacEntry;
    GT_U32                    i;
    TGF_PACKET_L2_STC         *packetL2Ptr;

    switch (testPacketType)
    {
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E:
            packetInfoPtr = &prvTgfPacketEthOtherInfo;
            break;
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_IPV4_SIP_E:
            packetInfoPtr = &prvTgfIpv4OtherPacketInfo;
            break;
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_LAB0_E:
            packetInfoPtr = &prvTgfMplsPacketInfo;
            break;
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_IPvX_PASSENGER_E:
            packetInfoPtr = &prvTgfMplsIpvxPassengerPacketInfo;
            break;
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E:
            TRUNK_PRV_UTF_LOG0_MAC(
                "prvTgfTrunkForHashTestTrafficGenerateAndCheck : PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E not supported by this test \n");
            return;
        default:
            TRUNK_PRV_UTF_LOG0_MAC(
                "prvTgfTrunkForHashTestTrafficGenerateAndCheck bad pktType\n");
            return;
    }

    packetL2Ptr = (TGF_PACKET_L2_STC*)packetInfoPtr->partsArray[0].partPtr;

    cpssOsMemSet(&brgMacEntry, 0, sizeof(brgMacEntry));
    brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    for (i = 0; (i < 6); i++)
    {
        brgMacEntry.key.key.macVlan.macAddr.arEther[i] =
            packetL2Ptr->daMac[i];
    }
    brgMacEntry.key.key.macVlan.vlanId = 1;
    brgMacEntry.daCommand = PRV_TGF_PACKET_CMD_FORWARD_E;
    brgMacEntry.saCommand = PRV_TGF_PACKET_CMD_FORWARD_E;

    brgMacEntry.dstInterface.type    = CPSS_INTERFACE_TRUNK_E;
    brgMacEntry.dstInterface.trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(brgMacEntry.dstInterface.trunkId);
    brgMacEntry.dstOutlif.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;

    brgMacEntry.isStatic = GT_TRUE;

    rc = prvTgfBrgFdbMacEntrySet(&brgMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntrySet: failed\n");


    if(testPacketType == PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_IPvX_PASSENGER_E)
    {
        /* save default Ipv6 mode */
        rc = prvTgfTrunkHashIpv6ModeGet(prvTgfDevNum, &prvTgfDefHashModeIpv6);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrunkHashIpv6ModeGet: failed\n");


        /* save default L4 mode */
        rc = prvTgfTrunkHashL4ModeGet(prvTgfDevNum, &prvTgfDefHashModeL4);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrunkHashL4ModeGet: failed\n");

        utfGeneralStateMessageSave(10,"L3L4ParsingOverMplsEnable = %d",GT_TRUE);

        /* set <L3L4ParsingOverMplsEnable> = GT_TRUE */
        rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");

        if(rc == GT_OK)
        {
            /* this mode check the LBH of IP over MPLS , so need to enable IP,L4 hashing */
            rc = prvTgfTrunkHashIpModeSet(GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfTrunkHashIpModeSet: failed \n");

            /* restore default Ipv6 mode */
            rc = prvTgfTrunkHashIpv6ModeSet(PRV_TGF_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfTrunkHashIpv6ModeSet: failed\n");

            /* restore default L4 mode */
            rc = prvTgfTrunkHashL4ModeSet(PRV_TGF_TRUNK_L4_LBH_LONG_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfTrunkHashL4ModeSet: failed\n");

            /* check when L3L4ParsingOverMplsEnabled = enabled */
            mplsWithIpPassengerLbhSendAndCheck(packetInfoPtr ,
                GT_TRUE/*L3L4ParsingOverMplsEnabled - enabled */);

            utfGeneralStateMessageSave(10,"L3L4ParsingOverMplsEnable = %d",GT_FALSE);

            rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");
        }
        /* check when L3L4ParsingOverMplsEnabled = disabled  */
        mplsWithIpPassengerLbhSendAndCheck(packetInfoPtr ,
            GT_FALSE/*L3L4ParsingOverMplsEnabled - disabled */);

        /* only now - disable the IP , restore the L4,IPv6 modes ! */
        rc = prvTgfTrunkHashIpModeSet(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrunkHashIpModeSet: failed \n");

        /* restore default Ipv6 mode */
        rc = prvTgfTrunkHashIpv6ModeSet(prvTgfDefHashModeIpv6);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrunkHashIpv6ModeSet: failed\n");

        /* restore default L4 mode */
        rc = prvTgfTrunkHashL4ModeSet(prvTgfDefHashModeL4);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfTrunkHashL4ModeSet: failed\n");
    }
    else
    {
    prvTgfTrunkForHashTestSendPacketAndCheck(
        packetInfoPtr,
        burstCount,
        expectedEgressFromPort0,
        expectedEgressFromPort1);
    }

    rc = prvTgfBrgFdbMacEntryDelete(&(brgMacEntry.key));
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryDelete: failed\n");

}


/**
* @internal prvTgfTrunkHashTestTrafficMplsModeEnableSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashMplsModeEnableSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
* @param[in] L3L4MplsParsingEnable    -  flag that state if the <enable L3 L4 parsing over MPLS> is enabled/disabled
*                                      when enabled and IPvX over MPLS packet is doing trunk LBH according to L3/L4 of the
*                                      passenger (regardless to tunnel termination)
*                                       None
*/
GT_VOID prvTgfTrunkHashTestTrafficMplsModeEnableSet
(
    IN  GT_BOOL     L3L4MplsParsingEnable
)
{
    /*for devices that supports the LBH according to 'MPLS_IPvX_PASSENGER'*/
    if(L3L4MplsParsingEnable == GT_TRUE)
    {
        prvTgfVlansNum = 1;
        prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_MPLS_SIMPLE_HASH_E,GT_FALSE,
                           TRUNK_8_PORTS_IN_TEST_CNS );

        /* Generate traffic */
        prvTgfTrunkForHashTestTrafficGenerateAndCheck(
            PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_IPvX_PASSENGER_E,
            0,  /* not used parameter */
            0,  /* not used parameter */
            0); /* not used parameter */

        /*  prvTgfTrunkGenericRestore(GT_FALSE);
            done from prvTgfTrunkForHashTestConfigurationReset anyway !!!
        */
    }
    else
    {
        prvTgfTrunkHashMplsModeEnableSet(GT_TRUE);

        /* Generate traffic */
        prvTgfTrunkForHashTestTrafficGenerateAndCheck(
            PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_LAB0_E,
            2 /*burstCount*/,
            0 /*expectedEgressFromPort0*/,
            2 /*expectedEgressFromPort1*/);

        prvTgfTrunkHashMplsModeEnableSet(GT_FALSE);

        /* Generate traffic */
        prvTgfTrunkForHashTestTrafficGenerateAndCheck(
            PRV_TGF_TRUNK_HASH_TEST_PACKET_MPLS_LAB0_E,
            3 /*burstCount*/,
            3 /*expectedEgressFromPort0*/,
            0 /*expectedEgressFromPort1*/);
    }
}


/**
* @internal prvTgfTrunkHashTestTrafficMaskSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashMaskSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
*/
GT_VOID prvTgfTrunkHashTestTrafficMaskSet
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: set mask 0 for SA field */
    prvTgfTrunkHashMaskSet(
        PRV_TGF_TRUNK_LBH_MASK_MAC_SA_E, 0);

    /* AUTODOC: add FDB entry with MAC 00:11:22:33:34:00, VLAN 1, TRUNK 25 */
    /* AUTODOC: send 3 Ethernet packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:34:00, SA=00:66:77:88:34:01 */
    /* AUTODOC:   EtherType=0x1234 */
    /* AUTODOC: verify to get 3 Ethernet packets on port 0 */
    /* AUTODOC: delete created FDB entry */
    prvTgfTrunkForHashTestTrafficGenerateAndCheck(
        PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E,
        3 /*burstCount*/,
        3 /*expectedEgressFromPort0*/,
        0 /*expectedEgressFromPort1*/);

    /* AUTODOC: set mask 0x3F for SA field */
    prvTgfTrunkHashMaskSet(
        PRV_TGF_TRUNK_LBH_MASK_MAC_SA_E, 0x3F);

    /* AUTODOC: add FDB entry with MAC 00:11:22:33:34:00, VLAN 1, TRUNK 25 */
    /* AUTODOC: send 1 Ethernet packet from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:34:00, SA=00:66:77:88:34:01 */
    /* AUTODOC:   EtherType=0x1234 */
    /* AUTODOC: verify to get 1 Ethernet packet on port 2 */
    /* AUTODOC: delete created FDB entry */
    prvTgfTrunkForHashTestTrafficGenerateAndCheck(
        PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E,
        1 /*burstCount*/,
        0 /*expectedEgressFromPort0*/,
        1 /*expectedEgressFromPort1*/);

}

/**
* @internal prvTgfTrunkHashTestTrafficIpShiftSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashIpShiftSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
*/
GT_VOID prvTgfTrunkHashTestTrafficIpShiftSet
(
    GT_VOID
)
{
    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: enable IP hash generation mode */
    prvTgfTrunkHashIpModeSet(GT_TRUE);

    /* AUTODOC: set shift val 1 to IPv4 src addresses */
    prvTgfTrunkHashIpShiftSet(
        CPSS_IP_PROTOCOL_IPV4_E,
        GT_TRUE /*isSrcIp*/, 1 /*shiftValue*/);

    /* AUTODOC: add FDB entry with MAC 00:11:22:33:34:00, VLAN 1, TRUNK 25 */
    /* AUTODOC: send 2 IPv4 packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:34:00, SA=00:66:77:88:34:00 */
    /* AUTODOC:   srcIP=7.0.8.0, dstIP=1.1.1.0 */
    /* AUTODOC: verify to get 2 IPv4 packets on port 0 */
    /* AUTODOC: delete created FDB entry */
    prvTgfTrunkForHashTestTrafficGenerateAndCheck(
        PRV_TGF_TRUNK_HASH_TEST_PACKET_IPV4_SIP_E,
        2 /*burstCount*/,
        2 /*expectedEgressFromPort0*/,
        0 /*expectedEgressFromPort1*/);

    /* AUTODOC: set shift val 0 to IPv4 src addresses */
    prvTgfTrunkHashIpShiftSet(
        CPSS_IP_PROTOCOL_IPV4_E,
        GT_TRUE /*isSrcIp*/, 0 /*shiftValue*/);

    /* AUTODOC: add FDB entry with MAC 00:11:22:33:34:00, VLAN 1, TRUNK 25 */
    /* AUTODOC: send 3 IPv4 packets from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:34:00, SA=00:66:77:88:34:00 */
    /* AUTODOC:   srcIP=7.0.8.0, dstIP=1.1.1.0 */
    /* AUTODOC: verify to get 3 IPv4 packets on port 2 */
    /* AUTODOC: delete created FDB entry */
    prvTgfTrunkForHashTestTrafficGenerateAndCheck(
        PRV_TGF_TRUNK_HASH_TEST_PACKET_IPV4_SIP_E,
        3 /*burstCount*/,
        0 /*expectedEgressFromPort0*/,
        3 /*expectedEgressFromPort1*/);

    /* AUTODOC: disable IP hash generation mode */
    prvTgfTrunkHashIpModeSet(GT_FALSE);
}


/**
* @internal internalTrunkForDesignatedPortsTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
static GT_STATUS internalTrunkForDesignatedPortsTestConfigurationReset
(
    IN  GT_TRUNK_ID             trunkId
)
{
    GT_STATUS   rc;

    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* restore default 'no designated' */
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_FALSE,NULL);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvTgfTrunkForDesignatedPortsTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkForDesignatedPortsTestConfigurationReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    rc = internalTrunkForDesignatedPortsTestConfigurationReset(PRV_TGF_TRUNK_ID_CNS);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = internalTrunkForDesignatedPortsTestConfigurationReset(PRV_TGF_TRUNK_ID_2_CNS);
    return rc;
}

/**
* @internal prvTgfTrunkDesignatedPorts_basicCommonSetting function
* @endinternal
*
* @brief   Important note for all next tests:
*         Start test by setting Trunk2 with Ports p5,p6.
*         Those ports not need to be 'link up' or connected to traffic checks.
*/
static GT_VOID prvTgfTrunkDesignatedPorts_basicCommonSetting
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkId = PRV_TGF_TRUNK_ID_2_CNS;

    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    trunk_2_Members[0].port   =   16;
    trunk_2_Members[0].hwDevice =   prvTgfDevsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];

    trunk_2_Members[1].port   =   20;
    trunk_2_Members[1].hwDevice =   prvTgfDevsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];

    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        2 /*numOfEnabledMembers*/,
        trunk_2_Members,
        0 /*numOfDisabledMembers*/,
        NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return ;
    }


    /* define members in trunk 1 */
    trunk_1_Members[0].hwDevice = prvTgfDevsArray[2];
    trunk_1_Members[0].port   = prvTgfPortsArray[2];
    trunk_1_Members[1].hwDevice = prvTgfDevsArray[3];
    trunk_1_Members[1].port   = prvTgfPortsArray[3];

    /*define designated ports*/
    designatedMember3 = trunk_1_Members[0];
    designatedMember4 = trunk_1_Members[1];

}

/**
* @internal prvTgfTrunkDesignatedPorts_trunk2ConfigCheck function
* @endinternal
*
* @brief   Important note for all next tests:
*         --on trunk2 : before every traffic sending in those tests, check trunk tr2 ports in designated trunk table.
*/
static GT_VOID prvTgfTrunkDesignatedPorts_trunk2ConfigCheck
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       devNum = (GT_U8)trunk_2_Members[0].hwDevice;
    CPSS_PORTS_BMP_STC  designatedPorts_even_value;
    CPSS_PORTS_BMP_STC  designatedPorts_odd_value;
    CPSS_PORTS_BMP_STC  designatedPorts_mask;
    CPSS_PORTS_BMP_STC  designatedPorts;
    CPSS_PORTS_BMP_STC  tmpPortsBmp;
#ifdef EXMXPM_FAMILY
    GT_U32  numDesig = 16;
#else
    GT_U32  numDesig = 8;
#endif
    GT_U32  ii;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&designatedPorts_mask);

    CPSS_PORTS_BMP_PORT_SET_MAC((&designatedPorts_mask),trunk_2_Members[0].port);
    CPSS_PORTS_BMP_PORT_SET_MAC((&designatedPorts_mask),trunk_2_Members[1].port);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&designatedPorts_even_value);
    CPSS_PORTS_BMP_PORT_SET_MAC((&designatedPorts_even_value),trunk_2_Members[0].port);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&designatedPorts_odd_value);
    CPSS_PORTS_BMP_PORT_SET_MAC((&designatedPorts_odd_value),trunk_2_Members[0].port);

    for(ii = 0 ; ii < numDesig; ii++)
    {
        rc = prvTgfTrunkDesignatedPortsEntryGet(devNum,ii,&designatedPorts);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            return;
        }

        CPSS_PORTS_BMP_BITWISE_AND_MAC(&tmpPortsBmp,&designatedPorts,&designatedPorts_mask);

        if(ii & 1)
        {
            /* odd entry */
            if(0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&tmpPortsBmp,&designatedPorts_odd_value))
            {
                TRUNK_PRV_UTF_LOG0_MAC("[TGF]: prvTgfTrunkDesignatedPorts_trunk2ConfigCheck: odd - designated ports not as expected");
            }
        }
        else
        {
            /* even entry */
            if(0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&tmpPortsBmp,&designatedPorts_even_value))
            {
                TRUNK_PRV_UTF_LOG0_MAC("[TGF]: prvTgfTrunkDesignatedPorts_trunk2ConfigCheck: even - designated ports not as expected");
            }
        }
    }
}


/**
* @internal prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic function
* @endinternal
*
* @brief   9.    send multi-destination traffic , meaning from port p1:
*         a.    Send BC (FF:FF:FF:FF:FF:FF) with 100 incremental SA.
*         b.    Send unregistered MC (01:02:03:04:05:06) with 100 incremental SA.
*         c.    Send unknown unicast (00:09:99:99:99:99) with 100 incremental DA and static SA
*         d.    Send unknown unicast (00:09:99:99:99:99) with 100 static DA and incremental SA
*/
static void prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic
(
    GT_VOID
)
{
    /* check configuration of trunk'2' before sending traffic*/
    prvTgfTrunkDesignatedPorts_trunk2ConfigCheck();

    /* reset counters */
    clearAllTestedCounters();

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* reduce number of permutations */
        prvTgfCommonMultiDestinationTrafficTypeSend(prvTgfDevNum,
                                                prvTgfPortsArray[0],
                                                BURST_COUNT_CNS,
                                                GT_FALSE,
                                                NULL,
                                                PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_DA_INCREMENT_E);
    }
    else
    {
        /*send multi-destination traffic , meaning from port p1*/
        prvTgfCommonMultiDestinationTrafficSend(prvTgfDevNum,
                                                prvTgfPortsArray[0],
                                                BURST_COUNT_CNS,
                                                GT_FALSE);
    }
}


/**
* @internal prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck function
* @endinternal
*
* @brief   check that traffic counter match the needed value
*/
static void prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck
(
    IN GT_U32                   devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   expectedCounter
)
{
    prvTgfCommonMultiDestinationTrafficCounterCheck((GT_U8)devNum,portNum,expectedCounter);
}


/**
* @internal prvTgfTrunkDesignatedPorts_startTest function
* @endinternal
*
* @brief   1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_startTest
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    if(designatedPortsMutation != PRV_TGF_TRUNK_DESIGNATED_PORTS_SET_DESIGNATED_AFTER_TRUNK_SETTING_ENT)
    {
        /*1.        in trunk tr1 : Set p4 as designated*/
        rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember4);
        if (GT_OK != rc)
        {
            TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    switch(designatedPortsMutation)
    {
        case PRV_TGF_TRUNK_DESIGNATED_PORTS_BASIC_E:
        /* fall through */
        case PRV_TGF_TRUNK_DESIGNATED_PORTS_SET_DESIGNATED_AFTER_TRUNK_SETTING_ENT:
            if(designatedPort4AsDisabled == GT_FALSE)
            {
                /*2.        in trunk tr1 : Set p3,p4 as enabled members*/
                rc = prvTgfTrunkMembersSet(
                    trunkId /*trunkId*/,
                    2 /*numOfEnabledMembers*/,
                    trunk_1_Members,
                    0 /*numOfDisabledMembers*/,
                    NULL  /*disabledMembersArray*/);
            }
            else
            {
                rc = prvTgfTrunkMembersSet(
                    trunkId /*trunkId*/,
                    1 /*numOfEnabledMembers*/,
                    &trunk_1_Members[0],
                    1 /*numOfDisabledMembers*/,
                    &trunk_1_Members[1]  /*disabledMembersArray*/);
            }
            if (GT_OK != rc)
            {
                TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
                return rc;
            }

            if(designatedPortsMutation != PRV_TGF_TRUNK_DESIGNATED_PORTS_BASIC_E)
            {
                rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember4);
                if (GT_OK != rc)
                {
                    TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
                    return rc;
                }
            }

            break;
        case PRV_TGF_TRUNK_DESIGNATED_PORTS_ADD_PORTS_INSTEAD_OF_SET_ENT:
            rc = prvTgfTrunkMemberAdd(
                trunkId /*trunkId*/,
                &trunk_1_Members[0]);
            if (GT_OK != rc)
            {
                TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberAdd FAILED, rc = [%d]", rc);
                return rc;
            }
            rc = prvTgfTrunkMemberAdd(
                trunkId /*trunkId*/,
                &trunk_1_Members[1]);
            if (GT_OK != rc)
            {
                TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberAdd FAILED, rc = [%d]", rc);
                return rc;
            }

            if(designatedPort4AsDisabled == GT_TRUE)
            {
                rc = prvTgfTrunkMemberDisable(
                    trunkId /*trunkId*/,
                    &trunk_1_Members[1]);
                if (GT_OK != rc)
                {
                    TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberDisable FAILED, rc = [%d]", rc);
                    return rc;
                }
            }

        default:
            break;
    }

    return GT_OK;
}

/**
* @internal prvTgfTrunkDesignatedPorts_test1 function
* @endinternal
*
* @brief   1.3.1    Test 1 : Set designated before trunk members
*         1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*         3.    send multi-destination traffic
*         4.    check that all traffic egress p4 (not egress port p3) , p2 get all traffic
*         Check 'unset designated'
*         5.    in trunk tr1 : UnSet the designated
*         6.    send multi-destination traffic
*         7.    check that load balance of traffic between trunk members (p3,p4) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test1
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;
    GT_U32                  fullTrafficNum =  FULL_TRAFFIC_EXPECTED_CNS;

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* reduce number of permutations */
        fullTrafficNum =  BURST_COUNT_CNS;/* only single traffic is sent */
    }

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3,p4 as enabled members*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*4.        check that all traffic egress p4 (not egress port p3) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        fullTrafficNum);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        fullTrafficNum);

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* reduce number of permutations */
        return GT_OK;
    }

/*Check 'unset designated'*/
    /*5.        in trunk tr1 : UnSet the designated*/
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_FALSE,NULL);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }
/*6.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*7.        check that load balance of traffic between trunk members (p3,p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        HALF_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        HALF_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);

    return GT_OK;
}

/**
* @internal prvTgfTrunkDesignatedPorts_test2 function
* @endinternal
*
* @brief   1.3.2    Test 2 : remove designated (and later add back)
*         1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*         3.    remove p4 (designated) from the trunk tr1
*         4.    send multi-destination traffic
*         5.    check that all traffic not egress the trunk's port (not egress port p3) , p2 get all traffic , P4 is not in trunk and should get all traffic.
*         6.    ------------
*         7.    add p4 (designated) to trunk tr1
*         8.    send multi-destination traffic
*         9.    check that all traffic egress p4 (not egress port p3) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test2
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3,p4 as enabled members*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        remove p4 (designated) from the trunk tr1*/
    rc = prvTgfTrunkMemberRemove(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberRemove FAILED, rc = [%d]", rc);
        return rc;
    }

/*4.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*5.        check that all traffic not egress the trunk's port (not egress port p3) ,
        p2 get all traffic , P4 is not in trunk and should get all traffic.*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*7.        add p4 (designated) to trunk tr1*/
    rc = prvTgfTrunkMemberAdd(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberAdd FAILED, rc = [%d]", rc);
        return rc;
    }
/*8.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*9.        check that all traffic egress p4 (not egress port p3) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);

    return GT_OK;
}


/**
* @internal prvTgfTrunkDesignatedPorts_test3 function
* @endinternal
*
* @brief   1.3.3    Test 3 : replace designated
*         1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*         3.    in trunk tr1 : Set p3 as designated (replace p4)
*         4.    send multi-destination traffic
*         5.    check that all traffic egress p3 (not egress port p4) , p2 get all traffic
*         6.    ------------
*         7.    in trunk tr1 : Set p4 as designated (replace p3)
*         8.    send multi-destination traffic
*         9.    check that all traffic egress p4 (not egress port p3) , p2 get all traffic
*         10.    ------------
*         11.    in trunk tr1 : Set p2 as designated (replace p4)
*         12.    send multi-destination traffic
*         13.    check that all traffic not egress any trunk port (not egress port p3,p4) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test3
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;
    CPSS_TRUNK_MEMBER_STC    designatedMember2;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3,p4 as enabled members*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        in trunk tr1 : Set p3 as designated (replace p4)*/
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember3);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }

/*4.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*5.        check that all traffic egress p3 (not egress port p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*7.        in trunk tr1 : Set p4 as designated (replace p3)*/
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }
/*8.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*9.        check that all traffic egress p4 (not egress port p3) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*11.        in trunk tr1 : Set p2 as designated (replace p4)*/
    designatedMember2.hwDevice = prvTgfDevsArray[1];
    designatedMember2.port = prvTgfPortsArray[1];
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember2);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }
/*12.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*13.        check that all traffic not egress any trunk port (not egress port p3,p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


    return GT_OK;
}


/**
* @internal prvTgfTrunkDesignatedPorts_test4 function
* @endinternal
*
* @brief   1.3.4    Test 4 : Disable designated member (and enable again)
*         1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*         3.    in trunk tr1 : disable port p4 (designated)
*         4.    send multi-destination traffic
*         5.    check that all traffic not egress any trunk port (not egress port p3 , p4) , p2 get all traffic
*         6.    ------------
*         7.    in trunk tr1 : enable port p4 (designated)
*         8.    send multi-destination traffic
*         9.    check that all traffic egress p4 (not egress port p3) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test4
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3,p4 as enabled members*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        in trunk tr1 : disable port p4 (designated)*/
    rc = prvTgfTrunkMemberDisable(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberDisable FAILED, rc = [%d]", rc);
        return rc;
    }

/*4.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*5.        check that all traffic not egress any trunk port (not egress port p3 , p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*7.        in trunk tr1 : enable port p4 (designated) */
    rc = prvTgfTrunkMemberEnable(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberEnable FAILED, rc = [%d]", rc);
        return rc;
    }
/*8.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*9.        check that all traffic egress p4 (not egress port p3) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);

    return GT_OK;
}

/**
* @internal prvTgfTrunkDesignatedPorts_test5 function
* @endinternal
*
* @brief   1.3.5    Test 5 : Disable designated member and set new designated
*         1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3,p4 as enabled members
*         3.    in trunk tr1 : disable port p4 (designated)
*         4.    in trunk tr1 : Set p3 as designated
*         5.    send multi-destination traffic
*         6.    check that all traffic egress p3 (not egress port p4) , p2 get all traffic
*         7.    ------------
*         8.    in trunk tr1 : enable port p4
*         9.    send multi-destination traffic
*         10.    check that all traffic egress p3 (not egress port p4) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test5
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3,p4 as enabled members*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        in trunk tr1 : disable port p4 (designated)*/
    rc = prvTgfTrunkMemberDisable(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberDisable FAILED, rc = [%d]", rc);
        return rc;
    }

/*4.        in trunk tr1 : Set p3 as designated*/
    rc = prvTgfTrunkDesignatedMemberSet(trunkId,GT_TRUE,&designatedMember3);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedMemberSet FAILED, rc = [%d]", rc);
        return rc;
    }

/*5.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*6.        check that all traffic egress p3 (not egress port p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*8.        in trunk tr1 : enable port p4*/
    rc = prvTgfTrunkMemberEnable(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberEnable FAILED, rc = [%d]", rc);
        return rc;
    }
/*9.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*10.        check that all traffic egress p3 (not egress port p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);

    return GT_OK;
}

/**
* @internal prvTgfTrunkDesignatedPorts_test6 function
* @endinternal
*
* @brief   1.    in trunk tr1 : Set p4 as designated
*         2.    in trunk tr1 : Set p3 as enabled member , Set P4 (designated) as disabled member
*         3.    send multi-destination traffic
*         4.    check that all traffic not egress any trunk port (not egress port p3 , p4) , p2 get all traffic
*         5.    ------------
*         6.    in trunk tr1 : enable port p4
*         7.    send multi-destination traffic
*         8.    check that all traffic egress p4 (not egress port p3) , p2 get all traffic
*/
static GT_STATUS prvTgfTrunkDesignatedPorts_test6
(
    GT_VOID
)
{
    GT_STATUS               rc;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

/*1.        in trunk tr1 : Set p4 as designated*/
/*2.        in trunk tr1 : Set p3 as enabled member , Set P4 (designated) as disabled member*/
    rc = prvTgfTrunkDesignatedPorts_startTest();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkDesignatedPorts_startTest FAILED, rc = [%d]", rc);
        return rc;
    }

/*3.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();

/*4.        check that all traffic not egress any trunk port (not egress port p3 , p4) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);


/*6.        in trunk tr1 : enable port p4*/
    rc = prvTgfTrunkMemberEnable(trunkId,&designatedMember4);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMemberEnable FAILED, rc = [%d]", rc);
        return rc;
    }
/*7.        send multi-destination traffic*/
    prvTgfTrunkDesignatedPorts_sendMultiDestinationTraffic();
/*8.        check that all traffic egress p4 (not egress port p3) , p2 get all traffic*/

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[0].hwDevice,trunk_1_Members[0].port,
        NO_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        trunk_1_Members[1].hwDevice,trunk_1_Members[1].port,
        FULL_TRAFFIC_EXPECTED_CNS);

    prvTgfTrunkDesignatedPorts_MultiDestinationTrafficCheck(
        prvTgfDevsArray[1],prvTgfPortsArray[1],
        FULL_TRAFFIC_EXPECTED_CNS);

    return GT_OK;
}

/**
* @internal internalTrunkDesignatedPorts function
* @endinternal
*
* @brief   Test for designated member in trunk.
*/
static GT_VOID internalTrunkDesignatedPorts
(
    GT_VOID
)
{
    GT_STATUS   rc;

    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test1();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* reduce number of permutations */
        return;
    }

    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test2();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test3();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test4();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test5();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    designatedPort4AsDisabled = GT_TRUE;
    prvTgfTrunkDesignatedPorts_basicCommonSetting();
    rc = prvTgfTrunkDesignatedPorts_test6();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfTrunkForDesignatedPortsTestConfigurationReset();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    designatedPort4AsDisabled = GT_FALSE;
}

/**
* @internal prvTgfTrunkDesignatedPorts function
* @endinternal
*
* @brief   Test for designated member in trunk.
*/
GT_VOID prvTgfTrunkDesignatedPorts
(
    GT_VOID
)
{
    GT_STATUS rc;

    initTrunkIds(GT_FALSE);
    /* set transmit time to 0 */

    /* mutation 0 */
    designatedPortsMutation = PRV_TGF_TRUNK_DESIGNATED_PORTS_BASIC_E;
    internalTrunkDesignatedPorts();

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* reduce number of permutations */

        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

        return;
    }

    /* mutation 1 - set designated after trunk setting */
    designatedPortsMutation = PRV_TGF_TRUNK_DESIGNATED_PORTS_SET_DESIGNATED_AFTER_TRUNK_SETTING_ENT;
    internalTrunkDesignatedPorts();
    /* mutation 2 - : 'add' ports instead of 'set'*/
    designatedPortsMutation = PRV_TGF_TRUNK_DESIGNATED_PORTS_ADD_PORTS_INSTEAD_OF_SET_ENT;
    internalTrunkDesignatedPorts();

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfTrunkVlanTestInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*
* @param[in] vlanId                   - vlan id.
*                                      portMembers - bit map of ports belonging to the vlan.
*                                       None
*/
GT_VOID prvTgfTrunkVlanTestInit
(
    IN GT_U16                                  vlanId,
    CPSS_PORTS_BMP_STC                         portsMembers
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfTrunkVlanRestore function
* @endinternal
*
* @brief   Restore\Cancel prvTgfTrunkToCpuTraffic settings
*
* @param[in] vlanId                   - vlan id.
*                                       None
*/
GT_VOID prvTgfTrunkVlanRestore
(
    IN GT_U16   vlanId
)
{
    GT_STATUS   rc = GT_OK;

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d", vlanId);
}

/**
* @internal prvTgfTrunkAndVlanTestConfigure function
* @endinternal
*
* @brief   This function configures trunk and vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkAndVlanTestConfigure
(
    GT_VOID
)
{
    GT_U32                  portIter;
    static CPSS_TRUNK_MEMBER_STC          enabledMembersArray[4];
    GT_STATUS                      rc;
    GT_TRUNK_ID             trunkId;

    /* AUTODOC: SETUP CONFIGURATION: */
    initTrunkIds(GT_FALSE);

    enabledMembersArray[0].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT0_INDEX_CNS];
    enabledMembersArray[0].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT0_INDEX_CNS];
    enabledMembersArray[1].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];
    enabledMembersArray[1].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT1_INDEX_CNS];
    enabledMembersArray[2].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT2_INDEX_CNS];
    enabledMembersArray[2].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT2_INDEX_CNS];
    enabledMembersArray[3].hwDevice = prvTgfDevsArray[PRV_TGF_TRUNK_PORT3_INDEX_CNS];
    enabledMembersArray[3].port   = prvTgfPortsArray[PRV_TGF_TRUNK_PORT3_INDEX_CNS];

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: create trunk 25 with ports [0,2,1,3] */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        4 /*numOfEnabledMembers*/,
        enabledMembersArray,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
        return rc;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsVlanMembers);

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    prvTgfTrunkVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

    return GT_OK;
}

/**
* @internal prvTgfTrunkTestPacketSend function
* @endinternal
*
* @brief   Function sends packets.
*
* @param[in] devNum                   - device number to send traffic from
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] burstCount               - number of packets
* @param[in] numVfd                   - number of VFDs in vfdArray
* @param[in] vfdArray[]               - vfd Array (can be NULL when numVfd == 0)
*                                       None
*/
GT_VOID prvTgfTrunkTestPacketSend
(
    IN GT_U8           devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(devNum, packetInfoPtr, burstCount, numVfd, vfdArray);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d , %d",
                                 devNum, burstCount, numVfd );

    utfPrintKeepAlive();

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 devNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfTrunkToCpuTrafficFromPortAndDevice function
* @endinternal
*
* @brief   Generate and test traffic from port on device
*
* @param[in] devNum                   - device number to send traffic from
* @param[in] portNum                  - port number to send traffic from
*                                       None
*/
static GT_VOID prvTgfTrunkToCpuTrafficFromPortAndDevice
(
    IN GT_U8           devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    static GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_PORT_NUM eportOfTrunk = 0;
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* reset counters */
    clearAllTestedCounters();

    /* AUTODOC: enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNum;

    if(appDemo_trunk_phy1690_WA_B)
    {
        rc = trunkEPortGet_phy1690_WA_B(trunkId,&eportOfTrunk);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "trunkEPortGet_phy1690_WA_B: prvTgfDevNum %d trunkId[%d]",
                                     prvTgfDevNum, trunkId);
        /* we must not kill the TTI lookups on the ingress port !!! */
        /* the WA is based on it */
        prvTgfCaptureForceTtiDisableModeSet(GT_TRUE);
        /* set PCL for the TRAP on the srcEPort */
        rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                     prvTgfDevNum, eportOfTrunk);
    }

    /* enable capture on port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: send unknown UC Ethernet packets from ports [0,1,2,3] with: */
    /* AUTODOC:   DA=00:09:99:99:99:99, SA=00:01:11:11:11:11, VID=5 */
    /* AUTODOC: verify to get no Tx traffic */
    prvTgfTrunkTestPacketSend(devNum, portNum, &prvTgfPacketVlanTagInfo ,1 ,0 ,NULL);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevsArray[portIter], prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevsArray[portIter], prvTgfPortsArray[portIter]);

        if ((devNum == prvTgfDevsArray[portIter]) &&
            (portNum == prvTgfPortsArray[portIter]))
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }


        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);


    if(appDemo_trunk_phy1690_WA_B)
    {
        /* restore settings */
        prvTgfCaptureForceTtiDisableModeSet(GT_FALSE);
        /* unset PCL for the TRAP on the srcEPort */
        rc = prvTgfDxChPclCaptureSet(prvTgfDevNum,eportOfTrunk,GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfDxChPclCaptureSet: prvTgfDevNum %d eportOfTrunk[%d]",
                                     prvTgfDevNum, eportOfTrunk);
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                  GT_TRUE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC(portNum,
                                 rxParam.portNum,
                                 "sampled packet port num different than expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(devNum,
                                 rxParam.hwDevNum,
                                 "sampled packet dev num different than expected");

    /* Get next entry from captured packet's table - no entry expected*/
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                  GT_FALSE, GT_TRUE, trigPacketBuff,
                                  &buffLen, &packetActualLength,
                                  &devNum, &queue,
                                  &rxParam);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet: %d, %d",
                                 TGF_PACKET_TYPE_CAPTURE_E, GT_TRUE);

    /* AUTODOC: disable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
}

/**
* @internal prvTgfTrunkToCpuTraffic function
* @endinternal
*
* @brief   Generate and test traffic
*/
GT_VOID prvTgfTrunkToCpuTraffic
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    for( i = 0 ; i <= PRV_TGF_TRUNK_PORT3_INDEX_CNS ; i++ )
    {
        /* send packet to port and device */
        prvTgfTrunkToCpuTrafficFromPortAndDevice(prvTgfDevsArray[i], prvTgfPortsArray[i]);
    }

    /* disable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Error in tgfTrafficTracePacketByteSet %d", GT_FALSE);
}

/**
* @internal prvTgfTrunkAndVlanTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk and vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkAndVlanTestConfigurationReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID             trunkId;

    /* AUTODOC: RESTORE CONFIGURATION: */

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: clear TRUNK 25 */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkMembersSet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* AUTODOC: invalidate VLAN 5 */
    prvTgfTrunkVlanRestore(PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: tgfTrafficTableRxPcktTblClear FAILED, rc = [%d]", rc);
        return rc;
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgFdbFlush FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_prvTgfTrunkLoadBalanceCheck function
* @endinternal
*
* @brief   check that total summary of counters of ports that belong to trunk got value
*         of numPacketSent.
* @param[in] trunkId                  - trunk Id that his ports should received the traffic and on
*                                      them we expect total of numPacketSent packets
* @param[in] mode                     - the  of expected load balance .
*                                      all traffic expected to be received from single port or with
*                                      even distribution on the trunk ports
* @param[in] numPacketSent            - number of packet that we expect the trunk to receive
* @param[in] tolerance                - relevant to PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E
*                                      state the number of packet that may get to other port in the trunk.
*                                      meaning that (numPacketSent - tolerance) should get to port A
*                                      and tolerance should get to port B , but other ports (in trunk)
*                                      should not get it.
* @param[in] trunkMemberSenderPtr     - when not NULL , indicates that the traffic to check
*                                      LBH that egress the trunk was originally INGRESSED from the trunk.
*                                      but since in the 'enhanced UT' the CPU send traffic to a port
*                                      due to loopback it returns to it, we need to ensure that the
*                                      member mentioned here should get the traffic since it is the
*                                      'original sender'
*
* @param[out] stormingDetectedPtr      - when not NULL , used to check if storming detected.
*                                      relevant only when trunkMemberSenderPtr != NULL
* @param[out] totalCountOnLocalPortsPtr - when not NULL , used to retrieve the total
*                                      number of packets that the local ports of the trunk received.
*                                      (relevant to trunk with remote ports)
*                                      relevant only to mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E
*                                       None
*/
static void internal_prvTgfTrunkLoadBalanceCheck
(
    IN GT_TRUNK_ID  trunkId,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  mode,
    IN GT_U32       numPacketSent,
    IN GT_U32       tolerance,
    IN CPSS_TRUNK_MEMBER_STC *trunkMemberSenderPtr,
    OUT GT_BOOL     *stormingDetectedPtr,
    OUT GT_U32      *totalCountOnLocalPortsPtr
)
{
    GT_STATUS   rc;
    GT_U8   devNum = prvTgfDevNum;
    GT_U32  ii;
    GT_U32  portIter;/* port iterator */
    GT_U32                  numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    static CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32  firstPortWithCounter = 0xFFFFFFFF;/*relevant only to mode 'Single port'*/
    GT_U32  expectedCountPerPort;/*relevant only to mode 'even'*/
    GT_U32  *expectedCountPtr = NULL;/*relevant to 'single port only'*/
    GT_U32  expectedCount = numPacketSent - tolerance;/*relevant to 'single port only'*/
    CPSS_TRUNK_TYPE_ENT trunkType;/* trunk type*/
    CPSS_TRUNK_MEMBER_STC currMember;/* currentMember*/
    CPSS_PORTS_BMP_STC cascadeTrunkPorts;/* cascade trunk ports */
    GT_U32  cascadePortIndex;/*cascade port index*/
    GT_U32  membersIndexesBmp = trunkLbhMembersIndexesBmp;/* members indexes that may get traffic */
    GT_U32  numMembersReceive = 0;
    GT_U32  originalNumPacketSent = numPacketSent;/* original number of packet sent to the trunk */
    GT_BOOL trafficReceivedOnSender = GT_FALSE;/* was traffic switched back to the sender */
    GT_U32  maxIterations;/* max iterations for break of loopback */
    GT_U32  sleepTime;/*time to sleep -- in milliseconds */
    static CPSS_TRUNK_WEIGHTED_MEMBER_STC  weightedMembers[64];
    GT_U32  totalWeight = 0;/*total weight of the weighted members */
    GT_U32  totalPacketsReceivedOnTrunkPorts = 0;/* total packets received on the trunk ports */
    GT_U32  totalPacketsReceivedOnTrunkPorts_localOnly = 0;/* total packets received on LOCAL ports of the trunk */
    GT_BOOL checkOnlySummaryOnTrunkPorts = GT_FALSE;
    GT_BOOL trunkWithRemoteMembers = GT_FALSE;/* is this trunk with remote members */
    GT_BOOL modeMultiDestinationEven = GT_FALSE;/* is the traffic to this trunk is "mode multi destination even"*/
    GT_U32  numOfDesignatedTrunkEntriesHw = PRV_CPSS_TRUNK_DESIGNATED_TABLE_SIZE_MAC(devNum);/*PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw;*/
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  origMode = mode;
    CPSS_CSCD_PORT_TYPE_ENT isIngressTrunkCascade;/* indication that sender port of the trunk is cascade (DSA expected) */

    TRUNK_PRV_UTF_LOG4_MAC("=======  internal_prvTgfTrunkLoadBalanceCheck: trunkId[%d] mode[%d] numPacketSent[%d] burst tolerance[%d]=======\n",
            trunkId,mode,numPacketSent,tolerance);


    if(stormingDetectedPtr)
    {
        *stormingDetectedPtr = GT_FALSE;
    }

    switch(mode)
    {
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E:
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E:
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E:
            mode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
            numPacketSent = 0;
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E:
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E:
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_CHECK_ONLY_TOTAL_E:
            checkOnlySummaryOnTrunkPorts = GT_TRUE;
            break;
        case PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_MULTI_DESTINATION_EVEN_E:
            mode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
            modeMultiDestinationEven = GT_TRUE;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
            return;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cascadeTrunkPorts);
    cascadePortIndex = 0;

    TRUNK_PRV_UTF_LOG0_MAC("=======  : trunkId type get =======\n");
    rc = prvCpssGenericTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(trunkType != CPSS_TRUNK_TYPE_CASCADE_E)
    {
        TRUNK_PRV_UTF_LOG0_MAC("=======  : trunkId members get =======\n");
        rc = prvCpssGenericTrunkDbEnabledMembersGet(devNum,trunkId,&numOfEnabledMembers,enabledMembersArray);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, numOfEnabledMembers);
            return;
        }

        /* convert HW devNum to SW devNum */
        for (ii = 0; ii < numOfEnabledMembers; ii++)
        {
            rc = prvUtfSwFromHwDeviceNumberGet(enabledMembersArray[ii].hwDevice,&enabledMembersArray[ii].hwDevice);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }


        if(trunkMemberSenderPtr &&
           origMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E &&
            PRV_CPSS_SIP_5_CHECK_MAC(devNum) &&
            !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
            trunkId >= 256)
        {
            /* check the port cascading type */
            rc = prvTgfCscdPortTypeGet(prvTgfDevNum, trunkMemberSenderPtr->port,
                               CPSS_PORT_DIRECTION_RX_E, &isIngressTrunkCascade);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if(isIngressTrunkCascade == CPSS_CSCD_PORT_NETWORK_E)
            {
                PRV_UTF_LOG0_MAC("=======  We expect flood back due to : \n"
                    "JIRA : EGF-1008 : For non-DSA packets received on network port we can use only 255 trunks =======\n");
                /* we assume that this is sender is from the trunk.
                   but the src trunk filtering is not working .

                   JIRA : EGF-1008 : For non-DSA packets received on network port
                   we can use only 255 trunks

                */
                mode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_CHECK_ONLY_TOTAL_E;
                origMode = mode;
                /* restore number of expected packets to egress from the trunk */
                numPacketSent = originalNumPacketSent;
            }
            else
            {
                /* no src filtering problem expected */
            }
        }
    }
    else
    {
        TRUNK_PRV_UTF_LOG0_MAC("=======  : cascade trunk ports get =======\n");

        if(trunkUseWeightedPorts == GT_TRUE)
        {
            numOfEnabledMembers = sizeof(weightedMembers) / sizeof(weightedMembers[0]);
            /* the cascade ports of the trunk used with weights */
            rc = prvTgfTrunkWithWeightedMembersGet(prvTgfDevNum,trunkId,
                    &numOfEnabledMembers,
                    weightedMembers);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            for (ii = 0; ii < numOfEnabledMembers; ii++)
            {
                totalWeight += weightedMembers[ii].weight;
            }
        }
        else
        {
            /* cascade trunk */
            rc = prvCpssGenericTrunkCascadeTrunkPortsGet(devNum,trunkId,&cascadeTrunkPorts);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            numOfEnabledMembers = 0;

            for (ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cascadeTrunkPorts,ii))
                {
                    continue;
                }

                numOfEnabledMembers++;
            }
        }


        if(numOfEnabledMembers == 0)
        {
            /* empty trunk ??? */
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, numOfEnabledMembers);
            return;
        }
    }


    /* relevant to modes 'even' and 'Specific members' */
    expectedCountPerPort = numPacketSent / numOfEnabledMembers;
    if (mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E)
    {
        numMembersReceive = 0;
        for (ii = 0; ii < 32; ii++)
        {
            if((1 << ii) &  membersIndexesBmp)
            {
                numMembersReceive++;
            }
        }

        expectedCountPerPort = numPacketSent / numMembersReceive;
    }
    else if (mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E)
    {
        expectedCountPerPort = numPacketSent / trunkLbhNumOfLimitedMembers;
        numMembersReceive = 0;/* number of members that actually get traffic */
    }

    TRUNK_PRV_UTF_LOG0_MAC("=======  : check members ports counters =======\n");
    /* check counters */
    for (ii = 0; ii < numOfEnabledMembers; ii++)
    {

        if(trunkType != CPSS_TRUNK_TYPE_CASCADE_E)
        {
            currMember.hwDevice = enabledMembersArray[ii].hwDevice;
            currMember.port = enabledMembersArray[ii].port;
        }
        else
        {
            if(trunkUseWeightedPorts == GT_TRUE)
            {
                currMember.hwDevice = weightedMembers[ii].member.hwDevice;
                currMember.port = weightedMembers[ii].member.port;

                if(mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E)
                {
                    /* each port is expected packets according to the relative weight */
                    expectedCountPerPort =
                        (numPacketSent * weightedMembers[ii].weight) / totalWeight;

                    PRV_UTF_LOG3_MAC("check cascade port [%d] ,weight[%d] out of[%d] \n",currMember.port,weightedMembers[ii].weight,totalWeight);
                }
            }
            else
            {
                for(/* continue */;cascadePortIndex < CPSS_MAX_PORTS_NUM_CNS ; cascadePortIndex++)
                {
                    if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&cascadeTrunkPorts,cascadePortIndex))
                    {
                        break;
                    }
                }

                currMember.port = cascadePortIndex;
                currMember.hwDevice = devNum;
                cascadePortIndex++;
            }
        }

        if(modeMultiDestinationEven == GT_TRUE)
        {
            if(trunkType == CPSS_TRUNK_TYPE_CASCADE_E && trunkUseWeightedPorts == GT_TRUE)
            {
                /* for cascade trunk with weights the expected count for this port was already calculated */
            }
            else
            {
                GT_U32  tmpWeightInDesignatedTable = numOfDesignatedTrunkEntriesHw / numOfEnabledMembers;

                if(ii < (numOfDesignatedTrunkEntriesHw % numOfEnabledMembers))
                {
                    /* this member will be represented 1 more time than the others */
                    tmpWeightInDesignatedTable++;
                }

                expectedCountPerPort =
                    (numPacketSent * tmpWeightInDesignatedTable) / numOfDesignatedTrunkEntriesHw;
            }

        }


        if(devNum != currMember.hwDevice)
        {
            /* support trunk with members on 'Remote device' */
            /* the traffic that is destined to this remote device should go to
               cascade port/trunk according to the device map table */
            /* this need to be checked elsewhere */

            totalPacketsReceivedOnTrunkPorts += expectedCountPerPort;/* emulate that the 'remote member' got the expected packets */

            trunkWithRemoteMembers = GT_TRUE;

            continue;
        }


        TRUNK_PRV_UTF_LOG2_MAC("=======  : dev[%d]port[%d] read counters  =======\n",
            currMember.hwDevice, currMember.port);
        /* read counters */

        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvTgfReadPortCountersEth((GT_U8)currMember.hwDevice, currMember.port, GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     currMember.hwDevice, currMember.port);

        TRUNK_PRV_UTF_LOG2_MAC("=======  : dev[%d]port[%d] check that port should get this traffic  =======\n",
            currMember.hwDevice, currMember.port);


        totalPacketsReceivedOnTrunkPorts += portCntrs.goodPktsSent.l[0];
        totalPacketsReceivedOnTrunkPorts_localOnly += portCntrs.goodPktsSent.l[0];

        if(trunkMemberSenderPtr)
        {
            if(trunkMemberSenderPtr->hwDevice == currMember.hwDevice &&
               trunkMemberSenderPtr->port   == currMember.port )
            {
                CPSS_PORT_MAC_COUNTER_SET_STC tmpPortCntrs;
                /* ignore the ORIGINAL number of packets sent to it (from the CPU). */
                portCntrs.goodPktsSent.l[0] -= originalNumPacketSent;
                totalPacketsReceivedOnTrunkPorts -= originalNumPacketSent;

                totalPacketsReceivedOnTrunkPorts_localOnly -= originalNumPacketSent;

                /* since this port (member of the trunk) is the sender of traffic ,
                   and the traffic may be <multi-destination local switching Enable> == GT_TRUE
                   meaning that we may get storming of packets.

                   */

                if(portCntrs.goodPktsSent.l[0])
                {
                    /* we need to wait until all packets processed */

                    /* let the device time to terminate the storming */
#ifdef ASIC_SIMULATION
                    maxIterations = 50; /* 0.5 seconds -- tests show that only 2 loops are used !!! */
                    sleepTime = 10;
#else /*!ASIC_SIMULATION*/
                    maxIterations = 5;  /* also in BM some cases needed 2 loops instead of 1 */
                    sleepTime = 10;
#endif /*!ASIC_SIMULATION*/
                    do
                    {
                        cpssOsTimerWkAfter(sleepTime);
                        /* read the counters */
                        rc = prvTgfReadPortCountersEth((GT_U8)currMember.hwDevice, currMember.port, GT_TRUE, &tmpPortCntrs);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                                     currMember.hwDevice, currMember.port);
                    }
                    while((--maxIterations) && (tmpPortCntrs.goodPktsSent.l[0] || tmpPortCntrs.goodPktsSent.l[1]));

                    if(maxIterations == 0 && (tmpPortCntrs.goodPktsSent.l[0] || tmpPortCntrs.goodPktsSent.l[1]))
                    {
                        /* error -- print it regardless to 'Printing mode' */
                        PRV_UTF_LOG2_DEBUG_MAC("== ERROR == ERROR ==  : dev[%d]port[%d] port storming detected but not terminated !  =======\n",
                                               currMember.hwDevice, currMember.port);
                    }

                    /*7. skip further processing of this port*/
                    trafficReceivedOnSender = GT_TRUE;
                    if(stormingDetectedPtr)
                    {
                        *stormingDetectedPtr = GT_TRUE;
                    }

/*                    cpssOsPrintf("$%d$",maxIterations);*/

                    continue;
                }

            }
        }

        if(checkOnlySummaryOnTrunkPorts == GT_TRUE)
        {
            /* no need to check the LBH */
            continue;
        }

        if(mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E)
        {
            if(portCntrs.goodPktsSent.l[0] != 0)
            {
                if(firstPortWithCounter == 0)
                {
                    /* this is the third port that got traffic ? */
                    UTF_VERIFY_EQUAL3_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                        "trunk load balance: this is the third port that got traffic ? : in trunk[%d] expected [%d] got [%d]",
                                                 trunkId,0, portCntrs.goodPktsSent.l[0]);
                }
                else if(firstPortWithCounter == 0xFFFFFFFF)
                {
                    expectedCountPtr = portCntrs.goodPktsSent.l[0] > tolerance ?
                                       &expectedCount : &tolerance;

                    firstPortWithCounter &= 0xffff;

                    if(portCntrs.goodPktsSent.l[0] != numPacketSent &&
                       portCntrs.goodPktsSent.l[0] != (numPacketSent - tolerance) &&
                       portCntrs.goodPktsSent.l[0] != tolerance)
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC((*expectedCountPtr), portCntrs.goodPktsSent.l[0],
                            "trunk load balance: in trunk[%d] expected [%d] got [%d]",
                                                     trunkId,(*expectedCountPtr), portCntrs.goodPktsSent.l[0]);
                    }
                }
                else if(tolerance)
                {
                    firstPortWithCounter &= 0xffff0000;

                    if(expectedCountPtr == (&tolerance))
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                            "trunk load balance: in trunk[%d] expected [%d] got [%d]",
                                                     trunkId,expectedCount, portCntrs.goodPktsSent.l[0]);
                    }
                    else /*if(expectedCountPtr == (&expectedCount))*/
                    {
                        UTF_VERIFY_EQUAL3_STRING_MAC(tolerance, portCntrs.goodPktsSent.l[0],
                            "trunk load balance: in trunk[%d] expected [%d] got [%d]",
                                                     trunkId,tolerance, portCntrs.goodPktsSent.l[0]);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL,
                        "trunk load balance: got packet not on single port in trunk[%d] ",
                                                 trunkId);
                }
            }
            else if(expectedCount == 0)
            {
                /* we expect no counter on the trunk's ports */
                UTF_VERIFY_EQUAL3_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                    "trunk load balance: in trunk[%d] expected [%d] got [%d]",
                                             trunkId,expectedCount, portCntrs.goodPktsSent.l[0]);
            }
        }
        else if (mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E)
        {
            /*  need to see that member that should get traffic will get it,
                and member that should not get it will not get it.
            */

            /* look for the member index in the test */

            for(portIter = 0 ; portIter < prvTgfPortsNum ; portIter++)
            {
                if(0 == ((1 << portIter) &  membersIndexesBmp))
                {
                    /* we expect no traffic from this trunk's port */
                    continue;
                }

                if(prvTgfDevsArray[portIter]  == currMember.hwDevice &&
                   prvTgfPortsArray[portIter] == currMember.port )
                {
                    /* this member should receive traffic */
                    break;
                }
            }

            if(portIter == prvTgfPortsNum)
            {
                /* we expect no traffic from this trunk's port */
                UTF_VERIFY_EQUAL5_STRING_MAC(0 , portCntrs.goodPktsSent.l[0],
                    "trunk load balance: expected[%d] received [%d] on dev[%d] on port[%d] in trunk[%d] ",
                                             0,portCntrs.goodPktsSent.l[0],
                                             currMember.hwDevice, currMember.port,
                                             trunkId);
            }
            else
            {
                if(portCntrs.goodPktsSent.l[0] != expectedCountPerPort &&
                   portCntrs.goodPktsSent.l[0] != (expectedCountPerPort + 1))
                {
                    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                    {
                        tolerance = 8;/*tolerance not relevant to this mode , so I use it */
                        if(numPacketSent < tolerance)
                        {
                            tolerance = numPacketSent;
                        }
                    }
                    else
                    {
                        tolerance = 0;/*tolerance not relevant to this mode , so I use it */
                    }

                    if(tolerance &&
                        ((portCntrs.goodPktsSent.l[0] >= ((tolerance > expectedCountPerPort) ? 0 : (expectedCountPerPort - tolerance))) &&
                         (portCntrs.goodPktsSent.l[0] <= (expectedCountPerPort + tolerance))))
                    {
                        /* we allow tolerance even in 'even mode' due to not 'smoth'
                           load balance on 'incremental' changes */
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL5_STRING_MAC(expectedCountPerPort , portCntrs.goodPktsSent.l[0],
                            "trunk load balance: expected[%d] received [%d] on dev[%d] on port[%d] in trunk[%d] ",
                                                     expectedCountPerPort,portCntrs.goodPktsSent.l[0],
                                                     currMember.hwDevice, currMember.port,
                                                     trunkId);
                    }
                }
            }


        }
        else if (mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E)
        {
            if(portCntrs.goodPktsSent.l[0] != 0)
            {
                /* count that this port got traffic */
                numMembersReceive++;

                /* check that it got 'Expected value'*/
                if(portCntrs.goodPktsSent.l[0] != expectedCountPerPort &&
                   portCntrs.goodPktsSent.l[0] != (expectedCountPerPort + 1))
                {
                    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                    {
                        tolerance = 8;/*tolerance not relevant to this mode , so I use it */
                        if(numPacketSent < tolerance)
                        {
                            tolerance = numPacketSent;
                        }
                    }
                    else
                    {
                        tolerance = 0;/*tolerance not relevant to this mode , so I use it */
                    }

                    if(tolerance &&
                        ((portCntrs.goodPktsSent.l[0] >= ((tolerance > expectedCountPerPort) ? 0 : (expectedCountPerPort - tolerance))) &&
                         (portCntrs.goodPktsSent.l[0] <= (expectedCountPerPort + tolerance))))
                    {
                        /* we allow tolerance even in 'even mode' due to not 'smoth'
                           load balance on 'incremental' changes */
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL5_STRING_MAC(expectedCountPerPort , portCntrs.goodPktsSent.l[0],
                            "trunk load balance: expected[%d] received [%d] on dev[%d] on port[%d] in trunk[%d] ",
                                                     expectedCountPerPort,portCntrs.goodPktsSent.l[0],
                                                     currMember.hwDevice, currMember.port,
                                                     trunkId);
                    }
                }
            }
        }
        else if(mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_CHECK_ONLY_TOTAL_E)
        {
            /* we not check specific port counters ... only the total at the end */
            /* because we not know the expected LBH */
        }
        /*PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E*/
        else if (GT_FALSE == prvTgfCommonIsDeviceForce(prvTgfDevNum))
        {
            if(portCntrs.goodPktsSent.l[0] != expectedCountPerPort &&
               portCntrs.goodPktsSent.l[0] != (expectedCountPerPort + 1))
            {
                if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                {
                    tolerance = 8;/*tolerance not relevant to this mode , so I use it */
                    if(numPacketSent < tolerance)
                    {
                        tolerance = numPacketSent;
                    }
                }
                else
                {
                    tolerance = 0;/*tolerance not relevant to this mode , so I use it */
                }

                if(tolerance &&
                    ((portCntrs.goodPktsSent.l[0] >= ((tolerance > expectedCountPerPort) ? 0 : (expectedCountPerPort - tolerance))) &&
                     (portCntrs.goodPktsSent.l[0] <= (expectedCountPerPort + tolerance))))
                {
                    /* we allow tolerance even in 'even mode' due to not 'smoth'
                       load balance on 'incremental' changes */
                }
                else
                {
                    UTF_VERIFY_EQUAL5_STRING_MAC(expectedCountPerPort , portCntrs.goodPktsSent.l[0],
                        "trunk load balance: expected[%d] received [%d] on dev[%d] on port[%d] in trunk[%d] ",
                                                 expectedCountPerPort,portCntrs.goodPktsSent.l[0],
                                                 currMember.hwDevice, currMember.port,
                                                 trunkId);
                }
            }
        }
    }

    if(totalCountOnLocalPortsPtr)
    {
        *totalCountOnLocalPortsPtr = totalPacketsReceivedOnTrunkPorts_localOnly;
    }


    if(origMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E)
    {
        /* must get 0 ... so no storming expected */

        UTF_VERIFY_EQUAL2_STRING_MAC(0, totalPacketsReceivedOnTrunkPorts,
            "trunk load balance: the trunk [%d] should get NO packets but got [%d] packets",
                                     trunkId,totalPacketsReceivedOnTrunkPorts);
    }
    else
    if(trafficReceivedOnSender == GT_FALSE) /* no storming */
    {

        if(totalCountOnLocalPortsPtr &&
           (trunkWithRemoteMembers == GT_TRUE) &&
           mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E)
        {
            /* we can't check here the expected number of packets on the trunk
               members due to load balance issues. */
            /* we will let the caller check it */
            TRUNK_PRV_UTF_LOG2_MAC("TrunkId[%d] got [%d] packets on local ports: "
                "The caller will check the total number of "
                "'even LBH' for packets received on trunk with remote ports \n",
                trunkId,totalPacketsReceivedOnTrunkPorts_localOnly);
        }
        else
        if(!totalCountOnLocalPortsPtr)
        {
            if(mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E &&
                (trunkWithRemoteMembers == GT_TRUE) &&
               (numPacketSent != totalPacketsReceivedOnTrunkPorts))
            {
                /* for the trunk with remote members we can't get the counters of
                   actual traffic that egress those ports , so we emulated the counting
                   on those ports
                   but this emulation can be not accurate.
                */
                tolerance = numPacketSent % numOfEnabledMembers;

                if((numPacketSent < totalPacketsReceivedOnTrunkPorts) ||
                    ((numPacketSent - tolerance) < totalPacketsReceivedOnTrunkPorts))
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(numPacketSent, totalPacketsReceivedOnTrunkPorts,
                        "trunk load balance: the trunk [%d] should get [%d] packets but got [%d] packets",
                                                 trunkId,numPacketSent, totalPacketsReceivedOnTrunkPorts);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(numPacketSent, totalPacketsReceivedOnTrunkPorts,
                    "trunk load balance: the trunk [%d] should get [%d] packets but got [%d] packets",
                                             trunkId,numPacketSent, totalPacketsReceivedOnTrunkPorts);
            }
        }
    }
    else
    {
        /* we had storming , we can not check specific number of packets that egress the trunk
           but we can check 'more than 1' */
        UTF_VERIFY_EQUAL2_STRING_MAC((totalPacketsReceivedOnTrunkPorts > 1) ? 1 : 0, 1,
            "trunk load balance: the trunk [%d] should get 'storming' packets (more than one) but got [%d] packets",
                                     trunkId, totalPacketsReceivedOnTrunkPorts);
    }

    if(mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E)
    {
        if(firstPortWithCounter == 0xFFFFFFFF && numPacketSent != 0 &&
           trafficReceivedOnSender == GT_FALSE)/* single port that received was the sender */
        {
            /* no port received the traffic */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL,
                "trunk load balance: no port receive packets in trunk [%d] ",
                                         trunkId);
        }
    }
    else if (mode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E)
    {
        if(numMembersReceive != trunkLbhNumOfLimitedMembers)
        {
            /* no port received the traffic */
            UTF_VERIFY_EQUAL3_STRING_MAC(trunkLbhNumOfLimitedMembers, numMembersReceive,
                "trunk load balance: in trunk [%d] received number of ports [%d] , instead of [%d]",
                                         trunkId, numMembersReceive, trunkLbhNumOfLimitedMembers);
        }
    }

    TRUNK_PRV_UTF_LOG0_MAC("======= internal_prvTgfTrunkLoadBalanceCheck : ended  =======\n");

    return;
}

/**
* @internal prvTgfTrunkLoadBalanceCheck function
* @endinternal
*
* @brief   check that total summary of counters of ports that belong to trunk got value
*         of numPacketSent.
* @param[in] trunkId                  - trunk Id that his ports should received the traffic and on
*                                      them we expect total of numPacketSent packets
* @param[in] mode                     - the  of expected load balance .
*                                      all traffic expected to be received from single port or with
*                                      even distribution on the trunk ports
* @param[in] numPacketSent            - number of packet that we expect the trunk to receive
* @param[in] tolerance                - relevant to PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E
*                                      state the number of packet that may get to other port in the trunk.
*                                      meaning that (numPacketSent - tolerance) should get to port A
*                                      and tolerance should get to port B , but other ports (in trunk)
*                                      should not get it.
* @param[in] trunkMemberSenderPtr     - when not NULL , indicates that the traffic to check
*                                      LBH that egress the trunk was originally INGRESSED from the trunk.
*                                      but since in the 'enhanced UT' the CPU send traffic to a port
*                                      due to loopback it returns to it, we need to ensure that the
*                                      member mentioned here should get the traffic since it is the
*                                      'original sender'
*
* @param[out] stormingDetectedPtr      - when not NULL , used to check if storming detected.
*                                      relevant only when trunkMemberSenderPtr != NULL
*                                       None
*/
void prvTgfTrunkLoadBalanceCheck
(
    IN GT_TRUNK_ID  trunkId,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  mode,
    IN GT_U32       numPacketSent,
    IN GT_U32       tolerance,
    IN CPSS_TRUNK_MEMBER_STC *trunkMemberSenderPtr,
    OUT GT_BOOL     *stormingDetectedPtr
)
{
    internal_prvTgfTrunkLoadBalanceCheck(trunkId,mode,numPacketSent,tolerance,
        trunkMemberSenderPtr,stormingDetectedPtr,NULL);
}

/**
* @internal prvTgfTrunkLoadBalanceSpecificMembersSet function
* @endinternal
*
* @brief   set the specific members of a trunk that will get traffic when the LBH
*         expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E.
* @param[in] membersIndexesBmp        - bmp of indexes in the arrays of :
*                                      prvTgfDevsArray[] , prvTgfPortsArray[]
*                                      the traffic should do LBH in those ports and not in other ports of
*                                      the trunk.
*                                      WARNING : this is NOT a good practice to set the system to get this behavior
*                                       None
*/
void prvTgfTrunkLoadBalanceSpecificMembersSet
(
    IN GT_U32   membersIndexesBmp
)
{
    trunkLbhMembersIndexesBmp = membersIndexesBmp;

    return;
}

/**
* @internal prvTgfTrunkLoadBalanceLimitedNumSet function
* @endinternal
*
* @brief   set the limited number of members of a trunk that will get traffic when the LBH
*         expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E.
*/
void prvTgfTrunkLoadBalanceLimitedNumSet
(
    IN GT_U32   numOfLimitedMembers
)
{
    trunkLbhNumOfLimitedMembers = numOfLimitedMembers;

    return;
}

/**
* @internal trunkHashPearsonFill function
* @endinternal
*
* @brief   fill Pearson hash table according to mode .
*         applicable devices: Lion and above
*/
static GT_STATUS trunkHashPearsonFill
(
    IN PEARSON_TEST_MODE_ENT   pearsonMode
)
{
    GT_STATUS   rc;
    GT_U32      index;
    GT_U32      value;

    for(index = 0 ; index < PRV_TGF_TRUNK_PEARSON_TABLE_SIZE_CNS ; index++)
    {
        switch(pearsonMode)
        {
            case PEARSON_TEST_MODE_0_E:              /*value = 0 (all indexes) */
                value = 0;
                break;
            case PEARSON_TEST_MODE_INDEX_E:          /*value = index (0..63)*/
                value = index;
                break;
            case PEARSON_TEST_MODE_REVERSE_INDEX_E:  /*value = 63 - index (63..0)*/
                value = (PRV_TGF_TRUNK_PEARSON_TABLE_SIZE_CNS - 1) - index;
                break;
            case PEARSON_TEST_MODE_START_MID_INDEX_E:/*value = (32 + index) % 64 (32..63,0..31)*/
                value = ((PRV_TGF_TRUNK_PEARSON_TABLE_SIZE_CNS / 2) + index) % PRV_TGF_TRUNK_PEARSON_TABLE_SIZE_CNS;
                break;
            case PEARSON_TEST_MODE_EVEN_INDEX_E:     /*value = index & 0x3e (0,0,2,2..62,62)*/
                value = index & 0xfffffffe;
                break;
            case PEARSON_TEST_MODE_MODE_8_INDEX_E:   /*value = index & 0x7 (0..7,0..7,0..7,...,0..7)*/
                value = index & 0x7;
                break;
            default:
                return GT_BAD_PARAM;
        }

        rc = prvTgfTrunkHashPearsonValueSet(index,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal trunkHashMaskCrcTableEntryClear function
* @endinternal
*
* @brief   clear the single entry or entire table of trunk hash mask CRC
*         applicable devices: Lion and above
* @param[in] index                    - indecx in the table .
*                                      PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS means clear
*                                      all table
*                                       None
*/
static GT_STATUS trunkHashMaskCrcTableEntryClear
(
    IN GT_U32              index
)
{
    GT_STATUS   rc;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC    entry;
    GT_U32   ii,iiStart,iiLast;

    cpssOsMemSet(&entry,0,sizeof(entry));

    if(index == PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS)
    {
        iiStart = 0;
        iiLast = hashMaskTableSize;
    }
    else
    {
        iiStart = index;
        iiLast = index + 1;
    }

    for(ii = iiStart ; ii < iiLast; ii++)
    {
        rc = prvTgfTrunkHashMaskCrcEntrySet(GT_TRUE/*use index*/,ii,0/*don't care*/,&entry);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
/**
* @internal trunkHashMaskCrcFieldSet function
* @endinternal
*
* @brief   set single field in the trunk hash mask CRC table
*         applicable devices: Lion and above
* @param[in] index                    -  in the table
* @param[in] fieldType                - field type
* @param[in] fieldSubType             - field type sub index
* @param[in] fieldByteIndex           - byte index in the field
*                                       None
*/
static GT_STATUS trunkHashMaskCrcFieldSet
(
    IN GT_U32              index,
    IN PRV_TGF_TRUNK_FIELD_TYPE_ENT fieldType,
    IN GT_U32              fieldSubType,
    IN GT_U32              fieldByteIndex
)
{
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC    entry;
    GT_U32  *fieldPtr;

    cpssOsMemSet(&entry,0,sizeof(entry));

    switch(fieldType)
    {
        case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
            if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E)
            {
                fieldPtr = &entry.l4SrcPortMaskBmp;
            }
            else
            {
                fieldPtr = &entry.l4DstPortMaskBmp;
            }
            break;
        case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
            if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E)
            {
                fieldPtr = &entry.ipSipMaskBmp;
            }
            else
            {
                fieldPtr = &entry.ipDipMaskBmp;
            }
            break;
        case PRV_TGF_TRUNK_FIELD_MAC_ADDR_E:
            if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E)
            {
                fieldPtr = &entry.macSaMaskBmp;
            }
            else
            {
                fieldPtr = &entry.macDaMaskBmp;
            }
            break;
        case PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E:
            switch(fieldSubType)
            {
                case 0:
                    fieldPtr = &entry.mplsLabel0MaskBmp;
                    break;
                case 1:
                    fieldPtr = &entry.mplsLabel1MaskBmp;
                    break;
                default:
                    fieldPtr = &entry.mplsLabel2MaskBmp;
                    break;
            }
            break;
        case PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E:
            fieldPtr = &entry.ipv6FlowMaskBmp;
            break;
        case PRV_TGF_TRUNK_FIELD_LOCAL_SRC_PORT_E:
            fieldPtr = &entry.localSrcPortMaskBmp;
            break;
        case PRV_TGF_TRUNK_FIELD_UDBS_E:
            fieldPtr = &entry.udbsMaskBmp;
            break;
        default:
            return GT_BAD_PARAM;
    }

    *fieldPtr = 1 << fieldByteIndex;

    entry.symmetricMacAddrEnable  = symmetricMacAddrEnable ;
    entry.symmetricIpv4AddrEnable = symmetricIpv4AddrEnable;
    entry.symmetricIpv6AddrEnable = symmetricIpv6AddrEnable;
    entry.symmetricL4PortEnable   = symmetricL4PortEnable  ;

    if(index < 16)
    {
        return prvTgfTrunkHashMaskCrcEntrySet(GT_TRUE/*use index*/ , index, 0/*don't care*/ ,&entry);
    }
    else
    {
        PRV_TGF_PCL_PACKET_TYPE_ENT    packetType;


        packetType = index - 16;

        return prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE/*use packet type*/,0/*don't care*/,packetType,&entry);
    }

}

/**
* @internal trunkSendCheckLbh1 function
* @endinternal
*
* @brief   for single destination :
*         check the 'Single destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) not
*         received flooding
*         for multi-destination :
*         check the 'Multi destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) also
*         received flooding
*         applicable devices: All DxCh devices
* @param[in] singleDestination        - is it single/multi destination traffic
* @param[in] sendingPortsBmp          - bitmap of ports that send bursts
* @param[in] burstCount               - number of packets in burst
* @param[in] lbhMode                  - expected LBH mode
* @param[in] trunkMemberSenderPtr     - when not NULL , indicates that the traffic to check
*                                      LBH that egress the trunk was originally INGRESSED from the trunk.
*                                      but since in the 'enhanced UT' the CPU send traffic to a port
*                                      due to loopback it returns to it, we need to ensure that the
*                                      member mentioned here should get the traffic since it is the
*                                      'original sender'
* @param[in] skipTrunkCheck           - do we skip check on trunk members
*                                      bmpOfPortsToSkipCheckPtr - (pointer to) relevant only when != NULL
*                                      local ports that the count of their counters need to be skipped
*                                       None
*/
static void trunkSendCheckLbh1
(
    IN GT_BOOL             singleDestination,
    IN GT_U32              sendingPortsBmp,
    IN GT_U32              burstCount,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN CPSS_TRUNK_MEMBER_STC *trunkMemberSenderPtr,
    IN GT_BOOL            skipTrunkCheck,
    IN CPSS_PORTS_BMP_STC *bmpOfPortsToCheckFloodPtr
)
{
    GT_STATUS   rc;
    GT_U8   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;/*device number to send packets , and port number */
    GT_U32  portIter;/* port iterator */
    GT_U32  lbhTolerance;/* LBH tolerance on port */
    GT_TRUNK_ID             trunkId;/* trunkId that should get the 'known UC' */
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_BOOL                         isEqualCntrs = GT_FALSE;
    GT_U32               totalCount;/* total packets sent count */
    CPSS_TRUNK_MEMBER_STC   trunkMember;/*trunk member*/
    GT_BOOL                 stormingDetected;/*used to check if storming detected
                                relevant only when trunkMemberSenderPtr != NULL */
    GT_U32                  numberOfPortsSending;/* number of sending ports */

    numberOfPortsSending = prvCpssPpConfigBitmapNumBitsGet(sendingPortsBmp);

    totalCount =  numberOfPortsSending * burstCount;
    lbhTolerance = 0;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    if(skipTrunkCheck == GT_FALSE)
    {
        prvTgfTrunkLoadBalanceCheck(trunkId,lbhMode,totalCount,lbhTolerance,trunkMemberSenderPtr,&stormingDetected);
    }
    else
    {
        stormingDetected = GT_FALSE;
    }

    /* check that other ports in vlan got/not the traffic */
    for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
    {
        devNum  = prvTgfDevsArray [portIter];
        portNum = prvTgfPortsArray[portIter];

        if(devNum != prvTgfDevNum)
        {
            /* skip ports on remote device (members of the network trunk) */
            continue;
        }

        trunkMember.hwDevice = devNum;
        trunkMember.port   = portNum;
        /* skip ports members in the trunk */
        if(GT_OK == prvTgfTrunkDbIsMemberOfTrunk(devNum,&trunkMember,NULL))
        {
            /* trunk member --> skip it */
            continue;
        }

        if(bmpOfPortsToCheckFloodPtr)
        {
            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(bmpOfPortsToCheckFloodPtr,portNum))
            {
                continue;
            }
        }

        TRUNK_PRV_UTF_LOG2_MAC("=======  : dev[%d]port[%d] read counters  =======\n",
            devNum, portNum);
        /* read counters */
        rc = prvTgfReadPortCountersEth(devNum, portNum, GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     devNum, portNum);

        /* copy read counters to  expected counters */
        cpssOsMemCpy(&expectedCntrs, &portCntrs, sizeof(expectedCntrs));


        if(singleDestination == GT_TRUE)
        {
            if(sendingPortsBmp & (1<<portIter))
            {
                /* this port was egress traffic for the 'loopback' to ingress */
                expectedCntrs.goodPktsSent.l[0] = burstCount;
            }
            else
            {
                /* no packet should egress the ports that not sent traffic */
                expectedCntrs.goodPktsSent.l[0]   = 0;
            }
        }
        else
        {
            /* those ports need to be flooded for each packet sent from other ports */
            if(sendingPortsBmp & (1<<portIter))
            {
                /* this port will egress from other ports that send and also the 'loopback' to ingress*/
                expectedCntrs.goodPktsSent.l[0] = burstCount * numberOfPortsSending;
            }
            else
            {
                /* this port will egress from other ports that send */
                expectedCntrs.goodPktsSent.l[0] = burstCount * numberOfPortsSending;
            }
        }

        if(stormingDetected == GT_TRUE)
        {
            /* when storming detected , the ports get a lot of packets . */
            isEqualCntrs = GT_TRUE;

            if(expectedCntrs.goodPktsSent.l[0])
            {
                /* we expect a lot of packets */
                if(portCntrs.goodPktsSent.l[0] == 0)
                {
                    /* but we got nothing */
                    isEqualCntrs = GT_FALSE;
                }
            }
            else
            {
                /* we expected 0 , so even if storming still should be 0 */
                if(portCntrs.goodPktsSent.l[0] != 0)
                {
                    /* but got packets */
                    isEqualCntrs = GT_FALSE;
                }
            }



            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqualCntrs, "stormingDetected : get another counters values.");
            continue;
        }

        /* check Rx\Tx counters */
        PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqualCntrs, "get another counters values on dev[%d],port[%d]",devNum, portNum);

        /* print expected values if not equal */
        PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
    }
}
/**
* @internal trunkSendCheckLbh function
* @endinternal
*
* @brief   for single destination :
*         check the 'Single destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) not
*         received flooding
*         for multi-destination :
*         check the 'Multi destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) also
*         received flooding
*         applicable devices: All DxCh devices
* @param[in] singleDestination        - is it single/multi destination traffic
* @param[in] sendingPortsBmp          - bitmap of ports that send bursts
* @param[in] burstCount               - number of packets in burst
* @param[in] lbhMode                  - expected LBH mode
* @param[in] trunkMemberSenderPtr     - when not NULL , indicates that the traffic to check
*                                      LBH that egress the trunk was originally INGRESSED from the trunk.
*                                      but since in the 'enhanced UT' the CPU send traffic to a port
*                                      due to loopback it returns to it, we need to ensure that the
*                                      member mentioned here should get the traffic since it is the
*                                      'original sender'
*                                       None
*/
static void trunkSendCheckLbh
(
    IN GT_BOOL             singleDestination,
    IN GT_U32              sendingPortsBmp,
    IN GT_U32              burstCount,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN CPSS_TRUNK_MEMBER_STC *trunkMemberSenderPtr
)
{
    trunkSendCheckLbh1(singleDestination,sendingPortsBmp,burstCount,lbhMode,trunkMemberSenderPtr,GT_FALSE,NULL);

    /* make sure to clean ALL the counters of all the ports , in case that the function
        trunkSendCheckLbh1 failed in the middle and the 'fContinue flag was GT_FALSE' */
    clearAllTestedCounters();
}

GT_U32 do_calculatedHash_expectedTrunkMemberIndex(
    IN  TGF_PACKET_STC    *packetInfoPtr,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType,
    IN GT_BOOL  usedUdb,
    IN GT_U32   fieldByteIndex,
    IN GT_U32   localSrcPort,
    IN GT_U32   numTrunkMembers
)
{
    GT_STATUS   rc;
    PRV_TGF_TRUNK_LBH_INPUT_DATA_STC hashInputData;
    GT_U32  mplsLabel = 0;
    GT_U8   *mplsLabelPtr;
    TGF_PACKET_PART_STC   *partPtr;
    GT_U32  partIter;
    GT_U32  ii;

    if(numTrunkMembers == 0)
    {
        numTrunkMembers = 4;
    }
    else
    {
        /* explicit value from the caller */
        calculatedHash_numOfmembers = numTrunkMembers;
    }

    cpssOsMemSet(&hashInputData,0,sizeof(hashInputData));

    /* loop on all parts and add each part after the other */
    for(partIter = 0; partIter < packetInfoPtr->numOfParts; partIter++)
    {
        /* add the needed part of the packet */
        partPtr = &packetInfoPtr->partsArray[partIter];

        switch(partPtr->type)
        {
            case TGF_PACKET_PART_L2_E:
                {
                    TGF_PACKET_L2_STC* l2Ptr = partPtr->partPtr;
                    cpssOsMemCpy(hashInputData.macDaArray,l2Ptr->daMac,sizeof(l2Ptr->daMac));
                    cpssOsMemCpy(hashInputData.macSaArray,l2Ptr->saMac,sizeof(l2Ptr->saMac));
                }
                break;
            case TGF_PACKET_PART_MPLS_E:
                {
                    TGF_PACKET_MPLS_STC* mplsPtr = partPtr->partPtr;

                    if(mplsLabel == 0)
                    {
                        mplsLabelPtr = hashInputData.mplsLabel0Array;
                    }
                    else
                    if(mplsLabel == 1)
                    {
                        mplsLabelPtr = hashInputData.mplsLabel1Array;
                    }
                    else
                    /*if(mplsLabel == 2)*/
                    {
                        mplsLabelPtr = hashInputData.mplsLabel2Array;
                    }

                    mplsLabelPtr[2] = (GT_U8)( mplsPtr->label        & 0xFF);
                    mplsLabelPtr[1] = (GT_U8)((mplsPtr->label >> 8)  & 0xFF);
                    mplsLabelPtr[0] = (GT_U8)((mplsPtr->label >> 16) & 0x0F);

                    mplsLabel++;
                }
                break;

            case TGF_PACKET_PART_IPV4_E:
                {
                    TGF_PACKET_IPV4_STC* ipv4Ptr = partPtr->partPtr;

                    cpssOsMemCpy(&hashInputData.ipSipArray[12],ipv4Ptr->srcAddr,sizeof(ipv4Ptr->srcAddr));
                    cpssOsMemCpy(&hashInputData.ipDipArray[12],ipv4Ptr->dstAddr,sizeof(ipv4Ptr->dstAddr));
                }
                break;

            case TGF_PACKET_PART_IPV6_E:
                {
                    TGF_PACKET_IPV6_STC* ipv6Ptr = partPtr->partPtr;
                    GT_U32  iter;

                    /* convert ipv6Ptr->srcAddr with saving network order */
                    for (iter = 0; iter < 8; iter++)
                    {
                        hashInputData.ipSipArray[2 * iter]       = (GT_U8)(ipv6Ptr->srcAddr[iter] >> 8);
                        hashInputData.ipSipArray[(2 * iter) + 1] = (GT_U8)(ipv6Ptr->srcAddr[iter] & 0xFF);
                    }

                    /* convert ipv6Ptr->dstAddr with saving network order */
                    for (iter = 0; iter < 8; iter++)
                    {
                        hashInputData.ipDipArray[2 * iter]       = (GT_U8)(ipv6Ptr->dstAddr[iter] >> 8);
                        hashInputData.ipDipArray[(2 * iter) + 1] = (GT_U8)(ipv6Ptr->dstAddr[iter] & 0xFF);
                    }

                    hashInputData.ipv6FlowArray[2] = (GT_U8)( ipv6Ptr->flowLabel        & 0xFF);
                    hashInputData.ipv6FlowArray[1] = (GT_U8)((ipv6Ptr->flowLabel >> 8)  & 0xFF);
                    hashInputData.ipv6FlowArray[0] = (GT_U8)((ipv6Ptr->flowLabel >> 16) & 0x0F);

                }
                break;
            case TGF_PACKET_PART_TCP_E:
                {
                    TGF_PACKET_TCP_STC* tcpPtr = partPtr->partPtr;

                    hashInputData.l4DstPortArray[1] = (GT_U8)( tcpPtr->dstPort       & 0xFF);
                    hashInputData.l4DstPortArray[0] = (GT_U8)((tcpPtr->dstPort >> 8) & 0xFF);

                    hashInputData.l4SrcPortArray[1] = (GT_U8)( tcpPtr->srcPort       & 0xFF);
                    hashInputData.l4SrcPortArray[0] = (GT_U8)((tcpPtr->srcPort >> 8) & 0xFF);
                }

                if(symmetricL4PortEnable == GT_TRUE)
                {
                    for(ii = 0 ; ii < 2 ; ii++)
                    {
                        hashInputData.l4DstPortArray[ii] += hashInputData.l4SrcPortArray[ii];
                        hashInputData.l4SrcPortArray[ii] = 0;
                    }
                }
                break;

            case TGF_PACKET_PART_UDP_E:
                {
                    TGF_PACKET_UDP_STC* udpPtr = partPtr->partPtr;

                    hashInputData.l4DstPortArray[1] = (GT_U8)( udpPtr->dstPort       & 0xFF);
                    hashInputData.l4DstPortArray[0] = (GT_U8)((udpPtr->dstPort >> 8) & 0xFF);

                    hashInputData.l4SrcPortArray[1] = (GT_U8)( udpPtr->srcPort       & 0xFF);
                    hashInputData.l4SrcPortArray[0] = (GT_U8)((udpPtr->srcPort >> 8) & 0xFF);
                }

                if(symmetricL4PortEnable == GT_TRUE)
                {
                    for(ii = 0 ; ii < 2 ; ii++)
                    {
                        hashInputData.l4DstPortArray[ii] += hashInputData.l4SrcPortArray[ii];
                        hashInputData.l4SrcPortArray[ii] = 0;
                    }
                }

                break;
            default:
                /* not in the hash calc */
                break;
        }
    }

    if(usedUdb)
    {
        TGF_PACKET_PAYLOAD_STC *payloadPtr;/* pointer to payload info of packet */
        payloadPtr = packetInfoPtr->partsArray[packetInfoPtr->numOfParts - 1].partPtr;

        /* test uses byte 4 from the payload */
        hashInputData.udbsArray[fieldByteIndex] = payloadPtr->dataPtr[4];
    }

    hashInputData.localSrcPortArray[0] = (GT_U8)(localSrcPort >> 0);

    rc = prvTgfTrunkHashIndexCalculate(prvTgfDevNum,trafficType,
        &hashInputData,
        PRV_TGF_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
        calculatedHash_numOfmembers,
        &calculatedHash_expectedTrunkMemberIndex);
    if(rc != GT_OK)
    {
        PRV_UTF_LOG0_DEBUG_MAC(" debug ERROR ---> prvTgfTrunkHashIndexCalculate failed \n");
    }

    if(calculatedHash_expectedTrunkMemberIndex >= 8 && numTrunkMembers < 8)
    {
        PRV_UTF_LOG1_DEBUG_MAC(" ERROR ---> calculatedHash_expectedTrunkMemberIndex[%d] > 8 !? \n",
            calculatedHash_expectedTrunkMemberIndex);

        calculatedHash_expectedTrunkMemberIndex %= 8;
    }

    /* the trunk actually hold only 4 members */
    calculatedHash_expectedTrunkMemberIndex %= numTrunkMembers;

    return calculatedHash_expectedTrunkMemberIndex;
}

/**
* @internal trunkHashCrcLbhTrafficSend function
* @endinternal
*
* @brief   send traffic according to field checking
*         applicable devices: Lion and above
* @param[in] trafficPtr               - (pointer to) traffic to send
* @param[in] trafficType              - traffic type
*                                      maskCrcTableFieldType - field type
*                                      maskCrcTableFieldSubType - field type sub index
*                                      maskCrcTableFieldByteIndex - byte index in the field
* @param[in] lbhMode                  - LBH expected
* @param[in] L3Offset                 - for fields in the L3/L4 headers , define the number of bytes
*                                      from start of the packet .
* @param[in] checkSimpleHash          - check simple hash or CRC.
*                                      when check simple hash , the 'increment' gives 'perfect' LBH
*                                      even on low numbers of packets.
*                                      and since only 6 bits of byte are used , then limit number of
*                                      packets to 64.
* @param[in] reduceTimeFactor         - reduce time factor
*
* @param[out] wasIgnoredDueToReduceTimePtr - pointer to indication that the sending
*                                      was not doe due to reduce time factor.
*                                      when pointer is NULL --> ignored.
*                                       None
*/
static GT_STATUS trunkHashCrcLbhTrafficSend
(
    IN TGF_PACKET_STC      *trafficPtr,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType,
    IN PRV_TGF_TRUNK_FIELD_TYPE_ENT fieldType,
    IN GT_U32              fieldSubType,
    IN GT_U32              fieldByteIndex,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN GT_U32               L3Offset,
    IN GT_BOOL              checkSimpleHash,
    IN GT_U32               reduceTimeFactor,
    OUT GT_BOOL             *wasIgnoredDueToReduceTimePtr
)
{
    GT_STATUS   rc;
    GT_U8   devNum;/*device number to send packets */
    GT_PHYSICAL_PORT_NUM portNum;/*port number to send packets */
    GT_BOOL isL4,isMpls,isIpv4,isIpv6,isUde;/* flags */
    GT_BOOL srcPortTested;/* is src port tested */
    TGF_PACKET_PAYLOAD_STC *payloadPtr;/* pointer to payload info of packet */
    GT_U32  offset;/* offset of byte from start of packet */
    GT_U32  bitOffset;/*offset of bit from start of byte */
    GT_U32  numBits;/*number of bits in the tested field */
    GT_U32  numBitsInLastByte;/*number of bits in the last byte of tested field */
    GT_U32               burstCount;/* burst count */
    GT_U32               numVfd = 1;/* number of VFDs in vfdArray */
    static TGF_VFD_INFO_STC     vfdArray[1] = {{0}};/* vfd Array -- used for increment the tested bytes , and for vlan tag changing */
    TGF_VFD_INFO_STC     *vrfPtr = &vfdArray[0];
    GT_U32  ii;
    GT_U32  portIter;/* port iterator */
    GT_U32  numberOfPortsSending;/* number of ports sending traffic */
    GT_U32  sendingPortsBmp;/* bitmap of ports that send bursts */
    GT_U32  numVlans;/* number of vlans to test */

    if(wasIgnoredDueToReduceTimePtr)
    {
        *wasIgnoredDueToReduceTimePtr = GT_FALSE;
    }

    if(reduceTimeFactor == 0)
    {
        reduceTimeFactor = 1;
    }

    isL4 = GT_FALSE;

    isMpls = GT_FALSE;
    isIpv4 = GT_FALSE;
    isIpv6 = GT_FALSE;
    isUde = GT_FALSE;
    srcPortTested = GT_FALSE;
    bitOffset = 0;
    numVlans = 1;

    switch(trafficType)
    {
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E      :
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E      :
            isL4 = GT_TRUE;
            isIpv4 = GT_TRUE;
            break;
        case PRV_TGF_PCL_PACKET_TYPE_MPLS_E          :
            isMpls = GT_TRUE;
            break;
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_FRAGMENT_E :
        case PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E    :
            isIpv4 = GT_TRUE;
            break;
        case PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E :
            break;
        case PRV_TGF_PCL_PACKET_TYPE_IPV6_E          :
            isIpv6 = GT_TRUE;
            break;
        case PRV_TGF_PCL_PACKET_TYPE_UDE_E           :
        case PRV_TGF_PCL_PACKET_TYPE_UDE_1_E         :
        case PRV_TGF_PCL_PACKET_TYPE_UDE_2_E         :
        case PRV_TGF_PCL_PACKET_TYPE_UDE_3_E         :
        case PRV_TGF_PCL_PACKET_TYPE_UDE_4_E         :
            isUde = GT_TRUE;

            /* update the ether type field */
            if(trafficType != PRV_TGF_PCL_PACKET_TYPE_UDE_E)
            {
                prvTgfPacketUdeTypePart.etherType = (TGF_ETHER_TYPE)(UDE_BASE_CNS +
                    (trafficType - PRV_TGF_PCL_PACKET_TYPE_UDE_1_E) + 1);
            }
            else
            {
                prvTgfPacketUdeTypePart.etherType = UDE_BASE_CNS;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }

    switch(fieldType)
    {
        case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
            RETURN_IF_FIELD_NOT_RELEVANT_MAC((fieldByteIndex < 2));

            numBits = 16;

            if(isIpv4 == GT_TRUE)
            {
                offset = L3Offset +
                     TGF_IPV4_HEADER_SIZE_CNS;/*ipv4 header*/
            }
            else
            {
                offset = L3Offset +
                         TGF_IPV6_HEADER_SIZE_CNS;/*ipv6 header*/
            }

            if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E)
            {
                offset += 2;
            }

            offset += fieldByteIndex;

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(fieldByteIndex != 1)
                {

                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }

                    /* reduce the time */
                    return GT_OK;
                }
            }

            break;
        case PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E:
            RETURN_IF_FIELD_NOT_RELEVANT_MAC(isIpv6);
            RETURN_IF_FIELD_NOT_RELEVANT_MAC((fieldByteIndex <= 2));

            numBits = 20;

            offset = L3Offset +
                     1;/*start in mid of second byte in ipv6 header */

            offset += fieldByteIndex;

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(fieldByteIndex != 2)
                {
                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }
                    /* reduce the time */
                    return GT_OK;
                }
            }

            break;
        case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
            RETURN_IF_FIELD_NOT_RELEVANT_MAC((isIpv6 || isIpv4));
            RETURN_IF_FIELD_NOT_RELEVANT_MAC((fieldByteIndex < 16));

            offset = L3Offset;

            if(isIpv4 == GT_TRUE)
            {
                /* the Ipv4 bytes start from 12..15 , where : 12 is MSB for Ipv4 */
                RETURN_IF_FIELD_NOT_RELEVANT_MAC((fieldByteIndex >= IPV4_OFFSET_FROM_BYTE_12_CNS));

                numBits = 32;

                offset += 12;
                if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E)
                {
                    offset += 4;
                }

                offset += fieldByteIndex - IPV4_OFFSET_FROM_BYTE_12_CNS;
            }
            else /* ipv6 */
            {
                numBits = 128;

                offset += 8;
                if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E)
                {
                    offset += 16;
                }

                offset += fieldByteIndex;
            }

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(fieldByteIndex != 15)
                {
                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }
                    return GT_OK;
                }
            }

            break;
        case PRV_TGF_TRUNK_FIELD_MAC_ADDR_E:
            numBits = 48;

            offset = 0;
            if(fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E)
            {
                offset += 6;
            }
            offset += fieldByteIndex;

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(fieldByteIndex != 5)
                {
                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }
                    /* reduce the time */
                    return GT_OK;
                }
            }
            break;
        case PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E:
            RETURN_IF_FIELD_NOT_RELEVANT_MAC(isMpls);
            RETURN_IF_FIELD_NOT_RELEVANT_MAC((fieldByteIndex <= 2));

            numBits = 20;/* field of 20 bits , 4 bits in last byte */
            bitOffset = 4;

            offset = L3Offset;

            switch(fieldSubType)
            {
                case 0:
                case 1:
                case 2:
                    break;
                default:
                    return GT_BAD_PARAM;

            }

            offset += 4 * fieldSubType; /* 4 bytes offset for each label */

            offset += fieldByteIndex;

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(fieldByteIndex != 2)
                {
                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }
                    /* reduce the time */
                    return GT_OK;
                }
            }
            break;
        case PRV_TGF_TRUNK_FIELD_LOCAL_SRC_PORT_E:
            srcPortTested = GT_TRUE;
            numBits = 6;
            offset = 0;/* use the mac DA field */
            break;

        case PRV_TGF_TRUNK_FIELD_UDBS_E:

            if(getSendingInfo == 0 && (forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                if(isUde == GT_FALSE || fieldByteIndex != 3)
                {
                    if(wasIgnoredDueToReduceTimePtr)
                    {
                        *wasIgnoredDueToReduceTimePtr = GT_TRUE;
                    }
                    /* reduce the time */
                    return GT_OK;
                }
            }

            if(trafficPtr->numOfParts < 1)
            {
                /*prevent access violation*/
                return GT_BAD_PARAM;
            }

            if(TGF_PACKET_PART_PAYLOAD_E != trafficPtr->partsArray[trafficPtr->numOfParts - 1].type)
            {
                /* assume payload is the last part of the packet */
                return GT_BAD_PARAM;
            }

            numBits = 8;

            /* pointer to the payload of the packet */
            payloadPtr = trafficPtr->partsArray[trafficPtr->numOfParts - 1].partPtr;
            /* calculate the number of bytes from the start of the packet */
            offset = trafficPtr->totalLen - payloadPtr->dataLength;

            rc = prvTgfPclUserDefinedByteSet(
                0/* not relevant */, trafficType,
                CPSS_PCL_DIRECTION_INGRESS_E,
                fieldByteIndex, PRV_TGF_PCL_OFFSET_L2_E,
                (GT_U8)offset);
            if(rc != GT_OK)
            {
                if(debugMode)
                {
                    PRV_UTF_LOG0_DEBUG_MAC(" debug ERROR ---> prvTgfPclUserDefinedByteSet failed \n");
                }
                return rc;
            }

            break;

        default:
            return GT_BAD_PARAM;
    }

    numBitsInLastByte = numBits & 0x7;

    if(fieldType == PRV_TGF_TRUNK_FIELD_MAC_ADDR_E &&
       fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E)
    {
        /* according to the FDB learned MACs */
        burstCount = FDB_MAC_COUNT_CNS;
    }
    else
    {
        burstCount = CRC_HASH_BURST_COUNT_NEW_CNS;
    }

    if((prvUtfIsGmCompilation() == GT_TRUE) || (GT_FALSE != prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E)))
    {   /* reduce to 64 packets */
        if(burstCount > 64)
        {
            burstCount = 64;
        }
    }


    vrfPtr->modeExtraInfo = 0;
    vrfPtr->offset = offset;
    cpssOsMemSet(vrfPtr->patternPtr,0,sizeof(TGF_MAC_ADDR));
    vrfPtr->patternPtr[0] = 0;
    vrfPtr->cycleCount = 1;/*single byte*/
    if(bitOffset == 0)
    {
        vrfPtr->mode = TGF_VFD_MODE_INCREMENT_E;

        if(numBitsInLastByte && (((numBits - 1) / 8) == fieldByteIndex))
        {
            /* we do last byte */
            burstCount = 1 << numBitsInLastByte;
        }
    }
    else
    {
        vrfPtr->mode = TGF_VFD_MODE_INCREMENT_VALUE_E;
        vrfPtr->incValue = (1 << bitOffset);

        if(fieldByteIndex != 0 && vrfPtr->offset != 0)
        {
            /* due to usage of 2 bytes set offset as previous byte */
            vrfPtr->offset--;
            vrfPtr->cycleCount = 2;/* 2 bytes */
        }
        else
        {
            burstCount = 1 << (8 - bitOffset) ;
        }
    }

    if(getSendingInfo)
    {
        /*we only need to get info about the sending without doing the sending */
        sendingInfo.vfdArray[0] = *vrfPtr;

        /* reset the flag so you will not forget to leave it ON */
        getSendingInfo = 0;
        return GT_OK;
    }

    if(burstCount > CRC_HASH_BURST_COUNT_NEW_CNS)
    {
        burstCount = CRC_HASH_BURST_COUNT_NEW_CNS;
    }

    if(debug_burstNum)
    {
        burstCount = debug_burstNum;
    }

    if(srcPortTested == GT_TRUE)
    {
        /* 4 ports will send the traffic */
        numberOfPortsSending = NOT_IN_TRUNK_PORTS_NUM_CNS;
        burstCount = 1;
    }
    else
    {
        /* single port will sent the traffic */
        numberOfPortsSending = 1;
    }


    if(checkSimpleHash == GT_TRUE &&
       burstCount > 64)
    {
        burstCount = 64;
    }

    if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
    {
        if(wasIgnoredDueToReduceTimePtr)
        {
            *wasIgnoredDueToReduceTimePtr = GT_TRUE;
        }

        /* only every 50 times that we get here for GM , we will actually check the permutation ! */
        REDUCE_RUNTIME_BY_FACTOR_WITH_RETURN_GT_OK_MAC(reduceTimeFactor);

        if(wasIgnoredDueToReduceTimePtr)
        {
            *wasIgnoredDueToReduceTimePtr = GT_FALSE;
        }

#if 0 /* function to debug behavior of macro REDUCE_RUNTIME_BY_FACTOR_WITH_RETURN_GT_OK_MAC */
        if(GT_FALSE == reduceRuntimeByFactorWithReturn(reduceTimeFactor))
        {
            /* skip this permutation */
            return GT_OK;
        }
#endif
    }


    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        cpssOsPrintf("Start Send [%d] packets , for[%s] [%s] [%s] fieldByteIndex[%d] , isL4=[%d],isMpls=[%d],isIpv4=[%d],isIpv6=[%d],isUde=[%d],srcPortTested=[%d] \n",
            burstCount,
            TO_STRING_PACKET_TYPE_MAC(trafficType),
            TO_STRING_HASH_MASK_FIELD_TYPE_MAC(fieldType),
            ((fieldSubType == PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E) ? "destination" : "source") ,
            fieldByteIndex,
            isL4,isMpls,isIpv4,isIpv6,isUde,srcPortTested
            );
    }

    if(use_calculatedHash_expectedEgressPortNum == GT_TRUE)
    {
        numberOfPortsSending = 1;
        numVlans = 1;
        /* state the engine to send only single packet */
        /*tgfTrafficGeneratorSendOnlyLastPacketSet(GT_TRUE);*/
        burstCount = 1;
        numVfd = 0;

        /* do calc of calculatedHash_expectedTrunkMemberIndex */
        do_calculatedHash_expectedTrunkMemberIndex(trafficPtr,trafficType,
            fieldType == PRV_TGF_TRUNK_FIELD_UDBS_E ? GT_TRUE : GT_FALSE,
            fieldByteIndex,
            prvTgfPortsArray[0],
            0/*numTrunkMembers -- 0 means use default '4' */);
    }

#ifdef ASIC_SIMULATION
    if(debug_withSimulationLog)
    {
        startSimulationLog();
    }
#endif

    for(portIter = 0 ; portIter < numberOfPortsSending; portIter++)
    {
        devNum  = prvTgfDevsArray [portIter];
        portNum = prvTgfPortsArray[portIter];

        for(ii = 0 ; ii < numVlans ; ii++)
        {
            /* modify the vid part */
            prvTgfPacketVlanTagPart.vid = (TGF_VLAN_ID)prvTgfVlansArr[ii];

            vrfPtr->modeExtraInfo = 0;
            /* send the burst of packets with incremental byte */
            prvTgfTrunkTestPacketSend(devNum, portNum, trafficPtr ,burstCount ,numVfd ,vfdArray);
        }
    }

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        cpssOsPrintf("End sending start check LBH \n");
    }

    if(use_calculatedHash_expectedEgressPortNum == GT_TRUE)
    {
        GT_U32              membersIndexesBmp =
            1 << (NOT_IN_TRUNK_PORTS_NUM_CNS + calculatedHash_expectedTrunkMemberIndex);

        /* prepare for the trunkSendCheckLbh(...) */
        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E;
        prvTgfTrunkLoadBalanceSpecificMembersSet(membersIndexesBmp);

        burstCount = 1;
    }

    sendingPortsBmp = (1 << numberOfPortsSending) - 1;
    /* check the LBH and no flooding */
    trunkSendCheckLbh(GT_TRUE,sendingPortsBmp,burstCount * numVlans ,lbhMode,NULL);

    return GT_OK;
}


/**
* @internal prvTgfTrunkCrcHashTest function
* @endinternal
*
* @brief   check CRC hash feature.
*         applicable devices: Lion and above
* @param[in] L3L4ParsingOverMplsEnabled - check CRC hash L3L4 for traffic over MPLS.
*                                      or test the CRC hash for all traffic types.
*                                      GT_TRUE - tests CRC for L3L4 traffic over MPLS.
*                                      GT_FALSE - test CRC for all traffic types (not include L3L4 over MPLS)
*                                       None
*/
void prvTgfTrunkCrcHashTest
(
    IN GT_BOOL          L3L4ParsingOverMplsEnabled
)
{
    GT_STATUS   rc;
    PRV_TGF_TRUNK_LBH_CRC_MODE_ENT  crcMode;
    GT_U32              seedIndex,numSeeds,currentSeed;
    PEARSON_TEST_MODE_ENT   pearsonMode,pearsonModeLast;
    GT_U32              hashMaskTableIndex;/* index of entry in hash mask table - 0..27 */
    PRV_TGF_TRUNK_FIELD_TYPE_ENT hashMaskFieldType;/* index of field in the 70 bytes for hash - 0..69 */
    GT_U32              hashMaskFieldSubType,hashMaskFieldSubTypeStart,hashMaskFieldSubTypeEnd;
    GT_U32              hashMaskFieldByteIndex,hashMaskFieldByteIndexStart,hashMaskFieldByteIndexEnd;
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;/*traffic type*/
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;/* LBH mode expected */
    GT_U32              hashMaskTableIndexFirst,hashMaskTableIndexLast;/* first,last index for hashMaskTableIndex */
    GT_BOOL             isDebugFlagImplicitlySet = GT_FALSE;

    if(L3L4ParsingOverMplsEnabled == GT_TRUE)
    {
        utfGeneralStateMessageSave(10,"L3L4ParsingOverMplsEnable = %d",GT_TRUE);
        /* set <L3L4ParsingOverMplsEnable> = GT_TRUE */
        rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");

        if(rc == GT_OK)
        {
            /* check when L3L4ParsingOverMplsEnabled = enabled */
            mplsWithIpPassengerLbhSendAndCheck(&prvTgfMplsIpvxPassengerPacketInfo ,
                GT_TRUE/*L3L4ParsingOverMplsEnabled - enabled */);

            utfGeneralStateMessageSave(10,"L3L4ParsingOverMplsEnable = %d",GT_FALSE);

            rc = prvTgfPclL3L4ParsingOverMplsEnableSet(GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclL3L4ParsingOverMplsEnableSet: failed \n");
        }

        /* check when L3L4ParsingOverMplsEnabled = disabled  */
        mplsWithIpPassengerLbhSendAndCheck(&prvTgfMplsIpvxPassengerPacketInfo ,
            GT_FALSE/*L3L4ParsingOverMplsEnabled - disabled */);

        return;
    }

#if 0
    if((forceCrcRunAllPermutations == 0) && ((prvUtfIsGmCompilation() == GT_TRUE) || (GT_FALSE != prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E))))
    {
        isDebugFlagImplicitlySet = GT_TRUE;

        hashMaskTableIndex = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;
        hashMaskFieldType = 0;
        hashMaskFieldSubType = 0;
        hashMaskFieldByteIndex = 0;
        pearsonMode = 0;
        crcMode = PRV_TGF_TRUNK_LBH_CRC_6_MODE_E;
        seedIndex = 0;

        /*reduce the number of iterations*/
        tgfTrunkDebugCrcHash(hashMaskTableIndex,hashMaskFieldType,
            hashMaskFieldSubType,hashMaskFieldByteIndex,
            pearsonMode,crcMode,seedIndex);
    }
#endif

    if(debugMode)
    {
        PRV_UTF_LOG1_DEBUG_MAC("debug_hashMaskTableIndex = %d \n",debug_hashMaskTableIndex);
        PRV_UTF_LOG2_DEBUG_MAC("debug_hashMaskFieldType = %d %s \n",debug_hashMaskFieldType,
                               TO_STRING_HASH_MASK_FIELD_TYPE_MAC(debug_hashMaskFieldType));
        switch(debug_hashMaskFieldType)
        {
            case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
            case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
            case PRV_TGF_TRUNK_FIELD_MAC_ADDR_E:
                PRV_UTF_LOG2_DEBUG_MAC("debug_hashMaskFieldSubType = %d %s \n",debug_hashMaskFieldSubType,
                                       TO_STRING_HASH_MASK_FIELD_SUB_TYPE_MAC(debug_hashMaskFieldSubType));
                break;
            default:
                PRV_UTF_LOG1_DEBUG_MAC("debug_hashMaskFieldSubType = %d \n",debug_hashMaskFieldSubType);
                break;
        }
        PRV_UTF_LOG1_DEBUG_MAC("debug_hashMaskFieldByteIndex = %d \n",debug_hashMaskFieldByteIndex);
        PRV_UTF_LOG2_DEBUG_MAC("debug_pearsonMode = %d %s\n",debug_pearsonMode,
                         TO_STRING_PEARSONE_MODE_MAC(debug_pearsonMode));

        PRV_UTF_LOG2_DEBUG_MAC("debug_crcMode = %d , %s \n",debug_crcMode,
                         TO_STRING_CRC_MODE_MAC(debug_crcMode));

        PRV_UTF_LOG1_DEBUG_MAC("debug_seedIndex = %d \n",debug_seedIndex);
    }

    hashMaskTableIndexLast = PRV_TGF_TRUNK_HASH_MASK_TABLE_SIZE_CNS;
    hashMaskTableIndexFirst = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;

    for(crcMode = UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) ?
                  PRV_TGF_TRUNK_LBH_CRC_32_MODE_E  :
                  PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E;
        crcMode >= PRV_TGF_TRUNK_LBH_CRC_6_MODE_E;
        crcMode--)
    {/* level 1 */

        if(use_calculatedHash_expectedEgressPortNum == GT_TRUE &&
           crcMode != PRV_TGF_TRUNK_LBH_CRC_32_MODE_E)
        {
            /* check only CRC_32 mode for calculation */
            continue;
        }

        if(debugMode)
        {
            crcMode = debug_crcMode;
        }

        if((forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
        {
            /* to make single iteration on modes */
            crcMode = PRV_TGF_TRUNK_LBH_CRC_6_MODE_E;
        }

        if(crcMode == PRV_TGF_TRUNK_LBH_CRC_6_MODE_E)
        {
            numSeeds = SEED_CRC_6_NUM_CNS;/* the last 2 values are redundant for crc_6 */
            pearsonModeLast = 1;
        }
        else
        {
            numSeeds = SEED_CRC_16_NUM_CNS;
            pearsonModeLast = PEARSON_TEST_MODE_LAST_E;
        }

        if(use_calculatedHash_expectedEgressPortNum == GT_TRUE)
        {
            numSeeds = 1;
            pearsonModeLast = 1;
        }


#ifdef FORCE_REDUCED_TEST_MAC
        {
            /* for debug */
            CPSS_TBD_BOOKMARK   /*--> to be removed when stable */
            numSeeds = 1;
            pearsonModeLast = 1;
        }
#endif /*FORCE_REDUCED_TEST_MAC*/

        for(seedIndex = 0 ;
            seedIndex < numSeeds ;
            seedIndex++)
        {/* level 2 */
            if(debugMode)
            {
                seedIndex = debug_seedIndex;
            }

            if((forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
            {
                /* to make single iteration on seeds */
                seedIndex = numSeeds -1;
            }

            currentSeed = crcSeeds[seedIndex];

            /* set new mode and seed */
            TRUNK_PRV_UTF_LOG0_MAC("=======  : set CRC sub-mode and seed =======\n");
            rc = prvTgfTrunkHashCrcParametersSet(crcMode,currentSeed);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            TRUNK_PRV_UTF_LOG0_MAC("=======  : clear mask hash CRC table =======\n");
            /* clear full table */
            rc = trunkHashMaskCrcTableEntryClear(PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            for(hashMaskTableIndex = hashMaskTableIndexFirst,
                trafficType = 0;
                hashMaskTableIndex < hashMaskTableIndexLast;
                hashMaskTableIndex++,trafficType++)
            {/* level 3 */
                for(hashMaskFieldType = 0 ;
                    hashMaskFieldType < PRV_TGF_TRUNK_FIELD_LAST_E;
                    hashMaskFieldType++)
                {/* level 4 */
                    hashMaskFieldSubTypeStart = 0;
                    hashMaskFieldSubTypeEnd   = 0;

                    hashMaskFieldByteIndexStart = 0;
                    hashMaskFieldByteIndexEnd = 1;

                    switch(hashMaskFieldType)
                    {
                        case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
                            hashMaskFieldSubTypeStart = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
                            hashMaskFieldSubTypeEnd   = PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E;
                            hashMaskFieldByteIndexEnd = 2; /* 2 bytes */
                            if(trafficType != PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E &&
                               trafficType != PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E )
                            {
                                /* TBD --> need to allow IPv6 check with L4 fields !
                                    --> checked by the 'L3L4 MPLS parsing passenger' */
                                continue;
                            }
                            break;
                        case PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E:
                                if(trafficType != PRV_TGF_PCL_PACKET_TYPE_IPV6_E)
                                {
                                    continue;
                                }
                            hashMaskFieldByteIndexEnd = 3;
                            break;
                        case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
                            hashMaskFieldSubTypeStart = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
                            hashMaskFieldSubTypeEnd   = PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E;
                            hashMaskFieldByteIndexEnd = 16;/* 16 bytes */
                            break;
                        case PRV_TGF_TRUNK_FIELD_MAC_ADDR_E:
                            hashMaskFieldSubTypeStart = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
                            hashMaskFieldSubTypeEnd   = PRV_TGF_TRUNK_FIELD_DIRECTION_DESTINATION_E;
                            hashMaskFieldByteIndexEnd = 6;/* 6 bytes */
                            break;
                        case PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E:
                            hashMaskFieldSubTypeEnd = 2;  /* labels 0,1,2 */
                            hashMaskFieldByteIndexEnd = 3;/* 3 bytes */
                            break;
                        case PRV_TGF_TRUNK_FIELD_UDBS_E:
                            hashMaskFieldByteIndexStart = 14; /* start UDB 14 */
                            hashMaskFieldByteIndexEnd = 23;  /* end UDB 22 */
                            break;
                        default:
                            break;
                    }

                    for(hashMaskFieldSubType = hashMaskFieldSubTypeStart;
                        hashMaskFieldSubType <= hashMaskFieldSubTypeEnd;
                        hashMaskFieldSubType++)
                    {/* level 5 */

                        for(hashMaskFieldByteIndex = hashMaskFieldByteIndexStart;
                            hashMaskFieldByteIndex < hashMaskFieldByteIndexEnd;
                            hashMaskFieldByteIndex++)
                        {/* level 6 */

                            if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_MAC_ADDR_E &&
                               hashMaskFieldByteIndex == 0)
                            {
                                /*For DA :  this is the MSB of the DA , and we not set
                                   learning on this bytes because half of them
                                   are 'MCAST' */

                                /* For SA : this is the MSB of the SA , and
                                   the device not bridge SA which is 'MCAST' */

                                continue;
                            }

                            if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IP_ADDR_E &&
                               hashMaskFieldByteIndex == 0)
                            {
                                /*For DIP :  this is the MSB of the DIP
                                    value >= 224 is IPMC (but the DA is not matching) */

                                /* For SIP : this is the MSB of the SIP,
                                    value >= 224 is IPMC (not allowed) */

                                continue;
                            }

                            if(debugMode)
                            {
                                hashMaskTableIndex      = debug_hashMaskTableIndex;
                                hashMaskFieldType       = debug_hashMaskFieldType;
                                hashMaskFieldSubType    = debug_hashMaskFieldSubType;
                                hashMaskFieldByteIndex  = debug_hashMaskFieldByteIndex;
                                if(debug_hashMaskTableIndex >= hashMaskTableIndexFirst)
                                {
                                    trafficType = debug_hashMaskTableIndex -
                                                hashMaskTableIndexFirst;
                                }
                                else
                                {
                                    trafficType = 0;
                                }
                            }

                            rc = trunkHashMaskCrcFieldSet(
                                    hashMaskTableIndex,
                                    hashMaskFieldType,
                                    hashMaskFieldSubType,
                                    hashMaskFieldByteIndex);
                            if(rc != GT_OK)
                            {
                                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                break;
                            }

                            for(pearsonMode = 0 ;
                                pearsonMode < pearsonModeLast ;
                                pearsonMode++)
                            {/* level 7 */
                                if(debugMode)
                                {
                                    pearsonMode = debug_pearsonMode;
                                }

                                if((forceCrcRunAllPermutations == 0) && prvUtfIsGmCompilation() == GT_TRUE)
                                {
                                    /* to make single iteration on modes */
                                    pearsonMode = pearsonModeLast -1;
                                }

                                /* expect good LBH */
                                lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;

                                if(crcMode == PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E)
                                {
                                    switch(pearsonMode)
                                    {
                                        case PEARSON_TEST_MODE_0_E:
                                            /* no LBH expected */
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
                                            break;
                                        case PEARSON_TEST_MODE_EVEN_INDEX_E:
                                            /* LBH only on %<numTrunkMemebers> = 0 or 2 members of the trunk */
                                            /* because we use 4 members in the trunk then
                                               only member in index 0,2 will get traffic while
                                               members in index 1,3 will not get it */
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E;
                                            break;
                                        default:
                                            break;
                                    }

                                    /*****************/
                                    /* special cases */
                                    /*****************/
                                    if(lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E)
                                    {
                                        /* forced to single port */
                                    }
                                    else if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_LOCAL_SRC_PORT_E)
                                    {
                                        /* it seems that the Pearson hash will
                                           result ONLY 2 values and both of them
                                           with same %4

                                           so because our trunk with 4 members only
                                           single port receive the traffic !
                                        */
                                        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                        prvTgfTrunkLoadBalanceLimitedNumSet(prvTgfLbhSelectedPortsNum);

                                    }
                                    else if (hashMaskFieldType == PRV_TGF_TRUNK_FIELD_UDBS_E &&
                                             hashMaskFieldByteIndex == 15)
                                    {
                                        /* it seems that the hash goes to only
                                           2 ports !

                                           NOTE: other UDBs (14,16..22) are ok !
                                        */

                                        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                        prvTgfTrunkLoadBalanceLimitedNumSet(2);
                                    }
                                    else if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E &&
                                       hashMaskFieldSubType == 2 && /* label 2 */
                                       hashMaskFieldByteIndex == 2)/* byte 2 */
                                    {
                                        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                                        {
                                            /* it seems that the hash goes to only
                                               2 ports !

                                               NOTE: other labels (0,2) are ok !
                                            */
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                            prvTgfTrunkLoadBalanceLimitedNumSet(2);

                                            if(pearsonMode == PEARSON_TEST_MODE_EVEN_INDEX_E)
                                            {
                                                /* it seems that the 'even values in the Pearson'
                                                   combined with the fact that only 2 ports will
                                                   gets be reduced to only 1 port in the trunk !!!
                                                */
                                                prvTgfTrunkLoadBalanceLimitedNumSet(1);
                                            }
                                        }
                                        else
                                        {
                                            /* no issue here ... */
                                        }
                                    }
                                    else if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_MPLS_LABEL_E &&
                                       hashMaskFieldSubType == 0 && /* label 0 */
                                       hashMaskFieldByteIndex == 0)/* byte 0 */
                                    {
                                        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                                        {
                                            /* new LBH in SIP5 so this issue is gone ... */
                                        }
                                        else
                                        {
                                            /* it seems that the hash goes to only
                                               2 ports !

                                               NOTE: other labels (0,2) are ok !
                                            */
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                            prvTgfTrunkLoadBalanceLimitedNumSet(2);

                                            if(pearsonMode == PEARSON_TEST_MODE_EVEN_INDEX_E)
                                            {
                                                /* it seems that the 'even values in the Pearson'
                                                   combined with the fact that only 2 ports will
                                                   gets be reduced to only 1 port in the trunk !!!
                                                */
                                                prvTgfTrunkLoadBalanceLimitedNumSet(1);
                                            }
                                        }

                                    }
                                    else if(hashMaskFieldType == PRV_TGF_TRUNK_FIELD_IPV6_FLOW_E &&
                                        hashMaskFieldByteIndex == 2 &&
                                        lbhMode == PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E)
                                    {
                                        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
                                        {
                                            /* new LBH in SIP5 so this issue is gone ... */
                                        }
                                        else
                                        {
                                            /* the 4 LSB (in third word) don't give good LBH -->
                                                only 2 ports from 4 do LBH
                                                in simulation and in HW.
                                            */
                                            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E;
                                            prvTgfTrunkLoadBalanceLimitedNumSet(2);
                                        }
                                    }
                                }

                                {
                                    utfGeneralStateMessageSave(0,"hashMaskTableIndex = %d",hashMaskTableIndex);
                                    utfGeneralStateMessageSave(1,"hashMaskFieldType = %d %s ",hashMaskFieldType,
                                        TO_STRING_HASH_MASK_FIELD_TYPE_MAC(hashMaskFieldType));
                                    switch(hashMaskFieldType)
                                    {
                                        case PRV_TGF_TRUNK_FIELD_L4_PORT_E:
                                        case PRV_TGF_TRUNK_FIELD_IP_ADDR_E:
                                        case PRV_TGF_TRUNK_FIELD_MAC_ADDR_E:
                                            utfGeneralStateMessageSave(2,"hashMaskFieldSubType = %d %s ",hashMaskFieldSubType,
                                                TO_STRING_HASH_MASK_FIELD_SUB_TYPE_MAC(hashMaskFieldSubType));
                                        break;
                                        default:
                                            utfGeneralStateMessageSave(2,"hashMaskFieldSubType = %d ",hashMaskFieldSubType);
                                        break;
                                    }
                                    utfGeneralStateMessageSave(3,"hashMaskFieldByteIndex = %d ",hashMaskFieldByteIndex);
                                    utfGeneralStateMessageSave(4,"pearsonMode = %d %s ",pearsonMode,
                                        TO_STRING_PEARSONE_MODE_MAC(pearsonMode));

                                    utfGeneralStateMessageSave(5,"crcMode = %d , %s ",crcMode,
                                        TO_STRING_CRC_MODE_MAC(crcMode));

                                    utfGeneralStateMessageSave(6,"seedIndex = %d ",seedIndex);
                                    utfGeneralStateMessageSave(7,"lbhMode = %d %s",lbhMode,
                                        TO_STRING_LBH_MODE_MAC(lbhMode)
                                        );
                                }

                                /* set Pearson table */
                                rc = trunkHashPearsonFill(pearsonMode);
                                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                if(rc != GT_OK)
                                {
                                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                    break;
                                }

                                /* send traffic according to field checking */
                                rc = trunkHashCrcLbhTrafficSend(
                                        prvTgfTrunkTrafficTypes[trafficType],
                                        trafficType,
                                        hashMaskFieldType,
                                        hashMaskFieldSubType,
                                        hashMaskFieldByteIndex,
                                        lbhMode,
                                        L2_VLAN_TAG_ETHERETYPE_SIZE_CNS,
                                        GT_FALSE,
                                        50,NULL);
                                if(rc != GT_OK)
                                {
                                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                                    break;
                                }


#ifdef FORCE_REDUCED_TEST_MAC
                                if(crcMode == PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E)
                                {
                                    /* for debug */
                                    CPSS_TBD_BOOKMARK   /*--> to be removed when stable */
                                    break;
                                }
#endif /*FORCE_REDUCED_TEST_MAC*/

                                IF_DEBUG_MODE_BREAK_MAC(debugMode);
                            }/* level 7 */
                            IF_DEBUG_MODE_BREAK_MAC(debugMode);
                        }/* level 6 */
                        IF_DEBUG_MODE_BREAK_MAC(debugMode);
                    }/* level 5 */
                    IF_DEBUG_MODE_BREAK_MAC(debugMode);
                }/* level 4 */
                IF_DEBUG_MODE_BREAK_MAC(debugMode);
            }/* level 3 */
            IF_DEBUG_MODE_BREAK_MAC(debugMode);
        }/* level 2 */
        IF_DEBUG_MODE_BREAK_MAC(debugMode);
    }/* level 1 */

    if(isDebugFlagImplicitlySet == GT_TRUE)
    {
        /* restore 'no debug' */
        debugMode = 0;
    }

}

/**
* @internal findPortInArr function
* @endinternal
*
* @brief   find the (dev,port) in the arrays of prvTgfDevsArray[],prvTgfPortsArray[]
*
* @param[in] dev                      - the device
* @param[in] port                     - the port
*
* @param[out] indexPtr                 - (pointer to) the index in the arrays of prvTgfDevsArray[],prvTgfPortsArray[]
*
* @retval GT_OK                    - found
* @retval GT_NOT_FOUND             - not found
*/
static GT_STATUS findPortInArr
(
    IN GT_U32    dev ,
    IN GT_U32    port ,
    OUT GT_U32   *indexPtr
)
{
    GT_U32  portIter;

    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        if(port == prvTgfPortsArray[portIter] &&
           dev  == prvTgfDevsArray[portIter])
        {
            /* port found */
            *indexPtr = portIter;
            return GT_OK;
        }
    }

    *indexPtr = 0;

    /* the added port not found ?! */
    return GT_NOT_FOUND;
}

static GT_U32   needed_wa = 0;
/**
* @internal associateIngressPortWithEPortInsteadOfTrunkId_fullTrunk function
* @endinternal
*
* @brief   in 'sorting mode' the functions prvTgfTrunkMemberAdd() / prvTgfTrunkMemberRemove() / prvTgfTrunkMembersSet(),
*         set trunkId to the ingress port.
*         this function will make it 0 again.
*         NOTE: it is bad in terms of 'Interval time' but this is just 'patch' for tests.
*         relevant when globalEPortRepresentTrunk_sortingEnabled == GT_TRUE
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X.
* @param[in] notNeededTrunkId         - the trunk Id that need to detach from the port in index portIter
*                                       None
*/
static void associateIngressPortWithEPortInsteadOfTrunkId_fullTrunk
(
    IN GT_TRUNK_ID          notNeededTrunkId
)
{
    if(needed_wa)
    {
        GT_STATUS   rc;
        GT_U8  devNum = prvTgfDevNum;
        static CPSS_TRUNK_MEMBER_STC   membersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
        GT_U32  ii;
        GT_U32  numOfMembers,numOfEnabledMembers,numOfDisabledMembers;

        /* check sort of members */

        numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;

        /*get enabled members */
        rc = prvCpssGenericTrunkDbEnabledMembersGet(devNum,notNeededTrunkId,
            &numOfEnabledMembers,&membersArray[0]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        numOfDisabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS - numOfEnabledMembers;

        /*get disabled members */
        rc = prvCpssGenericTrunkDbEnabledMembersGet(devNum,notNeededTrunkId,
            &numOfDisabledMembers,&membersArray[numOfEnabledMembers]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        numOfMembers = numOfEnabledMembers + numOfDisabledMembers;

        /* convert HW device id to SW number */
        for(ii = 0 ; ii < numOfMembers; ii++)
        {
            rc = prvUtfSwFromHwDeviceNumberGet(membersArray[ii].hwDevice,&membersArray[ii].hwDevice);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if(devNum == membersArray[ii].hwDevice)

            /* detach the ingress port from the trunkId */
            rc = prvTgfTrunkPortTrunkIdSet(devNum, membersArray[ii].port, GT_FALSE , 0);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    return;
}

/**
* @internal associateIngressPortWithEPortInsteadOfTrunkId function
* @endinternal
*
* @brief   the function prvTgfTrunkMemberAdd() / prvTgfTrunkMembersSet(),
*         set the ingress port associated with the trunkId ,
*         but we need it to be associated with the 'global eport' which represents the trunk
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon, Aldrin, AC3X.
* @param[in] portIter                 - port iterator in prvTgfDevsArray[],prvTgfPortsArray[]
* @param[in] notNeededTrunkId         - the trunk Id that need to detach from the port in index portIter
*                                       None
*/
static void associateIngressPortWithEPortInsteadOfTrunkId
(
    IN GT_U32               portIter,
    IN GT_TRUNK_ID          notNeededTrunkId
)
{
    GT_STATUS   rc;
    GT_PORT_NUM          newEPort;
    GT_U32               device;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_INTERFACE_INFO_STC          physicalInfo;

    device  = prvTgfDevsArray [portIter];

    if(device != prvTgfDevNum)
    {
        return;
    }

    newEPort = DEFAULT_SOURCE_EPORT_FROM_TRUNK_ID_MAC(notNeededTrunkId);
    portNum = prvTgfPortsArray[portIter];

    if(globalEPortRepresentTrunk_e2phyReady == GT_FALSE)
    {
        /* set the E2PHY to point to the trunk */
        globalEPortRepresentTrunk_e2phyReady = GT_TRUE;

        physicalInfo.type = CPSS_INTERFACE_TRUNK_E;
        physicalInfo.trunkId = notNeededTrunkId;

        TRUNK_PRV_UTF_LOG2_MAC("Set E2PHY global EPort[%d] to point trunkId[%d] \n",
            newEPort , notNeededTrunkId);

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,newEPort,
                                                               &physicalInfo);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }

    TRUNK_PRV_UTF_LOG3_MAC("Use global EPort[%d] instead of trunkId[%d] for ingress physical port[%d] \n",
        newEPort , notNeededTrunkId , portNum);

    /* detach the ingress port from the trunkId */
    rc = prvTgfTrunkPortTrunkIdSet(prvTgfDevNum, portNum, GT_FALSE , 0);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(0 == defaultEPortWasChanged[portIter])
    {
        /* save the default EPort of the ingress port */
        rc = prvTgfCfgPortDefaultSourceEportNumberGet(prvTgfDevNum,portNum,&defaultEPort[portIter]);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    /* modify the default EPort of the ingress port */
    rc = prvTgfCfgPortDefaultSourceEportNumberSet(prvTgfDevNum,portNum,newEPort);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    defaultEPortWasChanged[portIter] = 1;

    TRUNK_PRV_UTF_LOG2_MAC("Use global EPort[%d] instead of trunkId[%d] \n",
        newEPort , notNeededTrunkId);

    if(globalEPortRepresentTrunk_sortingEnabled == GT_TRUE)
    {
        associateIngressPortWithEPortInsteadOfTrunkId_fullTrunk(notNeededTrunkId);
    }

}




/**
* @internal prvTgfTrunkGenericInit function
* @endinternal
*
* @brief   generic init for trunk tests
*         applicable devices: All DxCh,Puma devices
* @param[in] testType                 - test type
* @param[in] cascadeTrunk             - do we create cascade trunk / regular trunk
* @param[in] numOfPortsInTest         - number of ports in test.
*                                      set first 4 outside a trunk , set others as trunk members.
*                                       None
*/
static void prvTgfTrunkGenericInit
(
    IN TRUNK_TEST_TYPE_ENT testType,
    IN GT_BOOL cascadeTrunk,
    IN GT_U32   numOfPortsInTest
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj,kk;
    GT_U32  portIter;/* port iterator */
    CPSS_TRUNK_MEMBER_STC member;
    GT_TRUNK_ID             trunkId;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    CPSS_PORTS_BMP_STC  portsBmp;
    GT_U32              membersIndexesBmp = 0;
    GT_BOOL             isPortConfigForced = GT_FALSE;
    GT_PHYSICAL_PORT_NUM    physicalPort;
    static          int initDone = 0;
    static CPSS_TRUNK_WEIGHTED_MEMBER_STC  weightedMembers[64];
    GT_U32          *weightedPortsArray;
    PRV_TGF_CFG_GLOBAL_EPORT_STC    ecmpRange;
    PRV_TGF_CFG_GLOBAL_EPORT_STC    globalRange;
    GT_U32  allowHighPorts = 0;
    GT_BOOL mustBeAscending = GT_FALSE;

    if(prvUtfIsDoublePhysicalPortsModeUsed())
    {
        for(portIter = 0 ; portIter < TRUNK_MAX_PORTS_IN_TEST_CNS ; portIter++)
        {
            testPorts_bobcat3[portIter] |= BIT_8;/* align to port numbers of cpssInitSystem */
        }

        for(portIter = 0 ; portIter < TRUNK_8_PORTS_IN_TEST_CNS ; portIter++)
        {
            trunkA_MembersArray_bobcat3[portIter].port |= BIT_8;/* align to port numbers of cpssInitSystem */
        }
    }

    {
        GT_U32 value;
        if((appDemoDbEntryGet("all_trunks_fixed_size", &value) == GT_OK) && (value != 0))
        {
            all_trunks_fixed_size = value;
            /* all trunk with explicit given 'value' of max members */
            /* NOTE: because we use 4 members in the tests that use this value
                the CPSS fill it up to 4 times */
            if((value / 4) > 4)
            {
                value = 16;
            }
            else
            {
                /* remove integer leftovers */
                value = (value / 4) * 4;
            }

            calculatedHash_numOfmembers = value;
        }
        else
        {
            calculatedHash_numOfmembers = 8;/* BWC */
        }
    }

    initTrunkIds(cascadeTrunk);

    hashMaskTableSize = PRV_TGF_TRUNK_HASH_MASK_TABLE_SIZE_CNS;

    if(initDone == 0)
    {
        initDone = 1;

        portIter = NOT_IN_TRUNK_PORTS_NUM_CNS;
        testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[portIter] =
        testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[portIter] =
            PORT_OF_CASCADE_TRUNK_LION_A_WITH_OFFSET_MAC(3);
            portIter++;
        testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[portIter] =
        testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[portIter] =
            PORT_OF_CASCADE_TRUNK_LION_A_WITH_OFFSET_MAC(4);
            portIter++;
        testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[portIter] =
        testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[portIter] =
            PORT_OF_CASCADE_TRUNK_LION_A_WITH_OFFSET_MAC(9);
            portIter++;
        testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[portIter] =
        testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[portIter] =
            PORT_OF_CASCADE_TRUNK_LION_A_WITH_OFFSET_MAC(11);
            portIter++;

#if (defined EXMXPM_FAMILY)
        /* the LBH of 'SRC interface' in PUMA requires 2 vlans that differ
           in bits [10:2] (when using SRC ports : 0,8,18,23) !!!
           and 4 members in flooding trunk */
        prvTgfVlansArr[0] = 0x1;
        prvTgfVlansArr[1] = 0xF;
#endif /* EXMXPM_FAMILY */

        if(PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numOfDesignatedTrunkEntriesHw < 64)
        {
            portsArray_total_weight_Ptr = portsArray_total_weight_8;
        }
        else
        {
            portsArray_total_weight_Ptr = portsArray_total_weight_64;
        }
    }

    /* check if the device has ports that used with force configuration */
    isPortConfigForced = prvTgfCommonIsDeviceForce(prvTgfDevNum);

    if(prvTgfTrunkGenericRestorePorts_done == GT_TRUE)
    {
        for(portIter = 0 ; portIter < PRV_TGF_MAX_PORTS_NUM_CNS ; portIter++)
        {
            testDbPortsOrig[portIter]  = prvTgfPortsArray[portIter];
            testDbDevsOrig[portIter] = prvTgfDevsArray[portIter];
        }
    }

    prvTgfTrunkGenericRestorePorts_done = GT_FALSE;

    /* save first 4 ports of test */
    for(portIter = 0 ; portIter < TRUNK_MAX_PORTS_IN_TEST_CNS ; portIter++)
    {
        if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E) &&
           GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E &&
               0 == PRV_CPSS_PP_MAC(prvTgfDevNum)->revision &&
               testType == TRUNK_TEST_TYPE_CASCADE_TRUNK_E)
            {
                /* lion A */
                /* ports of cascade trunk - in the same port group -- limitation for Lion A */
                physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor60PortsConfigForced_cascadeTrunkPortsSamePortGroup[portIter]
                                                                 : testPortsFor60Ports_cascadeTrunkPortsSamePortGroup[portIter]);
            }
            else
                if( (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) &&
                      PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts > 63 )
            {
                /* lion2 */
                if(cascadeTrunk == GT_TRUE)
                {

                    if (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)
                    {
                        /* ports of cascade trunk - in the same hemisphere -- limitation for Lion2 */
                        physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor124PortsConfigForced_cascadeTrunkPortsSameHemisphere[portIter]
                                                                         : testPortsFor124Ports_XLG_cascadeTrunkPortsSameHemisphere[portIter]);
                    }
                    else
                    {
                        /* ports of cascade trunk - in the same hemisphere -- limitation for Lion2 */
                        physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor124PortsConfigForced_cascadeTrunkPortsSameHemisphere[portIter]
                                                                         : testPortsFor124Ports_cascadeTrunkPortsSameHemisphere[portIter]);
                    }
                }
                else
                {
                    if (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)
                    {
                        physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor124PortsConfigForced[portIter]
                                                                     : testPortsFor124Ports_XLG[portIter]);
                    }
                    else
                    {
                        physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor124PortsConfigForced[portIter]
                                                                     : testPortsFor124Ports[portIter]);
                    }
                }
            }
            else
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
            {
                /*allow port value higher than 'numOfPorts' */
                allowHighPorts = 1;
                /*BobCat2*/
                physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor60PortsConfigForced[portIter] :
                        (prvTgfXcat3xExists() == GT_TRUE)  ? testPorts_xCat3x[portIter] : /* AC3X ports */
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType     == CPSS_98DX8332_Z0_CNS) ?  /* aldrin Z0 SPECIFIC limited PORTS*/
                                 testPorts_aldrinZ0[portIter] :
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily   == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ? /* aldrin SPECIFIC limited PORTS*/
                                 testPorts_aldrin[portIter] :
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily   == CPSS_PP_FAMILY_DXCH_AC3X_E) ? /* AC3X SPECIFIC limited PORTS*/
                                 testPorts_xCat3x[portIter] :
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily   == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? /* bobcat3 SPECIFIC limited PORTS*/
                            testPorts_bobcat3[portIter] :
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily   == CPSS_PP_FAMILY_DXCH_FALCON_E) ? /* falcon SPECIFIC limited PORTS*/
                            testPorts_falcon[portIter] :
                        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily   == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ?
                            testPorts_aldrin2[portIter] :
                        IS_BOBK_DEV_MAC(prvTgfDevNum) ?             /* BOBK SPECIFIC limited PORTS*/
                            IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum) ?
                                testPortsFor72Ports_bobk_cetus[portIter] :
                                testPortsFor72Ports_bobk[portIter] :
                            testPortsFor72Ports[portIter]);
            }
            else  /* LionB */
            {
                physicalPort =  ((GT_TRUE == isPortConfigForced) ? testPortsFor60PortsConfigForced[portIter]
                                                                 : testPortsFor60Ports[portIter]);
            }
            mustBeAscending = GT_TRUE;
        }
        else
        {
            if (testType == TRUNK_TEST_TYPE_CRC_HASH_E)
            {
                physicalPort = crcTestPortsFor28Ports[portIter];
            }
            else
            {
                physicalPort = testPortsFor28Ports[portIter];
            }

            if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
            {
                if( CPSS_PP_FAMILY_PUMA_E == PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily )
                {
                    /* the puma supports ports 0..23 */
                    physicalPort %= 24;
                }
                else
                {
                    /* puma3 supports 0..11,16..27 */
                }
            }
        }

        if(allowHighPorts == 0)
        {
            while(physicalPort >= PRV_CPSS_PP_MAC(prvTgfDevNum)->numOfPorts)
            {
                physicalPort -= 2;/* keep the 'odd'/'even' number */
            }
        }

        prvTgfPortsArray[portIter] = physicalPort;
    }

    /* the ports must be ascending otherwise test tgfTrunkCascadeTrunkWithWeights will fail */
    if(mustBeAscending == GT_TRUE)
    {
        if(prvTgfPortsArray[4] >= prvTgfPortsArray[5] ||
           prvTgfPortsArray[5] >= prvTgfPortsArray[6] ||
           prvTgfPortsArray[6] >= prvTgfPortsArray[7])
        {
            /* those 4 ports must be in ascending order because the 'get function'
              of weights can not keep order of ports */
            PRV_UTF_LOG0_MAC("======= ports must be in ascending order =======\n");
            for(portIter = 4;portIter < 7;portIter++)
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(0,
                    ((prvTgfPortsArray[portIter] >= prvTgfPortsArray[portIter+1])?1:0) ,
                    "======= index[%d] hold port[%d] must be less than at index [%d] that hold port[%d]=======",
                    portIter,
                    prvTgfPortsArray[portIter],
                    portIter+1,
                    prvTgfPortsArray[portIter+1]
                    );
            }
            return;
        }
    }


    /* save original L2 header */
    prvTgfPacketL2PartOrig = prvTgfPacketL2Part;
    /* save original Vlan tag header */
    prvTgfPacketVlanTagPartOrig = prvTgfPacketVlanTagPart;
    prvTgfPacketVlanTagPartOrigValid = GT_TRUE;

    /* update the number of ports in test */
    prvTgfPortsNum = (GT_U8)numOfPortsInTest;

    TRUNK_PRV_UTF_LOG0_MAC("======= clear counters =======\n");
    /* clear also counters at end of test */
    clearAllTestedCounters();

    TRUNK_PRV_UTF_LOG0_MAC("======= force links up =======\n");

    /* force linkup on all ports involved */
    forceLinkUpOnAllTestedPorts();

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        rc = prvTgfCfgGlobalEportGet(prvTgfDevNum, &origGlobalRange , &origEcmpRange);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportGet: dev %d",
                                     prvTgfDevNum);

        /* no need for L2Ecmp global ePorts */
        ecmpRange.enable = GT_FALSE;
        /* set the global ePorts for the needed trunks */
        globalRange.enable = GT_TRUE;
        globalRange.pattern = PATTERN_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS;
        globalRange.mask    = MASK_FOR_GLOBAL_EPORT_REPRESENT_TRUNKID_CNS;

        /* AUTODOC: set Global ePorts */
        rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &globalRange , &ecmpRange);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                     prvTgfDevNum);
    }

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        rc = prvTgfTrunkDbMembersSortingEnableGet(prvTgfDevNum , &globalEPortRepresentTrunk_sortingEnabled);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        globalEPortRepresentTrunk_sortingEnabled = GT_FALSE;/* don't care */
    }

    if(cascadeTrunk == GT_FALSE)
    {
         rc = prvTgfTrunkMembersSet(
            trunkId /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
        {
            member.hwDevice = prvTgfDevsArray [portIter];
            member.port   = prvTgfPortsArray[portIter];

            rc = prvTgfTrunkMemberAdd(trunkId,&member);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            if(((portIter - NOT_IN_TRUNK_PORTS_NUM_CNS) & 1) == 0)
            {
                /* the first and third .. members */
                /* used when mode is PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E */
                U32_SET_FIELD_MAC(membersIndexesBmp,portIter,1,1);
            }

            if(tgfTrunkGlobalEPortRepresentTrunk)
            {
                /* the function prvTgfTrunkMemberAdd() , set the ingress port associated with the trunkId ,
                   but we need it to be associated with the 'global eport' which represents the trunk */
                associateIngressPortWithEPortInsteadOfTrunkId(portIter , trunkId);
            }
        }

        prvTgfTrunkLoadBalanceSpecificMembersSet(membersIndexesBmp);

        /* next tests need the FDB with 'known UC' */
        if((testSaLearning == 0) &&
           (testType == TRUNK_TEST_TYPE_CRC_HASH_E ||
            testType == TRUNK_TEST_TYPE_CASCADE_TRUNK_E ||
            testType == TRUNK_TEST_TYPE_SIMPLE_HASH_E))
        {
            if(tgfTrunkGlobalEPortRepresentTrunk == 0)
            {
                macEntry.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
                macEntry.dstInterface.trunkId = trunkId;
            }
            else
            {
                macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
                /* global eport that represents trunk --> MUST be on 'ownDev' */
                macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
                macEntry.dstInterface.devPort.portNum = DEFAULT_SOURCE_EPORT_FROM_TRUNK_ID_MAC(trunkId);
            }

            /* make sure that ALL the mac DA that will be sent during the test will
               be known in the FDB on the needed vlans */
            for(ii = 0 ; ii < prvTgfVlansNum ; ii++)
            {
                macEntry.key.key.macVlan.vlanId = prvTgfVlansArr[ii];
                for(jj = 0; jj < 6; jj++)
                {
                    macEntry.key.key.macVlan.macAddr.arEther[jj] =
                        prvTgfPacketL2Part.daMac[jj];
                }

                rc = prvTgfBrgFdbMacEntrySet(&macEntry);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                for(jj = 0; jj < 6; jj++)
                {
                    if(testType == TRUNK_TEST_TYPE_SIMPLE_HASH_E)
                    {
                        /* only last byte is considered in simple hash */
                        jj = 5;
                    }

                    if((forceCrcRunAllPermutations == 0) && ((prvUtfIsGmCompilation() == GT_TRUE) || (GT_FALSE != prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E))))
                    {
                        if(testType == TRUNK_TEST_TYPE_CRC_HASH_E)
                        {
                            /* skip Fdb updating --> to reduce the time for GM , as we not manipulate the MAC DA in this case ! */
                            jj = 5;
                        }
                    }

                    if(jj == 0)
                    {
                        /* the MAC address MSB will cause 'MCAST address' so do not
                           check those */
                        continue;
                    }

                    for(kk = 0 ; kk < FDB_MAC_COUNT_CNS; kk++)
                    {
                        macEntry.key.key.macVlan.macAddr.arEther[jj] = (GT_U8)(kk);
    /*                        prvTgfPacketL2Part.daMac[jj] + kk);*/

                        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
                        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    }

                    macEntry.key.key.macVlan.macAddr.arEther[jj] =
                        prvTgfPacketL2Part.daMac[jj];
                }
            }
        }
        else if(testType == TRUNK_TEST_TYPE_MPLS_SIMPLE_HASH_E ||
                testType == TRUNK_TEST_TYPE_MPLS_CRC_HASH_E)
        {
            TGF_PACKET_L2_STC         *packetL2Ptr;

            packetL2Ptr = (TGF_PACKET_L2_STC*)prvTgfMplsIpvxPassengerPacketInfo.partsArray[0].partPtr;

            cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

            macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
            for (jj = 0; (jj < 6); jj++)
            {
                macEntry.key.key.macVlan.macAddr.arEther[jj] =
                    packetL2Ptr->daMac[jj];
            }
            macEntry.key.key.macVlan.vlanId = prvTgfVlansArr[0];
            macEntry.daCommand = PRV_TGF_PACKET_CMD_FORWARD_E;
            macEntry.saCommand = PRV_TGF_PACKET_CMD_FORWARD_E;

            if(tgfTrunkGlobalEPortRepresentTrunk == 0)
            {
                macEntry.dstInterface.type    = CPSS_INTERFACE_TRUNK_E;
                macEntry.dstInterface.trunkId = trunkId;
            }
            else
            {
                macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
                /* global eport that represents trunk --> MUST be on 'ownDev' */
                macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
                macEntry.dstInterface.devPort.portNum = DEFAULT_SOURCE_EPORT_FROM_TRUNK_ID_MAC(trunkId);
            }

            macEntry.dstOutlif.outlifType = PRV_TGF_OUTLIF_TYPE_LL_E;

            macEntry.isStatic = GT_TRUE;

            rc = prvTgfBrgFdbMacEntrySet(&macEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }
    else
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);

        if(portsArray_total_weight_Ptr == portsArray_total_weight_8)
        {
            if((prvTgfPortsNum - NOT_IN_TRUNK_PORTS_NUM_CNS) > (sizeof(portsArray_total_weight_8) / sizeof(portsArray_total_weight_8[0])))
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
                return /*GT_NOT_SUPPORTED*/;
            }
        }
        else /*portsArray_total_weight_64*/
        {
            if((prvTgfPortsNum - NOT_IN_TRUNK_PORTS_NUM_CNS) > (sizeof(portsArray_total_weight_64) / sizeof(portsArray_total_weight_64[0])))
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
                return /*GT_NOT_SUPPORTED*/;
            }
        }

        if(trunkUseWeightedPorts == GT_TRUE)
        {
            weightedPortsArray = portsArray_total_weight_Ptr[prvTgfPortsNum-NOT_IN_TRUNK_PORTS_NUM_CNS].weightsArray;

            for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,prvTgfPortsArray[portIter]);
                weightedMembers[portIter-NOT_IN_TRUNK_PORTS_NUM_CNS].member.hwDevice = prvTgfDevsArray[portIter];
                weightedMembers[portIter-NOT_IN_TRUNK_PORTS_NUM_CNS].member.port = prvTgfPortsArray[portIter];
                weightedMembers[portIter-NOT_IN_TRUNK_PORTS_NUM_CNS].weight = weightedPortsArray[portIter-NOT_IN_TRUNK_PORTS_NUM_CNS];
            }

            /* the cascade ports of the trunk used with weights */
            rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,trunkId,
                    (prvTgfPortsNum-NOT_IN_TRUNK_PORTS_NUM_CNS),
                    weightedMembers);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
        else
        {
            for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,prvTgfPortsArray[portIter]);
            }

            rc = prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&portsBmp);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        jj = 0;
        for(ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS ; ii++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp,ii))
            {
                continue;
            }

            if((jj & 1) == 0)
            {
                /* the first and third .. members */
                /* used when mode is PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E */

                /*find the port Iterator that match this port*/
                for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
                {
                    if(prvTgfPortsArray[portIter] == ii)
                    {
                        break;
                    }
                }

                if(portIter == prvTgfPortsNum)
                {
                    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(prvTgfPortsNum, portIter, "bad state");
                    return;
                }


                U32_SET_FIELD_MAC(membersIndexesBmp,portIter,1,1);
            }

            jj++;
        }

        prvTgfTrunkLoadBalanceSpecificMembersSet(membersIndexesBmp);
    }

    for(ii = 0 ; ii < prvTgfVlansNum ; ii++)
    {
        if(prvTgfVlansArr[ii] == 1)
        {
            /* default VLAN entry -- don't touch it */
            continue;
        }

        /* set VLAN entry -- with ports in test */
        rc = prvTgfBrgDefVlanEntryWrite(prvTgfVlansArr[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                     prvTgfVlansArr[ii]);
    }

    return;
}
#if 0
static struct {
    GT_U32                 valid;
    CPSS_DXCH_PORT_MAP_STC portMap;
    GT_U32                 phyPort;
}orig_portMapArr[PRV_TGF_MAX_PORTS_NUM_CNS];

/* logic relate to code from : wrCpssDxChPortPhysicalPortMapUpdateOrAdd */
static GT_STATUS prvTgfPortPhysicalPortMapUpdateOrAdd
(
    IN GT_U32   portIndex,/*index in prvTgfPortsArray[] and in orig_portMapArr []*/
    IN GT_U32   newLocalDevSrcPort /* if 0xFFFFFFFF meaning 'restore' */
)
{
    static CPSS_DXCH_PORT_MAP_STC   portMapArray[256];
    GT_STATUS       rc,rc1;
    GT_U8           devNum;
    GT_BOOL         isValid;
    CPSS_DXCH_PORT_MAP_STC  newInfo;
    GT_U32      currPortMap;
    GT_U32      index;
    GT_PHYSICAL_PORT_NUM                physicalPortNumber;
    GT_BOOL     found = GT_FALSE;
    GT_U32      foundAtIndex = 0;

    physicalPortNumber = prvTgfPortsArray[portIndex];

    currPortMap = 0;
    index = 0;

    cpssOsMemSet(&newInfo,0,sizeof(newInfo));


    while((currPortMap < 256) && (index < 256))
    {
        rc = cpssDxChPortPhysicalPortMapGet(devNum, currPortMap, 1, &portMapArray[index]);
        rc1 = cpssDxChPortPhysicalPortMapIsValidGet(devNum, currPortMap, &isValid);

        currPortMap++;

        if(isValid != GT_TRUE)
        {
            continue;
        }
        else
        if((result != GT_OK) || (result1 != GT_OK))
        {
            continue;
        }

        if(newLocalDevSrcPort != 0xFFFFFFFF)
        {
            if((currPortMap - 1) == physicalPortNumber)
            {
                /* ignore the entry as we going to override it ...but we will put it
                    as last entry in the array ... to unify code with 'new port' */

                /* save previous */
                orig_portMapArr[portIndex].portMap = portMapArray[index];
                orig_portMapArr[portIndex].phyPort = physicalPortNumber;
                orig_portMapArr[portIndex].valid = 1;

                newInfo = portMapArray[index];
                newInfo.physicalPortNumber = newLocalDevSrcPort;

                found = GT_TRUE;
            }
            else
            {
                index++;/* valid entry */
            }
        }
        else /* restore the orig values */
        {
            GT_U32  ii;

            if(found == GT_FALSE)
            {
                for(ii = 0 ; ii < PRV_TGF_MAX_PORTS_NUM_CNS; ii++)
                {
                    if((currPortMap - 1) == orig_portMapArr[ii].phyPort &&
                        orig_portMapArr[ii].valid)
                    {
                        newInfo = orig_portMapArr[ii].portMap;

                        orig_portMapArr[ii].valid = 0;
                        found = GT_TRUE;
                        break;
                    }
                }
            }

            if(found == GT_FALSE)
            {
                index++;/* valid entry */
            }

        }
    }

    if(index >= 256)
    {
        return GT_FULL;
    }

    if(found == GT_FALSE)
    {
        return GT_NOT_FOUND;
    }


    /* add new port / update new port as the 'last entry' */
    portMapArray[index] = newInfo;
    index ++;

    rc = cpssDxChPortPhysicalPortMapSet(devNum, index, portMapArray);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}
#endif
/**
* @internal prvTgfTrunkCrcHashTestInit function
* @endinternal
*
* @brief   init for CRC hash feature test
*         applicable devices: Lion and above
* @param[in] L3L4ParsingOverMplsEnabled - check CRC hash L3L4 for traffic over MPLS.
*                                      or test the CRC hash for all traffic types.
*                                      GT_TRUE - tests CRC for L3L4 traffic over MPLS.
*                                      GT_FALSE - test CRC for all traffic types (not include L3L4 over MPLS)
*                                       None
*/
void prvTgfTrunkCrcHashTestInit
(
    IN GT_BOOL          L3L4ParsingOverMplsEnabled
)
{
    GT_STATUS   rc;
    GT_U32  ii;

    prvTgfVlansNum = 2;

    if((prvUtfIsGmCompilation() == GT_TRUE) || (GT_FALSE != prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E)))
    {
        prvTgfVlansNum = 1;
    }

    if(L3L4ParsingOverMplsEnabled == GT_FALSE)
    {
        prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_CRC_HASH_E,GT_FALSE,
                           TRUNK_8_PORTS_IN_TEST_CNS );
    }
    else
    {
        prvTgfVlansNum = 1;
        prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_MPLS_CRC_HASH_E,GT_FALSE,
                           TRUNK_8_PORTS_IN_TEST_CNS );
    }

    /* set UDE registers */
    for(ii = 0 ; ii < NUM_UDE_CNS ; ii++)
    {
        rc = prvTgfPclUdeEtherTypeSet(ii,UDE_BASE_CNS + ii);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum) ||
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS) ||
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        /* the CETUS selected port numbers ,
           makes is with 'Better LBH' */
        prvTgfLbhSelectedPortsNum = 2;
    }
    else
    {
        prvTgfLbhSelectedPortsNum = 1;
    }

    return;
}

/**
* @internal prvTgfTrunkFdBFlush function
* @endinternal
*
* @brief   flush the FDB
*         applicable devices: all DxCh devices
*/
static void prvTgfTrunkFdBFlush
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  numOfValid = 0 ;/* number of valid entries in the FDB */
    GT_U32  numOfSkip = 0; /* number of skipped entries in the FDB */
    GT_U32  numOfSp = 0; /* number of Storm Protection entries in the FDB */
    GT_U32  numOfValidMinusSkip=0;/* number of (valid - skip) entries in the FDB*/
    GT_U32  maxFlush;/* up to 10 times flush FDB , due to still processing of AppDemo or Asic simulation */

#ifdef ASIC_SIMULATION
    maxFlush = 10;
#else
    maxFlush = 1;
#endif /*ASIC_SIMULATION*/

    TRUNK_PRV_UTF_LOG0_MAC("=======  : flush FDB =======\n");

    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    do{
        /* delete mac addresses , include static */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(prvTgfFdbShadowUsed == GT_TRUE)
        {
            /* the GM takes too match time to quarry the FDB.
               we will need to assume that the FDB was cleared ! */
            numOfValidMinusSkip = 0;
            maxFlush = 0;
        }
        else
        {
            /* Check FDB capacity (FDB should be empty) */
            /* get FDB counters */
            rc = prvTgfBrgFdbCount(&numOfValid,&numOfSkip,NULL,&numOfSp,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            numOfValidMinusSkip = numOfValid - numOfSkip - numOfSp;

            maxFlush--;
        }

        if(numOfValidMinusSkip && maxFlush)
        {
            cpssOsTimerWkAfter(1000);
            utfPrintKeepAlive();
        }
    }while(numOfValidMinusSkip && maxFlush);


    if(numOfValidMinusSkip)
    {
        prvTgfBrgFdbDump();
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfValidMinusSkip,
                    "FDB not empty , got [%d] entries ",
                    numOfValidMinusSkip);

}

/**
* @internal prvTgfTrunkGenericRestore function
* @endinternal
*
* @brief   restore setting after trunk test
*         applicable devices: All DxCh devices
* @param[in] cascadeTrunk             - do we destroy cascade trunk / regular trunk
*                                       None
*/
static void prvTgfTrunkGenericRestore
(
    IN GT_BOOL cascadeTrunk
)
{
    GT_STATUS   rc;
    GT_U32  portIter;/* port iterator */
    GT_TRUNK_ID             trunkId;
    GT_U32  ii;

    prvTgfTrunkCrcHash_cpssPredictEgressPortEnableSet(GT_FALSE);
    prvTgfTrunkCrcHash_symmetricModeEnable(GT_FALSE);

    /* restore first ports of test */
    for(portIter = 0 ; portIter < TRUNK_MAX_PORTS_IN_TEST_CNS ; portIter++)
    {
        if(tgfTrunkGlobalEPortRepresentTrunk && defaultEPortWasChanged[portIter])
        {
            /* restore the default EPort of the ingress port */
            rc = prvTgfCfgPortDefaultSourceEportNumberSet(prvTgfDevNum,
                prvTgfPortsArray[portIter],
                defaultEPort[portIter]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            defaultEPortWasChanged[portIter] = 0;
        }

        prvTgfPortsArray[portIter] = testDbPortsOrig[portIter];
        prvTgfDevsArray[portIter] = testDbDevsOrig[portIter];
    }
    prvTgfTrunkGenericRestorePorts_done = GT_TRUE;

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        /* AUTODOC: restore Global ePorts */
        rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &origGlobalRange , &origEcmpRange);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgGlobalEportSet: dev %d",
                                     prvTgfDevNum);

        globalEPortRepresentTrunk_e2phyReady = GT_FALSE;
    }


    /* restore default number of ports */
    prvTgfPortsNum = PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS;

    if(prvTgfPacketVlanTagPartOrigValid == GT_TRUE)
    {
        /* restore L2 header from original */
        prvTgfPacketL2Part = prvTgfPacketL2PartOrig;
        /* restore Vlan tag header from original */
        prvTgfPacketVlanTagPart = prvTgfPacketVlanTagPartOrig;
    }

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    TRUNK_PRV_UTF_LOG0_MAC("======= clear counters =======\n");
    /* clear also counters at end of test */
    clearAllTestedCounters();

    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(cascadeTrunk == GT_FALSE)
    {
        rc = prvTgfTrunkMembersSet(
            trunkId /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        if(trunkUseWeightedPorts == GT_TRUE)
        {
            rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,trunkId,0,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
        else
        {
            rc = prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    prvTgfTrunkFdBFlush();

    for(ii = 0 ; ii < prvTgfVlansNum ; ii++)
    {
        if(prvTgfVlansArr[ii] == 1)
        {
            /* default VLAN entry -- don't touch it */
            continue;
        }

        /* invalidate the other vlans */
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlansArr[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                     prvTgfVlansArr[ii]);
    }

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);

    return;
}
/**
* @internal prvTgfTrunkCrcHashTestRestore function
* @endinternal
*
* @brief   restore for CRC hash feature test
*         applicable devices: Lion and above
*/
void prvTgfTrunkCrcHashTestRestore
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  ii;

    prvTgfTrunkGenericRestore(GT_FALSE);

    /* restore UDE registers */
    for(ii = 0 ; ii < NUM_UDE_CNS ; ii++)
    {
        rc = prvTgfPclUdeEtherTypeSet(ii,0xFFFF);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
}
/**
* @internal tgfTrunkDebugBurstNumSet function
* @endinternal
*
* @brief   debug function to run test of traffic with smaller burst -->
*         less number of entries to the FDB.
*         to to allow fast debug of error in test and not wait till end of traffic
*         learn...
*/
GT_STATUS tgfTrunkDebugBurstNumSet
(
    IN GT_U32   burstNum
)
{
    debug_burstNum = burstNum;
    return GT_OK;
}

/**
* @internal prvTgfTrunkCrcHashMaskPriorityTtiConfigSet function
* @endinternal
*
* @brief   sets TTI Action Configuration for single and dual lookup
*         applicable devices: EArch only
* @param[in] isFirstRule
*                                      GT_TRUE - single lookup enabled
*                                      GT_FALSE - dual lookup enabled. TTI1 overrides TTI0 hashMaskIndex
*/
static GT_VOID prvTgfTrunkCrcHashMaskPriorityTtiConfigSet
(
    GT_BOOL isFirstRule
)
{
    PRV_TGF_TTI_ACTION_2_STC ttiAction2 = {0};
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    GT_STATUS                rc         = GT_FAIL;

    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0 , 3);
    prvTgfTtiRuleLookup1Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1 , 0);

    /* AUTODOC: SETUP TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear TTI Rule Pattern, Mask to match all packets */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));

    /* AUTODOC: set the TTI Rule for ETH Type Key */
    ttiAction2.hashMaskIndex    = isFirstRule ? 7 : 8;
    ttiAction2.continueToNextTtiLookup = isFirstRule ? GT_FALSE : GT_TRUE;;

    /* AUTODOC: add TTI rule  with ETH key on port 0 VLAN 5 with action: redirect ot egress */
    rc = prvTgfTtiRule2Set(isFirstRule ? prvTgfTtiRuleLookup0Index : prvTgfTtiRuleLookup1Index,
                           PRV_TGF_TTI_KEY_ETH_E, &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal prvTgfTrunkCrcHashMaskPriorityTrafficGenerate function
* @endinternal
*
* @brief   clear hash table, set field in hash table and send traffic
*         applicable devices: Lion and above
* @param[in] hashMaskTableIndex       - index of entry in hash mask table - 0..27
* @param[in] hashMaskFieldType        - index of field in the 70 bytes for hash - 0..69
* @param[in] hashMaskFieldSubType
* @param[in] hashMaskFieldByteIndex
* @param[in] trafficType
* @param[in] reduceTimeFactor
* @param[in] wasIgnoredDueToReduceTime
* @param[in] lbhMode                  - LBH mode expected
*                                       None
*/
static GT_VOID prvTgfTrunkCrcHashMaskPriorityTrafficGenerate
(
    GT_U32                                       hashMaskTableIndex,
    PRV_TGF_TRUNK_FIELD_TYPE_ENT                 hashMaskFieldType,
    GT_U32                                       hashMaskFieldSubType,
    GT_U32                                       hashMaskFieldByteIndex,
    PRV_TGF_PCL_PACKET_TYPE_ENT                  trafficType,
    GT_U32                                       reduceTimeFactor,
    GT_BOOL                                      *wasIgnoredDueToReduceTime,
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT lbhMode
)
{
    GT_STATUS rc = GT_FAIL;

    /* AUTODOC: clear full table */
    rc = trunkHashMaskCrcTableEntryClear(PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set the CRC hash mask table according to 'Override index' */
    rc = trunkHashMaskCrcFieldSet(
            hashMaskTableIndex,
            hashMaskFieldType,
            hashMaskFieldSubType,
            hashMaskFieldByteIndex);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: send traffic according to field checking */
    rc = trunkHashCrcLbhTrafficSend(
            prvTgfTrunkTrafficTypes[trafficType],
            trafficType,
            hashMaskFieldType,
            hashMaskFieldSubType,
            hashMaskFieldByteIndex,
            lbhMode,
            L2_VLAN_TAG_ETHERETYPE_SIZE_CNS,
            GT_FALSE,
            reduceTimeFactor,
            wasIgnoredDueToReduceTime);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal prvTgfTrunkCrcHashMaskPriorityTtiConfigurationRestore function
* @endinternal
*
* @brief   restore TTI configuration
*         applicable devices: EArch only
*/
static GT_VOID prvTgfTrunkCrcHashMaskPriorityTtiConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc                        = GT_FAIL;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: disable TTI lookup for port 0, key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI0 rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup0Index, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate TTI1 rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleLookup1Index, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
}

/**
* @internal prvTgfTrunkCrcHashMaskPriorityTest function
* @endinternal
*
* @brief   check CRC hash mask priority:
*         default of accessing the CRC mask hash table is :
*         'Traffic type' --> index 16..27
*         higher priority for the 'per port' --> index 0..15
*         highest priority to the TTI action --> index 1..15 (without index 0)
*         applicable devices: Lion and above
*/
GT_VOID prvTgfTrunkCrcHashMaskPriorityTest
(
    GT_VOID
)
{
    GT_STATUS                      rc;
    PRV_TGF_TRUNK_LBH_CRC_MODE_ENT crcMode;
    PEARSON_TEST_MODE_ENT          pearsonMode;
    PRV_TGF_TRUNK_FIELD_TYPE_ENT   hashMaskFieldType; /* index of field in the 70 bytes for hash - 0..69 */
    PRV_TGF_PCL_PACKET_TYPE_ENT    trafficType;       /*traffic type*/
    GT_PHYSICAL_PORT_NUM           portNum;           /*port number to send packets */

    GT_U32  seedIndex;
    GT_U32  hashMaskTableIndex;         /* index of entry in hash mask table - 0..27 */
    GT_U32  hashMaskFieldSubType;
    GT_U32  hashMaskFieldByteIndex;
    GT_U8   devNum;                     /*device number to send packets */
    GT_U32  portIter;                   /* port iterator */
    GT_U32  portOverrideMaskEntryIndex; /* the 'per port' override index */
    GT_U32  hashMaskTableIndexFirst;    /* first index for hashMaskTableIndex */
    GT_U32  reduceTimeFactor =  PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;
    GT_BOOL wasIgnoredDueToReduceTime = GT_FALSE;

    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;/* LBH mode expected */

    crcMode = UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) ?
        PRV_TGF_TRUNK_LBH_CRC_32_MODE_E :
        PRV_TGF_TRUNK_LBH_CRC_16_AND_PEARSON_MODE_E;
    seedIndex = 1;
    pearsonMode = PEARSON_TEST_MODE_INDEX_E;
    trafficType = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
    hashMaskFieldType = PRV_TGF_TRUNK_FIELD_L4_PORT_E;
    hashMaskFieldSubType = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;

    /* AUTODOC: for reduce time purposes the function trunkHashCrcLbhTrafficSend must
       get index 1 for src TCP port */
    hashMaskFieldByteIndex = prvUtfIsGmCompilation() ? 1 : 0;

    /* AUTODOC: set new mode and seed */
    rc = prvTgfTrunkHashCrcParametersSet(crcMode,crcSeeds[seedIndex]);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set Pearson table */
    rc = trunkHashPearsonFill(pearsonMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    hashMaskTableIndexFirst = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS;

    hashMaskTableIndex = trafficType + hashMaskTableIndexFirst;
    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;

    prvTgfTrunkCrcHashMaskPriorityTrafficGenerate(hashMaskTableIndex,
                                                      hashMaskFieldType,
                                                      hashMaskFieldSubType,
                                                      hashMaskFieldByteIndex,
                                                      trafficType,
                                                      reduceTimeFactor,
                                                      &wasIgnoredDueToReduceTime,
                                                      lbhMode);
    if((prvUtfIsGmCompilation() == GT_TRUE) &&
        (wasIgnoredDueToReduceTime == GT_FALSE))
    {
        PRV_UTF_LOG0_MAC("Tested according to 'Traffic type' (before per index) \n");
    }

    portIter = 0;
    devNum  = prvTgfDevsArray [portIter];
    portNum = prvTgfPortsArray[portIter];

    for(portOverrideMaskEntryIndex = 0 ;
        portOverrideMaskEntryIndex < hashMaskTableIndexFirst;
        portOverrideMaskEntryIndex++)
    {
        /* AUTODOC: set the 'src port' to be with 'Override mask index' */
        rc = prvTgfTrunkPortHashMaskInfoSet(devNum,portNum,GT_TRUE,portOverrideMaskEntryIndex);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        hashMaskTableIndex = portOverrideMaskEntryIndex;

        prvTgfTrunkCrcHashMaskPriorityTrafficGenerate(hashMaskTableIndex,
                                                          hashMaskFieldType,
                                                          hashMaskFieldSubType,
                                                          hashMaskFieldByteIndex,
                                                          trafficType,
                                                          reduceTimeFactor,
                                                          &wasIgnoredDueToReduceTime,
                                                          lbhMode);
        if((prvUtfIsGmCompilation() == GT_TRUE) &&
            (wasIgnoredDueToReduceTime == GT_FALSE))
        {
            PRV_UTF_LOG1_MAC("Tested index [%d] \n",
                portOverrideMaskEntryIndex);
        }
    }

    portOverrideMaskEntryIndex = 5;
    /* AUTODOC: set the 'src port' to be with 'Override mask index' */

    rc = prvTgfTrunkPortHashMaskInfoSet(devNum,portNum,GT_FALSE,portOverrideMaskEntryIndex);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    hashMaskTableIndex = portOverrideMaskEntryIndex;

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
    /* AUTODOC: set the CRC hash mask table according to 'Traffic type' */
    prvTgfTrunkCrcHashMaskPriorityTrafficGenerate(hashMaskTableIndex,
                                                      hashMaskFieldType,
                                                      hashMaskFieldSubType,
                                                      hashMaskFieldByteIndex,
                                                      trafficType,
                                                      reduceTimeFactor,
                                                      &wasIgnoredDueToReduceTime,
                                                      lbhMode);

    /* AUTODOC:  Test according to 'TTI Action' hashMaskIndex */

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: test with single lookup */
        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
        hashMaskTableIndex = 7;

        prvTgfTrunkCrcHashMaskPriorityTtiConfigSet(GT_FALSE);
        prvTgfTrunkCrcHashMaskPriorityTrafficGenerate(hashMaskTableIndex,
                                                  hashMaskFieldType,
                                                  hashMaskFieldSubType,
                                                  hashMaskFieldByteIndex,
                                                  trafficType,
                                                  reduceTimeFactor,
                                                  &wasIgnoredDueToReduceTime,
                                                  lbhMode);

        /* AUTODOC: test with dual lookup */
        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
        prvTgfTrunkCrcHashMaskPriorityTtiConfigSet(GT_TRUE);
        prvTgfTrunkCrcHashMaskPriorityTrafficGenerate(hashMaskTableIndex,
                                                  hashMaskFieldType,
                                                  hashMaskFieldSubType,
                                                  hashMaskFieldByteIndex,
                                                  trafficType,
                                                  reduceTimeFactor,
                                                  &wasIgnoredDueToReduceTime,
                                                  lbhMode);

        /* AUTODOC: restore TTI configuration */
        prvTgfTrunkCrcHashMaskPriorityTtiConfigurationRestore();

        if((prvUtfIsGmCompilation() == GT_TRUE) &&
            (wasIgnoredDueToReduceTime == GT_FALSE))
        {
            PRV_UTF_LOG0_MAC("Tested according to 'TTI Action' hashMaskIndex \n");
        }
    }

}

/**
* @internal tgfTrunkCascadeTrunkTestInit function
* @endinternal
*
* @brief   init for cascade trunks test
*         applicable devices: ALL DxCh devices
* @param[in] useWeightedPorts         - use weighted ports
*                                       None
*/
void tgfTrunkCascadeTrunkTestInit
(
    IN  GT_BOOL     useWeightedPorts
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID             trunkId;
    PRV_TGF_BRG_MAC_ENTRY_STC      macEntry = PRV_TGF_BRG_FDB_MAC_ENTRY_DEFAULT_MAC;
    GT_U32      ii,kk;
    GT_PHYSICAL_PORT_NUM         targetPortNum;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    static GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];
    TGF_VFD_INFO_STC    vfd;
    GT_U32               burstCount;/* burst count */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    /* save info for all other places that need this info */
    trunkUseWeightedPorts = useWeightedPorts;

    prvTgfVlansNum = 2;

    prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_CASCADE_TRUNK_E,GT_TRUE,
                           TRUNK_8_PORTS_IN_TEST_CNS );

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* add the remote device to the FDB device table , so FDB entries on current
       device which are registered on the remote device will not be deleted by
       the aging daemon */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] |= (1 << REMOTE_DEVICE_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    /* set mac entries on 'Remote device' so we can point this device via the
       cascade trunk */
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = REMOTE_DEVICE_NUMBER_CNS;
    macEntry.dstInterface.devPort.portNum = 7;/* dummy port number */

    /* set the info for the increment of the mac */
    vfd.cycleCount = 6;
    burstCount = FDB_MAC_COUNT_CNS;
    vfd.mode = TGF_VFD_MODE_INCREMENT_E;
    vfd.modeExtraInfo = 0;
    vfd.offset = 0;
    cpssOsMemCpy(vfd.patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* make sure that ALL the mac DA that will be sent during the test will
       be known in the FDB on the needed vlans */
    for(ii = 0 ; ii < prvTgfVlansNum ; ii++)
    {
        cpssOsMemCpy(vfd.patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
        macEntry.key.key.macVlan.vlanId = prvTgfVlansArr[ii];

        for(kk = 0 ; kk < burstCount; kk++)
        {
            /* increment the mac address */
            vfd.modeExtraInfo = kk;

            /* over ride the buffer with VFD info */
            rc = tgfTrafficEnginePacketVfdApply(&vfd,
                    macEntry.key.key.macVlan.macAddr.arEther,
                    sizeof(TGF_MAC_ADDR));
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            rc = prvTgfBrgFdbMacEntrySet(&macEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    /* cascade link info */
    cascadeLink.linkNum = trunkId;
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;

    /* set this 'Remote device' to point to the cascade trunk */
    for(targetPortNum = 0 ;targetPortNum < CPSS_MAX_PORTS_NUM_CNS;targetPortNum++)
    {
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE_NUMBER_CNS,
                targetPortNum,&cascadeLink,
                PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    return;
}

/**
* @internal tgfTrunkCascadeTrunkTestRestore function
* @endinternal
*
* @brief   restore values after cascade trunks test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkCascadeTrunkTestRestore
(
    void
)
{
    GT_STATUS   rc;
    GT_PHYSICAL_PORT_NUM         targetPortNum;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink;
    static GT_U32  devTableArr[PRV_TGF_FDB_DEV_TABLE_SIZE_CNS];

    /* remove the remote device from the FDB device table */
    /* do it before FDB flush that is done in prvTgfTrunkGenericRestore(...)
       otherwise the 'Remote device entries' will not be deleted */
    rc = prvTgfBrgFdbDeviceTableGet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    devTableArr[0] &= ~(1 << REMOTE_DEVICE_NUMBER_CNS);

    rc = prvTgfBrgFdbDeviceTableSet(devTableArr);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    prvTgfTrunkGenericRestore(GT_TRUE);

    /* cascade link info */
    cascadeLink.linkNum = CPSS_NULL_PORT_NUM_CNS;
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;

    /* set this 'Remote device' to point 'NULL port' */
    for(targetPortNum = 0 ;targetPortNum < CPSS_MAX_PORTS_NUM_CNS;targetPortNum++)
    {
        rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,REMOTE_DEVICE_NUMBER_CNS,
                targetPortNum,&cascadeLink,
                PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    /* set the desigMode table */
    rc = prvTgfTrunkHashDesignatedTableModeSet(PRV_TGF_TRUNK_DESIGNATED_TABLE_DEFAULT_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    trunkUseWeightedPorts = GT_FALSE;

}
/**
* @internal tgfTrunkCascadeTrunkTest function
* @endinternal
*
* @brief   The cascade trunks test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkCascadeTrunkTest
(
    void
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *trafficPtr;/* pointer to traffic to sent */
    GT_U8   devNum;/*device number to send packets */
    GT_PHYSICAL_PORT_NUM portNum;/*port number to send packets */
    GT_U32  portIter;/* port iterator */
    GT_U32  numberOfPortsSending;/* number of ports sending traffic */
    GT_U32  sendingPortsBmp;/* bitmap of ports that send bursts */
    GT_U32  numVlans;/* number of vlans to test */
    GT_U32               burstCount;/* burst count */
    GT_U32               numVfd = 1;/* number of VFDs in vfdArray */
    static TGF_VFD_INFO_STC     vfdArray[1] = {{0}};/* vfd Array -- used for increment the tested bytes , and for vlan tag changing */
    TGF_VFD_INFO_STC     *vrfPtr = &vfdArray[0];
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;/*traffic type*/
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;/* LBH mode expected */
    GT_U32  ii;
    PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT desigMode;/* designated table mode */
    GT_U32  lastPortIndex;/* index of last port in trunk */
    GT_U32  numMembers;/* number of current trunk members */
    GT_TRUNK_ID             trunkId;
    CPSS_PORTS_BMP_STC cascadeTrunkPorts;/* cascade trunk ports */
    CPSS_PORTS_BMP_STC origCascadeTrunkPorts;/* original cascade trunk ports */
    static CPSS_TRUNK_WEIGHTED_MEMBER_STC  origWeightedMembers[8];/* original weighted cascade trunk ports */
    static CPSS_TRUNK_WEIGHTED_MEMBER_STC  weightedMembers[8];/* original weighted cascade trunk ports */
    GT_U32      origNumOfMembers = 0;/*original number of members in the trunk*/

    PRV_UTF_LOG1_MAC(" tgfTrunkCascadeTrunkTest - start : %s mode \n" , (trunkUseWeightedPorts == GT_FALSE) ? "no weights" : "with weights");
    /*****************/
    /* test known UC */
    /*****************/
    trafficType = PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E;
    trafficPtr = prvTgfTrunkTrafficTypes[trafficType];

    utfGeneralStateMessageSave(0,"trafficType = %d",trafficType);

    burstCount = FDB_MAC_COUNT_CNS;

    vrfPtr->mode = TGF_VFD_MODE_INCREMENT_E;
    vrfPtr->modeExtraInfo = 0;
    vrfPtr->offset = 0;/* mac DA field */
    cpssOsMemCpy(vrfPtr->patternPtr,prvTgfPacketL2Part.daMac,sizeof(TGF_MAC_ADDR));
    vrfPtr->cycleCount = 6;/*6 bytes*/

    if(debug_burstNum)
    {
        burstCount = debug_burstNum;
    }

    for(desigMode = PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E ;
        desigMode <= PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E ;
        desigMode++)
    {
        utfGeneralStateMessageSave(1,"desigMode = %d %s",desigMode,
            TO_STRING_DESIGNATED_TABLE_MODE_MAC(desigMode)
            );

        /* known UC is not influenced by those designated table modes */
        lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
        numberOfPortsSending = NOT_IN_TRUNK_PORTS_NUM_CNS;
        numVlans = prvTgfVlansNum;

        switch(desigMode)
        {
            case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E:
                /* send from single vlan , so we will not have LBH due to vlan changing*/
                numVlans = 1;
                break;
            case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E:
                if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E))
                {
                    /* the device not support this mode */
                    continue;
                }
                /* send from single port , so we will not have LBH due to port changing*/
                numberOfPortsSending = 1;

                break;
            default:
                break;
        }

        utfGeneralStateMessageSave(2,"lbhMode = %d %s",lbhMode,
            TO_STRING_LBH_MODE_MAC(lbhMode)
            );

        /* set the desigMode table */
        rc = prvTgfTrunkHashDesignatedTableModeSet(desigMode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(prvUtfIsGmCompilation() == GT_TRUE)
        {
            numberOfPortsSending = 1;
            numVlans = 1;
        }

        for(portIter = 0 ; portIter < numberOfPortsSending; portIter++)
        {
            devNum  = prvTgfDevsArray [portIter];
            portNum = prvTgfPortsArray[portIter];

            for(ii = 0 ; ii < numVlans ; ii++)
            {
                /* modify the vid part */
                prvTgfPacketVlanTagPart.vid = (TGF_VLAN_ID)prvTgfVlansArr[ii];

                vrfPtr->modeExtraInfo = 0;
                /* send the burst of packets with incremental byte */
                prvTgfTrunkTestPacketSend(devNum, portNum, trafficPtr ,burstCount ,numVfd ,vfdArray);
            }
        }

        sendingPortsBmp = (1 << numberOfPortsSending) - 1;
        /* check the LBH and no flooding */
        trunkSendCheckLbh(GT_TRUE,sendingPortsBmp,burstCount * numVlans ,lbhMode,NULL);

        /* reduce the runtime of tests when prvUtfSkipLongTestsFlagGet() */
        if((prvUtfIsGmCompilation() == GT_TRUE) || (prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE))
        {
            /* reduce iterations */
            break;
        }
    }

    /*********************************************************/
    /* test to for CQ# 127292 -                              */
    /* must NOT flood back from the cascade into the cascade */
    /*********************************************************/

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E;
    if(trunkUseWeightedPorts == GT_TRUE)
    {
        origNumOfMembers = sizeof(origWeightedMembers) / sizeof(origWeightedMembers[0]);
        rc = prvTgfTrunkWithWeightedMembersGet(prvTgfDevsArray[0],trunkId,&origNumOfMembers,origWeightedMembers);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        rc = prvCpssGenericTrunkCascadeTrunkPortsGet(prvTgfDevsArray[0],trunkId,&origCascadeTrunkPorts);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    utfGeneralStateMessageSave(2,"lbhMode = %d %s",lbhMode,
        TO_STRING_LBH_MODE_MAC(lbhMode)
        );
    utfGeneralStateMessageSave(3,"cascade check: multi destination from the cascade (no flood back)");

    for(desigMode = PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E ;
        desigMode <= PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E ;
        desigMode++)
    {
        switch(desigMode)
        {
            case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E:
                if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E))
                {
                    /* the device not support this mode */
                    continue;
                }
            default:
                break;
        }

        utfGeneralStateMessageSave(1,"desigMode = %d %s",desigMode,
            TO_STRING_DESIGNATED_TABLE_MODE_MAC(desigMode)
            );

        if(trunkUseWeightedPorts == GT_TRUE)
        {
            /* restore original 4 members*/
            rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,trunkId,origNumOfMembers,origWeightedMembers);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* the members to be tested initialized as the original ,
               but weighs will be changed according to the number of ports in trunk */
            for(ii = 0 ; ii < origNumOfMembers; ii++)
            {
                weightedMembers[ii] = origWeightedMembers[ii];
            }
        }
        else
        {
            /* restore original 4 members*/
            prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&origCascadeTrunkPorts);

            /* copy the original members to local parameter that modified in the loop of members */
            cascadeTrunkPorts = origCascadeTrunkPorts;
        }

        /* check removing of ports from the cascade trunk */
        for(numMembers = 4 ; /* no break*/ ;numMembers--)
        {
            utfGeneralStateMessageSave(4,"cascade check: with %d ports in the trunk ",numMembers);

            lastPortIndex = NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers - 1;


            if(trunkUseWeightedPorts == GT_TRUE)
            {
                PRV_UTF_LOG1_MAC("number of ports in weighted trunk : %d \n",numMembers);
                /* update the weights of the new members*/
                for(ii = 0 ; ii < numMembers; ii++)
                {
                    weightedMembers[ii].weight = portsArray_total_weight_Ptr[numMembers].weightsArray[ii];
                    PRV_UTF_LOG2_MAC("port[%d] , weight[%d] \n",weightedMembers[ii].member.port , weightedMembers[ii].weight);
                }


                rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,
                        trunkId,
                        numMembers,
                        weightedMembers);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }
            else
            {
                if(numMembers != 4)/* we need to check the trunk with 4 ports (not remove any port) */
                {
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&cascadeTrunkPorts,prvTgfPortsArray[lastPortIndex + 1]);

                    /* remove a port from the trunk */
                    prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&cascadeTrunkPorts);
                }
            }

            /* b. send traffic from the trunk --> check no flood back to the trunk */
            mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

            if(numMembers == 0)
            {
                break;
            }

            /* reduce the runtime of tests when prvUtfSkipLongTestsFlagGet() */
            if((prvUtfIsGmCompilation() == GT_TRUE) /*|| (prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE)*/)
            {
                /* reduce iterations */
                break;
            }
        }

        /* reduce the runtime of tests when prvUtfSkipLongTestsFlagGet() */
        if((prvUtfIsGmCompilation() == GT_TRUE) || (prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE))
        {
            /* reduce iterations */
            break;
        }
    }

    PRV_UTF_LOG0_MAC(" tgfTrunkCascadeTrunkTest - end \n");
}

/**
* @internal tgfTrunkDesignatedTableModesTestInit function
* @endinternal
*
* @brief   init for designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTestInit
(
    void
)
{
    prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_DESIGNATED_TABLE_MODES_E,GT_FALSE,
                           TRUNK_8_PORTS_IN_TEST_CNS );

    /* flush the FDB because we test 'Multi-destination traffic' */
    /*prvTgfTrunkFdBFlush(); --> fdb should be empty */

}


/**
* @internal tgfTrunkDesignatedTableModesTestRestore function
* @endinternal
*
* @brief   restore values after designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTestRestore
(
    void
)
{
    GT_STATUS   rc;

    prvTgfTrunkGenericRestore(GT_FALSE);

    /* set the desigMode table */
    rc = prvTgfTrunkHashDesignatedTableModeSet(PRV_TGF_TRUNK_DESIGNATED_TABLE_DEFAULT_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}


/**
* @internal tgfTrunkDesignatedTableModesTest function
* @endinternal
*
* @brief   The designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTest
(
    void
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *trafficPtr;/* pointer to traffic to sent */
    GT_U8   devNum;/*device number to send packets */
    GT_PHYSICAL_PORT_NUM portNum;/*port number to send packets */
    GT_U32  portIter;/* port iterator */
    GT_U32  numberOfPortsSending;/* number of ports sending traffic */
    GT_U32  sendingPortsBmp;/* bitmap of ports that send bursts */
    GT_U32  numVlans;/* number of vlans to test */
    GT_U32               burstCount;/* burst count */
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;/*traffic type*/
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;/* LBH mode expected */
    GT_U32  ii;
    PRV_TGF_TRUNK_DESIGNATED_TABLE_MODE_ENT desigMode;/* designated table mode */
    PRV_TGF_MULTI_DESTINATION_TYPE_ENT  multiDestType;/* multi destination traffic type */
    GT_U32  multiDestCount;/* number of destination traffics */
    enum {LBH_FALSE_E,LBH_TRUE_E}   lbhGenerated;

    /*******************/
    /* test unknown UC */
    /*******************/

    trafficType = PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E;
    trafficPtr = prvTgfTrunkTrafficTypes[trafficType];

    utfGeneralStateMessageSave(0,"trafficType = %d",trafficType);

    for(desigMode = PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E ;
        desigMode <= PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E ;
        desigMode++)
    {
        if(debugMode)
        {
            desigMode = debug_desigMode;
        }

        utfGeneralStateMessageSave(1,"desigMode = %d %s",desigMode,
            TO_STRING_DESIGNATED_TABLE_MODE_MAC(desigMode)
            );

        if(desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E)
        {
            if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E))
            {
                /* the device not support this mode */
                continue;
            }
        }
        else if(desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E ||
                desigMode == PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E)
        {
            if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
            {
                /* puma not support ingress hash (with / without vid) */
                continue;
            }
        }

        /* set the desigMode table */
        rc = prvTgfTrunkHashDesignatedTableModeSet(desigMode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        for(lbhGenerated = LBH_FALSE_E ;
            lbhGenerated <= LBH_TRUE_E;
            lbhGenerated++)
        {
            if((prvUtfIsGmCompilation() == GT_TRUE))
            {
                /* reduce iterations */
                lbhGenerated = LBH_TRUE_E;
            }

            if(debugMode)
            {
                lbhGenerated = debug_lbhGenerated;
            }

            utfGeneralStateMessageSave(2,"lbhGenerated = %d %s",lbhGenerated,
                lbhGenerated == LBH_FALSE_E ? "LBH_FALSE_E" : "LBH_TRUE_E"
                );


            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
            numberOfPortsSending = NOT_IN_TRUNK_PORTS_NUM_CNS;
            numVlans = prvTgfVlansNum;
            burstCount = CRC_HASH_BURST_COUNT_NEW_CNS / MULTI_DEST_COUNT_MAC;
#if ((defined EXMXPM_FAMILY) && (defined ASIC_SIMULATION))
            burstCount = BURST_COUNT_CNS / MULTI_DEST_COUNT_MAC;
#endif /* PUMA_GM */

            if(debug_burstNum)
            {
                burstCount = debug_burstNum;
            }

            if(lbhGenerated == LBH_FALSE_E)
            {
                /* we want to send traffic that will go to single port in trunk */

                switch(desigMode)
                {
                    case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_E:
                        burstCount = 1;
                        break;
                    case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_INGRESS_HASH_AND_VID_E:
                        burstCount = 1;
                        numVlans = 1;
                        break;
                    case PRV_TGF_TRUNK_DESIGNATED_TABLE_USE_SOURCE_INFO_E:
                        numberOfPortsSending = 1;
                        numVlans = 1;
                        break;
                    default:
                        break;
                }

                lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;

                multiDestType  = PRV_TGF_MULTI_DESTINATION_TYPE_BC_E;
                multiDestCount = 1;
            }
            else
            {
                multiDestType = PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E;
                multiDestCount =  MULTI_DEST_COUNT_MAC;
            }

            if((prvUtfIsGmCompilation() == GT_TRUE))
            {
                if(lbhGenerated == LBH_FALSE_E)
                {
                    burstCount = BURST_COUNT_CNS;  /*30*/
                }
                else
                {
                    burstCount = CRC_HASH_BURST_COUNT_NEW_CNS / 2;/*64*/
                }

                multiDestCount = 1;
                multiDestType = PRV_TGF_MULTI_DESTINATION_TYPE_MC_E;

                numberOfPortsSending = 1;
                numVlans = 1;
            }

            utfGeneralStateMessageSave(3,"lbhMode = %d %s",lbhMode,
                TO_STRING_LBH_MODE_MAC(lbhMode)
                );

            for(portIter = 0 ; portIter < numberOfPortsSending; portIter++)
            {
                devNum  = prvTgfDevsArray [portIter];
                portNum = prvTgfPortsArray[portIter];

                for(ii = 0 ; ii < numVlans ; ii++)
                {
                    /* modify the vid part */
                    prvTgfPacketVlanTagPart.vid = (TGF_VLAN_ID)prvTgfVlansArr[ii];

                    /* send the multi-destination traffic */
                    /* NOTE: this function sent 4 types of traffic with the burst count !!! */
                    prvTgfCommonMultiDestinationTrafficTypeSend(devNum, portNum, burstCount ,
                                                                GT_FALSE, trafficPtr , multiDestType);

                }
            }

            sendingPortsBmp = (1 << numberOfPortsSending) - 1;

            /* check the LBH and flooding */
            trunkSendCheckLbh(GT_FALSE,sendingPortsBmp,burstCount * numVlans * multiDestCount,lbhMode,NULL);

            IF_DEBUG_MODE_BREAK_MAC(debugMode);
        }
        IF_DEBUG_MODE_BREAK_MAC(debugMode);

        if((prvUtfIsGmCompilation() == GT_TRUE))
        {
            /* reduce iterations */
            break;
        }
    }

}

/**
* @internal tgfTrunkDebugDesignatedTable function
* @endinternal
*
* @brief   debug function to run test of the designated table for specific input
*/
GT_STATUS tgfTrunkDebugDesignatedTable
(
    IN GT_U32   desigMode,
    IN GT_U32   lbhGenerated
)
{
    debugMode = 1;
    debug_desigMode       = desigMode       ;
    debug_lbhGenerated    = lbhGenerated    ;

    return GT_OK;
}

/**
* @internal tgfTrunkMcLocalSwitchingTestInit function
* @endinternal
*
* @brief   init for trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkMcLocalSwitchingTestInit
(
    void
)
{
    prvTgfVlansNum = 2;

    prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_MC_LOCAL_SWITCHING_E,GT_FALSE,
                           TRUNK_8_PORTS_IN_TEST_CNS );

    /* flush the FDB because we test 'Multi-destination traffic' */
    /*prvTgfTrunkFdBFlush(); --> fdb should be empty */
}


/**
* @internal tgfTrunkMcLocalSwitchingTestRestore function
* @endinternal
*
* @brief   restore values after trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkMcLocalSwitchingTestRestore
(
    void
)
{
    GT_U32  portIter;/* port iterator */
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* DO NOT allow multi-destination to flood back - on the port */
    /* need to be called before prvTgfTrunkGenericRestore that modify prvTgfPortsArray[...]*/
    portIter = NOT_IN_TRUNK_PORTS_NUM_CNS;
    prvTgfBrgPortEgressMcLocalEnable(prvTgfDevsArray [portIter],
                                      prvTgfPortsArray[portIter],
                                      GT_FALSE);

    prvTgfTrunkGenericRestore(GT_FALSE);

    /* DO NOT allow multi-destination to flood back - on the trunk */
    prvTgfTrunkMcLocalSwitchingEnableSet(trunkId,GT_FALSE);

    /* state that storming NOT expected */
    tgfTrafficGeneratorStormingExpected(GT_FALSE);

}

/**
* @internal mcLocalSwitchingSendAndCheck function
* @endinternal
*
* @brief   the function sends multi-destination packets from all port of the trunk ,
*         and check that the trunk get/not get packets flooded back to the trunk.
*         the function also check that all other ports that are not in trunk will get
*         the flooding
*         applicable devices: ALL DxCh devices
* @param[in] lastPortIndex            - index of last port in trunk
* @param[in] lbhMode                  - the mode of expected load balance .
*                                      all traffic expected to be received from single port or
*                                      not received at all.
*                                       None
*/
static void mcLocalSwitchingSendAndCheck
(
    IN GT_U32  lastPortIndex,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode
)
{
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;/*traffic type*/
    GT_U8   devNum;/*device number to send packets */
    GT_PHYSICAL_PORT_NUM portNum;/*port number to send packets */
    GT_U32  portIter;/* port iterator */
    GT_U32               burstCount;/* burst count */
    TGF_PACKET_STC      *trafficPtr;/* pointer to traffic to sent */
    GT_U32  multiDestCount;/* number of destination traffics */
    GT_U32      ii,jj;
    GT_U32      maxPackets;
    GT_TRUNK_ID             trunkId;
    PRV_TGF_MULTI_DESTINATION_TYPE_ENT  multiDestType;/*multi destination traffic type*/
    CPSS_TRUNK_MEMBER_STC trunkMemberSender;/*indicates that the traffic to check
                   LBH that egress the trunk was originally INGRESSED from the trunk.
                   but since in the 'enhanced UT' the CPU send traffic to a port
                   due to loopback it returns to it, we need to ensure that the
                   member mentioned here should get the traffic since it is the
                   'original sender'*/

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    trafficType = PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E;
    trafficPtr = prvTgfTrunkTrafficTypes[trafficType];

    burstCount = 1;/* single packet -- so there is no LBH back into the trunk */
    maxPackets = 3;/* number of times to send the same packet */

    for(jj = 0 ; jj < prvTgfVlansNum ; jj++)
    {
        /* modify the vid part */
        prvTgfPacketVlanTagPart.vid = (TGF_VLAN_ID)prvTgfVlansArr[jj];
        /* send all types of multi-destination */
        for(multiDestType = 0 ; multiDestType < PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E ;
            multiDestType +=2)/* reduce time of test*/
        {
            for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter <= lastPortIndex ;
                portIter+=2)/* reduce time of test*/
            {
                devNum  = prvTgfDevsArray [portIter];
                portNum = prvTgfPortsArray[portIter];

                /* clear all counters before sending traffic */
                clearAllTestedCounters();

                for(ii = 0 ; ii < maxPackets ; ii++)
                {
                   /* send the multi-destination traffic */
                    prvTgfCommonMultiDestinationTrafficTypeSend(devNum, portNum, burstCount ,
                                                                GT_FALSE, trafficPtr , multiDestType);
                }

                /* the test result in loops on send ports.
                  wait for processing of all packets after loop break. */
                (void)tgfTrafficGeneratorCheckProcessignDone(GT_FALSE, 0);

                multiDestCount = 1;

                /* state that the traffic ingress from the trunk (but for that we send
                   it from CPU , to egress from this port) */
                trunkMemberSender.port = portNum;
                trunkMemberSender.hwDevice = devNum;

                /* check the LBH and flooding / no flooding */
                trunkSendCheckLbh(GT_FALSE,/* singleDestination -->  Multi-destination */
                        1,/*sendingPortsBmp*/
                        burstCount * maxPackets * multiDestCount,/*burstCount*/
                        lbhMode,
                        &trunkMemberSender);/*trunkMemberSenderPtr*/
            }
        }
    }
}

/**
* @internal basicTrunkMcLocalSwitchingTest function
* @endinternal
*
* @brief   The trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*         The test:
*         ========== section 1 ===========
*         a. set trunk with 2 ports
*         b. send flooding from it  --> check no flood back
*         c. 'disable filter' on the trunk
*         ========== section 2 ===========
*         d. send flooding from it  --> check flood back to only one of the 2 ports of the trunk
*         e. add port to the trunk
*         f. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         g. 'enable filter' on the trunk
*         h. send flooding from it  --> check no flood back
*         i. 'disable filter' on the trunk
*         j. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         k. remove port from the trunk
*         l. send flooding from it  --> check flood back to only one of the 2 ports of the trunk, check that the removed port get flood.
* @param[in] sectionNumber            - section number:
*                                      section 1 : a..c
*                                      section 2 : d..l
* @param[in] clearTheTrunk            - do we need to clear the trunk.
*                                      note : on section 1 we clear the trunk anyway.
*                                       None
*/
static void basicTrunkMcLocalSwitchingTest
(
    GT_U32      sectionNumber,
    GT_BOOL     clearTheTrunk
)
{
    GT_STATUS rc;
    GT_U32  portIter;/* port iterator */
    GT_U32  lastPortIndex;/* index of last port in trunk */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;
    GT_U32  numMembers;/* number of current trunk members */
    CPSS_TRUNK_MEMBER_STC member;/*trunk member*/
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    PRV_UTF_LOG3_MAC("start section [%d] , clearTheTrunk[%d] : clear the trunk [%d] \n",
        sectionNumber ,
        clearTheTrunk ? 1 : 0,
        trunkId);


    /* work with 3 members in the trunk */
    numMembers = 3;
    lastPortIndex = NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers - 1;

    if(clearTheTrunk == GT_TRUE || sectionNumber == 1)
    {
        PRV_UTF_LOG1_MAC("clear the trunk [%d] \n",trunkId);
        /* clear the trunk */
        rc = prvTgfTrunkMembersSet(
            trunkId /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* a. add 3 members to the trunk */
        numMembers = 3;
        PRV_UTF_LOG1_MAC("a. add 3 members to the trunk [%d]--> \n",trunkId);

        for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS;
            portIter < (NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers) ;
            portIter++)
        {
            member.hwDevice = prvTgfDevsArray [portIter];
            member.port   = prvTgfPortsArray[portIter];

            rc = prvTgfTrunkMemberAdd(trunkId,&member);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        if(sectionNumber == 1)
        {
            PRV_UTF_LOG1_MAC("section 1 trunk [%d] \n",trunkId);

            lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E;

            PRV_UTF_LOG1_MAC("b. send traffic from the trunk [%d]--> check no flood back to the trunk \n",trunkId);
            /* b. send traffic from the trunk --> check no flood back to the trunk */
            mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

            PRV_UTF_LOG1_MAC("c. allow multi-destination to flood back. trunk [%d] \n",trunkId);
            /* c. allow multi-destination to flood back */
            prvTgfTrunkMcLocalSwitchingEnableSet(trunkId,GT_TRUE);

            PRV_UTF_LOG1_MAC("state that storming expected. trunk [%d] \n",trunkId);
            /* state that storming expected */
            tgfTrafficGeneratorStormingExpected(GT_TRUE);

            /* section 1 -- done*/
            PRV_UTF_LOG3_MAC("done section [%d] , clearTheTrunk[%d] : clear the trunk [%d] \n",
                sectionNumber ,
                clearTheTrunk ? 1 : 0,
                trunkId);
            return;
        }
    }

    PRV_UTF_LOG1_MAC("section 2 trunk [%d] \n",trunkId);
    /* section 2 */
    portIter = (NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers);

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
    /* d. send traffic from the trunk --> check flood back to the trunk --> single port */
    PRV_UTF_LOG1_MAC("d. send traffic from the trunk --> check flood back to the trunk --> single port. trunk [%d] \n",trunkId);
    mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

    /* e. add port to trunk */
    member.hwDevice = prvTgfDevsArray [portIter];
    member.port   = prvTgfPortsArray[portIter];
    PRV_UTF_LOG2_MAC("e. add port [%d] to trunk. trunk [%d] \n",member.port,trunkId);

    rc = prvTgfTrunkMemberAdd(trunkId,&member);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numMembers ++;
    lastPortIndex = NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers - 1;

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
    PRV_UTF_LOG1_MAC("f. send traffic from the trunk --> check flood back to the trunk --> single port. trunk [%d] \n",trunkId);
    /* f. send traffic from the trunk --> check flood back to the trunk --> single port */
    mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

    PRV_UTF_LOG1_MAC("g. don't allow multi-destination to flood back. trunk [%d] \n",trunkId);
    /* g. don't allow multi-destination to flood back */
    prvTgfTrunkMcLocalSwitchingEnableSet(trunkId,GT_FALSE);

    PRV_UTF_LOG1_MAC("state that storming NOT expected . trunk [%d] \n",trunkId);
    /* state that storming NOT expected */
    tgfTrafficGeneratorStormingExpected(GT_FALSE);

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E;
    PRV_UTF_LOG1_MAC("h. send traffic from the trunk --> check no flood back to the trunk . trunk [%d] \n",trunkId);
    /* h. send traffic from the trunk --> check no flood back to the trunk */
    mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

    PRV_UTF_LOG1_MAC("i. allow multi-destination to flood back . trunk [%d] \n",trunkId);
    /* i. allow multi-destination to flood back */
    prvTgfTrunkMcLocalSwitchingEnableSet(trunkId,GT_TRUE);

    /* state that storming expected */
    PRV_UTF_LOG1_MAC("state that storming expected . trunk [%d] \n",trunkId);
    tgfTrafficGeneratorStormingExpected(GT_TRUE);

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
    PRV_UTF_LOG1_MAC("j. send traffic from the trunk --> check flood back to the trunk --> single port . trunk [%d] \n",trunkId);
    /* j. send traffic from the trunk --> check flood back to the trunk --> single port */
    mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);

    /* k. remove the last port from the trunk*/
    PRV_UTF_LOG1_MAC("k. remove the last port from the trunk. trunk [%d] \n",trunkId);
    rc = prvTgfTrunkMemberRemove(trunkId,&member);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    numMembers --;
    lastPortIndex = NOT_IN_TRUNK_PORTS_NUM_CNS + numMembers - 1;

    /* l. send traffic from the trunk --> check flood back to the trunk --> single port */
    PRV_UTF_LOG1_MAC("l. send traffic from the trunk --> check flood back to the trunk --> single port. trunk [%d] \n",trunkId);
    mcLocalSwitchingSendAndCheck(lastPortIndex,lbhMode);


    PRV_UTF_LOG3_MAC("done section [%d] , clearTheTrunk[%d] : clear the trunk [%d] \n",
        sectionNumber ,
        clearTheTrunk ? 1 : 0,
        trunkId);
}
/**
* @internal tgfTrunkMcLocalSwitchingTest function
* @endinternal
*
* @brief   The trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*         The test:
*         ========== section 1 ===========
*         a. set trunk with 2 ports
*         b. send flooding from it  --> check no flood back
*         c. 'disable filter' on the trunk
*         ========== section 2 ===========
*         d. send flooding from it  --> check flood back to only one of the 2 ports of the trunk
*         e. add port to the trunk
*         f. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         g. 'enable filter' on the trunk
*         h. send flooding from it  --> check no flood back
*         i. 'disable filter' on the trunk
*         j. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         k. remove port from the trunk
*         l. send flooding from it  --> check flood back to only one of the 2 ports of the trunk, check that the removed port get flood.
*         ========== permutations ===========
*         m. remove all ports from trunk and than add 2 or 3 ports, back to existing test phases from <d>
*         n. Add manipulation of prvTgfBrgPortEgressMcastLocalEnable() > prvTgfBrgPortEgressMcLocalEnable()
*/
void tgfTrunkMcLocalSwitchingTest
(
    void
)
{
    GT_STATUS rc;
    GT_U32  portIter;/* port iterator */
    GT_TRUNK_ID             trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* run section 1 (a..c) */
    basicTrunkMcLocalSwitchingTest(1,GT_TRUE);
    /* run section 2 (d..l) */
    basicTrunkMcLocalSwitchingTest(2,GT_FALSE);

    /* m. remove all ports from trunk and than add 2 or 3 ports, run section 2 again . */
    /* motivation of this section is to check that if we remove all ports from
       the trunk , still the trunk keeps the settings of 'multi-destination local switching enable' settings

       and at this point the trunk is set as <multi-destination local switching enable> = GT_TRUE
    */

    /* run section 2 (d..l) starting with clearing the trunk and add members */
    basicTrunkMcLocalSwitchingTest(2,GT_TRUE);

    /* n. Add manipulation of prvTgfBrgPortEgressMcastLocalEnable() ***> prvTgfBrgPortEgressMcLocalEnable()*/
    /* enable switch back on PORT that is member of the trunk  */
    portIter = NOT_IN_TRUNK_PORTS_NUM_CNS;
    rc = prvTgfBrgPortEgressMcLocalEnable(prvTgfDevsArray [portIter],
                                          prvTgfPortsArray[portIter],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* o. DONT allow multi-destination to flood back */
    prvTgfTrunkMcLocalSwitchingEnableSet(trunkId,GT_FALSE);

    /* state that storming NOT expected */
    tgfTrafficGeneratorStormingExpected(GT_FALSE);

    /* p. run section 1 (a..c) --> check that no flood back to the trunk */
    basicTrunkMcLocalSwitchingTest(1,GT_TRUE);


}

/* debug function to change port numbers for trunk UTs */
GT_STATUS prvUtfTrunkTestPortSet
(
    IN GT_U32      numPorts,
    IN GT_BOOL     isCrcOrForcePorts,
    IN GT_U32 index,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_PHYSICAL_PORT_NUM    *portsArrayPtr;

    if (index >= TRUNK_MAX_PORTS_IN_TEST_CNS)
    {
        return GT_FAIL;
    }

    if (numPorts <= 28)
    {
        if (isCrcOrForcePorts == 0)
        {
            portsArrayPtr = &testPortsFor28Ports[0];
        }
        else
        {
            portsArrayPtr = &crcTestPortsFor28Ports[0];
        }
    }
    else if(numPorts <= 64)
    {
        if (isCrcOrForcePorts == 0)
        {
            if (prvTgfXcat3xExists() == GT_TRUE)
            {
                portsArrayPtr = &testPorts_xCat3x[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) /* bobcat3 SPECIFIC limited PORTS*/
            {
                portsArrayPtr = &testPorts_bobcat3[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) /* SPECIFIC limited PORTS*/
            {
                portsArrayPtr = &testPorts_falcon[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                portsArrayPtr = &testPorts_aldrin2[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) /* aldrin SPECIFIC limited PORTS*/
            {
                portsArrayPtr = &testPorts_aldrin[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) /* AC3X SPECIFIC limited PORTS*/
            {
                portsArrayPtr = &testPorts_xCat3x[0];
            }
            else
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS) /* aldrin Z0 SPECIFIC limited PORTS*/
            {
                portsArrayPtr = &testPorts_aldrinZ0[0];
            }
            else
            if(IS_BOBK_DEV_MAC(prvTgfDevNum))/* BOBK SPECIFIC limited PORTS*/
            {
                portsArrayPtr = IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum) ?
                                    &testPortsFor72Ports_bobk_cetus[0] :
                                    &testPortsFor72Ports_bobk[0] ;
            }
            else
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
            {
                /*BobCat2*/
                portsArrayPtr = &testPortsFor72Ports[0];
            }
            else
            {
                portsArrayPtr = &testPortsFor60Ports[0];
            }
        }
        else
        {
            portsArrayPtr = &testPortsFor60PortsConfigForced[0];
        }
    }
    else if(numPorts <= 124)
    {
        if (isCrcOrForcePorts == 0)
        {
            portsArrayPtr = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                            &testPortsFor124Ports_XLG[0]: &testPortsFor124Ports[0];
        }
        else
        {
            portsArrayPtr = &testPortsFor124PortsConfigForced[0];
        }
    }
    else
    {
        return GT_BAD_PARAM;
    }

    portsArrayPtr[index] = portNum;


    return GT_OK;
}


/**
* @internal tgfTrunkBasicIpv4UcRoutingTestInit function
* @endinternal
*
* @brief   Init for IPv4 routing with trunk(s)
*         applicable devices: ALL
* @param[in] senderIsTrunk            - send is trunk ?
*                                      GT_FALSE - the sender is port (index 0)
*                                      GT_FALSE - the sender is trunk (trunk A)
* @param[in] nextHopIsTrunk           - next hop is trunk ?
*                                      GT_FALSE - the NH is port (index 0)
*                                      GT_FALSE - the NH is trunk (trunk B)
* @param[in] useSameTrunk             - trunk A and trunk B are the same trunk
*                                      relevant when   senderIsTrunk = GT_TRUE and nextHopIsTrunk = GT_TRUE
*                                       None
*/
void tgfTrunkBasicIpv4UcRoutingTestInit
(
    GT_BOOL     senderIsTrunk,
    GT_BOOL     nextHopIsTrunk,
    GT_BOOL     useSameTrunk
)
{
    GT_STATUS                      rc;
    static CPSS_TRUNK_MEMBER_STC          enabledMembersArray[4];/*trunk members*/
    GT_TRUNK_ID             trunkId;/*trunk Id*/
    GT_BOOL         useSingleTrunk;/*check if trunk A = trunk B*/
    GT_U32          ii;/*iterator*/

    initTrunkIds(GT_FALSE);

    for(ii = 0 ; ii < 4 ; ii++)
    {
        enabledMembersArray[ii].hwDevice = prvTgfDevsArray[ii];
        enabledMembersArray[ii].port   = prvTgfPortsArray[ii];
    }

    if(useSameTrunk == GT_TRUE &&
       senderIsTrunk == GT_TRUE &&
       nextHopIsTrunk == GT_TRUE)
    {
        useSingleTrunk = GT_TRUE;
    }
    else
    {
        useSingleTrunk = GT_FALSE;
    }

    /* save old mode */
    rc = prvTgfTrunkHashIpAddMacModeGet(&prvTgfTrunkHashIpAddMacMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set new mode */
    rc = prvTgfTrunkHashIpAddMacModeSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(senderIsTrunk == GT_TRUE)
    {
        trunkId = PRV_TGF_TRUNK_ID_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        rc = prvTgfTrunkMembersSet(
            trunkId /*trunkId*/,
            (useSingleTrunk == GT_TRUE) ? 4 : 2 /*numOfEnabledMembers*/,
            &enabledMembersArray[0],
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    if(useSingleTrunk == GT_TRUE)
    {
        return;
    }

    if(nextHopIsTrunk == GT_TRUE)
    {
        trunkId = PRV_TGF_TRUNK_ID_2_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        rc = prvTgfTrunkMembersSet(
            trunkId /*trunkId*/,
            2 /*numOfEnabledMembers*/,
            &enabledMembersArray[2],
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    return;
}
/**
* @internal tgfTrunkBasicIpv4UcRoutingTestRestore function
* @endinternal
*
* @brief   restore trunk configuration relate to IPv4 routing with trunk(s)
*         applicable devices: ALL
*/
void tgfTrunkBasicIpv4UcRoutingTestRestore
(
    void
)
{
    GT_STATUS                      rc;
    GT_TRUNK_ID             trunkId;/*trunk Id*/

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /*restore old mode*/
    rc = prvTgfTrunkHashIpAddMacModeSet(prvTgfTrunkHashIpAddMacMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        NULL,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        0 /*numOfEnabledMembers*/,
        NULL,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}


/**
* @internal prvTgfTrunkSimpleHashMacSaDaTestInit function
* @endinternal
*
* @brief   Init for Simple hash for mac SA,DA LBH
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTestInit
(
    void
)
{
    GT_STATUS   rc;/*return code*/
    GT_U32   numOfPortsInTest;/* number of ports in the test */
    /* we have at least 4 non-trunk members , and up to 'max' trunk members */

    prvTgfVlansNum  = 1;
    numOfPortsInTest = TRUNK_MAX_NUM_OF_MEMBERS_CNS + NOT_IN_TRUNK_PORTS_NUM_CNS;

    prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_SIMPLE_HASH_E,GT_FALSE,
                           numOfPortsInTest );

    /* save old mode */
    rc = prvTgfTrunkHashIpAddMacModeGet(&prvTgfTrunkHashIpAddMacMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set new mode */
    rc = prvTgfTrunkHashIpAddMacModeSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal prvTgfTrunkSimpleHashMacSaDaTestRestore function
* @endinternal
*
* @brief   restore for Simple hash feature test
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTestRestore
(
    void
)
{
    GT_STATUS   rc;/*return code*/

    prvTgfTrunkGenericRestore(GT_FALSE);
    prvTgfVlansNum  = 2;

    /*restore old mode*/
    rc = prvTgfTrunkHashIpAddMacModeSet(prvTgfTrunkHashIpAddMacMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal simpleHashTest function
* @endinternal
*
* @brief   check Simple hash for Mac SA DA.
*         2.1.1.1    Mac DA LBH
*         1.    Define trunk-Id 255 with 12 members.
*         2.    Send 1K incremental macs for 'learning' from one of the trunk ports (mac A , mac A+1 ...)
*         3.    from port that is not in trunk , send 1K packets with incremental DA (constant SA) that associated with the trunk (DA = incremental mac A , mac A+1 ...)
*         a.    do the same from 3 other ports that not in trunk
*         4.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         5.    check that ports that are not in the trunk got no packets (no flooding)
*         6.    If we left with 1 port in the trunk ' Stop test.
*         7.    Remove one port from the trunk.
*         8.    go to #3
*         2.1.1.2    Mac SA LBH
*         9.    Define trunk-Id 255 with 12 members.
*         10.    Send 1 mac for 'learning' from one of the trunk ports (mac A)
*         11.    from port that is not in trunk , send 1K packets with constant DA and incremental SA that associated with the trunk (DA = constant mac A)
*         a.    do the same from 3 other ports that not in trunk
*         12.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         13.    check that ports that are not in the trunk got no packets (no flooding)
*         14.    If we left with 1 port in the trunk ' Stop test.
*         15.    Remove one port from the trunk.
*         16.    go to #3
*         applicable devices: All DX , All Puma
* @param[in] testedField              - tested field (SA/DA)
*                                       None
*/
static void simpleHashTest
(
    IN PRV_TGF_TRUNK_HASH_TEST_PACKET_ENT  testedField
)
{
    GT_U32               burstCount;/* burst count */
    GT_U32               numVfd = 1;/* number of VFDs in vfdArray */
    static TGF_VFD_INFO_STC     vfdArray[1] = {{0}};/* vfd Array -- used for increment the tested bytes , and for vlan tag changing */
    TGF_VFD_INFO_STC     *vrfPtr = &vfdArray[0];
    GT_U32  ii;
    GT_U32  portIter;/* port iterator */
    GT_U32  numberOfPortsSending;/* number of ports sending traffic */
    GT_U32  sendingPortsBmp;/* bitmap of ports that send bursts */
    GT_U32  numVlans;/* number of vlans to test */
    TGF_PACKET_STC *trafficPtr;/*traffic to send*/
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;

    switch(testedField)
    {
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E:
            vrfPtr->offset = 11;/* LSB of mac SA */
            break;
        case PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E:
            vrfPtr->offset = 5;/* LSB of mac DA */
            break;
        default:
            TRUNK_PRV_UTF_LOG0_MAC(
                "simpleHashTest bad pktType\n");
            return;
    }

    lbhMode = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;

    /* 4 ports will send the traffic */
    numberOfPortsSending = NOT_IN_TRUNK_PORTS_NUM_CNS;

    numVlans = prvTgfVlansNum;

    /* according to the FDB learned MACs */
    burstCount = FDB_MAC_COUNT_CNS;

    if(sortMode)
    {
        numberOfPortsSending = 1;
        numVlans = 1;
    }

    vrfPtr->modeExtraInfo = 0;
    cpssOsMemSet(vrfPtr->patternPtr,0,sizeof(TGF_MAC_ADDR));
    vrfPtr->patternPtr[0] = 0;
    vrfPtr->cycleCount = 1;/*single byte*/
    vrfPtr->mode = TGF_VFD_MODE_INCREMENT_E;

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        /* the GM is too lazy that can't let it do all permutations .*/
        numVlans = 1;
        portIter = numberOfPortsSending - 1;/* start from last port .. so do only one iteration */
    }
    else
    {
        portIter = 0;
    }

    for(/*already initialized */ ; portIter < numberOfPortsSending; portIter++)
    {
        /* each port will sent different type of flow
           (L3 flow --> but the same L2 flow) */
        trafficPtr = packetsForSimpleHashMacSaDaArr[portIter];

        for(ii = 0 ; ii < numVlans ; ii++)
        {
            /* modify the vid part */
            prvTgfPacketVlanTagPart.vid = (TGF_VLAN_ID)prvTgfVlansArr[ii];

            vrfPtr->modeExtraInfo = 0;
            /* send the burst of packets with incremental byte */
            prvTgfTrunkTestPacketSend(prvTgfDevsArray [portIter], prvTgfPortsArray[portIter],
                trafficPtr ,
                burstCount ,numVfd ,vfdArray);

            /*bitmap of single sender*/
            sendingPortsBmp = (1 << portIter);

            /* check the LBH and no flooding */
            trunkSendCheckLbh(GT_TRUE,sendingPortsBmp,burstCount ,lbhMode,NULL);
        }
    }

}


/**
* @internal prvTgfTrunkSimpleHashMacSaDaTest function
* @endinternal
*
* @brief   check Simple hash for Mac SA DA.
*         2.1.1.1    Mac DA LBH
*         1.    Define trunk-Id 255 with 12 members.
*         2.    Send 1K incremental macs for 'learning' from one of the trunk ports (mac A , mac A+1 ...)
*         3.    from port that is not in trunk , send 1K packets with incremental DA (constant SA) that associated with the trunk (DA = incremental mac A , mac A+1 ...)
*         a.    do the same from 3 other ports that not in trunk
*         4.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         5.    check that ports that are not in the trunk got no packets (no flooding)
*         6.    If we left with 1 port in the trunk ' Stop test.
*         7.    Remove one port from the trunk.
*         8.    go to #3
*         2.1.1.2    Mac SA LBH
*         9.    Define trunk-Id 255 with 12 members.
*         10.    Send 1 mac for 'learning' from one of the trunk ports (mac A)
*         11.    from port that is not in trunk , send 1K packets with constant DA and incremental SA that associated with the trunk (DA = constant mac A)
*         a.    do the same from 3 other ports that not in trunk
*         12.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         13.    check that ports that are not in the trunk got no packets (no flooding)
*         14.    If we left with 1 port in the trunk ' Stop test.
*         15.    Remove one port from the trunk.
*         16.    go to #3
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTest
(
    void
)
{
    GT_STATUS   rc;/*return code*/
    GT_U32  numPortsInTrunk;/* current number of port in the trunk*/
    GT_U32  lastPortIndex;/* index of last port in trunk */
    CPSS_TRUNK_MEMBER_STC member;/*trunk member*/
    GT_TRUNK_ID           trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* index of the last port in the trunk */
    lastPortIndex = prvTgfPortsNum - 1;

    /* remove port from the trunk on every loop */
    for(numPortsInTrunk = TRUNK_MAX_NUM_OF_MEMBERS_CNS ;
        numPortsInTrunk;
        numPortsInTrunk--,lastPortIndex--)
    {
        utfGeneralStateMessageSave(1,"numPortsInTrunk [%d]",numPortsInTrunk);

        /*check DA hash with this number of trunk members */
        utfGeneralStateMessageSave(0,"simple hash [mac DA]");
        simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E);

        if(prvUtfIsGmCompilation() == GT_TRUE)
        {
            /* the GM is so lazy that can't let it do all permutations .
            (without the 'break') in win32 it takes 1300 seconds (1100 in linux GM)*/

            break;
        }

        /*check SA hash with this number of trunk members */
        utfGeneralStateMessageSave(0,"simple hash [mac SA]");
        simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_SA_E);

        /* member to remove */
        member.hwDevice = prvTgfDevsArray [lastPortIndex];
        member.port   = prvTgfPortsArray[lastPortIndex];

        /* remove port from the trunk */
        rc = prvTgfTrunkMemberRemove(trunkId,&member);
        if (GT_OK != rc)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        if(tgfTrunkGlobalEPortRepresentTrunk &&
           globalEPortRepresentTrunk_sortingEnabled == GT_TRUE)
        {
            associateIngressPortWithEPortInsteadOfTrunkId_fullTrunk(trunkId);
        }

        if(prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE)
        {
            /* allow only single iteration for 'Sanity' check of the simple LBH */
            break;
        }
    }
}

/**
* @internal prvTgfTrunkWithWeightedMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the trunk with the Weighted specified members
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'trunk table' (regular trunks) and in 'designated table'(regular trunks and cascade trunks) .
*         The trunk may be invalidated by numOfMembers = 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkWithWeightedMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    GT_STATUS           rc;
    CPSS_TRUNK_TYPE_ENT trunkType;/* trunk type*/

    rc = prvCpssGenericTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvCpssGenericTrunkDbTrunkTypeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if(trunkType == CPSS_TRUNK_TYPE_REGULAR_E)
    {
        return GT_NOT_SUPPORTED;
    }

    return prvTgfTrunkCascadeTrunkWithWeightedPortsSet(devNum,trunkId,numOfMembers,weightedMembersArray);
}

/**
* @internal prvTgfTrunkWithWeightedMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted members of trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'trunk table' (regular trunks) and in 'designated table'(regular trunks and cascade trunks) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkWithWeightedMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
)
{
    GT_STATUS           rc;
    CPSS_TRUNK_TYPE_ENT trunkType;/* trunk type*/

    rc = prvCpssGenericTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvCpssGenericTrunkDbTrunkTypeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if(trunkType == CPSS_TRUNK_TYPE_REGULAR_E)
    {
        return GT_NOT_SUPPORTED;
    }

    return prvTgfTrunkCascadeTrunkWithWeightedPortsGet(devNum,trunkId,numOfMembersPtr,weightedMembersArray);

}

static void checkTrunkSort(GT_TRUNK_ID trunkId)
{
    GT_STATUS   rc;
    GT_U32                  numOfMembers;
    static CPSS_TRUNK_MEMBER_STC   membersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32  ii;

    /* check sort of enabled members */
    numOfMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    rc = prvCpssGenericTrunkDbEnabledMembersGet(prvTgfDevNum,trunkId,&numOfMembers,membersArray);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(numOfMembers)
    {
        for(ii = 1 ; ii < (numOfMembers - 1); ii++)
        {
            if((membersArray[ii].hwDevice > membersArray[ii+1].hwDevice) ||
                (membersArray[ii].hwDevice == membersArray[ii+1].hwDevice &&
                 membersArray[ii].port > membersArray[ii+1].port))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE," enabled members of the trunk not sorted \n");
            }
        }
    }

    /* check sort of disabled members */
    numOfMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    rc = prvCpssGenericTrunkDbDisabledMembersGet(prvTgfDevNum,trunkId,&numOfMembers,membersArray);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(numOfMembers)
    {
        for(ii = 1 ; ii < (numOfMembers - 1); ii++)
        {
            if((membersArray[ii].hwDevice > membersArray[ii+1].hwDevice) ||
                (membersArray[ii].hwDevice == membersArray[ii+1].hwDevice &&
                 membersArray[ii].port > membersArray[ii+1].port))
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE," disabled members of the trunk not sorted \n");
            }
        }
    }

}


static GT_STATUS manuelTest_prvTgfTrunkMemberAdd(GT_U32  port)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkId;
    CPSS_TRUNK_MEMBER_STC member;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice =  prvTgfDevNum;
    member.port = port;

    PRV_UTF_LOG2_MAC("add member : {dev=%d , port=%d}  \n" ,
                    member.hwDevice ,
                    member.port);

    rc = prvTgfTrunkMemberAdd(trunkId, &member);
    if(sortMode == GT_TRUE)
    {
        /*after every action check that the trunk is sorted */
        checkTrunkSort(trunkId);
    }

    return rc;
}

static GT_STATUS manuelTest_prvTgfTrunkMemberRemove(GT_U32  port)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkId;
    CPSS_TRUNK_MEMBER_STC member;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.hwDevice =  prvTgfDevNum;
    member.port = port;

    PRV_UTF_LOG2_MAC("remove member : {dev=%d , port=%d}  \n" ,
                    member.hwDevice ,
                    member.port);

    rc = prvTgfTrunkMemberRemove(trunkId, &member);

    if(sortMode == GT_TRUE)
    {
        /*after every action check that the trunk is sorted */
        checkTrunkSort(trunkId);
    }

    return rc;
}

static GT_STATUS manuelTest_prvTgfTrunkMembersSet(
    IN GT_U32   numOfPorts,
    IN GT_U32   port_0,
    IN GT_U32   port_1,
    IN GT_U32   port_2,
    IN GT_U32   port_3,
    IN GT_U32   port_4,
    IN GT_U32   port_5,
    IN GT_U32   port_6,
    IN GT_U32   port_7
)
{
    GT_STATUS   rc;
    static CPSS_TRUNK_MEMBER_STC   membersArray[8];
    GT_U32                  ii;
    static GT_U32                  ports[8];
    GT_TRUNK_ID trunkId;

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    PRV_UTF_LOG0_MAC(
    "param 0 : numOfPorts \n"
    "param 1 : port_0 \n"
    "param 2 : port_1 \n"
    "... \n"
    "param  : port_7 \n"
    );

    if(numOfPorts > 8)
    {
        return GT_BAD_PARAM;
    }

    ii=0;
    ports[ii]   =    port_0;
    ii++;
    ports[ii]   =    port_1;
    ii++;
    ports[ii]   =    port_2;
    ii++;
    ports[ii]   =    port_3;
    ii++;
    ports[ii]   =    port_4;
    ii++;
    ports[ii]   =    port_5;
    ii++;
    ports[ii]   =    port_6;
    ii++;
    ports[ii]   =    port_7;
    ii++;

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        membersArray[ii].port = ports[ii];
        membersArray[ii].hwDevice = prvTgfDevNum;

        PRV_UTF_LOG2_MAC("set member : {dev=%d , port=%d}  \n" ,
                        membersArray[ii].hwDevice ,
                        membersArray[ii].port);
    }


    rc = prvTgfTrunkMembersSet(trunkId, numOfPorts, membersArray,
                                 0, NULL);

    if(sortMode == GT_TRUE)
    {
        /*after every action check that the trunk is sorted */
        checkTrunkSort(trunkId);
    }

    return rc;
}



static void manuelTest_prvTgfTrunkMembers_manipulations(
    IN GT_U32   numOfPorts,
    IN GT_U32   port_0,
    IN GT_U32   port_1,
    IN GT_U32   port_2,
    IN GT_U32   port_3,
    IN GT_U32   port_4,
    IN GT_U32   port_5,
    IN GT_U32   port_6,
    IN GT_U32   port_7
)
{
    GT_STATUS rc;
    GT_U32                  ii,jj;
    static GT_U32                  ports[8];

    ii=0;
    ports[ii]   =    port_0;
    ii++;
    ports[ii]   =    port_1;
    ii++;
    ports[ii]   =    port_2;
    ii++;
    ports[ii]   =    port_3;
    ii++;
    ports[ii]   =    port_4;
    ii++;
    ports[ii]   =    port_5;
    ii++;
    ports[ii]   =    port_6;
    ii++;
    ports[ii]   =    port_7;
    ii++;

    rc = manuelTest_prvTgfTrunkMembersSet(numOfPorts,
        port_0,
        port_1,
        port_2,
        port_3,
        port_4,
        port_5,
        port_6,
        port_7
        );
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return ;
    }

    prvTgfVlansNum = 1;

    for(ii = 0 ; ii < numOfPorts ; ii++)
    {
        rc = manuelTest_prvTgfTrunkMemberRemove(ports[ii]);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            return;
        }
        simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E);
        for(jj = 0 ; jj < numOfPorts ; jj++)
        {
            if(jj == ii)
            {
                continue;
            }
            rc = manuelTest_prvTgfTrunkMemberRemove(ports[jj]);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                return;
            }
            simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E);
            rc = manuelTest_prvTgfTrunkMemberAdd(ports[jj]);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                return;
            }
            simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E);
        }
        rc = manuelTest_prvTgfTrunkMemberAdd(ports[ii]);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            return;
        }
        simpleHashTest(PRV_TGF_TRUNK_HASH_TEST_PACKET_ETH_MAC_DA_E);

        if(prvUtfSkipLongTestsFlagGet(UTF_ALL_FAMILY_E) != GT_FALSE)
        {
            /* allow only single iteration for 'Sanity' check of the simple LBH */
            break;
        }
    }

    return ;

}

/**
* @internal tgfTrunkSortModeTest function
* @endinternal
*
* @brief   The sort mode test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkSortModeTest
(
    void
)
{
    GT_STATUS rc;

    sortMode = GT_TRUE;
    /* enable the sorting */
    rc = prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, sortMode);
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    PRV_UTF_LOG0_MAC("tgfTrunkSortModeTest : start \n");

    /* this test based on the 'DA hash' test */
    prvTgfTrunkSimpleHashMacSaDaTestInit();

    manuelTest_prvTgfTrunkMembers_manipulations(5,
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 0],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 1],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 2],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 3],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 4],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 5],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 6],
        prvTgfPortsArray[NOT_IN_TRUNK_PORTS_NUM_CNS + 7]
        );

    prvTgfTrunkSimpleHashMacSaDaTestRestore();

    sortMode = GT_FALSE;
    /* disable the sorting */
    prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, sortMode);

    PRV_UTF_LOG0_MAC("tgfTrunkSortModeTest : end \n");

    return ;
}

/* function for registration in the UT engine to be called on error .
*       callback function that need to be called on error .
*       this to allow a test to 'dump' into LOG/terminal important info that may
*       explain why the test failed. -- this is advanced debug tool
*/
/*
static void tgfTrunkDumpFunc
(
    void
)
{
    GT_U8   devNum = prvTgfDevNum;

    prvTgfTrunkTrunkDump(devNum);

    return;
}
*/


/**
* @internal trunkMembersStateMessageSave function
* @endinternal
*
* @brief   This function saves message about the trunk members.
*         the info is printed when test fail
* @param[in] levelIndexOfDebug        - index in the debug DB
* @param[in] trunkId                  - the trunk Id to save it's members info
*
* @retval GT_OK                    -  the info saved to DB
* @retval GT_BAD_PARAM             -  the index >= PRV_UTF_GENERAL_STATE_INFO_NUM_CNS
*/
static void trunkMembersStateMessageSave
(
    IN GT_U32   levelIndexOfDebug,
    IN GT_TRUNK_ID  trunkId
)
{
    GT_STATUS   rc;
    GT_U32                  numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    static CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32  ii;
    CPSS_TRUNK_TYPE_ENT trunkType;/* trunk type*/
    CPSS_PORTS_BMP_STC cascadeTrunkPorts;
    GT_U8   devNum = prvTgfDevNum;

    rc = prvCpssGenericTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    switch(trunkType)
    {
        case CPSS_TRUNK_TYPE_REGULAR_E:
        {
            for(ii = 0 ;ii < numOfEnabledMembers;ii++)
            {
                enabledMembersArray[ii].port = enabledMembersArray[ii].hwDevice = 0xFF;
            }

            rc = prvCpssGenericTrunkDbEnabledMembersGet(devNum,trunkId,&numOfEnabledMembers,enabledMembersArray);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            /* convert HW devNum to SW devNum */
            for (ii = 0; ii < numOfEnabledMembers; ii++)
            {
                rc = prvUtfSwFromHwDeviceNumberGet(enabledMembersArray[ii].hwDevice,&enabledMembersArray[ii].hwDevice);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            utfGeneralStateMessageSave(levelIndexOfDebug,"REGULAR trunk[%d] num members = %d , current members{port,dev}="
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            "{%d,%d},"
            ,trunkId
            ,numOfEnabledMembers
            ,enabledMembersArray[0].port  , enabledMembersArray[0].hwDevice
            ,enabledMembersArray[1].port  , enabledMembersArray[1].hwDevice
            ,enabledMembersArray[2].port  , enabledMembersArray[2].hwDevice
            ,enabledMembersArray[3].port  , enabledMembersArray[3].hwDevice
            ,enabledMembersArray[4].port  , enabledMembersArray[4].hwDevice
            ,enabledMembersArray[5].port  , enabledMembersArray[5].hwDevice
            ,enabledMembersArray[6].port  , enabledMembersArray[6].hwDevice
            ,enabledMembersArray[7].port  , enabledMembersArray[7].hwDevice
            ,enabledMembersArray[8].port  , enabledMembersArray[8].hwDevice
            ,enabledMembersArray[9].port  , enabledMembersArray[9].hwDevice
            ,enabledMembersArray[10].port , enabledMembersArray[10].hwDevice
            ,enabledMembersArray[11].port , enabledMembersArray[11].hwDevice
            );
        }
        break;

        case CPSS_TRUNK_TYPE_CASCADE_E:
        {
            rc = prvCpssGenericTrunkCascadeTrunkPortsGet(devNum,trunkId,&cascadeTrunkPorts);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            utfGeneralStateMessageSave(levelIndexOfDebug,"CASCADE trunk[%d] ports bmp="
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            "0x%8.8x, "
            ,trunkId
            ,cascadeTrunkPorts.ports[0]
            ,cascadeTrunkPorts.ports[1]
            ,cascadeTrunkPorts.ports[2]
            ,cascadeTrunkPorts.ports[3]
            ,cascadeTrunkPorts.ports[4]
            ,cascadeTrunkPorts.ports[5]
            ,cascadeTrunkPorts.ports[6]
            ,cascadeTrunkPorts.ports[7]
            );
        }
        break;

        case CPSS_TRUNK_TYPE_FREE_E:
            utfGeneralStateMessageSave(levelIndexOfDebug,"trunk[%d] is free",trunkId);
        break;

        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_NOT_SUPPORTED);
    }
}

/*  Fill the array of tgfTrunkNonUsedPorts  */
static GT_STATUS tgfTrunkNonUsedPortsFill
(
    void
)
{
    GT_U32  ii,jj , portIter, localPort;
    GT_U32  currNonUsedPortsIndex = 0;
    PRV_TGF_MEMBER_FORCE_INFO_STC *currMemPtr;
    GT_U8 devNum = prvTgfDevNum;

    if(prvTgfXcat3xExists() == GT_TRUE)
    {
        /* GT_U32   myarr[5]={0,4,11,9,12}; ports that allow cascade */
        GT_U32   my_arr[5]={20,27,38,55,45};
                 /*not in trunk : 0,1,2,3 ,
                 local ports for trunk A : 59,64
                 local ports for trunk b : 18,36,
                 local ports for trunk c : 80,58
                 local ports for remote dev 6 : 65
                 */
        for(portIter = 0 ; portIter < 5; portIter++)
        {
            tgfTrunkNonUsedPorts[currNonUsedPortsIndex] = my_arr[portIter];
            PRV_UTF_LOG1_MAC("found non used port[%d] to use \n",tgfTrunkNonUsedPorts[currNonUsedPortsIndex]);
            currNonUsedPortsIndex++;
        }

        return GT_OK;
    }
    else
    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)|| /* bobcat3 SPECIFIC limited PORTS*/
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_FALCON_E)||
       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))/*aldrin2 like bc3 */
    {
        GT_U32   bc3arr[5]={16,36,50,58,65 };
                 /*not in trunk : 0,1,2,3 ,
                 local ports for trunk A :  79,64
                 local ports for trunk b : 16,36,
                 local ports for trunk c : 50,58
                 local ports for remote dev 6 : 65
                 */
        for(portIter = 0 ; portIter < 5; portIter++)
        {
            if(prvUtfIsDoublePhysicalPortsModeUsed())
            {
                bc3arr[portIter] |= BIT_8;/* align to port numbers of cpssInitSystem */
            }

            tgfTrunkNonUsedPorts[currNonUsedPortsIndex] = bc3arr[portIter];
            PRV_UTF_LOG1_MAC("found non used port[%d] to use \n",tgfTrunkNonUsedPorts[currNonUsedPortsIndex]);
            currNonUsedPortsIndex++;
        }

        return GT_OK;
    }



    for(ii = 0 ; ii < PRV_CPSS_PP_MAC(devNum)->numOfPorts; ii++)
    {
        if(0 == PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum,ii))
        {
            continue;
        }

        /* skip special port with MAC 62 in Caelum/Cetus devices */
        if ((ii == 62) && IS_BOBK_DEV_MAC(devNum))
        {
            continue;
        }

        if (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)
        {
            localPort = ii & 0xF;
            if ((localPort != 0) && (localPort != 4) && (localPort != 8) && (localPort != 9))
            {
                continue;
            }
        }

        /* check if the port is part of 'internal configurations' */
        if(prvTgfCommonIsDeviceForce(devNum))
        {
            /* look for this member in the DB */
            currMemPtr = &prvTgfDevPortForceArray[0];
            for(jj = 0 ; jj < prvTgfDevPortForceNum; jj++,currMemPtr++)
            {
                if(devNum != currMemPtr->member.devNum)
                {
                    continue;
                }

                if(ii == currMemPtr->member.portNum)
                {
                    if(currMemPtr->forceToVlan == GT_TRUE)
                    {
                        /* this is internal port that we can't use */
                        break;
                    }
                }
            }

            if(jj != prvTgfDevPortForceNum)
            {
                /* port ii is used as internal configuration port */
                continue;
            }
        }

        for(portIter = 0 ; portIter < prvTgfPortsNum; portIter++)
        {
            if(prvTgfDevsArray[portIter] != devNum)
            {
                continue;
            }

            if(prvTgfPortsArray[portIter] != ii)
            {
                continue;
            }

             /* if we are here ii == prvTgfPortsArray[portIter] */
            /* the port is used already */
            break;
        }

        if(portIter != prvTgfPortsNum)
        {
            /* port ii is already used in the test */
            continue;
        }

        /* port ii is not used , so we can use it in the test */
        tgfTrunkNonUsedPorts[currNonUsedPortsIndex] = ii;

        PRV_UTF_LOG1_MAC("found non used port[%d] to use \n",tgfTrunkNonUsedPorts[currNonUsedPortsIndex]);

        currNonUsedPortsIndex++;

        if(currNonUsedPortsIndex == TGF_NUM_NON_USED_PORTS_CNS)
        {
            /* we filled the ports that we needed for the test */
            return GT_OK;
        }
    }

    /* we failed to get full ports that we needed for the test */
    return GT_FULL;
}

/* check LBH for single/multi destination for the test 'Trunk with remote members' */
static void internalTrunkWithRemoteMembersTestCheckLbh(
    IN GT_BOOL             singleDestination,
    IN GT_U32              sendingPortsBmp,
    IN GT_U32              burstCount,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode
)
{
    GT_STATUS   rc;
    GT_TRUNK_ID trunkId;
    CPSS_PORTS_BMP_STC bmpOfPortsToSkip,cascadeTrunkPorts;
    GT_U8 devNum = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32                  numOfEnabledMembers = PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    static CPSS_TRUNK_MEMBER_STC   enabledMembersArray[PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    GT_U32      numMembersInTrunkA_toTrunkB = 0;
    GT_U32      numMembersInTrunkA_toTrunkC = 0;
    GT_U32      numMembersInTrunkA_toCascadePort = 0;
    GT_U32      burstCountForTrunkB;
    GT_U32      burstCountForTrunkC;
    GT_U32      burstCountForCascadePort;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32      ii;
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT    cascadeTrunksLbhMode;/* load balance on the cascade ports */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT    networkTrunksLbhMode;/* load balance on the network ports */
    GT_U32      burstCountForTrunkB_leftOver = 0;
    GT_U32      burstCountForTrunkC_leftOver = 0;
    GT_U32      burstCountForCascadePort_leftOver = 0;
    GT_U32      indexBmp_devNumForTrunkB = 0;
    GT_U32      indexBmp_devNumForTrunkC = 0;
    GT_U32      indexBmp_devNumForCascadePort = 0;
    GT_U32      totalCountOnLocalPorts_trunkA;
    GT_U32      expected;

    cascadeTrunksLbhMode = (singleDestination == GT_TRUE) ?
            PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_CHECK_ONLY_TOTAL_E :
            PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_MULTI_DESTINATION_EVEN_E;

    networkTrunksLbhMode = (singleDestination == GT_TRUE) ? lbhMode :
            PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_MULTI_DESTINATION_EVEN_E;

    /* check LBH on trunk A */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    totalCountOnLocalPorts_trunkA = 0;
    /* get the number of packets that we got on local members of trunk A */
    internal_prvTgfTrunkLoadBalanceCheck(trunkId,networkTrunksLbhMode,burstCount,0,NULL,NULL,
        &totalCountOnLocalPorts_trunkA);

    TRUNK_PRV_UTF_LOG1_MAC("=======  : trunkId members get =======\n",trunkId);
    rc = prvCpssGenericTrunkDbEnabledMembersGet(devNum,trunkId,&numOfEnabledMembers,enabledMembersArray);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    for (ii = 0; ii < numOfEnabledMembers; ii++)
    {
        /* convert HW devNum to SW devNum */
        rc = prvUtfSwFromHwDeviceNumberGet(enabledMembersArray[ii].hwDevice,&enabledMembersArray[ii].hwDevice);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(devNumForTrunkB == enabledMembersArray[ii].hwDevice)
        {
            indexBmp_devNumForTrunkB |= 1<<ii;
            numMembersInTrunkA_toTrunkB++;

            if(ii < (burstCount % numOfEnabledMembers))
            {
                /* the first members will get 1 packet more than others */
                burstCountForTrunkB_leftOver++;
            }

        }
        else if(devNumForTrunkC == enabledMembersArray[ii].hwDevice)
        {
            indexBmp_devNumForTrunkC |= 1<<ii;
            numMembersInTrunkA_toTrunkC++;
            if(ii < (burstCount % numOfEnabledMembers))
            {
                /* the first members will get 1 packet more than others */
                burstCountForTrunkC_leftOver++;
            }
        }
        else if(devNumForCascadePort == enabledMembersArray[ii].hwDevice)
        {
            indexBmp_devNumForCascadePort |= 1<<ii;
            numMembersInTrunkA_toCascadePort ++;
            if(ii < (burstCount % numOfEnabledMembers))
            {
                /* the first members will get 1 packet more than others */
                burstCountForCascadePort_leftOver++;
            }
        }
    }

    if(all_trunks_fixed_size > 8 && (singleDestination == GT_TRUE))
    {
        GT_U32  stretchFactor,stretch,countIndex,hashIndex;

        burstCountForTrunkC_leftOver      = 0;
        burstCountForTrunkB_leftOver      = 0;
        burstCountForCascadePort_leftOver = 0;
        burstCountForTrunkB = 0;
        burstCountForTrunkC = 0;
        burstCountForCascadePort = 0;

        /* emulate CPSS logic */
        stretchFactor =  all_trunks_fixed_size / numOfEnabledMembers;
        if(stretchFactor > 4)
        {
            stretchFactor = 4;
        }

        /* this is actual number of members in HW */
        stretch = stretchFactor * numOfEnabledMembers;

        for(countIndex = 0 ; countIndex < burstCount ; countIndex++)
        {
            hashIndex = (countIndex * stretch/*numberOfMembers*/) >> 6/*numOfBitsInHash*/;

            ii = hashIndex % numOfEnabledMembers;

            if((1<<ii) & indexBmp_devNumForTrunkB)
            {
                burstCountForTrunkB++;
            }
            else
            if((1<<ii) & indexBmp_devNumForTrunkC)
            {
                burstCountForTrunkC++;
            }
            else
            if((1<<ii) & indexBmp_devNumForCascadePort)
            {
                burstCountForCascadePort++;
            }
        }
    }
    else
    if(singleDestination == GT_TRUE)
    {
        /* the number of packet to trunk B not depend on number of ports in trunk B */
        burstCountForTrunkB = (burstCount * numMembersInTrunkA_toTrunkB) / numOfEnabledMembers;
        /* the number of packet to trunk C not depend on number of ports in trunk C */
        burstCountForTrunkC = (burstCount * numMembersInTrunkA_toTrunkC) / numOfEnabledMembers;
        /* the number of packet to cascade port */
        burstCountForCascadePort = (burstCount * numMembersInTrunkA_toCascadePort) / numOfEnabledMembers;

        /* the first members will get 1 packet more than others */
        burstCountForTrunkB += burstCountForTrunkB_leftOver;
        burstCountForTrunkC += burstCountForTrunkC_leftOver;
        burstCountForCascadePort += burstCountForCascadePort_leftOver;

    }
    else
    {
        /* flooding should go to all the devices ! ,
           so all the cascade interfaces should get all the count */
        burstCountForTrunkB = burstCount;
        burstCountForTrunkC = burstCount;
        burstCountForCascadePort = burstCount;
    }

    if(singleDestination == GT_TRUE)
    {
        /* after calculated the expected number of packets to the remote ports we
           can check if local ports got proper value */
        expected = (burstCount - (burstCountForTrunkB + burstCountForTrunkC + burstCountForCascadePort));
        UTF_VERIFY_EQUAL3_STRING_MAC(expected,totalCountOnLocalPorts_trunkA,
            "trunk load balance: local ports of 'trunkA'(trunk[%d]) expected [%d] got [%d] \n",
                                     trunkId,expected,totalCountOnLocalPorts_trunkA);
    }


    /* check LBH on trunk B */
    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    prvTgfTrunkLoadBalanceCheck(trunkId,cascadeTrunksLbhMode,burstCountForTrunkB,0,NULL,NULL);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&bmpOfPortsToSkip);

    rc = prvCpssGenericTrunkCascadeTrunkPortsGet(devNum,trunkId,&cascadeTrunkPorts);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* we need to skip the 'cascade ports' ports - of trunk B - to remote device 2 */
    CPSS_PORTS_BMP_BITWISE_OR_MAC(&bmpOfPortsToSkip,&bmpOfPortsToSkip,&cascadeTrunkPorts);

    /* check LBH on trunk C */
    trunkId = PRV_TGF_TRUNK_ID_3_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    prvTgfTrunkLoadBalanceCheck(trunkId,cascadeTrunksLbhMode,burstCountForTrunkC,0,NULL,NULL);

    rc = prvCpssGenericTrunkCascadeTrunkPortsGet(devNum,trunkId,&cascadeTrunkPorts);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* we need to skip the 'cascade ports' ports - of trunk C - to remote device 4 */
    CPSS_PORTS_BMP_BITWISE_OR_MAC(&bmpOfPortsToSkip,&bmpOfPortsToSkip,&cascadeTrunkPorts);

    portNum = tgfTrunkNonUsedPorts[TGF_CASCADE_PORT_INDEX_E-TGF_CASCADE_TRUNK_B_MEMBER_0_INDEX_E];
    /* we need to skip the 'cascade ports' port - to remote device 6 */
    CPSS_PORTS_BMP_PORT_SET_MAC(&bmpOfPortsToSkip,portNum);

    rc = prvTgfReadPortCountersEth(devNum, portNum, GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 devNum, tgfTrunkNonUsedPorts[TGF_CASCADE_PORT_INDEX_E-TGF_CASCADE_TRUNK_B_MEMBER_0_INDEX_E]);

    UTF_VERIFY_EQUAL3_STRING_MAC(burstCountForCascadePort, portCntrs.goodPktsSent.l[0],
        "the 'cascade' port[%d]: expected [%d] packets , but got [%d]\n",
                                 portNum,burstCountForCascadePort, portCntrs.goodPktsSent.l[0]);

    /* check that ports outside the trunk A and 'cascade' ports not get any packet */
    trunkSendCheckLbh1(singleDestination,sendingPortsBmp,burstCount,lbhMode,NULL,GT_TRUE,&bmpOfPortsToSkip);
}

/**
* @internal basicMustiDestinationSendAndCheck function
* @endinternal
*
* @brief   send multi destination traffic and check that LBH is OK.
*         check the 'Multi destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) also
*         received flooding
*         applicable devices: All devices
* @param[in] usedForTrunkWithRemoteMembers - is this check for tests with trunk A
*                                      that hold remote members ? trunk B and trunk C represent remote devices 2,4
*                                      and another port represent remote device 6
* @param[in] portIter                 - index in the array of prvTgfDevsArray ,prvTgfPortsArray
* @param[in] vlanId                   - vlanId
* @param[in] burstCount               - burst Count
* @param[in] lbhMode                  - LBH expected
* @param[in] trafficPtr               - (pointer to) traffic to send
* @param[in] multiDestType            - multi destination traffic type
*                                       None
*/
static void    basicMustiDestinationSendAndCheck(
    IN GT_BOOL          usedForTrunkWithRemoteMembers,
    IN  GT_U32          portIter,
    IN  TGF_VLAN_ID     vlanId,
    IN  GT_U32          burstCount,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN  TGF_PACKET_STC  *trafficPtr,
    IN  PRV_TGF_MULTI_DESTINATION_TYPE_ENT  multiDestType
)
{
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  sendingPortsBmp;
    GT_U32                  multiDestCount;

    devNum  = prvTgfDevsArray [portIter];
    portNum = prvTgfPortsArray[portIter];

    /* modify the vid part */
    prvTgfPacketVlanTagPart.vid = vlanId;

    /* send the multi-destination traffic */
    prvTgfCommonMultiDestinationTrafficTypeSend(devNum, portNum, burstCount ,
                                                GT_FALSE, trafficPtr , multiDestType);

    sendingPortsBmp = (1 << portIter);

    if(multiDestType == PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E)
    {
        multiDestCount =  MULTI_DEST_COUNT_MAC;
    }
    else
    {
        multiDestCount = 1;
    }

    burstCount *= multiDestCount;

    if(usedForTrunkWithRemoteMembers == GT_FALSE)
    {
        /* check the LBH and flooding */
        trunkSendCheckLbh(GT_FALSE,sendingPortsBmp,burstCount,lbhMode,NULL);
    }
    else
    {
        internalTrunkWithRemoteMembersTestCheckLbh(GT_FALSE,sendingPortsBmp,burstCount,lbhMode);
    }
}

/**
* @internal basicSingleDestinationSendAndCheck function
* @endinternal
*
* @brief   send single destination incremental traffic and check that LBH is OK.
*         check the 'Single destination traffic' did LBH (according to mode)
*         on trunk ports , and check that other ports (out of the trunk) NOT
*         received packets
*         applicable devices: All devices
* @param[in] usedForTrunkWithRemoteMembers - is this check for tests with trunk A
*                                      that hold remote members ? trunk B and trunk C represent remote devices 2,4
*                                      and another port represent remote device 6
* @param[in] portIter                 - index in the array of prvTgfDevsArray ,prvTgfPortsArray
* @param[in] vlanId                   - vlanId
* @param[in] burstCount               - burst Count
* @param[in] lbhMode                  - LBH expected
* @param[in] trafficPtr               - (pointer to) traffic to send
*                                      multiDestType - multi destination traffic type
*                                       None
*/
static void    basicSingleDestinationSendAndCheck(
    IN GT_BOOL          usedForTrunkWithRemoteMembers,
    IN  GT_U32          portIter,
    IN  TGF_VLAN_ID     vlanId,
    IN  GT_U32          burstCount,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode,
    IN  TGF_PACKET_STC  *trafficPtr
)
{
    GT_U32               numVfd = 1;/* number of VFDs in vfdArray */
    static TGF_VFD_INFO_STC     vfdArray[1] = {{0}};/* vfd Array -- used for increment the tested bytes , and for vlan tag changing */
    TGF_VFD_INFO_STC     *vrfPtr = &vfdArray[0];
    GT_U32  sendingPortsBmp;/* bitmap of ports that send bursts */

    vrfPtr->offset = 5;/* LSB of mac DA */

    /* according to the FDB learned MACs */
    burstCount = FDB_MAC_COUNT_CNS;

    vrfPtr->modeExtraInfo = 0;
    cpssOsMemSet(vrfPtr->patternPtr,0,sizeof(TGF_MAC_ADDR));
    vrfPtr->patternPtr[0] = 0;
    vrfPtr->cycleCount = 1;/*single byte*/
    vrfPtr->mode = TGF_VFD_MODE_INCREMENT_E;

    /* each port will sent different type of flow
       (L3 flow --> but the same L2 flow) */
    trafficPtr = packetsForSimpleHashMacSaDaArr[portIter];

    /* modify the vid part */
    prvTgfPacketVlanTagPart.vid = vlanId;

    vrfPtr->modeExtraInfo = 0;
    /* send the burst of packets with incremental byte */
    prvTgfTrunkTestPacketSend(prvTgfDevsArray [portIter], prvTgfPortsArray[portIter],
        trafficPtr ,
        burstCount ,numVfd ,vfdArray);

    /*bitmap of single sender*/
    sendingPortsBmp = (1 << portIter);

    if(usedForTrunkWithRemoteMembers == GT_FALSE)
    {
        /* check that trunk got it's LBH and other ports not got any packet */
        trunkSendCheckLbh(GT_TRUE,sendingPortsBmp,burstCount,lbhMode,NULL);
    }
    else
    {
        internalTrunkWithRemoteMembersTestCheckLbh(GT_TRUE,sendingPortsBmp,burstCount,lbhMode);
    }

    return;
}

/**
* @internal tgfTrunkWithRemoteMembersTestInit function
* @endinternal
*
* @brief   init for the 'trunk with remote members'
*         do extra specific init for this test:
*         1. define the trunk A : with 8 members that 6 of them on remote devices and 2 members are local.
*         3 devices :
*         3 members on device 2
*         2 members on device 4
*         1 member on device 6
*         2. define 2 cascade trunks to represent:
*         trunk B - device 2
*         trunk C - device 4
*         applicable devices: ALL DxCh devices
*/
static void tgfTrunkWithRemoteMembersTestInit
(
    void
)
{
    GT_STATUS rc;
    GT_U8 devNum = prvTgfDevNum;
    CPSS_PORTS_BMP_STC      trunkB_cascadeTrunkPorts;/* trunk B - cascade trunk ports */
    GT_U32                  trunkB_NumOfMembers = 2;
    CPSS_PORTS_BMP_STC      trunkC_cascadeTrunkPorts;/* trunk C - cascade trunk ports */
    GT_U32                  trunkC_NumOfMembers = 2;
    GT_U32                  trunkA_NumOfMembers = TRUNK_8_PORTS_IN_TEST_CNS;
    GT_TRUNK_ID             trunkId;
    GT_U32          *weightedPortsArray;
    GT_U32          portIter;
    CPSS_TRUNK_MEMBER_STC  *trunkA_MembersArrayPtr; /* pointer to array of members */

    initTrunk2Ids(GT_TRUE);
    initTrunk3Ids(GT_TRUE);

    if(trunkUseWeightedPorts == GT_TRUE)
    {
        goto startTrunkB_lbl;
    }

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    if((TRUNK_8_PORTS_IN_TEST_CNS + NOT_IN_TRUNK_PORTS_NUM_CNS) != prvTgfPortsNum)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC((TRUNK_8_PORTS_IN_TEST_CNS + NOT_IN_TRUNK_PORTS_NUM_CNS), prvTgfPortsNum);
        return;
    }

    PRV_UTF_LOG1_MAC("set trunk [%d] with members{dev,port}:",trunkId);

    trunkA_MembersArrayPtr =    (prvTgfXcat3xExists() == GT_TRUE)  ? trunkA_MembersArray_xCat3x :
                                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? /* bobcat3 SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_bobcat3 :
                                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_FALCON_E) ? /* SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_falcon :
                                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ? /* aldrin2 like bc3 */
                                    trunkA_MembersArray_aldrin2 :
                                (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType     == CPSS_98DX8332_Z0_CNS) ?  /* aldrin Z0 SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_aldrinZ0 :
                                IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum) ? trunkA_MembersArray_bobk_cetus :
                                (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                                trunkA_MembersArray_XLG: trunkA_MembersArray;

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
        {
            /* restore what was set in the init on the 'generic' trunk members */
            if(tgfTrunkGlobalEPortRepresentTrunk && defaultEPortWasChanged[portIter])
            {
                /* restore the default EPort of the ingress port */
                rc = prvTgfCfgPortDefaultSourceEportNumberSet(prvTgfDevNum,
                    prvTgfPortsArray[portIter],
                    defaultEPort[portIter]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                defaultEPortWasChanged[portIter] = 0;
            }
        }
    }

    for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
    {
        prvTgfDevsArray [portIter] = (GT_U8)trunkA_MembersArrayPtr[portIter - NOT_IN_TRUNK_PORTS_NUM_CNS].hwDevice;
        prvTgfPortsArray[portIter] = trunkA_MembersArrayPtr[portIter - NOT_IN_TRUNK_PORTS_NUM_CNS].port;
        PRV_UTF_LOG2_MAC("{%d,%d}",prvTgfDevsArray [portIter],prvTgfPortsArray[portIter]);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* set trunk A */
    rc = prvTgfTrunkMembersSet(
        trunkId /*trunkId*/,
        trunkA_NumOfMembers /*numOfEnabledMembers*/,
        trunkA_MembersArrayPtr /*enabledMembersArray*/,
        0 /*numOfDisabledMembers*/,
        (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        for(portIter = NOT_IN_TRUNK_PORTS_NUM_CNS; portIter < prvTgfPortsNum ; portIter++)
        {
            /* the function prvTgfTrunkMembersSet() , set the ingress port associated with the trunkId ,
               but we need it to be associated with the 'global eport' which represents the trunk */
            associateIngressPortWithEPortInsteadOfTrunkId(portIter , trunkId);
        }
    }

    trunkMembersStateMessageSave(4,trunkId);

    /*  Fill the array of tgfTrunkNonUsedPorts  */
    rc = tgfTrunkNonUsedPortsFill();
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    /* set trunk B - cascade trunk */

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&trunkB_cascadeTrunkPorts);
    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkB_cascadeTrunkPorts,tgfTrunkNonUsedPorts[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkB_cascadeTrunkPorts,tgfTrunkNonUsedPorts[1]);

    PRV_UTF_LOG1_MAC("set cascade trunk [%d] with ports:",trunkId);

    for(portIter = 0; portIter < trunkB_NumOfMembers ; portIter++)
    {
        prvTgfDevsArray [prvTgfPortsNum] = devNum;
        prvTgfPortsArray[prvTgfPortsNum] = tgfTrunkNonUsedPorts[portIter];
        PRV_UTF_LOG1_MAC("{%d}",prvTgfPortsArray[prvTgfPortsNum]);
        prvTgfPortsNum++;
    }
    PRV_UTF_LOG0_MAC("\n");

    /* set trunk B - cascade trunk */
    if(trunkUseWeightedPorts == GT_TRUE)
    {
startTrunkB_lbl:
        trunkId = PRV_TGF_TRUNK_ID_2_CNS;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

        PRV_UTF_LOG1_MAC("cascade trunk [%d]is weighted: \n",trunkId);

        weightedPortsArray = portsArray_total_weight_Ptr[trunkB_NumOfMembers].weightsArray;

        trunkB_weightedMembers[0].member.port = tgfTrunkNonUsedPorts[0];
        trunkB_weightedMembers[0].member.hwDevice = devNum;
        trunkB_weightedMembers[0].weight = weightedPortsArray[0];
        PRV_UTF_LOG2_MAC("port [%d] with weight [%d] \n ",trunkB_weightedMembers[0].member.port,trunkB_weightedMembers[0].weight);
        trunkB_weightedMembers[1].member.port = tgfTrunkNonUsedPorts[1];
        trunkB_weightedMembers[1].member.hwDevice = devNum;
        trunkB_weightedMembers[1].weight = weightedPortsArray[1];
        PRV_UTF_LOG2_MAC("port [%d] with weight [%d] \n ",trunkB_weightedMembers[1].member.port,trunkB_weightedMembers[1].weight);

        /* the cascade ports of the trunk used with weights */
        rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,trunkId,
                trunkB_NumOfMembers,
                trunkB_weightedMembers);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        trunkMembersStateMessageSave(5,trunkId);

        /* no more to do */
        return;
    }
    else
    {
        rc = prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&trunkB_cascadeTrunkPorts);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        trunkMembersStateMessageSave(5,trunkId);
    }

    trunkId = PRV_TGF_TRUNK_ID_3_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    /* set trunk C - cascade trunk */

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&trunkC_cascadeTrunkPorts);
    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkC_cascadeTrunkPorts,tgfTrunkNonUsedPorts[2]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&trunkC_cascadeTrunkPorts,tgfTrunkNonUsedPorts[3]);

    PRV_UTF_LOG1_MAC("set cascade trunk [%d] with ports:",trunkId);

    for(portIter = 0; portIter < trunkC_NumOfMembers ; portIter++)
    {
        prvTgfDevsArray [prvTgfPortsNum] = devNum;
        prvTgfPortsArray[prvTgfPortsNum] = tgfTrunkNonUsedPorts[portIter+trunkB_NumOfMembers];
        PRV_UTF_LOG1_MAC("{%d}",prvTgfPortsArray[prvTgfPortsNum]);
        prvTgfPortsNum++;
    }
    PRV_UTF_LOG0_MAC("\n");
    rc = prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&trunkC_cascadeTrunkPorts);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    trunkMembersStateMessageSave(6,trunkId);

    /* set port that represent remote device 6 */
    prvTgfDevsArray [prvTgfPortsNum] = devNum;
    prvTgfPortsArray[prvTgfPortsNum] = tgfTrunkNonUsedPorts[trunkB_NumOfMembers+trunkC_NumOfMembers];
    prvTgfPortsNum++;
}
/**
* @internal tgfTrunkWithRemoteMembersTestInitDeviceMapTable function
* @endinternal
*
* @brief   init for the 'trunk with remote members'
*         do extra specific init for this test: set the device map table:
*         device 2 --> trunk B
*         device 4 --> trunk C
*         device 6 --> regular port (tgfTrunkNonUsedPorts[4])
*         applicable devices: ALL DxCh devices
* @param[in] start                    - indication to configure 'start' config or to 'undo'
*                                      GT_TRUE - start.
*                                      GT_FALSE - undo --> set target devices via NULL port
*                                       None
*/
static void tgfTrunkWithRemoteMembersTestInitDeviceMapTable
(
    IN GT_BOOL  start
)
{
    GT_STATUS   rc;
    GT_U32  ii,index;
    CPSS_CSCD_LINK_TYPE_STC      cascadeLink; /* cascade link info */
    GT_TRUNK_ID             trunkId;
    GT_U8   remoteDevNum;

    for(index = TGF_CASCADE_TRUNK_B_MEMBER_0_INDEX_E ; index <= TGF_CASCADE_PORT_INDEX_E; index ++)
    {
        if(index == TGF_CASCADE_PORT_INDEX_E)
        {
            cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
            cascadeLink.linkNum = prvTgfPortsArray[index];
            remoteDevNum = devNumForCascadePort;
        }
        else if (index < TGF_CASCADE_TRUNK_C_MEMBER_0_INDEX_E)
        {             /*trunk B*/
            trunkId = PRV_TGF_TRUNK_ID_2_CNS;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
            cascadeLink.linkNum = trunkId;
            remoteDevNum = devNumForTrunkB;
        }
        else
        {             /*trunk C*/
            trunkId = PRV_TGF_TRUNK_ID_3_CNS;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
            cascadeLink.linkNum = trunkId;
            remoteDevNum = devNumForTrunkC;
        }

        if(start == GT_FALSE)
        {
            cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
            cascadeLink.linkNum  = CPSS_NULL_PORT_NUM_CNS;
        }

        PRV_UTF_LOG3_MAC("set device map table : remote device[%d] via %s[%d]\n",
            remoteDevNum,
            (cascadeLink.linkType == CPSS_CSCD_LINK_TYPE_TRUNK_E) ? "trunk" : "port",
            cascadeLink.linkNum);


        /* set this 'Remote device' to point to the cascade trunk/port */
        for(ii = 0 ;ii < CPSS_MAX_PORTS_NUM_CNS;ii++)
        {
            rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                    remoteDevNum,
                    ii,
                    &cascadeLink,
                    PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,
                    GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        if(index < TGF_CASCADE_PORT_INDEX_E)
        {
            index++;/* there are 2 members in turnk B and trunk C , so we jump to next one*/
        }
    }


    return;
}

static void tgfTrunkWithRemoteMembersTestConfig
(
    IN GT_U32  configId
)
{
    GT_STATUS       rc;
    CPSS_PORTS_BMP_STC      trunkB_cascadeTrunkPorts;/* trunk B - cascade trunk ports */
    GT_U32                  trunkB_NumOfMembers;
    GT_TRUNK_ID             trunkId;
    GT_U32  trunkA_memberIndex = 2;/*index in trunkA_MembersArray*/
    CPSS_TRUNK_MEMBER_STC   trunkA_member;
    GT_U32          *weightedPortsArray;
    GT_U8   devNum = prvTgfDevNum;


    PRV_UTF_LOG1_MAC("tgfTrunkWithRemoteMembersTestConfig #%d : start \n",configId);

    switch(configId)
    {
        case 1:
            /* nothing to do more */
            utfGeneralStateMessageSave(2,"configId = %d ",configId);
            break;
        case 2:
            PRV_UTF_LOG0_MAC("weighted cascade trunk \n");
            trunkUseWeightedPorts = GT_TRUE;
            /* enable the sorting */
            rc = prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, trunkUseWeightedPorts);
            if(rc != GT_OK)
            {
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                return;
            }

            utfGeneralStateMessageSave(2,"configId = %d ",configId);
            /* set specific trunk init */
            tgfTrunkWithRemoteMembersTestInit();
            break;
        case TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E:    /* remove port from Cascade trunk B */
        case TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_CASCADE_TRUNK_B_E:    /* restore port to  Cascade trunk B */
            utfGeneralStateMessageSave(2,"configId = %d %s",configId,
                (configId == TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E) ?
                "TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E":
                "TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_CASCADE_TRUNK_B_E");

            trunkB_NumOfMembers = (configId == TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E) ?
                                    1 : 2;

            trunkId = PRV_TGF_TRUNK_ID_2_CNS;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&trunkB_cascadeTrunkPorts);
            CPSS_PORTS_BMP_PORT_SET_MAC(&trunkB_cascadeTrunkPorts,tgfTrunkNonUsedPorts[0]);
            if(trunkB_NumOfMembers > 1)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&trunkB_cascadeTrunkPorts,tgfTrunkNonUsedPorts[1]);
            }

            if(configId == TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E)
            {
                PRV_UTF_LOG2_MAC("remove port %d from Cascade trunk %d  \n",tgfTrunkNonUsedPorts[1],trunkId);
            }
            else
            {
                PRV_UTF_LOG2_MAC("restore port %d to Cascade trunk %d  \n",tgfTrunkNonUsedPorts[1],trunkId);
            }

            if(trunkUseWeightedPorts == GT_TRUE)
            {
                PRV_UTF_LOG1_MAC("cascade trunk [%d]is weighted: \n",trunkId);
                weightedPortsArray = portsArray_total_weight_Ptr[trunkB_NumOfMembers].weightsArray;

                trunkB_weightedMembers[0].member.port = tgfTrunkNonUsedPorts[0];
                trunkB_weightedMembers[0].member.hwDevice = devNum;
                trunkB_weightedMembers[0].weight = weightedPortsArray[0];
                PRV_UTF_LOG2_MAC("port [%d] with weight [%d] \n ",trunkB_weightedMembers[0].member.port,trunkB_weightedMembers[0].weight);
                trunkB_weightedMembers[1].member.port = tgfTrunkNonUsedPorts[1];
                trunkB_weightedMembers[1].member.hwDevice = devNum;
                trunkB_weightedMembers[1].weight = weightedPortsArray[1];
                PRV_UTF_LOG2_MAC("port [%d] with weight [%d] \n ",trunkB_weightedMembers[1].member.port,trunkB_weightedMembers[1].weight);

                /* the cascade ports of the trunk used with weights */
                rc = prvTgfTrunkWithWeightedMembersSet(prvTgfDevNum,trunkId,
                        trunkB_NumOfMembers,
                        trunkB_weightedMembers);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                trunkMembersStateMessageSave(5,trunkId);
                /* no more to do */
                return;
            }
            else
            {
                rc = prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,&trunkB_cascadeTrunkPorts);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                trunkMembersStateMessageSave(5,trunkId);
            }
            break;
        case TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E:    /* remove port from trunk A */
        case TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_TRUNK_A_E:    /* restore port to trunk A */

            utfGeneralStateMessageSave(2,"configId = %d %s",configId,
                (configId == TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E) ?
                "TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E":
                "TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_TRUNK_A_E");

            trunkId = PRV_TGF_TRUNK_ID_CNS;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

            trunkA_member = (prvTgfXcat3xExists() == GT_TRUE)  ? trunkA_MembersArray_xCat3x[trunkA_memberIndex] :
                            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? /* bobcat3 SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_bobcat3[trunkA_memberIndex] :
                            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_FALCON_E) ? /* SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_falcon[trunkA_memberIndex] :
                            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily    == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ? /* aldrin2 like bc3 */
                                    trunkA_MembersArray_aldrin2[trunkA_memberIndex] :
                            (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType     == CPSS_98DX8332_Z0_CNS) ?  /* aldrin Z0 SPECIFIC limited PORTS*/
                                    trunkA_MembersArray_aldrinZ0[trunkA_memberIndex] :
                            IS_BOBK_DEV_CETUS_MAC(prvTgfDevNum) ? trunkA_MembersArray_bobk_cetus[trunkA_memberIndex] :
                            (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E) ?
                            trunkA_MembersArray_XLG[trunkA_memberIndex]:
                            trunkA_MembersArray[trunkA_memberIndex];

            if(configId == TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E)
            {
                PRV_UTF_LOG3_MAC("remove {port %d,device %d} from trunk %d  \n",trunkA_member.port,trunkA_member.hwDevice,trunkId);
                rc = prvTgfTrunkMemberRemove(trunkId,&trunkA_member);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                if(tgfTrunkGlobalEPortRepresentTrunk &&
                   globalEPortRepresentTrunk_sortingEnabled == GT_TRUE)
                {
                    associateIngressPortWithEPortInsteadOfTrunkId_fullTrunk(trunkId);
                }
            }
            else
            {
                PRV_UTF_LOG3_MAC("restore {port %d,device %d} to trunk %d  \n",trunkA_member.port,trunkA_member.hwDevice,trunkId);
                rc = prvTgfTrunkMemberAdd(trunkId,&trunkA_member);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                if(tgfTrunkGlobalEPortRepresentTrunk)
                {
                    GT_U32  portIter;

                    rc = findPortInArr(trunkA_member.hwDevice,trunkA_member.port,&portIter);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                    associateIngressPortWithEPortInsteadOfTrunkId(portIter,trunkId);
                }
            }

            trunkMembersStateMessageSave(4,trunkId);

            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
            break;
    }

    PRV_UTF_LOG1_MAC("tgfTrunkWithRemoteMembersTestConfig #%d : end \n",configId);
}

/* check the 'trunk with remote members' for LBH on 2 cases:
    1. known UC
    2. flooding (unknown UC)
*/
static void tgfTrunkWithRemoteMembersTestCheckLbh
(
    void
)
{
    GT_U32  portIter = 2;
    GT_U32  burstCount;

    PRV_UTF_LOG0_MAC("check known UC \n");

    utfGeneralStateMessageSave(3,"%s","send and check known DA");
    /*send and check known DA */
    PRV_UTF_LOG0_MAC("send and check known DA \n");
    burstCount = FDB_MAC_COUNT_CNS;
    basicSingleDestinationSendAndCheck(GT_TRUE,/*usedForTrunkWithRemoteMembers*/
            portIter,/*portIter*/
            prvTgfVlansArr[0],/*vlanId*/
            burstCount,   /*burstCount*/
            PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,/*lbhMode*/
            packetsForSimpleHashMacSaDaArr[portIter]);/*trafficPtr*/

    /* reset counters */
    clearAllTestedCounters();
    if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
    {
        utfGeneralStateMessageSave(3,"%s","done sending");
        return;
    }
    utfGeneralStateMessageSave(3,"%s","send and check unknown DA (flooding)");
    /*send and check unknown DA (flooding) */
    PRV_UTF_LOG0_MAC("send and check unknown DA (flooding) \n");
    basicMustiDestinationSendAndCheck(GT_TRUE,/*usedForTrunkWithRemoteMembers*/
            portIter,/*portIter*/
            prvTgfVlansArr[0],/*vlanId*/
            64,   /*burstCount*/
            PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,/*lbhMode*/
            packetsForSimpleHashMacSaDaArr[portIter],/*trafficPtr*/
            PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_DA_INCREMENT_E);/*multiDestType*/

    /* reset counters */
    clearAllTestedCounters();

    utfGeneralStateMessageSave(3,"%s","done sending");
    return;
}



static void tgfTrunkWithRemoteMembersTestId
(
    IN GT_U32  testId
)
{
    static TRUNK_WITH_REMOTE_MEMBERS_ACTIONS_ENT  configurationsArr[] = {
           TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_CASCADE_TRUNK_B_E /* remove port from Cascade trunk B */
          ,TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_CASCADE_TRUNK_B_E  /* restore port to  Cascade trunk B */
          ,TRUNK_WITH_REMOTE_MEMBERS_ACTION_REMOVE_PORT_FROM_TRUNK_A_E         /* remove port from trunk A */
          ,TRUNK_WITH_REMOTE_MEMBERS_ACTION_RESTORE_PORT_TO_TRUNK_A_E          /* restore port to trunk A */
          };
    GT_U32  numOfConfigurations = sizeof(configurationsArr) / sizeof(configurationsArr[0]);
    GT_U32  ii;

    PRV_UTF_LOG1_MAC("tgfTrunkWithRemoteMembersTest #%d : start \n",testId);

    utfGeneralStateMessageSave(1,"testId = %d ",testId);

    /* do configuration*/
    tgfTrunkWithRemoteMembersTestConfig(testId); /* 1 or 2 */
    /*check LBH */
    tgfTrunkWithRemoteMembersTestCheckLbh();


    if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
    {
        /* reduce number of iterations */
        return;
    }


    for(ii = 0 ; ii < numOfConfigurations; ii++)
    {
        /* do configuration*/
        tgfTrunkWithRemoteMembersTestConfig(configurationsArr[ii]);
        /*check LBH */
        tgfTrunkWithRemoteMembersTestCheckLbh();
    }

    PRV_UTF_LOG1_MAC("tgfTrunkWithRemoteMembersTest #%d : end \n",testId);
}

static void tgfTrunkWithRemoteMembersTestRestore
(
    void
)
{
    GT_TRUNK_ID             trunkId;

    /* this test based on the 'DA hash' test -- use it's restore */
    prvTgfTrunkSimpleHashMacSaDaTestRestore();

    trunkUseWeightedPorts = GT_FALSE;
    /* disable the sorting */
    prvTgfTrunkDbMembersSortingEnableSet(prvTgfDevNum, trunkUseWeightedPorts);


    trunkId = PRV_TGF_TRUNK_ID_2_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,NULL);

    trunkId = PRV_TGF_TRUNK_ID_3_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    prvTgfTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkId,NULL);

    /* undo the device map table*/
    tgfTrunkWithRemoteMembersTestInitDeviceMapTable(GT_FALSE);

}

static void tgfTrunkWithRemoteMembersTestConfigInit
(
    void
)
{
    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTestConfigInit: this test based on the 'DA hash' test \n");
    /* this test based on the 'DA hash' test */
    prvTgfTrunkSimpleHashMacSaDaTestInit();

    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTestConfigInit: set specific trunk init \n");
    /* set specific trunk init */
    tgfTrunkWithRemoteMembersTestInit();
    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTestConfigInit: set the device map table \n");
    /* set the device map table*/
    tgfTrunkWithRemoteMembersTestInitDeviceMapTable(GT_TRUE);

    /* force linkup on all ports involved */
    forceLinkUpOnAllTestedPorts();

    /* clear all counter (maybe left from previous tests) */
    clearAllTestedCounters();

}

/**
* @internal tgfTrunkWithRemoteMembersTest function
* @endinternal
*
* @brief   design according to : http://docil.marvell.com/webtop/drl/objectId/0900dd88801254fd
*         Purpose:
*         Define enh-UT / RDE test for LBH (load balance hash) on trunk members with �remote� members.
*         The test require single device
*         Configuration 1:
*         FDB : Set 64 mac addresses related to trunk A
*         Trunk A "regular" with 8 members : 2 locals + 6 remote
*         Trunk B "cascade" with 2 ports
*         Trunk C "cascade" with 2 ports
*         Dev 2 "Remote" reached via trunk B (3 members in trunk A) --> device map table
*         Dev 3 "Remote" reached via trunk C (2 members in trunk A) --> device map table
*         Dev 4 "Remote" reached via port (1 member in trunk A) --> device map table
*         Configuration 2:
*         Same as configuration 1 + next:
*         Trunk B is "weighted cascade" the 2 ports weights are (2,6)
*         Configuration 3:
*         Same as configuration 1 or 2 + next:
*         Case 1 : Remove one of the ports from Cascade trunk A or B
*         Case 2 : Remove one of the members from Trunk A , a member of Dev 2 or Dev 3
*         Configuration 4:
*         After configuration 3 : restore the removed port
*         Traffic generated:
*         Send traffic from a port that is not relate to any trunk / remote device:
*         1. known UC � 64 mac DA
*         2. unknown UC � 64 mac DA (other than the known)
*         Check:
*         1. a. when 3 members of Dev 2 in trunk A : the ports of trunk B got 3/numMembersOf_trunkA of traffic
*         b. when 2 members of Dev 2 in trunk A : the ports of trunk B got 2/numMembersOf_trunkA of traffic
*         c. when �flood� trunk B get FULL traffic
*         2. a. when 2 members of Dev 3 in trunk A : the ports of trunk C got 2/numMembersOf_trunkA of traffic
*         b. when 1 members of Dev 3 in trunk A : the ports of trunk C got 1/numMembersOf_trunkA of traffic
*         c. when �flood� trunk C get FULL traffic
*         3. a. when 2 ports in trunk B : the ports share traffic according to weight (when no weight --> equal)
*         b. when 1 port in trunk B : the port get all traffic
*         4. Local Dev 1 Member 1 : get 1/numMembersOf_trunkA
*         5. Local Dev 1 Member 5 : get 1/numMembersOf_trunkA
*         6. a. Local Dev 1 Member 24 : get 1/numMembersOf_trunkA
*         b. when �flood� Local Dev 1 Member 24 get FULL traffic
*         Test 1:
*         1. Configuration 1 ,traffic generate ,check
*         2. Configuration 3 ,traffic generate ,check
*         3. Configuration 4 ,traffic generate ,check
*         Test 2: (weighted cascade trunk)
*         Like test 1 , use configuration 2 instead of configuration 1
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkWithRemoteMembersTest
(
    void
)
{
    /*register function to be called on error -- the 'trunk dump' */
/*    utfCallBackFunctionOnErrorSave(0,tgfTrunkDumpFunc);*/

    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTest: do basic configurations \n");
    /* do basic configurations */
    tgfTrunkWithRemoteMembersTestConfigInit();

    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTest: test with cascade trunk B as 'non-weighted' \n");
    /* test with cascade trunk B as 'non-weighted' */
    tgfTrunkWithRemoteMembersTestId(1);

    if((forceCrcRunAllPermutations == 0) && (prvUtfIsGmCompilation() == GT_TRUE))
    {
        PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTest: restore configurations \n");
        /* restore configurations */
        tgfTrunkWithRemoteMembersTestRestore();

        /* reduce number of iterations */
        return;
    }

    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTest: test with cascade trunk B as 'weighted' \n");
    /* test with cascade trunk B as 'weighted' */
    tgfTrunkWithRemoteMembersTestId(2);

    PRV_UTF_LOG0_MAC("tgfTrunkWithRemoteMembersTest: restore configurations \n");
    /* restore configurations */
    tgfTrunkWithRemoteMembersTestRestore();
}


/**
* @internal tgfTrunkSaLearningTestTraffic function
* @endinternal
*
* @brief   Test mac SA learning from ports that are trunk members in local device,
*         and from from ports that are trunk members in remote device (via the DSA tag)
* @param[in] testDsa                  - indication to test learning for packets that received from remote device (via the DSA tag)
* @param[in] testSourceFilter         - indication to test source filtering (in the bridge for known UC).
* @param[in] doBypassBridge           - When DSA tag mode = GT_TRUE , filter mode = GT_TRUE ,
*                                      indication that to set bypass bridge or not.
*                                      in bypass bridge = GT_TRUE  --> no filter expected !!! (as ingress device should have done it)
*                                      in bypass bridge = GT_FALSE --> filter expected.
*                                       None
*/
static void tgfTrunkSaLearningTestTraffic
(
    IN GT_BOOL      testDsa,
    IN GT_BOOL      testSourceFilter,
    IN GT_BOOL      doBypassBridge
)
{
    GT_STATUS   rc;/*return code*/
    GT_TRUNK_ID           trunkId;/* expected trunkId that FDB entry learned on */
    GT_U32  portIter,portIterStart,portIterEnd;
    TGF_PACKET_STC *packetInfoPtr = &prvTgfPacketVlanTagInfo;
    GT_U8                 devNum;
    GT_PHYSICAL_PORT_NUM  portNum;/*physical port to send packet into the device from it (with/without DSA)*/
    PRV_TGF_BRG_MAC_ENTRY_STC brgMacEntry;
    GT_PORT_NUM     expectedEport;/* expected eport that FDB entry learned on */
    GT_PHYSICAL_PORT_NUM  cascadePortNum = 7;/* the cascade port */
    GT_PHYSICAL_PORT_NUM  expectedEgressPort = 5;
    GT_PORT_NUM             fdbDaEgressPort;/* the eport associated with the mac DA in the FDB */
    GT_BOOL                         isEqualCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32          ii;/*iterator*/
    GT_U32          rxCount,txCount;/* rx,tx counters of packets on a port */
    GT_BOOL         expectFail;

    TRUNK_PRV_UTF_LOG3_MAC("tgfTrunkSaLearningTestTraffic testDsa[%d] , testSourceFilter[%d] ,doBypassBridge[%d] \n",
        testDsa,testSourceFilter,doBypassBridge);

    /* used for regular trunk (in FDB)*/
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    /* used for eport represent trunk(in FDB)*/
    expectedEport = DEFAULT_SOURCE_EPORT_FROM_TRUNK_ID_MAC(trunkId);

    brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    cpssOsMemCpy(&brgMacEntry.key.key.macVlan.macAddr ,
                 multiDestination_prvTgfPacketL2Part_unk_UC.saMac ,
                 sizeof(GT_ETHERADDR));
    brgMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    /* remove the mac from the FDB */
    rc = prvTgfBrgFdbMacEntryDelete(&(brgMacEntry.key));
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryDelete: failed\n");

    /* check the mac was removed */
    rc = prvTgfBrgFdbMacEntryGet(&brgMacEntry.key , &brgMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: should NOT found the entry \n");

    fdbDaEgressPort = prvTgfPortsArray[1];/* port in index 1 */

    if(tgfTrunkGlobalEPortRepresentTrunk)
    {
        /**********************************************************************/
        /* set needed parameters on the ingress ePort (relevant when non-dsa) */
        /**********************************************************************/

        rc = prvTgfBrgFdbNaToCpuPerPortSet(prvTgfDevNum, expectedEport, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbNaToCpuPerPortSet");

        /* do not allow auto learning */
        rc = prvTgfBrgFdbPortAutoLearnEnableSet(prvTgfDevNum, expectedEport, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbPortAutoLearnEnableSet");
    }

    if(testDsa == GT_TRUE)
    {
        /* single port iteration */
        portIterStart = NOT_IN_TRUNK_PORTS_NUM_CNS - 1;
        portIterEnd   = NOT_IN_TRUNK_PORTS_NUM_CNS;

        /* add configurations for cascade port */
        cascadePortNum = prvTgfPortsArray[portIterStart];
        expectedEgressPort = prvTgfPortsArray[portIterStart - 1];/* port in index 2 */

        /* AUTODOC: set ingress port in cascaded mode (RX direction) */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum, cascadePortNum, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorIngressCscdPortEnableSet: %d %d",
                                     prvTgfDevNum, cascadePortNum);

        /* state that the packet should have DSA tag */

        if(0 == tgfTrunkGlobalEPortRepresentTrunk)
        {
            /* AUTODOC: state that packet will be with DSA and info srcHwDev = 'remote dev' and srcTrunk = 'The trunk' */
            dsaFromRemotePortOfTrunkPart_forward.srcHwDev         = REMOTE_DEVICE_NUMBER_CNS;
            dsaFromRemotePortOfTrunkPart_forward.source.trunkId = trunkId;
            dsaFromRemotePortOfTrunkPart_forward.srcIsTrunk = GT_TRUE;
        }
        else
        {
            /* AUTODOC: state that packet will be with DSA and info srcHwDev = 'remote dev' and srcEPort = 'eport represent trunk' */
            dsaFromRemotePortOfTrunkPart_forward.srcHwDev         = REMOTE_DEVICE_NUMBER_CNS;
            dsaFromRemotePortOfTrunkPart_forward.source.portNum = expectedEport;
            dsaFromRemotePortOfTrunkPart_forward.srcIsTrunk = GT_FALSE;
        }

        /* AUTODOC: state that packet targeted to 'own dev' and dstEport = 'remote eport' ,
            BUT set the 'isTrgPhyPortValid = GT_TRUE' got set needed egress port

            isTrgPhyPortValid
        */
        dsaFromRemotePortOfTrunkPart_forward.dstEport       = PRV_TGF_TARGET_EPORT_CNS;
        dsaFromRemotePortOfTrunkPart_forward.dstInterface.devPort.hwDevNum = prvTgfDevNum;
        dsaFromRemotePortOfTrunkPart_forward.isTrgPhyPortValid = GT_TRUE;
        dsaFromRemotePortOfTrunkPart_forward.dstInterface.devPort.portNum = expectedEgressPort;

        dsaFromRemotePortOfTrunkPart.dsaInfo.forward = dsaFromRemotePortOfTrunkPart_forward;
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            dsaFromRemotePortOfTrunkPart.dsaType = TGF_DSA_4_WORD_TYPE_E;
        }
        else
        {
            dsaFromRemotePortOfTrunkPart.dsaType = TGF_DSA_2_WORD_TYPE_E;
        }

        tgfTrafficEnginePacketForceDsaOnPacket(&dsaFromRemotePortOfTrunkPart,GT_TRUE);

        if(doBypassBridge == GT_FALSE)
        {
            /* AUTODOC: state that the packet from DSA will not bypass bridge so the mac DA from FDB used */
            /* AUTODOC: NOTE: the "Local Switching of Known Unicast Packets" ignored for 'bypass bridge' */
            rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,cascadePortNum,GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }
    else
    {
        portIterStart = NOT_IN_TRUNK_PORTS_NUM_CNS;
        portIterEnd =  prvTgfPortsNum;
    }


    cpssOsMemSet(&brgMacEntry,0,sizeof(brgMacEntry));

    /* AUTODOC : add FDB entry associated with the mac DA of the packet */
    brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    cpssOsMemCpy(&brgMacEntry.key.key.macVlan.macAddr ,
                 multiDestination_prvTgfPacketL2Part_unk_UC.daMac ,
                 sizeof(GT_ETHERADDR));
    brgMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    brgMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    brgMacEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    brgMacEntry.dstInterface.devPort.portNum = fdbDaEgressPort;

    if(testSourceFilter == GT_TRUE)
    {
        if(testDsa == GT_TRUE)
        {
            /* set the FDB to the same eport as source info in the DSA */
            if(doBypassBridge == GT_FALSE)
            {
                expectedEgressPort = 0xFFFFFFFF;/* not expected to egress any port */
            }
        }
        else
        {
            expectedEgressPort = 0xFFFFFFFF;/* not expected to egress any port */
        }

        if(tgfTrunkGlobalEPortRepresentTrunk)
        {
            if(doBypassBridge == GT_FALSE)
            {
                /* according to FDB */
                brgMacEntry.dstInterface.devPort.portNum = expectedEport;
            }
            else
            {
                /* dummy value at the mac DA in the FDB should be ignored when bypass bridge */
                brgMacEntry.dstInterface.devPort.portNum = expectedEport + 1;
            }
        }
        else
        {
            brgMacEntry.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
            brgMacEntry.dstInterface.trunkId = trunkId;
        }
    }
    else
    {
        expectedEgressPort = fdbDaEgressPort;
    }

    /* add the mac entry that associated with the mac DA (so we get single destination) */
    prvTgfBrgFdbMacEntrySet(&brgMacEntry);


    if(prvUtfIsDoublePhysicalPortsModeUsed() &&
        testDsa == GT_TRUE && testSourceFilter == GT_TRUE)
    {
        /* the target physical port in the FORWARD DSA tag is only 8 bits , so
           it try to egress from port which is not in the vlan

           so we need to expect nothing to egress ... regardless to 'doBypassBridge'
        */
        expectFail = GT_TRUE;
    }
    else
    {
        expectFail = GT_FALSE;
    }


    /* check learn from all the ports in the trunk */
    for(portIter = portIterStart; portIter < portIterEnd ; portIter++)
    {

        if(testDsa == GT_TRUE)
        {
            /* send from the port that is outside the trunk (the cascade port) */
            portNum = cascadePortNum;
            devNum  = prvTgfDevNum;
        }
        else
        {
            portNum = prvTgfPortsArray[portIter];
            devNum  = prvTgfDevsArray[portIter];
        }

        /* send 1 packet */
        prvTgfTrunkTestPacketSend(devNum,portNum ,packetInfoPtr,1,0,NULL);

        if(testSourceFilter == GT_FALSE)
        {
            /* SA learning */

            /* allow the appDemo to add the mac entry */
            cpssOsTimerWkAfter(100);

            brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
            cpssOsMemCpy(&brgMacEntry.key.key.macVlan.macAddr ,
                         multiDestination_prvTgfPacketL2Part_unk_UC.saMac ,
                         sizeof(GT_ETHERADDR));
            brgMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

            /* get the FDB entry */
            rc = prvTgfBrgFdbMacEntryGet(&brgMacEntry.key , &brgMacEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfBrgFdbMacEntryGet: should have found the entry (but not found) \n");

            if(0 == tgfTrunkGlobalEPortRepresentTrunk)
            {
                /* check that mac SA learned on the proper trunk */
                UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_TRUNK_E, brgMacEntry.dstInterface.type,
                    "not get trunk interface \n");
                UTF_VERIFY_EQUAL2_STRING_MAC(trunkId, brgMacEntry.dstInterface.trunkId,
                    "got from trunk[0x%x] and not from proper trunk [0x%x] \n",
                        brgMacEntry.dstInterface.trunkId,
                        trunkId);
            }
            else
            {
                /* check that mac SA learned on the proper eport */
                UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_PORT_E, brgMacEntry.dstInterface.type,
                    "not get port interface \n");
                UTF_VERIFY_EQUAL2_STRING_MAC(expectedEport, brgMacEntry.dstInterface.devPort.portNum,
                    "got from eport[0x%x] and not from proper eport [0x%x] \n",
                        brgMacEntry.dstInterface.devPort.portNum,
                        expectedEport);

                UTF_VERIFY_EQUAL2_STRING_MAC(devNum, brgMacEntry.dstInterface.devPort.hwDevNum,
                    "got from device[0x%x] and not from proper device [0x%x] \n",
                        brgMacEntry.dstInterface.devPort.hwDevNum,
                        devNum);
            }

            /* remove the mac from the FDB */
            rc = prvTgfBrgFdbMacEntryDelete(&(brgMacEntry.key));
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfBrgFdbMacEntryDelete: failed\n");

            /* check the mac was removed */
            rc = prvTgfBrgFdbMacEntryGet(&brgMacEntry.key , &brgMacEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: should NOT found the entry \n");
        }


        /* check that packet egress from the expected port (and only from it) */
        for(ii = 0; ii < prvTgfPortsNum ; ii++)
        {
            if(prvTgfPortsArray[ii] == portNum)
            {
                /* this port is the sender , so expect it to send 1 packet and receive 1 packet (due to loopback port) */
                rxCount = 1;
                txCount = 1;
            }
            else
            {
                rxCount = 0;
                if(expectedEgressPort == prvTgfPortsArray[ii])
                {
                    /* we send packet from port member in the trunk , check single destination on the expected port */
                    txCount = 1;
                }
                else
                {
                    /* this port should not get the packet */
                    txCount = 0;
                }

                if(expectFail == GT_TRUE)
                {
                    txCount = 0;
                }
            }



            /* read counters */
            rc = prvTgfReadPortCountersEth(devNum, prvTgfPortsArray[ii], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         devNum, prvTgfPortsArray[ii]);

            /* copy read counters to  expected counters */
            cpssOsMemCpy(&expectedCntrs, &portCntrs, sizeof(expectedCntrs));

            expectedCntrs.goodPktsRcv.l[0]  = rxCount;
            expectedCntrs.goodPktsSent.l[0] = txCount;


            /* check Rx\Tx counters */
            PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, expectedCntrs, portCntrs);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isEqualCntrs, "get another counters values.");

            /* print expected values if not equal */
            PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs,expectedCntrs,portCntrs);
        }

        clearAllTestedCounters();

        if(prvUtfIsGmCompilation() == GT_TRUE)
        {
            /* reduce by factor 3 the number of iterations */
            portIter += 2;

            break;
        }
    }


    if(testDsa == GT_TRUE)
    {
        /* remove the cascade port configurations */

        /* AUTODOC: (restore) set ingress port as network port */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum, cascadePortNum, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorIngressCscdPortEnableSet: %d %d",
                                     prvTgfDevNum, cascadePortNum);

        /* remove the need to send DSA tag */
        tgfTrafficEnginePacketForceDsaOnPacket(NULL,GT_TRUE);


        if(doBypassBridge == GT_FALSE)
        {
            /* AUTODOC: (restore) state that the packet from DSA will bypass bridge . */
            rc = prvTgfCscdPortBridgeBypassEnableSet(prvTgfDevNum,cascadePortNum,GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    /* remove the mac from the FDB - of the mac DA */
    brgMacEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    cpssOsMemCpy(&brgMacEntry.key.key.macVlan.macAddr ,
                 multiDestination_prvTgfPacketL2Part_unk_UC.daMac ,
                 sizeof(GT_ETHERADDR));
    brgMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    /* remove the mac from the FDB */
    rc = prvTgfBrgFdbMacEntryDelete(&(brgMacEntry.key));
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgFdbMacEntryDelete: failed\n");

    /* check the mac was removed */
    rc = prvTgfBrgFdbMacEntryGet(&brgMacEntry.key , &brgMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: should NOT found the entry \n");


}

/**
* @internal tgfTrunkSaLearningTest function
* @endinternal
*
* @brief   Test mac SA learning from ports that are trunk members in local device,
*         and from from ports that are trunk members in remote device (via the DSA tag)
*/
void tgfTrunkSaLearningTest
(
    void
)
{
    GT_U32  portIter;/* port iterator */
    GT_BOOL     testDsa = GT_FALSE;
    GT_BOOL     testSourceFilter = GT_FALSE;
    GT_BOOL     doBypassBridge = GT_FALSE;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&localPortsVlanMembers);

    testSaLearning = 1;

    /* AUTODOC: this test based on the 'DA hash' test */
    prvTgfTrunkSimpleHashMacSaDaTestInit();

    /* set all ports as VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    prvTgfTrunkVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

    /* AUTODOC: send traffic to check SA learning - non DSA */
    tgfTrunkSaLearningTestTraffic(testDsa , testSourceFilter , doBypassBridge);

    testDsa = GT_TRUE;
    /* AUTODOC: send traffic to check SA learning - DSA */
    tgfTrunkSaLearningTestTraffic(testDsa , testSourceFilter , doBypassBridge);

    testDsa = GT_FALSE;
    testSourceFilter = GT_TRUE;
    /* AUTODOC: send traffic to check source filter - non DSA */
    tgfTrunkSaLearningTestTraffic(testDsa , testSourceFilter , doBypassBridge);

    testDsa = GT_TRUE;
    testSourceFilter = GT_TRUE;
    doBypassBridge = GT_TRUE;
    /* AUTODOC: send traffic to check source filter - DSA , with bypass bridge --> so no filter expected !!! */
    tgfTrunkSaLearningTestTraffic(testDsa , testSourceFilter , doBypassBridge);

    testDsa = GT_TRUE;
    testSourceFilter = GT_TRUE;
    doBypassBridge = GT_FALSE;
    /* AUTODOC: send traffic to check source filter - DSA , no bypass bridge --> so filter expected !!! */
    tgfTrunkSaLearningTestTraffic(testDsa , testSourceFilter , doBypassBridge);

    /* this test based on the 'DA hash' test -- use it's restore */
    prvTgfTrunkSimpleHashMacSaDaTestRestore();

    /* AUTODOC: invalidate VLAN 5 */
    prvTgfTrunkVlanRestore(PRV_TGF_VLANID_CNS);

    testSaLearning = 0;
}



#define L2_ECMP_MAX_PHYSICAL_PORTS_CNS  128
#define L2_ECMP_MAX_PACKETS_CNS         128

#define L2_ECMP_MAX_EPORTS_FOR_CAPTURE_CNS  128

typedef struct{
    GT_U32   numOfEportsMembers;
    GT_U32   numOfPhysicalMembers;
    GT_PORT_NUM  ecmpIndexBaseEport;
    PRV_TGF_CFG_GLOBAL_EPORT_STC ecmpRange;
    PRV_TGF_CFG_GLOBAL_EPORT_STC globalRange;

    GT_U32   hashMaskTableIndex;
    GT_U32   hashMaskFieldType;
    GT_U32   hashMaskFieldSubType;
    GT_U32   hashMaskFieldByteIndex;
    PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType;
    TGF_PACKET_STC       *trafficPtr;
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  lbhMode;
    GT_U32   L3Offset;

    GT_PHYSICAL_PORT_NUM    physicalPortsArr[L2_ECMP_MAX_PHYSICAL_PORTS_CNS];

    GT_U32                  indexIn_packetsOrder;/* index in packetsOrderPhysicalPort[][L2_ECMP_MAX_PACKETS_CNS]*/
    GT_U32                  packetsOrderPhysicalPort[2][L2_ECMP_MAX_PACKETS_CNS];/* hold 'portIter' that got the packet (index in prvTgfPortsArray[])*/
    GT_U32                  packetsOrderEPort[2][L2_ECMP_MAX_PACKETS_CNS];/* hold 'eport offset' (from nL2_SECONDARY_EPORT_MAC(0)) */
    PRV_TGF_TRUNK_LBH_CRC_MODE_ENT  crcMode;
    GT_U8                   saltValue;/* the salt for the tested byte */
    GT_U32                  saltByteIndex;/* the byte index of the tested byte */

    GT_BOOL                 testWithCapturedPackets;

    GT_U32                  targetEportsGotTraffic[(L2_ECMP_MAX_EPORTS_FOR_CAPTURE_CNS + 31) / 32];/* bmp of eports that got the traffic*/

    GT_BOOL                 l2EcmpEnable;

}L2_ECMP_INFO_STC;
/* global info for L2Ecmp 'restore' */
static L2_ECMP_INFO_STC origL2EcmpInfo;
/* global info of current L2Ecmp settings */
static L2_ECMP_INFO_STC currL2EcmpInfo;

#define L2_ECMP_BASE_EPORT_CNS           L2_ECMP_GLOBAL_EPORTS_PATTERN_CNS
/* mask for L2 ecmp global eports (use same mask as global eports of trunk uses)*/
#define L2_ECMP_GLOBAL_EPORTS_MASK_CNS   MASK_FOR_GLOBAL_EPORTS_CNS
/* mask for L2 ecmp global eports */
#define L2_ECMP_GLOBAL_EPORTS_PATTERN_CNS  (PATTERN_FOR_GLOBAL_EPORT_REPRESENT_L2ECMP_CNS)

/* primary EPort 1 that will represent the secondaries */
#define PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS (L2_ECMP_BASE_EPORT_CNS + 128)
/* primary EPort 2 that will represent the secondaries */
#define PRV_TGF_DESTINATION_PRIMARY_EPORT_2_CNS (PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS + 1)

/* the first eport that used as 'secondary' */
#define L2_SECONDARY_BASE_EPORT_NUM_CNS     512
#define L2_SECONDARY_EPORT_MAC(offset)      (L2_SECONDARY_BASE_EPORT_NUM_CNS + (offset))

#define CRC_32_SEED_CNS 0xFEDCBA98

static GT_U32  l2EcmpReduceFactor = 1;
/* indication that the L2ECMP should be filtered and no packet egress */
static GT_BOOL l2EcmpFilterEnabled = GT_FALSE;


static TGF_DSA_DSA_FORWARD_STC  dsaFromRemotePortOfL2EcmpPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/ /* set in runtime */
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    51,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    3,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum*/    /* set in runtime */
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  dsaFromRemotePortOfL2EcmpPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,/*vpt*/
        0,/*cfiBit*/
        PRV_TGF_VLANID_CNS,/*vid*/
        GT_FALSE,/*dropOnSource*/
        GT_FALSE/*packetIsLooped*/
    }/*commonParams*/

/*    union{
        TGF_DSA_DSA_TO_CPU_STC         toCpu;
        TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
        TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
        TGF_DSA_DSA_FORWARD_STC        forward;
    }dsaInfo;*/       /* set in runtime */
};

/**
* @internal l2Ecmpe2PhySet function
* @endinternal
*
* @brief   Set L2ECMP E2Phy for the secondary eports
*/
static void l2Ecmpe2PhySet
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    GT_U32  eportIndex;
    GT_PORT_NUM secondaryEport;
    GT_U32   phyPortNum;
    GT_U8   dev = prvTgfDevNum;
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    GT_U32  divider;

    /* set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo,0,sizeof(physicalInfo));
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;

    divider = (currL2EcmpInfo.numOfEportsMembers / currL2EcmpInfo.numOfPhysicalMembers);

    /* loop on physical ports */
    for(eportIndex = 0 ; eportIndex < currL2EcmpInfo.numOfEportsMembers; eportIndex++ )
    {
        /* use consecutive multiple EPORTs to the same physical port */
        portIter = eportIndex / divider;

        /* make sure no out of boundary */
        portIter %= currL2EcmpInfo.numOfPhysicalMembers;

        secondaryEport = L2_SECONDARY_EPORT_MAC(eportIndex);

        phyPortNum     = prvTgfPortsArray[portIter];
        physicalInfo.devPort.portNum = phyPortNum;
        currL2EcmpInfo.physicalPortsArr[portIter] = phyPortNum;

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(dev,secondaryEport,
                                                               &physicalInfo);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }

    return;

}

/**
* @internal l2EcmpInit function
* @endinternal
*
* @brief   Basic L2ECMP Init
*/
static void l2EcmpInit
(
    void
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   lttEntry;
    PRV_TGF_BRG_L2_ECMP_ENTRY_STC       ecmpEntry;
    GT_U32  l2EcmpIndex = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS - L2_ECMP_BASE_EPORT_CNS;
    CPSS_PORTS_BMP_STC portsVlanMembers = {{0, 0}};
    GT_U32  ii;
    GT_U32  ecmpMaxIndex;

    /* AUTODOC: force link up on all tested ports */
    forceLinkUpOnAllTestedPorts();

    hashMaskTableSize = PRV_TGF_TRUNK_HASH_MASK_TABLE_SIZE_CNS;

    rc = prvTgfBrgL2EcmpEnableGet(dev,&origL2EcmpInfo.l2EcmpEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpEnableGet");
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgL2EcmpEnableGet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: globally enable the L2ECMP */
    rc = prvTgfBrgL2EcmpEnableSet(dev,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpEnableSet");
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgL2EcmpEnableSet FAILED, rc = [%d]", rc);
        return;
    }


    /* save default hashMode */
    rc = prvTgfTrunkHashGlobalModeGet(dev, &prvTgfDefHashMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet");
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashGlobalModeGet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: set general hashing mode - hash is based on the packets data */
    rc = prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet");
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfTrunkHashGlobalModeSet FAILED, rc = [%d]", rc);
        return ;
    }

    /* save base index for L2 ECMP */
    rc = prvTgfBrgL2EcmpIndexBaseEportGet(dev,&origL2EcmpInfo.ecmpIndexBaseEport);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    currL2EcmpInfo.ecmpIndexBaseEport = L2_ECMP_BASE_EPORT_CNS;
    /* AUTODOC: set new base index for L2 ECMP (global eport) */
    rc = prvTgfBrgL2EcmpIndexBaseEportSet(dev,currL2EcmpInfo.ecmpIndexBaseEport);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* save global ePort configuration */
    rc = prvTgfCfgGlobalEportGet(dev, &origL2EcmpInfo.globalRange , &origL2EcmpInfo.ecmpRange);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);

    /* define global ePort range */
    currL2EcmpInfo.ecmpRange.enable = GT_TRUE;
    currL2EcmpInfo.ecmpRange.pattern = L2_ECMP_GLOBAL_EPORTS_PATTERN_CNS;
    currL2EcmpInfo.ecmpRange.mask    = L2_ECMP_GLOBAL_EPORTS_MASK_CNS;

    currL2EcmpInfo.globalRange = currL2EcmpInfo.ecmpRange;

    /* AUTODOC: configure Global ePorts ranges */
    rc = prvTgfCfgGlobalEportSet(dev, &currL2EcmpInfo.globalRange , &currL2EcmpInfo.ecmpRange);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);


    /* AUTODOC: add FDB entry attached to a global EPORT */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac, PRV_TGF_VLANID_CNS, dev,
            PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);

    /* AUTODOC: clear full hash mask table */
    rc = trunkHashMaskCrcTableEntryClear(PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: set E2PHY for the secondary eports */
    l2Ecmpe2PhySet();

    GET_MAX_ECMP_INDEX_MAC(dev,ecmpMaxIndex);

    if (ecmpMaxIndex >= _1K)
    {
        lttEntry.ecmpStartIndex = (ecmpMaxIndex - _1K) + 125;
    }
    else
    {
        lttEntry.ecmpStartIndex = 125 % ecmpMaxIndex;
    }

    lttEntry.ecmpNumOfPaths = currL2EcmpInfo.numOfEportsMembers;
    lttEntry.ecmpEnable = GT_TRUE;

    /* AUTODOC: set the L2ECMP LTT to point to the list of secondary eports */
    rc = prvTgfBrgL2EcmpLttTableSet(dev,l2EcmpIndex,&lttEntry);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    ecmpEntry.targetHwDevice = dev;

    /* AUTODOC: set the L2ECMP list of secondary eports */
    for(ii = 0 ; ii < currL2EcmpInfo.numOfEportsMembers; ii++)
    {
        ecmpEntry.targetEport = L2_SECONDARY_EPORT_MAC(ii);

        rc = prvTgfBrgL2EcmpTableSet(dev,lttEntry.ecmpStartIndex + ii ,&ecmpEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsVlanMembers);

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (ii = 0; ii < prvTgfPortsNum; ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsVlanMembers,prvTgfPortsArray[ii]);
    }

    /* AUTODOC: create VLAN with untagged ports  */
    prvTgfTrunkVlanTestInit(PRV_TGF_VLANID_CNS, portsVlanMembers);


    if(currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
    {
        /* set ALL the egress physical ports as 'egress' cascade port so we can
           capture with eDSA */

        /* AUTODOC: set egress physical ports as cascaded mode (TX direction) */
        for(ii = 0 ; ii < currL2EcmpInfo.numOfPhysicalMembers; ii++)
        {
            rc = prvTgfCscdPortTypeSet(dev, CPSS_PORT_DIRECTION_TX_E,
                prvTgfPortsArray[ii],
                CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                         dev, prvTgfPortsArray[ii]);
        }
    }

}


/**
* @internal l2EcmpCascadeInit function
* @endinternal
*
* @brief   Basic L2ECMP Cascade Init
*
* @param[in] stage                    -  0 or 1.
*                                      on stage 0 - set the src eport on the DSA to be 'primary eport 2'
*                                      on stage 1 - set the src eport on the DSA to be 'primary eport 1'
*                                       None
*/
static void l2EcmpCascadeInit
(
    IN GT_U32   stage
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM  senderPort = prvTgfPortsArray[currL2EcmpInfo.numOfPhysicalMembers];
    GT_U32  srcDsaEport = stage == 0 ?
                PRV_TGF_DESTINATION_PRIMARY_EPORT_2_CNS :
                PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS;

    if(stage == 0)
    {
        /* AUTODOC: set ingress port in cascaded mode (RX direction) */
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(dev, senderPort, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorIngressCscdPortEnableSet: %d %d",
                                     dev, senderPort);
    }

    /* AUTODOC: state that packet will be with DSA and info srcHwDev = 'remote dev' and srcEPort = 'L2ECMP EPort 2'(primary EPort 2) */
    dsaFromRemotePortOfL2EcmpPart_forward.srcHwDev         = REMOTE_DEVICE_NUMBER_CNS;
    dsaFromRemotePortOfL2EcmpPart_forward.source.portNum = srcDsaEport;

    /* AUTODOC: state that packet targeted to 'remote dev' and dstEport = 'L2ECMP EPort 1'(primary EPort 1) */
    dsaFromRemotePortOfL2EcmpPart_forward.dstEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS;
    dsaFromRemotePortOfL2EcmpPart_forward.dstInterface.devPort.hwDevNum = REMOTE_DEVICE_NUMBER_CNS + 0x111;

    dsaFromRemotePortOfL2EcmpPart.dsaInfo.forward = dsaFromRemotePortOfL2EcmpPart_forward;

    tgfTrafficEnginePacketForceDsaOnPacket(&dsaFromRemotePortOfL2EcmpPart,GT_TRUE);

    if(stage == 0)
    {
        /* AUTODOC: add FDB entry attached to a global L2ECMP EPort 2 , on the source mac address */
        /* AUTODOC: the mac is on 'ownDev' and need to check that it is not changed after the traffic pass */
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(
                prvTgfPacketL2Part.saMac, PRV_TGF_VLANID_CNS, dev,
                srcDsaEport, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }
    else
    {
        /* AUTODOC: state that the packet from DSA will not bypass bridge so the mac DA from FDB used */
        /* AUTODOC: NOTE: the "Local Switching of Known Unicast Packets" ignored for 'bypass bridge' */
        rc = prvTgfCscdPortBridgeBypassEnableSet(dev,senderPort,GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }


}

/**
* @internal l2EcmpCascadeRestore function
* @endinternal
*
* @brief   Basic L2ECMP Cascade Restore
*/
static void l2EcmpCascadeRestore
(
    void
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM  senderPort = prvTgfPortsArray[currL2EcmpInfo.numOfPhysicalMembers];

    /* AUTODOC: (restore) set ingress port as network port */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(dev, senderPort, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorIngressCscdPortEnableSet: %d %d",
                                 dev, senderPort);

    /* remove the configuration */
    tgfTrafficEnginePacketForceDsaOnPacket(NULL,GT_TRUE);

    /* AUTODOC: (restore) state that the packet from DSA will bypass bridge . */
    rc = prvTgfCscdPortBridgeBypassEnableSet(dev,senderPort,GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

}

/**
* @internal l2EcmpBridgeGlobalEportsSet function
* @endinternal
*
* @brief   enable/disable the Global EPorts in the bridge but not for the L2ECMP .
*/
static void l2EcmpBridgeGlobalEportsSet
(
    IN GT_BOOL  enable
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;

    /* AUTODOC: remove the global ePort range */
    PRV_TGF_CFG_GLOBAL_EPORT_STC globalRange;

    if(enable == GT_FALSE)
    {
        globalRange.enable = GT_FALSE;

        rc = prvTgfCfgGlobalEportSet(dev, &globalRange , &currL2EcmpInfo.ecmpRange);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }
    else
    {
        rc = prvTgfCfgGlobalEportSet(dev, &currL2EcmpInfo.globalRange , &currL2EcmpInfo.ecmpRange);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }
}


/**
* @internal l2EcmpSaLearnCheck function
* @endinternal
*
* @brief   check that the src eport not changed due to the remote port of L2ECMP .
*
* @param[in] doRelearning             - indication to do relearning.
*                                       None
*/
static void l2EcmpSaLearnCheck
(
    IN GT_BOOL doRelearning
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    PRV_TGF_MAC_ENTRY_KEY_STC    macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;
    GT_U32  srcDsaEport = PRV_TGF_DESTINATION_PRIMARY_EPORT_2_CNS;
    GT_U8   dev = prvTgfDevNum;
    GT_U32  srcDev = (doRelearning == GT_FALSE) ? dev : dsaFromRemotePortOfL2EcmpPart_forward.srcHwDev;

    if(doRelearning == GT_TRUE)
    {
        /* allow the FDB to be updated by the AppDemo */
        cpssOsTimerWkAfter(500);
    }

    /* prepare the key for the lookup in the FDB */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    for (ii = 0; (ii < 6); ii++)
    {
        macEntryKey.key.macVlan.macAddr.arEther[ii] =
            prvTgfPacketL2Part.saMac[ii];
    }

    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    /* do lookup in the FDB to get the associated dev,port */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey , &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    /*check that interface is as expected */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_PORT_E, macEntry.dstInterface.type, "[TGF]: the mac SA NOT learned on 'port' \n");
    UTF_VERIFY_EQUAL0_STRING_MAC(srcDev, macEntry.dstInterface.devPort.hwDevNum, "[TGF]: the mac SA NOT learned on device \n");
    UTF_VERIFY_EQUAL0_STRING_MAC(srcDsaEport, macEntry.dstInterface.devPort.portNum, "[TGF]: the mac SA NOT learned on 'ecmp primary port 2' \n");

    return;
}

/**
* @internal l2EcmpCaptureTrafficEn function
* @endinternal
*
* @brief   start L2ECMP capture packets to the CPU.
*/
static void l2EcmpCaptureTrafficEn(IN GT_BOOL enable)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  ii;

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    for(ii = 0 ; ii < currL2EcmpInfo.numOfPhysicalMembers; ii++)
    {
        portInterface.devPort.portNum = prvTgfPortsArray[ii];

        /* enable/disable capture on port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);
    }

    if(enable == GT_TRUE)
    {
        /* clear table */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }
}

/**
* @internal l2EcmpCheckTrgEportInTrappedPacket function
* @endinternal
*
* @brief   check the trapped packet to check to which target EPort it was send to.
*         by examine it's eDSA
*
* @param[out] trgEPortPtr              - pointer to the actual ePort on the eDSA
*                                       None
*/
static void l2EcmpCheckTrgEportInTrappedPacket
(
    OUT GT_U32 *trgEPortPtr
)
{
    GT_STATUS   rc;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    static  GT_U8   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC rxParam;
    TGF_PACKET_DSA_TAG_STC  dsaInfo;
    TGF_DSA_DSA_FORWARD_STC        *forwardInfoPtr;
    GT_U32          eportOffset;

    *trgEPortPtr = 0;

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E/*packetType*/,
        GT_TRUE/*getFirst always GT_TRUE - since we clear the table before an new send*/,
        GT_FALSE,/*trace*/
        packetBuff,
        &buffLen,
        &packetActualLength,
        &devNum,
        &queue,
        &rxParam);

    if(rc != GT_OK)
    {
        TRUNK_PRV_UTF_LOG0_MAC("[TGF]: not got packets in the CPU \n");

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    /* parse the DSA (it should be FORWARTD eDSA) so we can get the target ePort from it */
    rc = prvTgfTrafficGeneratorPacketDsaTagParse(
        &packetBuff[TGF_L2_HEADER_SIZE_CNS],/* start of DSA after the 12 bytes of the SA,DA mac addresses */
        &dsaInfo);

    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    forwardInfoPtr = &dsaInfo.dsaInfo.forward;

    if(dsaInfo.dsaCommand != TGF_DSA_CMD_FORWARD_E)
    {
        TRUNK_PRV_UTF_LOG0_MAC("[TGF]: not FORWARD DSA \n");
        rc = GT_BAD_PARAM;
    }
    else
    if(dsaInfo.dsaType != TGF_DSA_4_WORD_TYPE_E)
    {
        TRUNK_PRV_UTF_LOG0_MAC("[TGF]: not 4 words DSA \n");
        rc = GT_BAD_PARAM;
    }
    else
    if(forwardInfoPtr->dstEport < L2_SECONDARY_EPORT_MAC(0))
    {
        TRUNK_PRV_UTF_LOG2_MAC("[TGF]: trgEport[0x%x] under the range [0x%x] \n",
            forwardInfoPtr->dstEport ,
            L2_SECONDARY_EPORT_MAC(0));
        /* under the range */
        rc = GT_BAD_PARAM;
    }
    else
    if(forwardInfoPtr->dstEport >= L2_SECONDARY_EPORT_MAC(currL2EcmpInfo.numOfEportsMembers))
    {
        TRUNK_PRV_UTF_LOG2_MAC("[TGF]: trgEport[0x%x] above the range [0x%x] \n",
            forwardInfoPtr->dstEport ,
            L2_SECONDARY_EPORT_MAC(currL2EcmpInfo.numOfEportsMembers));
        /* above the range */
        rc = GT_BAD_PARAM;
    }

    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    eportOffset = forwardInfoPtr->dstEport - L2_SECONDARY_EPORT_MAC(0);

    TRUNK_PRV_UTF_LOG2_MAC("[TGF]: trgEport[0x%x] (offset from start of eports[0x%x])\n",
        forwardInfoPtr->dstEport,
        eportOffset);

    if(((eportOffset / 32) * sizeof(GT_U32)) >= sizeof(currL2EcmpInfo.targetEportsGotTraffic))
    {
        rc = GT_OUT_OF_RANGE;
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    /* set the bit in the array */
    rc = prvCpssFieldValueSet(currL2EcmpInfo.targetEportsGotTraffic,eportOffset,1,1);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    *trgEPortPtr = forwardInfoPtr->dstEport;

    return;

}

/**
* @internal l2EcmpCheckFinalEportsLoadBalance function
* @endinternal
*
* @brief   check the final eports load balance.
*/
static void l2EcmpCheckFinalEportsLoadBalance
(
    void
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32  maxEportsExpected,minEportsExpected;
    GT_U32  eportsActual = 0;
    GT_U32  ii;

    /* summary the number of different eports that got the traffic */
    for(ii = 0 ;
        ii < (sizeof(currL2EcmpInfo.targetEportsGotTraffic) / sizeof(GT_U32));
        ii++)
    {
        eportsActual +=
            prvCpssPpConfigBitmapNumBitsGet(currL2EcmpInfo.targetEportsGotTraffic[ii]);
    }

    TRUNK_PRV_UTF_LOG1_MAC("[TGF]: got [%d] different eports \n",
        eportsActual);


    if(currL2EcmpInfo.crcMode == PRV_TGF_TRUNK_LBH_CRC_32_MODE_E)
    {
        maxEportsExpected = currL2EcmpInfo.numOfEportsMembers;
    }
    else
    {
        /* max 64 */
        maxEportsExpected = currL2EcmpInfo.numOfEportsMembers >= 64 ?
                            64 :
                            currL2EcmpInfo.numOfEportsMembers;
    }

    maxEportsExpected /= l2EcmpReduceFactor;

    /* check for at least 80% distribution on the traffic that was sent */
    minEportsExpected = (maxEportsExpected * 80) / 100;

    if(eportsActual < minEportsExpected)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: less different eports than expected [%d] \n",
            minEportsExpected);
        rc = GT_BAD_PARAM;
    }
    else
    if(eportsActual > maxEportsExpected)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: more different eports than expected [%d] \n",
            maxEportsExpected);
        rc = GT_BAD_PARAM;
    }

    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    return;
}


/**
* @internal l2EcmpSendTraffic function
* @endinternal
*
* @brief   Basic L2ECMP send traffic
*/
static void l2EcmpSendTraffic
(
    void
)
{
    GT_STATUS   rc;
    GT_U8   dev = prvTgfDevNum;
    GT_U32  ii;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32  portTxPacketCounter;
    GT_U32  senderPort;
    GT_U32  portIter;
    GT_U32   phyPortNum;
    GT_U32  numOfPortsGotPacket;
    GT_U32  overAllPackets[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_U32  totalBurst = L2_ECMP_MAX_PACKETS_CNS;
    GT_U32  perfectLbh;
    GT_U32  tolerance = 1;/* +- one packet */
    GT_U32  error = 0;
    GT_U32  trgEPort;

    GT_U32  indexIn_packetsOrder = currL2EcmpInfo.indexIn_packetsOrder;

    if(currL2EcmpInfo.numOfPhysicalMembers > prvTgfPortsNum)
    {
        rc = GT_BAD_PARAM;
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    /* reduce number of sending*/
    totalBurst /= l2EcmpReduceFactor;


    senderPort = prvTgfPortsArray[currL2EcmpInfo.numOfPhysicalMembers];

    /* AUTODOC: use ipv4 UDP packets , with incremental SIP */
    currL2EcmpInfo.trafficType              = PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E;
    currL2EcmpInfo.trafficPtr               = &prvTgfIpv4UdpPacketInfo;
    currL2EcmpInfo.hashMaskTableIndex       = PRV_TGF_TRUNK_HASH_MASK_TABLE_TRAFFIC_TYPE_FIRST_INDEX_CNS + currL2EcmpInfo.trafficType;
    currL2EcmpInfo.hashMaskFieldType        = PRV_TGF_TRUNK_FIELD_IP_ADDR_E;
    currL2EcmpInfo.hashMaskFieldSubType     = PRV_TGF_TRUNK_FIELD_DIRECTION_SOURCE_E;
    currL2EcmpInfo.hashMaskFieldByteIndex   = IPV4_OFFSET_FROM_BYTE_12_CNS;
    currL2EcmpInfo.lbhMode                  = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E;
    currL2EcmpInfo.L3Offset                 = L2_VLAN_TAG_ETHERETYPE_SIZE_CNS;
    currL2EcmpInfo.saltByteIndex            = 23/*start byte for SIP*/ + IPV4_OFFSET_FROM_BYTE_12_CNS/*MSB byte of ipv4*/;

    if(totalBurst == 1)
    {
        /* single packet so no LBH */
        currL2EcmpInfo.lbhMode              = PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E;
    }


    if(currL2EcmpInfo.saltValue)
    {
        rc = prvTgfTrunkHashCrcSaltByteSet(dev,currL2EcmpInfo.saltByteIndex,currL2EcmpInfo.saltValue);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            return;
        }
    }


    /* AUTODOC: set hash mask table */
    rc = trunkHashMaskCrcFieldSet(
            currL2EcmpInfo.hashMaskTableIndex,
            currL2EcmpInfo.hashMaskFieldType,
            currL2EcmpInfo.hashMaskFieldSubType,
            currL2EcmpInfo.hashMaskFieldByteIndex);
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    getSendingInfo = 1;

    /* get sendingInfo.vfdArray[0] info */
    /* NOTE: the function will NOT send traffic ... only calculates sendingInfo.vfdArray[0] */
    rc = trunkHashCrcLbhTrafficSend(currL2EcmpInfo.trafficPtr,currL2EcmpInfo.trafficType,
            currL2EcmpInfo.hashMaskFieldType,currL2EcmpInfo.hashMaskFieldSubType,currL2EcmpInfo.hashMaskFieldByteIndex,
            currL2EcmpInfo.lbhMode,currL2EcmpInfo.L3Offset,GT_FALSE/*checkSimpleHash*/,
            0/*reduceTimeFactor*/,NULL);

    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }

    if(currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
    {
        /* AUTODOC: enable 'capture' of packets to CPU , to examine the target EPORT */
        l2EcmpCaptureTrafficEn(GT_TRUE);
    }

    /* AUTODOC: send 128 incremental packets and check load balance */
    for(ii = 0 ; ii < totalBurst ; ii++)
    {
        /* send one packet at a time , so we can record the egress port */
        prvTgfTrunkTestPacketSend(dev, senderPort, currL2EcmpInfo.trafficPtr ,1 /*burstCount*/ , 1/*numVfd*/ ,sendingInfo.vfdArray);
        sendingInfo.vfdArray[0].patternPtr[0] += (GT_U8)(l2EcmpReduceFactor | 1);

        numOfPortsGotPacket = 0;

        for(portIter = 0 ; portIter < currL2EcmpInfo.numOfPhysicalMembers ;portIter++)
        {
            phyPortNum = currL2EcmpInfo.physicalPortsArr[portIter];


            rc = prvTgfReadPortCountersEth(dev,phyPortNum,GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                dev,phyPortNum);

            portTxPacketCounter = portCntrs.goodPktsSent.l[0];

            if(portTxPacketCounter)
            {
                /* this port got the traffic */
                numOfPortsGotPacket++;

                if(portTxPacketCounter != 1)
                {
                    TRUNK_PRV_UTF_LOG1_MAC("[TGF]: got [%d] more than 1 packet, \n",
                        portTxPacketCounter);
                    rc = GT_BAD_PARAM;
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
                    goto cleanExit_lbl;
                }

                currL2EcmpInfo.packetsOrderPhysicalPort[indexIn_packetsOrder][ii] = portIter;
            }
        }

        if(l2EcmpFilterEnabled == GT_TRUE)
        {
            if(numOfPortsGotPacket != 0)
            {
                TRUNK_PRV_UTF_LOG0_MAC("[TGF]: the source Filtering did not filtered the packet (should have filtered it) \n");
                rc = GT_BAD_PARAM;
                UTF_VERIFY_EQUAL0_PARAM_MAC(0, numOfPortsGotPacket);
                goto cleanExit_lbl;
            }
        }
        else
        if(numOfPortsGotPacket == 0)
        {
            TRUNK_PRV_UTF_LOG0_MAC("[TGF]: no port got packet (should be one port) \n");
            rc = GT_BAD_PARAM;
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, numOfPortsGotPacket);
            goto cleanExit_lbl;
        }
        else
        if(numOfPortsGotPacket != 1)
        {
            TRUNK_PRV_UTF_LOG1_MAC("[TGF]: [%d] ports got packet instead of only one port \n",
                numOfPortsGotPacket);
            rc = GT_BAD_PARAM;
            UTF_VERIFY_EQUAL0_PARAM_MAC(1, numOfPortsGotPacket);
            goto cleanExit_lbl;
        }

        if(currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
        {
            /* check the target eport that was selected */
            l2EcmpCheckTrgEportInTrappedPacket(&trgEPort);

            if(trgEPort)
            {
                TRUNK_PRV_UTF_LOG1_MAC("got eport[0x%x] \n",
                            trgEPort);

                currL2EcmpInfo.packetsOrderEPort[indexIn_packetsOrder][ii] = trgEPort;
            }

            /* clear the trapped table after every send */
            tgfTrafficTableRxPcktTblClear();
        }
    }

    cpssOsMemSet(overAllPackets,0,sizeof(overAllPackets));

    if(totalBurst >= 32)/* when send little amount of packets can't expect good LBH */
    {
        /* calc overall LBH */
        for(ii = 0 ; ii < totalBurst ; ii++)
        {
            portIter = currL2EcmpInfo.packetsOrderPhysicalPort[indexIn_packetsOrder][ii];
            overAllPackets[portIter]++;
        }

        perfectLbh = totalBurst / currL2EcmpInfo.numOfPhysicalMembers;

        for(portIter = 0 ; portIter < currL2EcmpInfo.numOfPhysicalMembers; portIter++ )
        {
            if(overAllPackets[portIter] < (perfectLbh - tolerance) ||
               overAllPackets[portIter] > ((perfectLbh + tolerance)))
            {
                TRUNK_PRV_UTF_LOG3_MAC("[TGF]: port [%d] got [%d] packets instead of[%d] \n",
                    prvTgfPortsArray[portIter],
                    overAllPackets[portIter],
                    perfectLbh);
                error++;
            }
        }

        /* check for errors */
        UTF_VERIFY_EQUAL0_PARAM_MAC(0, error);
    }

    if(totalBurst >= 32 &&
       currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
    {
        /* AUTODOC: check the final eport's load balance */
        l2EcmpCheckFinalEportsLoadBalance();
    }


    if(indexIn_packetsOrder == 1)
    {
        error = 0;
        /* we can now compare the 2 arrays of 'trgEPort order' */
        for(ii = 0 ; ii < totalBurst ; ii++)
        {
            if(currL2EcmpInfo.packetsOrderEPort[0][ii] ==
               currL2EcmpInfo.packetsOrderEPort[1][ii])
            {
                /* expect different eport (out of the 128 eports) */
                error++;
            }
        }

        /* check for errors */
        UTF_VERIFY_EQUAL0_PARAM_MAC(0, error);
    }

    /* clean exit after we set capture and send traffic */
cleanExit_lbl:

    if(currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
    {
        /* AUTODOC: disable 'capture' of packets to CPU */
        l2EcmpCaptureTrafficEn(GT_FALSE);
    }

    /* AUTODOC: reset port's counters */
    clearAllTestedCounters();

    return;

}

/**
* @internal l2Ecmpe2PhyRestore function
* @endinternal
*
* @brief   restore L2ECMP E2Phy for the secondary eports
*/
static void l2Ecmpe2PhyRestore
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    GT_U32  eportIndex;
    GT_PORT_NUM secondaryEport;
    GT_U8   dev = prvTgfDevNum;
    CPSS_INTERFACE_INFO_STC             physicalInfo;
    GT_U32  divider;

    /* set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo,0,sizeof(physicalInfo));
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;

    divider = (currL2EcmpInfo.numOfEportsMembers / currL2EcmpInfo.numOfPhysicalMembers);

    /* loop on physical ports */
    for(eportIndex = 0 ; eportIndex < currL2EcmpInfo.numOfEportsMembers; eportIndex++ )
    {
        /* use consecutive multiple EPORTs to the same physical port */
        portIter = eportIndex / divider;

        /* make sure no out of boundary */
        portIter %= currL2EcmpInfo.numOfPhysicalMembers;

        secondaryEport = L2_SECONDARY_EPORT_MAC(eportIndex);

        physicalInfo.devPort.portNum = eportIndex & 0xFF;/* reset value */

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(dev,secondaryEport,
                                                               &physicalInfo);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);
    }

    return;

}
/**
* @internal l2EcmpRestore function
* @endinternal
*
* @brief   Basic L2ECMP restore
*/
static void l2EcmpRestore
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  l2EcmpIndex = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS - L2_ECMP_BASE_EPORT_CNS;
    GT_U8   dev = prvTgfDevNum;
    GT_U32  ii;

    /* AUTODOC: clear the L2ECMP ltt and the members */
    prvTgfBrgL2EcmpEntryClear(l2EcmpIndex , GT_TRUE);

    /* AUTODOC: clear full hash mask table */
    rc = trunkHashMaskCrcTableEntryClear(PRV_TGF_TRUNK_HASH_MASK_TABLE_INDEX_CLEAR_ALL_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore the E2PHY entries */
    l2Ecmpe2PhyRestore();

    /* AUTODOC: restore general hashing mode - hash is based on the packets data */
    rc = prvTgfTrunkHashGlobalModeSet(prvTgfDefHashMode);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: flush the FDB */
    prvTgfTrunkFdBFlush();

    /* AUTODOC: invalidate the tested vlan */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: reset port's counters */
    clearAllTestedCounters();

    /* AUTODOC: clear table of packets captured to the CPU */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore base index for L2 ECMP */
    rc = prvTgfBrgL2EcmpIndexBaseEportSet(dev,origL2EcmpInfo.ecmpIndexBaseEport);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: restore global ePort configuration */
    rc = prvTgfCfgGlobalEportSet(dev, &origL2EcmpInfo.globalRange , &origL2EcmpInfo.ecmpRange);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);

    if(currL2EcmpInfo.testWithCapturedPackets == GT_TRUE)
    {
        /* AUTODOC: (restore) set egress physical ports as regular network ports */
        for(ii = 0 ; ii < currL2EcmpInfo.numOfPhysicalMembers; ii++)
        {
            rc = prvTgfCscdPortTypeSet(dev, CPSS_PORT_DIRECTION_TX_E,
                prvTgfPortsArray[ii],
                CPSS_CSCD_PORT_NETWORK_E);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                         dev, prvTgfPortsArray[ii]);
        }
    }


    /* AUTODOC: restore global L2ECMP mode (enable/disable) */
    rc = prvTgfBrgL2EcmpEnableSet(dev,origL2EcmpInfo.l2EcmpEnable);
    if (GT_OK != rc)
    {
        TRUNK_PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgL2EcmpEnableGet FAILED, rc = [%d]", rc);
        return;
    }

    /* AUTODOC: restore SALT to be 0 */
    rc = prvTgfTrunkHashCrcSaltByteSet(dev,currL2EcmpInfo.saltByteIndex,0);
    if(rc != GT_OK)
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        return;
    }



}

/**
* @internal tgfL2EcmpBasicCrc32HashTest function
* @endinternal
*
* @brief   Basic L2ECMP test with crc32
*/
void tgfL2EcmpBasicCrc32HashTest
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  numOfEportsMembers = 128;/*128 eports are mapped to 2 physical ports */
    GT_U32  numOfPhysicalMembers = 2;
    GT_U32  origPort[2];/*2 = indexes < numOfPhysicalMembers */

    if(prvUtfIsGmCompilation() == GT_TRUE)
    {
        l2EcmpReduceFactor = 4;/* with factor 8 I got the physical ports 15 in one port and 1 in second port ... not good */
    }

    /* xCat3x not allow cascade ports on 88e1690 ports */
    /* the test need port in index [numOfPhysicalMembers] to support cascade config */
    PRV_TGF_XCAT3X_DEF_PORTS_SAVE_AND_FILTER_CASCADE_MAC(origPort,numOfPhysicalMembers);

    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));

    origL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    origL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    currL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    currL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    /* should be 0 when not doing change of 'SALT' */
    currL2EcmpInfo.indexIn_packetsOrder = 0;
    currL2EcmpInfo.testWithCapturedPackets = GT_TRUE;

    /* AUTODOC: init L2ECMP for testing CRC_32 use more than 6 bits hash */
    currL2EcmpInfo.crcMode = PRV_TGF_TRUNK_LBH_CRC_32_MODE_E;
    /* AUTODOC: set CRC_32 mode and seed */
    rc = prvTgfTrunkHashCrcParametersSet(currL2EcmpInfo.crcMode,CRC_32_SEED_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: initial configurations */
    l2EcmpInit();

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();

    if((prvUtfIsGmCompilation() == GT_FALSE))
    {
        cpssOsMemSet(currL2EcmpInfo.packetsOrderPhysicalPort,0,sizeof(currL2EcmpInfo.packetsOrderPhysicalPort));
        cpssOsMemSet(currL2EcmpInfo.targetEportsGotTraffic,0,sizeof(currL2EcmpInfo.targetEportsGotTraffic));

        currL2EcmpInfo.crcMode = PRV_TGF_TRUNK_LBH_CRC_6_MODE_E;
        /* AUTODOC: set CRC_6 mode and seed ... check that only first physical port get the traffic */
        rc = prvTgfTrunkHashCrcParametersSet(currL2EcmpInfo.crcMode,CRC_32_SEED_CNS);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* AUTODOC: send traffic and check */
        l2EcmpSendTraffic();
    }

    /* AUTODOC: restore configurations */
    l2EcmpRestore();

    /* clear ALL info */
    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));

    l2EcmpReduceFactor = 1;/*restore value*/
}

/**
* @internal tgfL2EcmpBasicCrc32HashSaltTest function
* @endinternal
*
* @brief   Basic L2ECMP test with crc32 and 'salt'
*         the tests uses SALT A and SALT B on the specific byte and check that
*         is produce different hash value (by select different secondary eport)
*/
void tgfL2EcmpBasicCrc32HashSaltTest
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  numOfEportsMembers = 128;/*128 eports are mapped to 2 physical ports */
    GT_U32  numOfPhysicalMembers = 2;
    GT_U32  origPort[2];/*2 = indexes < numOfPhysicalMembers */

    /* AC3X not allow cascade ports on 88e1690 ports */
    /* the test need port in index [numOfPhysicalMembers] to support cascade config */
    PRV_TGF_XCAT3X_DEF_PORTS_SAVE_AND_FILTER_CASCADE_MAC(origPort,numOfPhysicalMembers);

    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));


    /* make the test send only 4 packets */
    l2EcmpReduceFactor = L2_ECMP_MAX_PACKETS_CNS / 4;

    origL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    origL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    currL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    currL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    /* should be 0 when not doing change of 'SALT' */
    currL2EcmpInfo.indexIn_packetsOrder = 0;
    currL2EcmpInfo.testWithCapturedPackets = GT_TRUE;
    currL2EcmpInfo.saltValue = 0xAA;

    /* AUTODOC: init L2ECMP for testing CRC_32 use more than 6 bits hash */
    currL2EcmpInfo.crcMode = PRV_TGF_TRUNK_LBH_CRC_32_MODE_E;
    /* AUTODOC: set CRC_32 mode and seed */
    rc = prvTgfTrunkHashCrcParametersSet(currL2EcmpInfo.crcMode,CRC_32_SEED_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: initial configurations */
    l2EcmpInit();

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();


    currL2EcmpInfo.indexIn_packetsOrder = 1;
    currL2EcmpInfo.saltValue = 0x55;

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();


    /* AUTODOC: restore configurations */
    l2EcmpRestore();

    /* clear ALL info */
    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));

    l2EcmpReduceFactor = 1;/*restore value*/

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, origPort,
                 sizeof(origPort));
}

/**
* @internal tgfL2EcmpSaLearningAndFilterTest function
* @endinternal
*
* @brief   Basic L2ECMP test for SA learning and source filtering.
*         Test L2ECMP for SA learning and source filtering.
*         1. define the L2ECMP as global EPorts in the Bridge(to ignore srcDev,trgDev
*         in SA learning and source filtering) ,
*         2. in FDB set the MAC SA with 'ownDev',eport=L2ECMP .
*         3. send traffic from cascade port ,
*         from 'remote' srcHwDev and with 'srcEPort' of the L2ECMP.
*         4. check that no SA relearning due to change of source device.
*         4.a remove L2ECMP eport from 'global eports'
*         and check that there is re-learning.
*         (restore L2ECMP eport to 'global eports')
*         5. disable 'bride bypass' on cascade port
*         6. send traffic from cascade port ,
*         from 'remote' srcHwDev and with 'srcEPort' of the L2ECMP , with DA
*         that associated with 'ownDev',eport=L2ECMP
*         7. check that packet is filtered although <'ownDev'> != <'remote' srcHwDev>
*         7.a remove L2ECMP eport from 'global eports'
*         and check that packet not filtered.
*/
void tgfL2EcmpSaLearningAndFilterTest
(
    void
)
{
    GT_STATUS   rc;
    GT_U32  numOfEportsMembers   = 2;/*2 eports are mapped to 2 physical ports */
    GT_U32  numOfPhysicalMembers = 2;
    GT_U32  origPort[3];/*3 = [numOfPhysicalMembers+1]*/

    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));

    /* AC3X not allow cascade ports on 88e1690 ports */
    /* the test need port in index [numOfPhysicalMembers] to support cascade config */
    PRV_TGF_XCAT3X_DEF_PORTS_SAVE_AND_FILTER_CASCADE_MAC(origPort,(numOfPhysicalMembers+1));

    /* make the test send only 1 packet */
    l2EcmpReduceFactor = L2_ECMP_MAX_PACKETS_CNS;

    origL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    origL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    currL2EcmpInfo.numOfEportsMembers   = numOfEportsMembers;
    currL2EcmpInfo.numOfPhysicalMembers = numOfPhysicalMembers;

    /* should be 0 when not doing change of 'SALT' */
    currL2EcmpInfo.indexIn_packetsOrder = 0;
    currL2EcmpInfo.testWithCapturedPackets = GT_FALSE;

    /* AUTODOC: init L2ECMP for testing CRC_32 use more than 6 bits hash */
    currL2EcmpInfo.crcMode = PRV_TGF_TRUNK_LBH_CRC_32_MODE_E;
    /* AUTODOC: set CRC_32 mode and seed */
    rc = prvTgfTrunkHashCrcParametersSet(currL2EcmpInfo.crcMode,CRC_32_SEED_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: initial configurations */
    l2EcmpInit();

    /* AUTODOC: initial cascade configurations (stage 0)*/
    l2EcmpCascadeInit(0/*stage 0*/);

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();

    /* AUTODOC: check the SA not do re-learning */
    l2EcmpSaLearnCheck(GT_FALSE);

    /* AUTODOC: disable the global eports in the bridge (but keep in L2ECMP) */
    l2EcmpBridgeGlobalEportsSet(GT_FALSE);

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();

    /* AUTODOC: check the SA do re-learning */
    l2EcmpSaLearnCheck(GT_TRUE);

    /* AUTODOC: re-enable the global eports in the bridge (but keep in L2ECMP) */
    l2EcmpBridgeGlobalEportsSet(GT_TRUE);

    /* AUTODOC: initial cascade configurations (stage 1)*/
    l2EcmpCascadeInit(1/*stage 1*/);

    /* AUTODOC: state that we expect filtering due to 'Source filtering' */
    l2EcmpFilterEnabled = GT_TRUE;

    /* AUTODOC: send traffic and check */
    l2EcmpSendTraffic();

    /* AUTODOC: disable the global eports in the bridge (but keep in L2ECMP) */
    l2EcmpBridgeGlobalEportsSet(GT_FALSE);

    /* AUTODOC: clear the flag of filtering */
    l2EcmpFilterEnabled = GT_FALSE;

    /* AUTODOC: send traffic and check (no filtering) */
    l2EcmpSendTraffic();

    /* AUTODOC: restore cascade configurations */
    l2EcmpCascadeRestore();

    /* AUTODOC: restore configurations */
    l2EcmpRestore();

    /* clear ALL info */
    cpssOsMemSet(&origL2EcmpInfo,0,sizeof(origL2EcmpInfo));
    cpssOsMemSet(&currL2EcmpInfo,0,sizeof(currL2EcmpInfo));

    l2EcmpReduceFactor = 1;/*restore value*/

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, origPort,
                 sizeof(origPort));

}

/**
* @internal tgfTrunkIpclLoadBalancingHashConfig function
* @endinternal
*
* @brief   Configure/Deconfigure IPCL Load balancing hash test.
*
* @param[in] config                   - GT_TRUE  configure
*                                      GT_FALSE deconfigure
*                                       None
*/
void tgfTrunkIpclLoadBalancingHashConfig
(
    IN  GT_BOOL config
)
{
    GT_STATUS st;
    PRV_TGF_PCL_OVERRIDE_UDB_TRUNK_HASH_STC udbOverTrunkHash;
    PRV_TGF_PCL_LOOKUP_CFG_STC              lookupCfg;
    CPSS_INTERFACE_INFO_STC                 interfaceInfo;
    GT_U32                                  ii;
    PRV_TGF_PCL_RULE_FORMAT_UNT             mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pattern;
    PRV_TGF_PCL_ACTION_STC                  action;
    GT_U32                                  ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT        ruleFormat;
    static GT_U32                           rangeBitMap1[4] = {1,0,0,0};
    PRV_TGF_CNC_COUNTER_STC                 counterValue0;
    static GT_U32                           savedRangeBitMap[4];
    static GT_U32                           startBitSaved;
    static GT_U32                           numOfBitSaved;
    static GT_U8                            daMac5;
    static GT_U8                            saMac5;
    static GT_BOOL                          origClearByReadCnc;

    if(GT_TRUE == config)
    {
        /* AUTODOC: Init trunk */
        prvTgfTrunkGenericInit(TRUNK_TEST_TYPE_CRC_HASH_E, GT_FALSE,
            TRUNK_8_PORTS_IN_TEST_CNS );

        /* AUTODOC: Configure IPCL lookup for sender port */
        st = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_1_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(
           GT_OK, st, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

        /* AUTODOC: Set up IPCL key override */
        cpssOsMemSet(&udbOverTrunkHash, 0, sizeof(udbOverTrunkHash));
        udbOverTrunkHash.trunkHashEnableStdNotIp = GT_TRUE;
        udbOverTrunkHash.trunkHashEnableStdIpv4L4 = GT_TRUE;
        udbOverTrunkHash.trunkHashEnableStdUdb = GT_TRUE;
        st = prvTgfPclOverrideUserDefinedBytesByTrunkHashSet(&udbOverTrunkHash);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "prvTgfPclOverrideUserDefinedBytesByTrunkHashSet");

        /* AUTODOC: Set PCL rules */
        for(ii = 0; ii < BIT_6; ii++)
        {
            cpssOsMemSet(&mask,    0, sizeof(mask));
            cpssOsMemSet(&pattern, 0, sizeof(pattern));
            cpssOsMemSet(&action,  0, sizeof(action));
            ruleIndex     = appDemoDxChTcamIpclBaseIndexGet(prvTgfDevNum, 1) + ii;
            ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

            mask.ruleStdNotIp.trunkHash = 0x3F;
            pattern.ruleStdNotIp.trunkHash = ii;

            action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

            action.matchCounter.enableMatchCount  = GT_TRUE;
            if(ii >= BIT_5)
            {
                action.matchCounter.matchCounterIndex = 5;
            }
            else if(ii >= BIT_4)
            {
                action.matchCounter.matchCounterIndex = 4;
            }
            else if(ii >= BIT_3)
            {
                action.matchCounter.matchCounterIndex = 3;
            }
            else if(ii >= BIT_2)
            {
                action.matchCounter.matchCounterIndex = 2;
            }
            else if(ii >= BIT_1)
            {
                action.matchCounter.matchCounterIndex = 1;
            }
            else if(ii >= BIT_0)
            {
                action.matchCounter.matchCounterIndex = 0;
            }

            st = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern,
                &action);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvTgfPclRuleSet: %d, %d",
                ruleFormat, ruleIndex);
        }

        /* AUTODOC: Configure CNC counters to IPCL */
        st = prvTgfCncBlockClientEnableSet(PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "prvTgfCncBlockClientEnableSet %d %d %d",
            PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
            GT_TRUE);

        st = prvTgfCncBlockClientRangesGet(PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
            savedRangeBitMap);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvTgfCncBlockClientRangesGet %d %d", PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E);

        st = prvTgfCncBlockClientRangesSet(PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E, rangeBitMap1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvTgfCncBlockClientRangesSet %d %d", PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E);

        cpssOsMemSet(&counterValue0, 0, sizeof(counterValue0));
        for(ii = 0; ii < 6; ii++)
        {
            st = prvTgfCncCounterSet(PRV_TGF_CNC_BLOCK_IPCL1, ii,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counterValue0);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfCncCounterSet %d %d %d",
                PRV_TGF_CNC_BLOCK_IPCL1, ii,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        }


        st = prvTgfCncCounterClearByReadEnableGet(&origClearByReadCnc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfCncCounterClearByReadEnableGet %d", GT_TRUE);

        st = prvTgfCncCounterClearByReadEnableSet(
            GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfCncCounterClearByReadEnableSet %d", GT_TRUE);

        /* AUTODOC: Save selected bits from hash */
        st = prvTgfTrunkHashNumBitsGet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E,
            &startBitSaved, &numOfBitSaved);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsGet %d %d", prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E);


        /* AUTODOC: enable PCL clients for CNC */
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
        {
            st = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfCncCountingEnableSet");
        }

        /* AUTODOC: Save 5th byte of MAC address */
        daMac5 = multiDestination_prvTgfPacketL2Part_unk_UC.daMac[5];
        saMac5 = multiDestination_prvTgfPacketL2Part_unk_UC.saMac[5];
    }
    else
    {
        /* AUTODOC: Restore 5th byte of MAC address */
        multiDestination_prvTgfPacketL2Part_unk_UC.daMac[5] = daMac5;
        multiDestination_prvTgfPacketL2Part_unk_UC.saMac[5] = saMac5;

        /* AUTODOC: Save selected bits from hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E,
            startBitSaved, numOfBitSaved);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
            "prvTgfTrunkHashNumBitsSet %d %d %d %d", prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E,
            startBitSaved, numOfBitSaved);

        /* AUTODOC: Restore CNC */
        st = prvTgfCncCounterClearByReadEnableSet(
            origClearByReadCnc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfCncCounterClearByReadEnableSet %d", GT_FALSE);

        st = prvTgfCncBlockClientRangesSet(PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
            savedRangeBitMap);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "prvTgfCncBlockClientRangesSet %d %d", PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E);
        st = prvTgfCncBlockClientEnableSet(PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
            "prvTgfCncBlockClientEnableSet %d %d %d",
            PRV_TGF_CNC_BLOCK_IPCL1,
            PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E,
            GT_FALSE);

        /* AUTODOC: Restore PCL rules */
        for(ii = 0; ii < BIT_6; ii++)
        {
            cpssOsMemSet(&mask,    0, sizeof(mask));
            cpssOsMemSet(&pattern, 0, sizeof(pattern));
            cpssOsMemSet(&action,  0, sizeof(action));
            ruleIndex     = appDemoDxChTcamIpclBaseIndexGet(prvTgfDevNum, 1) + ii;
            ruleFormat    = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            st = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern,
                &action);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvTgfPclRuleSet: %d, %d",
                ruleFormat, ruleIndex);
            st = prvTgfPclRuleValidStatusSet(ruleFormat, ruleIndex, GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfPclRuleValidStatusSet: %d, %d %d",
                ruleFormat, ruleIndex, GT_FALSE);
        }

        /* AUTODOC: Disable IPCL key override */
        cpssOsMemSet(&udbOverTrunkHash, 0, sizeof(udbOverTrunkHash));
        st = prvTgfPclOverrideUserDefinedBytesByTrunkHashSet(&udbOverTrunkHash);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "prvTgfPclOverrideUserDefinedBytesByTrunkHashSet");

        /* AUTODOC: Restore IPCL configutation for sender port */
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

        interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.portNum    =
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        interfaceInfo.devPort.hwDevNum   = prvTgfDevNum;

        lookupCfg.enableLookup           = GT_FALSE;
        lookupCfg.dualLookup             = GT_FALSE;
        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        lookupCfg.groupKeyTypes.ipv6Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

        st = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_1_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfPclCfgTblSet");

        st = prvTgfPclIngressPolicyEnable(GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

        st = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvTgfPclPortIngressPolicyEnable: %d", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: enable PCL clients for CNC */
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
        {
            st = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfCncCountingEnableSet");
        }

        /* AUTODOC: Deconfigure trunk */
        prvTgfTrunkGenericRestore(GT_FALSE);
    }
}

/**
* @internal tgfTrunkIpclLoadBalancingHashExecute function
* @endinternal
*
* @brief   Execute IPCL Load balancing hash test.
*/
void tgfTrunkIpclLoadBalancingHashExecute
(
    void
)
{
    GT_U32 ii;
    GT_U32 jj;
    GT_STATUS st;
    GT_U32 expectedCounters1[6][6] =
    {
        {1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1}
    };
    PRV_TGF_CNC_COUNTER_STC counterValue;
    GT_U8  macs[] = {0x01, /*000001*/
                     0x03, /*000011*/
                     0x05, /*000101*/
                     0x09, /*001001*/
                     0x11, /*010001*/
                     0x21  /*100001*/};

    multiDestination_prvTgfPacketL2Part_unk_UC.daMac[5] = 0;
    multiDestination_prvTgfPacketL2Part_unk_UC.saMac[5] = 0;

    cpssOsMemSet(&counterValue, 0, sizeof(counterValue));

    for(ii = 0; ii < sizeof(macs)/sizeof(macs[0]); ii++)
    {
        multiDestination_prvTgfPacketL2Part_unk_UC.daMac[5] = macs[ii];

        /* AUTODOC: Set hash selected bits width equal width of actual hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 0, ii + 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 0, ii + 1);

        /* AUTODOC: Send packets */
        prvTgfTrunkTestPacketSend(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &prvTgfPacketVlanTagInfo, 1, 0, NULL);

        /* AUTODOC: Read CNC counters */
        for(jj = 0; jj < 6; jj++)
        {
            st = prvTgfCncCounterGet(PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counterValue);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfCncCounterSet %d %d %d",
                PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                expectedCounters1[ii][jj], counterValue.packetCount.l[0],
                "Unexpected CNC packet count");
        }

        /* AUTODOC: Select single non-zero bit */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, ii, 1);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, ii, 1);

        /* AUTODOC: Send packets */
        prvTgfTrunkTestPacketSend(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &prvTgfPacketVlanTagInfo, 1, 0, NULL);

        /* AUTODOC: Read CNC counters */
        for(jj = 0; jj < 6; jj++)
        {
            st = prvTgfCncCounterGet(PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counterValue);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfCncCounterSet %d %d %d",
                PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
            if(0 == jj)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(1, counterValue.packetCount.l[0],
                    "Unexpected CNC packet count for counter %d", jj);
            }
            else{
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counterValue.packetCount.l[0],
                    "Unexpected CNC packet count for counter %d", jj);
            }
        }
#if 0
        if(0 == ii)
        {
            continue;
        }

        /* AUTODOC: Set hash selected bits to cut non-zero MSB of actual hash */
        st = prvTgfTrunkHashNumBitsSet(prvTgfDevNum,
            PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 0, ii);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "prvTgfTrunkHashNumBitsSet %d %d %d %d",
            prvTgfDevNum, PRV_TGF_TRUNK_HASH_CLIENT_TYPE_IPCL_E, 0, ii);

        /* AUTODOC: Send packets */
        prvTgfTrunkTestPacketSend(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &prvTgfPacketVlanTagInfo, 1, 0, NULL);

        /* AUTODOC: Read CNC counters */
        for(jj = 0; jj < 6; jj++)
        {
            st = prvTgfCncCounterGet(PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counterValue);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "prvTgfCncCounterSet %d %d %d",
                PRV_TGF_CNC_BLOCK_IPCL1, jj,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
            if(0 == jj)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(
                1, counterValue.packetCount.l[0],
                "Unexpected CNC packet count on counter %d", jj);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(
                0, counterValue.packetCount.l[0],
                "Unexpected CNC packet count on counter %d", jj);
            }
        }
#endif /*The field "Hash Select Last Bit" (27:23) of Policy Engine Configuration Register (0x0200000C) should be fixed */

    }
}

