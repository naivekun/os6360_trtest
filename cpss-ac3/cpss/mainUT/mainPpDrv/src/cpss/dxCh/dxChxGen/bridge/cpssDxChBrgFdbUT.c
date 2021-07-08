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
* @file cpssDxChBrgFdbUT.c
*
* @brief Unit tests for cpssDxChBrgFdb, that provides
* FDB tables facility CPSS DxCh implementation.
*
* @version   97
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfBridgeGen.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <extUtils/auEventHandler/auFdbUpdateLock.h>

#include <stdlib.h>

#ifdef ASIC_SIMULATION
#include <gtOs/gtOsTimer.h>
    #ifdef GM_USED
        /*the GM not switch tasks for 'active' memories. all done in the same contexts */
        /*so if operation not ended ... it will never end ! */
        static GT_U32 utfMaxRetryOnSleep0 = 0;
        static GT_U32 utfMaxRetryOnSleep1 = 2;
    #else
        static GT_U32 utfMaxRetryOnSleep0 = 128;
        static GT_U32 utfMaxRetryOnSleep1 = _10K;
    #endif

#endif

/* Defines */
/* #define UT_FDB_DEBUG */

#ifdef UT_FDB_DEBUG
#define UT_FDB_DUMP(_x) cpssOsPrintf _x
#else
#define UT_FDB_DUMP(_x)
#endif

/* Default valid value for port id */
#define BRG_FDB_VALID_PHY_PORT_CNS  0

/* maximal value of portTrunk parameter */
#define MAX_PORT_TRUNK_MAC(_devNum)                                      \
  ((UTF_CPSS_PP_MAX_TRUNK_ID_MAC(_devNum) > UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(_devNum)) ?  \
    UTF_CPSS_PP_MAX_TRUNK_ID_MAC(_devNum) : UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(_devNum))

/* not valid value for VIDX */
#define MAX_VIDX_MAC(_devNum)                                            \
        (GT_U16)(((_devNum) < PRV_CPSS_MAX_PP_DEVICES_CNS) ?             \
            (1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(_devNum)) : \
            4096)

extern GT_STATUS appDemoDxChAuMsgHandle
(
    IN  GT_U8                devNum,
    IN  CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[],
    IN  GT_U32               numOfAu,
    OUT GT_U32              *errorCounterPtr,
    OUT GT_BOOL             *isFirstMissPtr,
    OUT GT_BOOL             *isFirstReorderDonePtr
);

extern GT_STATUS prvTgfBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
);

extern GT_STATUS prvTgfBrgFdbMacEntryIndexFind_MultiHash
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
);

extern GT_STATUS prvTgfConvertCpssToGenericMacEntry
(
    IN  CPSS_MAC_ENTRY_EXT_STC       *cpssMacEntryPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/* Internal functions forward declaration */
static GT_STATUS prvUtfCoreClockGet(IN GT_U8 dev, OUT GT_U32* coreClockPtr);

static GT_STATUS prvUtfMacTableSize(IN GT_U8 dev, OUT GT_U32* tableSizePtr);

static void prvUtBuildMacAddrKeyFromBitArray(IN GT_U8  dev,
                                             IN GT_U32 bitArray[3],
                                             INOUT CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr,
                                             IN GT_BOOL    fdb16BitFidHashEnable);

static GT_STATUS prvUtTestFdbHash(IN GT_U8 dev,
                                  IN CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr,
                                  IN GT_BOOL    useMultiHash,
                                  IN GT_BOOL    fdb16BitFidHashEnable);


static GT_STATUS prvUtTestFdbHashCases(IN GT_U8 dev,
                                       IN CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr);

static GT_STATUS prvUtCheckMacEntry(IN GT_BOOL valid, IN GT_BOOL skip,
                                    IN CPSS_MAC_ENTRY_EXT_STC *entryPtr,
                                    IN CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr);

static GT_VOID utfCpssDxChBrgFdbBankCounterUpdateStatusGet(IN GT_U8 devNum);

extern GT_STATUS prvTgfBrgFdbActionDoneWaitForAllDev
(
    IN  GT_BOOL  aaTaClosed
);

/* number of multi-hash calculations */
#define NUM_BANKS_MULTI_HASH_CNS     16

#define STR(strname)    \
    #strname

/* indication for sip5 devices that running in 'multi-hash-mode'*/
static GT_BOOL multiHashMode = GT_FALSE;


static GT_U32   orig_currPortGroupsBmp = 0;


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortLearnStatusSet
(
    IN GT_U8                    devNum,
    IN GT_U8                    port,
    IN GT_BOOL                  status,
    IN CPSS_PORT_LOCK_CMD_ENT   cmd
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortLearnStatusSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with status [GT_FALSE, GT_TRUE]
                     and cmd [CPSS_LOCK_FRWRD_E /
                              CPSS_LOCK_DROP_E /
                              CPSS_LOCK_TRAP_E /
                              CPSS_LOCK_MIRROR_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbPortLearnStatusGet
    Expected: GT_OK and the same statusPtr and cmdPtr.
    1.1.3. Call with status [GT_FALSE / GT_TRUE] and out of range cmd.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PORT_NUM            port     = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    CPSS_PORT_LOCK_CMD_ENT cmd      = CPSS_LOCK_FRWRD_E;
    GT_BOOL                stateGet = GT_FALSE;
    CPSS_PORT_LOCK_CMD_ENT cmdGet   = CPSS_LOCK_FRWRD_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with status [GT_FALSE, GT_TRUE]
                         and cmd [CPSS_LOCK_FRWRD_E /
                                  CPSS_LOCK_DROP_E /
                                  CPSS_LOCK_TRAP_E /
                                  CPSS_LOCK_MIRROR_E].
                Expected: GT_OK.
            */

            /*
                1.1.2. Call cpssDxChBrgFdbPortLearnStatusGet
                Expected: GT_OK and the same statusPtr and cmdPtr.
            */

            /* Call with status [GT_FALSE] and cmd [CPSS_LOCK_FRWRD_E] */
            state = GT_FALSE;
            cmd   = CPSS_LOCK_FRWRD_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* Call with status [GT_FALSE] and cmd [CPSS_LOCK_DROP_E] */
            cmd = CPSS_LOCK_DROP_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* Call with status [GT_FALSE] and cmd [CPSS_LOCK_TRAP_E] */
            cmd = CPSS_LOCK_TRAP_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* Call with status [GT_FALSE] and cmd [CPSS_LOCK_MIRROR_E] */
            cmd = CPSS_LOCK_MIRROR_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(cmd, cmdGet,
                       "get another cmd than was set: %d, %d", dev, port);

            /* Call with status [GT_TRUE] and cmd [CPSS_LOCK_FRWRD_E] */
            state = GT_TRUE;
            cmd   = CPSS_LOCK_FRWRD_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);

            /* Call with status [GT_TRUE] and cmd [CPSS_LOCK_DROP_E] */
            cmd = CPSS_LOCK_DROP_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);

            /* Call with status [GT_TRUE] and cmd [CPSS_LOCK_TRAP_E] */
            cmd = CPSS_LOCK_TRAP_E;

            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, cmd);

            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &stateGet, &cmdGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another status than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with status [GT_FALSE / GT_TRUE] and wrong enum values cmd.
                Expected: GT_BAD_PARAM.
            */
            state = GT_FALSE;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbPortLearnStatusSet
                                                        (dev, port, state, cmd),
                                                        cmd);
        }

        state = GT_TRUE;
        cmd   = CPSS_LOCK_FRWRD_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;
    cmd   = CPSS_LOCK_FRWRD_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortLearnStatusSet(dev, port, state, cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortLearnStatusGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   port,
    OUT GT_BOOL *statusPtr,
    OUT CPSS_PORT_LOCK_CMD_ENT *cmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortLearnStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null statusPtr and non-null cmdPtr.
    Expected: GT_OK.
    1.1.2. Call with null statusPtr [NULL] and non-null cmdPtr.
    Expected: GT_BAD_PTR.
    1.1.3. Call with non-null statusPtr and null cmdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS              st    = GT_OK;

    GT_U8                  dev;
    GT_PORT_NUM            port  = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL                state = GT_FALSE;
    CPSS_PORT_LOCK_CMD_ENT cmd   = CPSS_LOCK_FRWRD_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null statusPtr and non-null cmdPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null statusPtr [NULL] and non-null cmdPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, NULL, &cmd);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            /*
                1.1.3. Call with non-null statusPtr and null cmdPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, cmd = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortLearnStatusGet(dev, port, &state, &cmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaToCpuPerPortSet
(
    IN GT_U8    devNum,
    IN GT_U8    port,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaToCpuPerPortSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbNaToCpuPerPortGet
    Expected: GT_OK and the same enablePtr.
*/

    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     port     = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL   state    = GT_FALSE;
    GT_BOOL   stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* enable = GT_FALSE */
            state = GT_FALSE;

            st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgFdbNaToCpuPerPortGet
                Expected: GT_OK and the same enablePtr.
            */
            st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaToCpuPerPortGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* enablePtr = GT_TRUE*/
            state = GT_TRUE;

            st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgFdbNaToCpuPerPortGet
                Expected: GT_OK and the same enablePtr.
            */
            st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaToCpuPerPortGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d, %d", dev, port);
        }
        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        /* enable = GT_FALSE */
        state = GT_FALSE;

        st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

        /*
            1.4.1. Call cpssDxChBrgFdbNaToCpuPerPortGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbNaToCpuPerPortGet: %d, %d", dev, port);

        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                   "get another enable than was set: %d, %d", dev, port);

        /* enablePtr = GT_TRUE*/
        state = GT_TRUE;

        st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

        /*
            1.4.2. Call cpssDxChBrgFdbNaToCpuPerPortGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &stateGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbNaToCpuPerPortGet: %d, %d", dev, port);

        UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                   "get another enable than was set: %d, %d", dev, port);

    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaToCpuPerPortSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaToCpuPerPortGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    port,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaToCpuPerPortGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st    = GT_OK;

    GT_U8      dev;
    GT_PORT_NUM      port  = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL    state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaToCpuPerPortGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaStormPreventSet
(
    IN GT_U8                       devNum,
    IN GT_U8                       port,
    IN GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaStormPreventSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbNaStormPreventGet
    Expected: GT_OK and the same enablePtr.
*/

    GT_STATUS st       = GT_OK;

    GT_U8     dev;
    GT_PORT_NUM     port     = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL   state    = GT_FALSE;
    GT_BOOL   stateGet = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* enable = GT_FALSE */
            state = GT_FALSE;

            st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgFdbNaStormPreventGet
                Expected: GT_OK and the same enablePtr.
            */
            st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaStormPreventGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d, %d", dev, port);

            /* enablePtr = GT_TRUE*/
            state = GT_TRUE;

            st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            /*
                1.1.2. Call cpssDxChBrgFdbNaStormPreventGet
                Expected: GT_OK and the same enablePtr.
            */
            st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaStormPreventGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another enable than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = GT_TRUE;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaStormPreventSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaStormPreventGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       port,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaStormPreventGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (DxChx)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st    = GT_OK;

    GT_U8      dev;
    GT_PORT_NUM      port  = BRG_FDB_VALID_PHY_PORT_CNS;
    GT_BOOL    state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbNaStormPreventGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaStormPreventGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDeviceTableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  devTableBmp
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDeviceTableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with devTableBmp [1].
    Expected: GT_OK.
    1.2. Call with devTableBmp [0xFFFFFFFE] (no constraints in function's contract).
    Expected: GT_OK.
    1.3. Call cpssDxChBrgFdbDeviceTableGet
    Expected: GT_OK and the same devTableBmpPtr.
*/
    GT_STATUS  st        = GT_OK;

    GT_U8      dev;
    GT_U32     devTbl    = 0;
    GT_U32     devTblGet = 0;
    GT_U32     i;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with devTableBmp [1].
            Expected: GT_OK.
        */
        devTbl = 1;

        st = cpssDxChBrgFdbDeviceTableSet(dev, devTbl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, devTbl);

        /*
            1.3. Call cpssDxChBrgFdbDeviceTableGet
            Expected: GT_OK and the same devTableBmpPtr.
        */
        st = cpssDxChBrgFdbDeviceTableGet(dev, &devTblGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbDeviceTableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(devTbl, devTblGet,
                       "get another devTableBmp than was set: %d", dev);

        /*
            1.2. Call with devTableBmp [0xFFFFFFFE] (no constraints in function's contract).
            Expected: GT_OK.
        */
        devTbl = 0xFFFFFFFE;

        /* for dual HW devices clear the odd bit */
        for (i= 0; i < 32; i+=2)
        {
            if (PRV_CPSS_DXCH_IS_DUAL_HW_DEVICE_MAC(i))
            {
                U32_SET_FIELD_MAC(devTbl,i+1,1,0);
            }
        }
        st = cpssDxChBrgFdbDeviceTableSet(dev, devTbl);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, devTbl);

        /*
            1.3. Call cpssDxChBrgFdbDeviceTableGet
            Expected: GT_OK and the same devTableBmpPtr.
        */
        st = cpssDxChBrgFdbDeviceTableGet(dev, &devTblGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbDeviceTableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(devTbl, devTblGet,
                       "get another devTableBmp than was set: %d", dev);
    }

    devTbl = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbDeviceTableSet(dev, devTbl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDeviceTableSet(dev, devTbl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDeviceTableGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devTableBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDeviceTableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null devTableBmpPtr.
    Expected: GT_OK.
    1.2. Call with null devTableBmpPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS    st     = GT_OK;

    GT_U8        dev;
    GT_U32       devTbl = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null devTableBmpPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbDeviceTableGet(dev, &devTbl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null devTableBmpPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbDeviceTableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbDeviceTableGet(dev, &devTbl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDeviceTableGet(dev, &devTbl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* function will be commonly used from other FDB UT tests*/
void utfCpssDxChBrgFdbFromCpuAuMsgStatusGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  mustSucceed
)
{
    GT_STATUS   st;
    GT_BOOL completed;
    GT_BOOL succeeded;
    #ifdef ASIC_SIMULATION
    GT_U32  sleepTime = 0;
    GT_U32  maxRetry = utfMaxRetryOnSleep0;
    GT_U32  totalMaxRetry = utfMaxRetryOnSleep1;
    #endif/*ASIC_SIMULATION*/

    if(usePortGroupsBmp == GT_FALSE)
    {
        completed = GT_FALSE;
        succeeded = GT_FALSE;

        do{
            #ifdef ASIC_SIMULATION
            if ((sleepTime == 0) && maxRetry)
            {
                /* do max retries with 'busy wait' but with option to other tasks
                   to also run --> reduce run time */
                maxRetry--;
            }
            else if (maxRetry == 0)
            {
                sleepTime = 1;
            }

            totalMaxRetry--;
            if(totalMaxRetry == 0)
            {
                PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbFromCpuAuMsgStatusGet Timed out after [%d] seconds \n", 10);

                /* indicate ERROR */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, completed, devNum);
                return;
            }
            else if((totalMaxRetry % 1000) == 0)
            {
                /* show the wait process , every 1 seconds */
                PRV_UTF_LOG0_MAC("@");
            }

            osTimerWkAfter(sleepTime);
            #endif /*ASIC_SIMULATION*/
            st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(devNum, &completed, &succeeded,NULL);
            if(st != GT_OK)
            {
                PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbFromCpuAuMsgStatusGet FAILED, rc = [%d] \n", st);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
                return;
            }
        } while (completed == GT_FALSE);

        if(succeeded == GT_FALSE && mustSucceed == GT_TRUE)
        {
            /* operation finished but not succeeded */
            st = GT_FAIL;
            PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbFromCpuAuMsgStatusGet FAILED, rc = [%d] \n", st);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
    }
    else
    {
        tgfCpssDxChBrgFdbFromCpuAuMsgStatusPerPortGroupsGet(devNum,
                                                           currPortGroupsBmp,
                                                           mustSucceed);
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntrySet(devNum,macEntryPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntrySet(devNum,currPortGroupsBmp,macEntryPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;

        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_U32        index
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryInvalidate(devNum,index);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryInvalidate(devNum,currPortGroupsBmp,index);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_HW_DEV_NUM           *associatedHwDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryRead(devNum,index,validPtr,skipPtr,agedPtr,associatedHwDevNumPtr,entryPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryRead(devNum,currPortGroupsBmp,index,validPtr,skipPtr,agedPtr,associatedHwDevNumPtr,entryPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbQaSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbQaSend(devNum,macEntryKeyPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupQaSend(devNum,currPortGroupsBmp,macEntryKeyPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryDelete
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryDelete(devNum,macEntryKeyPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryDelete(devNum,currPortGroupsBmp,macEntryKeyPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryWrite(devNum,index,skip,macEntryPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryWrite(devNum,currPortGroupsBmp,index,skip,macEntryPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryStatusGet(devNum,index,validPtr,skipPtr);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryStatusGet(devNum,currPortGroupsBmp,index,validPtr,skipPtr);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/* function will be commonly used from other FDB UT tests*/
GT_STATUS utfCpssDxChBrgFdbMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_BOOL      age
)
{
    if(usePortGroupsBmp == GT_FALSE)
    {
        return cpssDxChBrgFdbMacEntryAgeBitSet(devNum,index,age);
    }
    else
    {
        GT_STATUS   rc;
        orig_currPortGroupsBmp = currPortGroupsBmp;
        TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

        rc = cpssDxChBrgFdbPortGroupMacEntryAgeBitSet(devNum,currPortGroupsBmp,index,age);

        /*restore value*/
        currPortGroupsBmp = orig_currPortGroupsBmp;
        return rc;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntrySet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                                    macVlan->vlanId[100],
                                                    macVlan->macAddr[00:1A:FF:FF:FF:FF]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [0]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},
                                               isStatic [GT_FALSE],
                                               daCommand [CPSS_MAC_TABLE_FRWRD_E],
                                               saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                                               daRoute [GT_FALSE],
                                               mirrorToRxAnalyzerPortEn [GT_FALSE],
                                               sourceID [0],
                                               userDefined [0],
                                               daQosIndex [0],
                                               saQosIndex [0],
                                               daSecurityLevel [0],
                                               saSecurityLevel [0],
                                               appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E,
                                                    ipMcast->vlanId[100],
                                                    ipMcast->sip={123.12.12.3},
                                                    ipMcast->dip = {123.34.67.8}},
                                                dstInterface {type[CPSS_INTERFACE_VID_E],
                                                              devPort {devNum [devNum],
                                                                       portNum [0]},
                                                              trunkId [0],
                                                              vidx [10],
                                                              vlanId [100]},
                                                 isStatic [GT_FALSE],
                                                 daCommand [CPSS_MAC_TABLE_INTERV_E],
                                                 saCommand [CPSS_MAC_TABLE_CNTL_E],
                                                 daRoute [GT_FALSE],
                                                 mirrorToRxAnalyzerPortEn [GT_FALSE],
                                                 sourceID [BIT_5-1],
                                                 userDefined [0],
                                                 daQosIndex [30],
                                                 saQosIndex [101],
                                                 daSecurityLevel [10],
                                                 saSecurityLevel [40],
                                                 appSpecificCpuCode [GT_TRUE]}
    Expected: GT_OK.
    1.3. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                   and other - the same as in 1.2.
    Expected: NON GT_OK (IP MC support only VIDX and VID).
    1.4. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E,
                                                    ipMcast->vlanId[100],
                                                    ipMcast->sip={123.12.12.3},
                                                    ipMcast->dip={123.34.67.8}},
                                               dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [0]},
                                                             trunkId [0],
                                                             vidx [10],
                                                             vlanId [100]},
                                               isStatic [GT_FALSE],
                                               daCommand [CPSS_MAC_TABLE_SOFT_DROP_E],
                                               saCommand [CPSS_MAC_TABLE_DROP_E],
                                               daRoute [GT_TRUE],
                                               mirrorToRxAnalyzerPortEn [GT_TRUE],
                                               sourceID [0],
                                               userDefined [0],
                                               daQosIndex [0],
                                               saQosIndex [0],
                                               daSecurityLevel [0],
                                               saSecurityLevel [0],
                                               appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                   and other - the same as in 1.4.
    Expected: NON GT_OK (IP MC support only VIDX and VID).
    1.6. Call function with out of range macEntryPtr->key->ipMcast->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                       key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E].
    Expected: GT_OUT_OF_RANGE.
    1.7. Call function with valid macEntryPtr {key { CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                               macVlan->vlanId[100],
                                               macVlan-> macAddr[00:1A:FF:FF:FF:FF]},
                                               dstInterface {type[CPSS_INTERFACE_PORT_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [0]},
                                                             trunkId [0],
                                                             vidx [10],
                                                             vlanId [100]},
                                                isStatic [GT_FALSE],
                                                daCommand [CPSS_MAC_TABLE_FRWRD_E],
                                                saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                                                daRoute [GT_FALSE],
                                                mirrorToRxAnalyzerPortEn [GT_FALSE],
                                                sourceID [0],
                                                userDefined [0],
                                                daQosIndex [0],
                                                saQosIndex [0],
                                                daSecurityLevel [0],
                                                saSecurityLevel [0],
                                                appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.7. Call function with null macEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call function with out of range key->entryType
                            and other parameters the same as in 1.7.
    Expected: GT_BAD_PARAM.
    1.9. Call function with out of range dstInterface->type
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.10. Call function with out of range macEntryPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                             key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: GT_OUT_OF_RANGE.
    1.12. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.7.
    Expected: NOT GT_OK.
    1.13. Call function with out of range dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.7.
    Expected: NON GT_OK.
    1.14. Call function with out of range dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.7.
    Expected: NOT GT_OK.
    1.15. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.7.
    Expected: NOT GT_OK.
    1.16. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                             and dstInterface->type [CPSS_INTERFACE_VID_E].
    Expected: GT_OK.
    1.17. Call function with out of range macEntryPtr->daCommand
                             and other parameters the same as in 1.7.
    Expected: GT_BAD_PARAM.
    1.18. Call function with out of range macEntryPtr->saCommand,
                             key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E] (saCommand is not important for IPv4 and IPv6 MC).
                             and other parameters the same as in 1.7.
    Expected: GT_BAD_PARAM.
    1.19. Call with userDefined [16] and other params from 1.7.
    Expected: NOT GT_OK.
    1.20. Call with daQosIndex [8] and other params from 1.7.
    Expected: NOT GT_OK.
    1.21. Call with saQosIndex [8] and other params from 1.7.
    Expected: NOT GT_OK.
    1.22. Call with daSecurityLevel [8] and other params from 1.7.
    Expected: NOT GT_OK.
    1.23. Call with saSecurityLevel [8] and other params from 1.7.
    Expected: NOT GT_OK.
    1.24. Call with sourceID [32] and other params from 1.7.
    Expected: NOT GT_OK.
*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    CPSS_MAC_ENTRY_EXT_STC  macEntry_dummy;
    static GT_U32 crcMultiHashArr[NUM_BANKS_MULTI_HASH_CNS];
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode,restoreHashMode;
    GT_U32                  notAppFamilyBmp = 0;
    GT_U32  hashIndexArr[16];
    GT_BOOL completed,succeeded;
    GT_U32  entryOffset;
    GT_U32  expected_entryIndex;
    GT_U32  ii;
    GT_U32  hashIndexMask = 0,hashIndexMask_mustBeZero = 0;
    GT_U32  numEntries = 0;


    if(multiHashMode == GT_TRUE)
    {
        /* this feature is on eArch devices */
        UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    }
    else
    {
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    }

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                                    macVlan->vlanId[100],
                                                    macVlan->macAddr[00:1A:FF:FF:FF:FF]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [0]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},
                                               isStatic [GT_FALSE],
                                               daCommand [CPSS_MAC_TABLE_FRWRD_E],
                                               saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                                               daRoute [GT_FALSE],
                                               mirrorToRxAnalyzerPortEn [GT_FALSE],
                                               sourceID [0],
                                               userDefined [0],
                                               daQosIndex [0],
                                               saQosIndex [0],
                                               daSecurityLevel [0],
                                               saSecurityLevel [0],
                                               appSpecificCpuCode [GT_FALSE]}.
            Expected: GT_OK.
        */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId = 100;

        macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum  = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        st = cpssDxChBrgFdbHashModeGet(dev, &restoreHashMode);
        if (st != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashModeGet FAILED, rc = [%d] \n", st);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            if(multiHashMode == GT_TRUE)
            {
                hashMode = CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
                st = cpssDxChBrgFdbHashModeSet(dev, hashMode);
                if (st != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashModeSet FAILED, rc = [%d] \n", st);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }
            }
            else
            {
                hashMode = restoreHashMode;
            }

            /* make the entry that inserted by message to go to 'non-zero' offset */
            if(hashMode != CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
            {
                /* calculate the entry hash index by CPSS hash algorithm */
                st = cpssDxChBrgFdbHashCalc(dev, &macEntry.key, &hashIndexArr[0]);
                if (st != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashCalc FAILED, rc = [%d] \n", st);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }

                hashIndexArr[1] = hashIndexArr[0] + 1;
                hashIndexArr[2] = hashIndexArr[0] + 2;
                hashIndexArr[3] = hashIndexArr[0] + 3;
                ii = 4;
                for(/*continue*/; ii < 16 ; ii++)
                {
                    hashIndexArr[ii] = 0;
                }

                numEntries = 3;/* to check entry at index [hash[3]]*/
                hashIndexMask               = 0x3;/* 2 bits */
            }
            else
            {
                /* we need to calculate the multiple hash results */
                st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev,
                            &macEntry.key,
                            0,/*multiHashStartBankIndex*/
                            NUM_BANKS_MULTI_HASH_CNS,/*numOfBanks*/
                            crcMultiHashArr);
                if (st != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashCrcMultiResultsCalc FAILED, rc = [%d] \n", st);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                }

                for(ii = 0 ; ii < 16 ; ii++)
                {
                    hashIndexArr[ii] = crcMultiHashArr[ii];
                }

                numEntries = 13;/* to check entry at index [hash[13]]*/
                hashIndexMask = 0xf;/* 4 bits */
            }

            /* the bits that are not in 'entry offset' must be ZERO */
            /* the field is 5 bits but not getting all bits         */
            hashIndexMask_mustBeZero = 0x1F - hashIndexMask;

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* entry should fail due to
                    macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
                */
                macEntry_dummy = macEntry;
                ii = 0 ;
                macEntry_dummy.key.key.macVlan.macAddr.arEther[5] = (GT_U8)(0x00 + ii);
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, hashIndexArr[ii], GT_FALSE, &macEntry_dummy);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, hashIndexArr[ii], GT_FALSE);

                /* allow next code to succeed with allowed command */
                macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
            }


            macEntry_dummy = macEntry;
            for(ii = 0 ; ii < numEntries ; ii++)
            {
                macEntry_dummy.key.key.macVlan.macAddr.arEther[5] = (GT_U8)(0x00 + ii);
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, hashIndexArr[ii], GT_FALSE, &macEntry_dummy);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, hashIndexArr[ii], GT_FALSE);
            }
        }

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, &succeeded,&entryOffset);
            if(st != GT_OK)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbFromCpuAuMsgStatusGet FAILED, rc = [%d] \n", st);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            expected_entryIndex = hashIndexArr[numEntries];

#ifndef GM_USED
            UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(0, entryOffset);
#else
            /* the GM always hold value 0 !!! */
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, entryOffset);
            entryOffset = expected_entryIndex & hashIndexMask;
            /*
                see opened JIRA :
                MT-327 - VERIFIER : not update of field <na_entry_offset> in register MESSAGE_FROM_CPU_MANAGEMENT .
            */

#endif /*GM_USED*/

            UTF_VERIFY_EQUAL4_STRING_MAC((expected_entryIndex & hashIndexMask), entryOffset,
                    "expected_entryIndex = 0x%x , (expected_entryIndex & 0x%x) = 0x%x, entryOffset= 0x%x",
                    expected_entryIndex , hashIndexMask , (expected_entryIndex & hashIndexMask) , entryOffset);

            UTF_VERIFY_EQUAL3_STRING_MAC((entryOffset & hashIndexMask_mustBeZero), 0,
                    "entryOffset = 0x%x , (entryOffset & 0x%x) = 0x%x, but Expected to be ZERO",
                    entryOffset , hashIndexMask_mustBeZero , (entryOffset & hashIndexMask_mustBeZero));


            /* we can delete now the dummy entries */
            for(ii = 0 ; ii < numEntries ; ii++)
            {
                st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, hashIndexArr[ii]);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
        }


        /*
            1.2. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E,
                                                    ipMcast->vlanId[100],
                                                    ipMcast->sip={123.12.12.3},
                                                    ipMcast->dip = {123.34.67.8}},
                                                dstInterface {type[CPSS_INTERFACE_VID_E],
                                                              devPort {devNum [devNum],
                                                                       portNum [0]},
                                                              trunkId [0],
                                                              vidx [10],
                                                              vlanId [100]},
                                                 isStatic [GT_FALSE],
                                                 daCommand [CPSS_MAC_TABLE_INTERV_E],
                                                 saCommand [CPSS_MAC_TABLE_CNTL_E],
                                                 daRoute [GT_FALSE],
                                                 mirrorToRxAnalyzerPortEn [GT_FALSE],
                                                 sourceID [BIT_5-1],
                                                 userDefined [0],
                                                 daQosIndex [3],
                                                 saQosIndex [5],
                                                 daSecurityLevel [1],
                                                 saSecurityLevel [4],
                                                 appSpecificCpuCode [GT_TRUE]}
            Expected: GT_OK.
        */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = 100;

        macEntry.key.key.ipMcast.sip[0] = 123;
        macEntry.key.key.ipMcast.sip[1] = 12;
        macEntry.key.key.ipMcast.sip[2] = 12;
        macEntry.key.key.ipMcast.sip[3] = 3;
        macEntry.key.key.ipMcast.dip[0] = 123;
        macEntry.key.key.ipMcast.dip[1] = 34;
        macEntry.key.key.ipMcast.dip[2] = 67;
        macEntry.key.key.ipMcast.dip[3] = 8;

        macEntry.dstInterface.type            = CPSS_INTERFACE_VID_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_CNTL_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = BIT_5-1;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 3;
        macEntry.saQosIndex               = 5;
        macEntry.daSecurityLevel          = 1;
        macEntry.saSecurityLevel          = 4;
        macEntry.appSpecificCpuCode       = GT_TRUE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saSecurityLevel          = 4;
                macEntry.saQosIndex               = 5;
                macEntry.daQosIndex               = 3;

                NOTE: the next not fail because not relevant to IPMC entry
                macEntry.saCommand                = CPSS_MAC_TABLE_CNTL_E;

            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.saSecurityLevel          = 0;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.saQosIndex               = 0;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.daQosIndex               = 0;
        }

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        if(macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E ||
           macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E)
        {
            /* other tests can not delete those entries from the FDB with the 'FLUSH FDB' */
            st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /*
            1.3. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                           and other - the same as in 1.2.
            Expected: NON GT_OK (IP MC support only VIDX and VID).
        */
        macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);

        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                        dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.4. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E,
                                                    ipMcast->vlanId[100],
                                                    ipMcast->sip={123.12.12.3},
                                                    ipMcast->dip={123.34.67.8}},
                                               dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [0]},
                                                             trunkId [0],
                                                             vidx [10],
                                                             vlanId [100]},
                                               isStatic [GT_FALSE],
                                               daCommand [CPSS_MAC_TABLE_SOFT_DROP_E],
                                               saCommand [CPSS_MAC_TABLE_DROP_E],
                                               daRoute [GT_TRUE],
                                               mirrorToRxAnalyzerPortEn [GT_TRUE],
                                               sourceID [0],
                                               userDefined [0],
                                               daQosIndex [0],
                                               saQosIndex [0],
                                               daSecurityLevel [0],
                                               saSecurityLevel [0],
                                               appSpecificCpuCode [GT_FALSE]}.
            Expected: GT_OK.
        */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = 100;

        macEntry.key.key.ipMcast.sip[0] = 123;
        macEntry.key.key.ipMcast.sip[1] = 12;
        macEntry.key.key.ipMcast.sip[2] = 12;
        macEntry.key.key.ipMcast.sip[3] = 3;
        macEntry.key.key.ipMcast.dip[0] = 123;
        macEntry.key.key.ipMcast.dip[1] = 34;
        macEntry.key.key.ipMcast.dip[2] = 67;
        macEntry.key.key.ipMcast.dip[3] = 8;

        macEntry.dstInterface.type            = CPSS_INTERFACE_VID_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_SOFT_DROP_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_DROP_E;
        macEntry.daRoute                  = GT_TRUE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
                macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        }


        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        if(macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E ||
           macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E)
        {
            /* other tests can not delete those entries from the FDB with the 'FLUSH FDB' */
            st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /*
            1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                           and other - the same as in 1.4.
            Expected: NON GT_OK (IP MC support only VIDX and VID).
        */
        macEntry.dstInterface.type = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                        dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.6. Call function with out of range macEntryPtr->key->ipMcast->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.key.ipMcast.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
        macEntry.dstInterface.type      = CPSS_INTERFACE_VID_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntryPtr->key->ipMcast->vlanId = %d",
                                     dev, macEntry.key.key.ipMcast.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.7. Call function with null macEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntrySet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.8. Call function with wrong enum values key->entryType
                                    and other parameters the same as in 1.7.
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.key.ipMcast.vlanId = 100;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.key.entryType);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.9. Call function with wrong enum values dstInterface->type
                                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.dstInterface.type);

        macEntry.dstInterface.type = CPSS_INTERFACE_VID_E;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        if(macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E ||
           macEntry.key.entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E)
        {
            /* other tests can not delete those entries from the FDB with the 'FLUSH FDB' */
            st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.10. Call function with out of range macEntryPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                                     key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.dstInterface.type      = CPSS_INTERFACE_VID_E;
        macEntry.key.key.macVlan.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "%d, macEntryPtr->key->key->macVlan->vlanId = %d",
                                     dev, macEntry.key.key.macVlan.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.12. Call function with out of range
            dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                                     dstInterface->type = CPSS_INTERFACE_PORT_E
                                     and other parameters the same as in 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.key.key.macVlan.vlanId       = 100;
        macEntry.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, macEntryPtr->dstInterface.devPort.portNum = %d",
                                         dev, macEntry.dstInterface.devPort.portNum);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.13. Call function with out of range
            dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                                     dstInterface->type = CPSS_INTERFACE_PORT_E
                                     and other parameters the same as in 1.7.
            Expected: NON GT_OK.
        */
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->dstInterface->devPort->hwDevNum = %d",
                                     dev, macEntry.dstInterface.devPort.hwDevNum);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.14. Call function with out of range
            dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                                     dstInterface->type = CPSS_INTERFACE_TRUNK_E
                                     and other parameters the same as in 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.dstInterface.devPort.hwDevNum = dev;
        macEntry.dstInterface.trunkId        = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.type           = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->trunkId = %d",
                                     dev, macEntry.dstInterface.trunkId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.15. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                                     dstInterface->type = CPSS_INTERFACE_VIDX_E
                                     and other parameters the same as in 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.dstInterface.trunkId = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx    = MAX_VIDX_MAC(dev);
        macEntry.dstInterface.type    = CPSS_INTERFACE_VIDX_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vidx = %d",
                                         dev, macEntry.dstInterface.vidx);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.16. Call function with out of range
                    dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                                     and dstInterface->type [CPSS_INTERFACE_VID_E].
            Expected: GT_OK.
        */
        macEntry.dstInterface.vidx   = 10;
        macEntry.dstInterface.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.dstInterface.type   = CPSS_INTERFACE_VID_E;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saCommand = CPSS_MAC_TABLE_DROP_E

                because the field is not part of the entry and the global mode
                is CPSS_MAC_TABLE_SOFT_DROP_E
            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.saCommand = CPSS_MAC_TABLE_SOFT_DROP_E;
        }

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vlanId = %d",
                                     dev, macEntry.dstInterface.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.17. Call function with wrong enum values macEntryPtr->daCommand
                                     and other parameters the same as in 1.7.
            Expected: GT_BAD_PARAM.
        */
        macEntry.dstInterface.vlanId = 100;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.daCommand);

        macEntry.daCommand  = CPSS_MAC_TABLE_SOFT_DROP_E;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.18. Call function with wrong enum values macEntryPtr->saCommand,
                                     key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E]
                                     (saCommand is not important for IPv4 and IPv6 MC).
                                     and other parameters the same as in 1.7.
            Expected: GT_BAD_PARAM.
        */
        macEntry.daCommand     = CPSS_MAC_TABLE_INTERV_E;
        macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.saCommand);

        macEntry.saCommand                = CPSS_MAC_TABLE_DROP_E;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.19. Call with userDefined [16] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.userDefined = 16;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->userDefined = %d",
                                         dev, macEntry.userDefined);

        macEntry.userDefined = 0;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.20. Call with daQosIndex [8] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.daQosIndex = 8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daQosIndex = %d",
                                         dev, macEntry.daQosIndex);

        macEntry.daQosIndex = 0;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.21. Call with saQosIndex [8] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.saQosIndex = 8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saQosIndex = %d",
                                         dev, macEntry.saQosIndex);

        macEntry.saQosIndex = 0;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.22. Call with daSecurityLevel [8] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.daSecurityLevel = 8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daSecurityLevel = %d",
                                         dev, macEntry.daSecurityLevel);

        macEntry.daSecurityLevel = 0;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.23. Call with saSecurityLevel [8] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.saSecurityLevel = 8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saSecurityLevel = %d",
                                         dev, macEntry.saSecurityLevel);

        macEntry.saSecurityLevel = 0;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.24. Call with sourceID [32] and other params from 1.7.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID = 32;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->sourceID = %d",
                                         dev, macEntry.sourceID);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            st = cpssDxChBrgFdbHashModeSet(dev, restoreHashMode);
            if (st != GT_OK)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgFdbHashModeSet FAILED, rc = [%d] \n", st);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }
    }

    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId = 100;

    macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.devPort.hwDevNum  = dev;
    macEntry.dstInterface.devPort.portNum = 0;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.vidx            = 10;
    macEntry.dstInterface.vlanId          = 100;

    macEntry.isStatic                 = GT_FALSE;
    macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
    macEntry.daRoute                  = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.sourceID                 = 0;
    macEntry.userDefined              = 0;
    macEntry.daQosIndex               = 0;
    macEntry.saQosIndex               = 0;
    macEntry.daSecurityLevel          = 0;
    macEntry.saSecurityLevel          = 0;
    macEntry.appSpecificCpuCode       = GT_FALSE;
    macEntry.age                      = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#if 0
/*----------------------------------------------------------------------------*/
/* testsing entires with key type = ipv4. ipv6 is not supported in adding by message

GT_STATUS cpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRouteEntrySet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[1.2.3.4],
                                                    ipv4Unicast->vrfId[0]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [1]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},

                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                              CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]
                                               }.
    Expected: GT_OK.
    1.2. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                   and other - the same as in 1.1.
    Expected: GT_OK
    1.3. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                   and other - the same as in 1.1.
    Expected: GT_OK
    1.4. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                       key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
    Expected: GT_OUT_OF_RANGE.
    1.5. Call function with null macEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call function with out of range key->entryType
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with out of range dstInterface->type
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                             key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
    Expected: GT_OUT_OF_RANGE.
    1.9. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.10. Call function with out of range dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
    Expected: NON GT_OK.
    1.11. Call function with out of range dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.12. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.13. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                             and dstInterface->type [CPSS_INTERFACE_VID_E].
    Expected: GT_BAD_PARAM.

    1.14. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                             and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.15. Call function with out of range macEntryPtr->qosProfileIndex[128],
                             and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.16. Call with out of range qosPrecedence and other params from 1.1.
    Expected: NOT GT_OK.
    1.17. Call with out of range dipAccessLevel [64] and other params from 1.1.
    Expected: NOT GT_OK.
    1.18. Call with with out of mtuProfileIndex [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.19. Call with with out of nextHopVlanId [8192] and other params from 1.1.
    Expected: NOT GT_OK.
    1.20. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
    Expected: NOT GT_OK.
    1.21. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
    Expected: NOT GT_OK.
    1.22. Call with with out of countSet [4] and other params from 1.1.
    Expected: NOT GT_OK.

*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    GT_U32                  notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /*1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[1.2.3.4],
                                                    ipv4Unicast->vrfId[0]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [1]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},

                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                              CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]
                                               }.
            Expected: GT_OK.
        */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 0;

        macEntry.key.key.ipv4Unicast.dip[0] = 1;
        macEntry.key.key.ipv4Unicast.dip[1] = 2;
        macEntry.key.key.ipv4Unicast.dip[2] = 3;
        macEntry.key.key.ipv4Unicast.dip[3] = 4;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum  = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.age                      = GT_TRUE;

        macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_TRUE;
        macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
        macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
        macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.qosProfileIndex=3;
        macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.countSet=1;
        macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.dipAccessLevel=5;
        macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.mtuProfileIndex=1;
        macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
        macEntry.fdbRoutingInfo.nextHopVlanId=6;
        macEntry.fdbRoutingInfo.nextHopARPPointer=7;
        macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*1.2. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                       and other - the same as in 1.1.
        Expected: GT_OK
        */
        macEntry.dstInterface.type      = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                        dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
        1.3. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                       and other - the same as in 1.1.
        Expected: GT_OK
        */
        macEntry.dstInterface.type      = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                        dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.4. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                       key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        macEntry.key.key.ipv4Unicast.dip[0] = 1;
        macEntry.key.key.ipv4Unicast.dip[1] = 2;
        macEntry.key.key.ipv4Unicast.dip[2] = 3;
        macEntry.key.key.ipv4Unicast.dip[3] = 4;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.key.key.ipv4Unicast.vrfId = %d",
                                     dev, macEntry.key.key.ipv4Unicast.vrfId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.5. Call function with null macEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntrySet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.6. Call function with out of range key->entryType
                            and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 2;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.key.entryType);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.7. Call function with out of range dstInterface->type
                                and other parameters the same as in 1.1.
            Expeted: GT_BAD_PARAM.
        */
        macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.dstInterface.type);

        macEntry.dstInterface.type = CPSS_INTERFACE_VIDX_E;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.8. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                 key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
            Expected: GT_OUT_OF_RANGE.

        */
        macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "%d, macEntry.key.key.ipv4Unicast.vrfId = %d",
                                     dev, macEntry.key.key.ipv4Unicast.vrfId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.9. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
            Expected: NOT GT_OK.
       */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 1;

        macEntry.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, macEntryPtr->dstInterface.devPort.portNum = %d",
                                         dev, macEntry.dstInterface.devPort.portNum);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.10. Call function with out of range dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
            Expected: NON GT_OK.
        */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 1;

        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->dstInterface->devPort->hwDevNum = %d",
                                     dev, macEntry.dstInterface.devPort.hwDevNum);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.11. Call function with out of range dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                                 dstInterface->type = CPSS_INTERFACE_TRUNK_E
                                 and other parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 1;

        macEntry.dstInterface.devPort.hwDevNum = dev;
        macEntry.dstInterface.trunkId        = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.type           = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->trunkId = %d",
                                     dev, macEntry.dstInterface.trunkId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.12. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                                     dstInterface->type = CPSS_INTERFACE_VIDX_E
                                     and other parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.dstInterface.trunkId = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx    = MAX_VIDX_MAC(dev);
        macEntry.dstInterface.type    = CPSS_INTERFACE_VIDX_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vidx = %d",
                                         dev, macEntry.dstInterface.vidx);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.13. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                                     and dstInterface->type [CPSS_INTERFACE_VID_E].
            Expected: GT_BAD_PARAM.
        */
        macEntry.dstInterface.vidx   = 10;
        macEntry.dstInterface.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.dstInterface.type   = CPSS_INTERFACE_VID_E;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vlanId = %d",
                                     dev, macEntry.dstInterface.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.14. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                                 and other parameters the same as in 1.1.
            Expected: GT_OUT_OF_RANGE.
        */

        macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=7;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = %d",
                                     dev, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex);


        macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.15. Call function with out of range macEntryPtr->qosProfileIndex[128],
                                 and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        macEntry.fdbRoutingInfo.qosProfileIndex=128;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntry.fdbRoutingInfo.qosProfileIndex = %d",
                                     dev, macEntry.fdbRoutingInfo.qosProfileIndex);


        macEntry.fdbRoutingInfo.qosProfileIndex=1;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.16. Call with out of range qosPrecedence and other params from 1.1.
            Expected: NOT GT_OK.
        */
         UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.fdbRoutingInfo.qosPrecedence);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.17. Call with out of range dipAccessLevel [64] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.fdbRoutingInfo.dipAccessLevel=64;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.dipAccessLevel = %d",
                                     dev, macEntry.fdbRoutingInfo.dipAccessLevel);


        macEntry.fdbRoutingInfo.dipAccessLevel=5;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.18. Call with with out of mtuProfileIndex [8] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.fdbRoutingInfo.mtuProfileIndex=8;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.mtuProfileIndex = %d",
                                     dev, macEntry.fdbRoutingInfo.mtuProfileIndex);


        macEntry.fdbRoutingInfo.mtuProfileIndex=2;


        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.19. Call with with out of nextHopVlanId [8192] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.fdbRoutingInfo.nextHopVlanId=8192;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopVlanId = %d",
                                     dev, macEntry.fdbRoutingInfo.nextHopVlanId);


        macEntry.fdbRoutingInfo.nextHopVlanId=100;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.20. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.fdbRoutingInfo.nextHopARPPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp <= _128K ?
                                                _128K:
                                                PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopARPPointer = %d",
                                     dev, macEntry.fdbRoutingInfo.nextHopARPPointer);


        macEntry.fdbRoutingInfo.nextHopARPPointer=444;


        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.21. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
            Expected: NOT GT_OK.
        */
        macEntry.fdbRoutingInfo.nextHopTunnelPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart <= _32K ?
                                                    _32K :
                                                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopTunnelPointer = %d",
                                     dev, macEntry.fdbRoutingInfo.nextHopTunnelPointer);


        macEntry.fdbRoutingInfo.nextHopTunnelPointer=666;


        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);


        /*
            1.22. Call with with out of countSet [4] and other params from 1.1.
            Expected: NOT GT_OK.
        */

        macEntry.fdbRoutingInfo.countSet=4;

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.countSet = %d",
                                     dev, macEntry.fdbRoutingInfo.countSet);


        macEntry.fdbRoutingInfo.countSet=2;


        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);


        /* Delete entry added */

        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 0;

        macEntry.key.key.ipv4Unicast.dip[0] = 1;
        macEntry.key.key.ipv4Unicast.dip[1] = 2;
        macEntry.key.key.ipv4Unicast.dip[2] = 3;
        macEntry.key.key.ipv4Unicast.dip[3] = 4;

        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

         /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
    }

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
    macEntry.key.key.ipv4Unicast.vrfId = 0;

    macEntry.key.key.ipv4Unicast.dip[0] = 1;
    macEntry.key.key.ipv4Unicast.dip[1] = 2;
    macEntry.key.key.ipv4Unicast.dip[2] = 3;
    macEntry.key.key.ipv4Unicast.dip[3] = 4;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.devPort.hwDevNum  = dev;
    macEntry.dstInterface.devPort.portNum = 0;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.hwDevNum  = dev;
    macEntry.dstInterface.vidx            = 10;
    macEntry.dstInterface.vlanId          = 100;

    macEntry.age                      = GT_TRUE;

    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_TRUE;
    macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.qosProfileIndex=3;
    macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.countSet=1;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.dipAccessLevel=5;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.mtuProfileIndex=1;
    macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
    macEntry.fdbRoutingInfo.nextHopVlanId=6;
    macEntry.fdbRoutingInfo.nextHopARPPointer=7;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }
#endif

/*----------------------------------------------------------------------------*/
/* testsing entires with key type = ipv4. ipv6 is not supported in adding by message

GT_STATUS cpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRouteEntrySet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[1.2.3.4],
                                                    ipv4Unicast->vrfId[0]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [1]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},

                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                              CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]
                                               }.
    Expected: GT_OK.
    1.2. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                   and other - the same as in 1.1.
    Expected: GT_OK
    1.3. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                   and other - the same as in 1.1.
    Expected: GT_OK
    1.4. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                       key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
    Expected: GT_BAD_PARAM.
    1.5. Call function with null macEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call function with out of range key->entryType
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with out of range dstInterface->type
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                             key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
    Expected: GT_BAD_PARAM.
    1.9. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.10. Call function with out of range dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.1.
    Expected: NON GT_OK.
    1.11. Call function with out of range dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.12. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.13. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                             and dstInterface->type [CPSS_INTERFACE_VID_E].
    Expected: GT_OK - when CPSS_INTERFACE_VID_E the vidx is set to 0xFFF.

    1.14. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                             and other parameters the same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.15. Call function with out of range macEntryPtr->qosProfileIndex[128],
                             and other parameters the same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.16. Call with out of range qosPrecedence and other params from 1.1.
    Expected: NOT GT_OK.
    1.17. Call with out of range dipAccessLevel [64] and other params from 1.1.
    Expected: NOT GT_OK.
    1.18. Call with with out of mtuProfileIndex [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.19. Call with with out of nextHopVlanId [8192] and other params from 1.1.
    Expected: NOT GT_OK.
    1.20. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
    Expected: NOT GT_OK.
    1.21. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
    Expected: NOT GT_OK.
    1.22. Call with with out of countSet [5] and other params from 1.1.
    Expected: NOT GT_OK.

*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    GT_U32                  notAppFamilyBmp = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /*1.1. Call function with valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[1.2.3.4],
                                                    ipv4Unicast->vrfId[0]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [1]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},

                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                              CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]
                                               }.
            Expected: GT_OK.
        */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 0;

        macEntry.key.key.ipv4Unicast.dip[0] = 1;
        macEntry.key.key.ipv4Unicast.dip[1] = 2;
        macEntry.key.key.ipv4Unicast.dip[2] = 3;
        macEntry.key.key.ipv4Unicast.dip[3] = 4;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum  = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.age                      = GT_TRUE;

        macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_TRUE;
        macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
        macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
        macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.qosProfileIndex=3;
        macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.countSet=1;
        macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.dipAccessLevel=5;
        macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.mtuProfileIndex=1;
        macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
        macEntry.fdbRoutingInfo.nextHopVlanId=6;
        macEntry.fdbRoutingInfo.nextHopARPPointer=7;
        macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
                macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
                macEntry.fdbRoutingInfo.qosProfileIndex=3;
                macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
                macEntry.fdbRoutingInfo.dipAccessLevel=5;
            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.ingressMirror=GT_FALSE;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_FALSE;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.qosProfileIndex=0;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_FALSE;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.fdbRoutingInfo.dipAccessLevel=0;
        }

        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);

        if(!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_FALSE);
        }
        else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*1.2. Call with macEntryPtr->dstInterface->type[CPSS_INTERFACE_PORT_E]
                           and other - the same as in 1.1.
            Expected: GT_OK
            */
            macEntry.dstInterface.type      = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                            dev, macEntry.key.entryType, macEntry.dstInterface.type);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
            1.3. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                           and other - the same as in 1.1.
            Expected: GT_OK
            */
            macEntry.dstInterface.type      = CPSS_INTERFACE_TRUNK_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, macEntryPtr->key->entryType = %d, macEntryPtr->dstInterface->type = %d",
                                            dev, macEntry.key.entryType, macEntry.dstInterface.type);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.4. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                           key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
                Expected: GT_BAD_PARAM
            */
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;

            macEntry.key.key.ipv4Unicast.dip[0] = 1;
            macEntry.key.key.ipv4Unicast.dip[1] = 2;
            macEntry.key.key.ipv4Unicast.dip[2] = 3;
            macEntry.key.key.ipv4Unicast.dip[3] = 4;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntry.key.key.ipv4Unicast.vrfId = %d",
                                         dev, macEntry.key.key.ipv4Unicast.vrfId);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.5. Call function with null macEntryPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.6. Call function with out of range key->entryType
                                and other parameters the same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 2;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                            (dev, &macEntry),
                                                            macEntry.key.entryType);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.7. Call function with out of range dstInterface->type
                                    and other parameters the same as in 1.1.
                Expeted: GT_BAD_PARAM.
            */
            macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntry.key.entryType = %d",
                                         dev, macEntry.key.entryType);

            macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, macEntry.key.entryType = %d",
                                         dev, macEntry.key.entryType);

            macEntry.dstInterface.type = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.8. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                     key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
                Expected: GT_BAD_PARAM.

            */
            macEntry.key.entryType     = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                         "%d, macEntry.key.key.ipv4Unicast.vrfId = %d",
                                         dev, macEntry.key.key.ipv4Unicast.vrfId);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.9. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                                 dstInterface->type = CPSS_INTERFACE_PORT_E
                                 and other parameters the same as in 1.1.
                Expected: NOT GT_OK.
           */
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 1;

            macEntry.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
            macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                    "%d, macEntryPtr->dstInterface.devPort.portNum = %d",
                                             dev, macEntry.dstInterface.devPort.portNum);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.10. Call function with out of range dstInterface->devPort->hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS],
                                 dstInterface->type = CPSS_INTERFACE_PORT_E
                                 and other parameters the same as in 1.1.
                Expected: NON GT_OK.
            */
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 1;

            macEntry.dstInterface.devPort.portNum = 0;
            macEntry.dstInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
            macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "%d, macEntryPtr->dstInterface->devPort->hwDevNum = %d",
                                         dev, macEntry.dstInterface.devPort.hwDevNum);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.11. Call function with out of range dstInterface->trunkId [MAX_TRUNK_ID_CNS  = 128],
                                     dstInterface->type = CPSS_INTERFACE_TRUNK_E
                                     and other parameters the same as in 1.1.
                Expected: NOT GT_OK.
            */

            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 1;

            macEntry.dstInterface.devPort.hwDevNum = dev;
            macEntry.dstInterface.trunkId        = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.type           = CPSS_INTERFACE_TRUNK_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->trunkId = %d",
                                         dev, macEntry.dstInterface.trunkId);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.12. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                                         dstInterface->type = CPSS_INTERFACE_VIDX_E
                                         and other parameters the same as in 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.dstInterface.trunkId = 10;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.vidx    = MAX_VIDX_MAC(dev);
            macEntry.dstInterface.type    = CPSS_INTERFACE_VIDX_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vidx = %d",
                                             dev, macEntry.dstInterface.vidx);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.13. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                                         and dstInterface->type [CPSS_INTERFACE_VID_E].
                Expected: GT_OK - when CPSS_INTERFACE_VID_E the vidx is set to 0xFFF.
            */
            macEntry.dstInterface.vidx   = 10;
            macEntry.dstInterface.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
            macEntry.dstInterface.type   = CPSS_INTERFACE_VID_E;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vlanId = %d",
                                         dev, macEntry.dstInterface.vlanId);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.14. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                                     and other parameters the same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */

            if(macEntry.fdbRoutingInfo.ingressMirror == GT_TRUE)
            {
                macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=7;

                st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = %d",
                                             dev, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex);
            }


            macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.15. Call function with out of range macEntryPtr->qosProfileIndex[128],
                                     and other parameters the same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            macEntry.fdbRoutingInfo.qosProfileIndex=128;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.fdbRoutingInfo.qosProfileIndex = %d",
                                         dev, macEntry.fdbRoutingInfo.qosProfileIndex);


            macEntry.fdbRoutingInfo.qosProfileIndex=1;

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.16. Call with out of range qosPrecedence and other params from 1.1.
                Expected: NOT GT_OK.
            */
             UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                            (dev, &macEntry),
                                                            macEntry.fdbRoutingInfo.qosPrecedence);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.17. Call with out of range dipAccessLevel [64] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.dipAccessLevel=64;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.dipAccessLevel = %d",
                                         dev, macEntry.fdbRoutingInfo.dipAccessLevel);


            macEntry.fdbRoutingInfo.dipAccessLevel=5;

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.18. Call with with out of mtuProfileIndex [8] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.mtuProfileIndex=8;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.mtuProfileIndex = %d",
                                         dev, macEntry.fdbRoutingInfo.mtuProfileIndex);


            macEntry.fdbRoutingInfo.mtuProfileIndex=2;


            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.19. Call with with out of nextHopVlanId [8192] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.nextHopVlanId=8192;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopVlanId = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopVlanId);


            macEntry.fdbRoutingInfo.nextHopVlanId=100;

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.20. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
                Expected: NOT GT_OK.
            */
             macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
            macEntry.fdbRoutingInfo.nextHopARPPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp <= _128K ?
                                                _128K:
                                                PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopARPPointer = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopARPPointer);


            macEntry.fdbRoutingInfo.nextHopARPPointer=444;


            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /*
                1.21. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.isTunnelStart = GT_TRUE;
            macEntry.fdbRoutingInfo.nextHopTunnelPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart <= _32K ?
                                                    _32K :
                                                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.nextHopTunnelPointer = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopTunnelPointer);


            macEntry.fdbRoutingInfo.nextHopTunnelPointer=666;


            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);


            /*
                1.22. Call with with out of countSet [5] and other params from 1.1.
                Expected: NOT GT_OK.
            */

            macEntry.fdbRoutingInfo.countSet=5;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.countSet = %d",
                                         dev, macEntry.fdbRoutingInfo.countSet);


            macEntry.fdbRoutingInfo.countSet=2;


            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
        }
        else if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            macEntry.fdbRoutingInfo.routingType = CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.routingType = %d",
                                dev, macEntry.fdbRoutingInfo.routingType);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            macEntry.fdbRoutingInfo.multipathPointer = 20;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.multipathPointer = %d",
                                dev, macEntry.fdbRoutingInfo.multipathPointer);
        }
        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
             /* Try to delete entry added */
            cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 0;

            macEntry.key.key.ipv4Unicast.dip[0] = 1;
            macEntry.key.key.ipv4Unicast.dip[1] = 2;
            macEntry.key.key.ipv4Unicast.dip[2] = 3;
            macEntry.key.key.ipv4Unicast.dip[3] = 4;

            st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
            if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev, PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E))
            {
                /* delete by message is not supported for bobcat2 - [JIRA]:[MT-231] [FE-2293984]
                   CPU NA message for deleting an entry does not work for UC route entries */
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
        }
    }

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
    macEntry.key.key.ipv4Unicast.vrfId = 0;

    macEntry.key.key.ipv4Unicast.dip[0] = 1;
    macEntry.key.key.ipv4Unicast.dip[1] = 2;
    macEntry.key.key.ipv4Unicast.dip[2] = 3;
    macEntry.key.key.ipv4Unicast.dip[3] = 4;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.devPort.hwDevNum  = dev;
    macEntry.dstInterface.devPort.portNum = 0;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.hwDevNum  = dev;
    macEntry.dstInterface.vidx            = 10;
    macEntry.dstInterface.vlanId          = 100;

    macEntry.age                      = GT_TRUE;

    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_TRUE;
    macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.qosProfileIndex=3;
    macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.countSet=1;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.dipAccessLevel=5;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.mtuProfileIndex=1;
    macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
    macEntry.fdbRoutingInfo.nextHopVlanId=6;
    macEntry.fdbRoutingInfo.nextHopARPPointer=7;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbQaSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbQaSend)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                 macVlan->vlanId[100],
                                                 macVlan->macAddr[00:1A:FF:FF:FF:FF]}
         For eArch devices (except bobcat a0) the function will be called twice:
             1) entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E
             2) entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
    Expected: GT_OK.
    1.2. FOR eARCH DEVICES ONLY (EXCEPT BOBCAT2 A0)!
         Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E.
         Note:
         It is useless to run this case for other devices because it will be
         failed till cpssDxChBrgFdbQaSend (on utfCpssDxChBrgFdbMacEntrySet).
    Expected: GT_OK
    1.3. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                                 ipMcast->vlanId[100],
                                                 ipMcast->sip[123.23.34.5],
                                                 ipMacst [123.45.67.8]}.
    Expected: GT_OK.
    1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.5. Call function with null macEntryKeyPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call function with out of range macEntryKeyPtr >entryType
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with out of range macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                            entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
    Expected: GT_OUT_OF_RANGE.
    1.8. Call function with out of range macEntryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                            entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS                  st       = GT_OK;

    GT_U8                      dev;
    CPSS_MAC_ENTRY_EXT_STC     macEntry;
    GT_STATUS                  expected;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        expected = GT_OK;

        /*
            1.1. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                         macVlan->vlanId[100],
                                                         macVlan->macAddr[00:1A:FF:FF:FF:FF]}
            Expected: GT_OK.
            1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
                      GT_BAD_PARAM - for other devices

        */
        expected = GT_OK;
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.vid1               = 123; /* relevant for eArch devices */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId = 100;

        macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum          = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_TRUE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
                macEntry.key.vid1               = 123; origVid1 [123] must be ZERO because not exists in the entry (due to muxing mode)
            */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.key.vid1                 = 0;
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }
        /* add MAC entry before QA send
           this should be done to get SUCCESS during
           utfCpssDxChBrgFdbFromCpuAuMsgStatusGet */
        st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the QA was done and with SUCCESS */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /* clean FDB */
        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the delete was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);


        /* 1.2. Same as 1.1 except entryType =
                                    CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E.
                eArch devices only (except bobcat a0)
           Expected: GT_OK
        */
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
            /* add MAC entry before QA send
               this should be done to get SUCCESS during
               utfCpssDxChBrgFdbFromCpuAuMsgStatusGet */
            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait for acknowledge that the QA was done and with SUCCESS */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

            /* clean FDB */
            st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry.key);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait for acknowledge that the delete was done */
                utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
        }
        /*
            1.3. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                                         ipMcast->vlanId[100],
                                                         ipMcast->sip[123.23.34.5],
                                                         ipMacst->dip[123.45.67.8]}.
            Expected: GT_OK.
        */
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = 100;

        macEntry.key.key.ipMcast.sip[0] = 123;
        macEntry.key.key.ipMcast.sip[1] = 23;
        macEntry.key.key.ipMcast.sip[2] = 34;
        macEntry.key.key.ipMcast.sip[3] = 5;

        macEntry.key.key.ipMcast.dip[0] = 123;
        macEntry.key.key.ipMcast.dip[1] = 45;
        macEntry.key.key.ipMcast.dip[2] = 67;
        macEntry.key.key.ipMcast.dip[3] = 8;

        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  wait for acknowledge that the QA was done and without SUCCESS */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_FALSE);

        /*
            1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
                      GT_BAD_PARAM - for other devices
        */
        expected = (PRV_CPSS_SIP_5_10_CHECK_MAC(dev) && !PRV_CPSS_SIP_6_CHECK_MAC(dev)) ?
            GT_OK : GT_BAD_PARAM;

        macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /*
            1.5. Call function with null macEntryKeyPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbQaSend(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);


        /*
            1.6. Call function with wrong enum values macEntryKeyPtr->entryType
                                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbQaSend
                            (dev, &macEntry.key),
                            macEntry.key.entryType);

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;


        /*
            1.7. Call function with out of range
                macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "%d, macEntryPtr->key->ipMcast->vlanId = %d",
                                     dev, macEntry.key.key.ipMcast.vlanId);

        /*
            1.8. Call function with out of range
                    macEntryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.key.ipMcast.vlanId     = 100;
        macEntry.key.key.macVlan.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "%d, macEntryPtr->key->macVlan->vlanId = %d",
                                     dev, macEntry.key.key.macVlan.vlanId);

      /* wait for acknowledge that the QA was done and don't care about success */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_FALSE);
    }

    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId = 100;

    macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbQaSend(dev, &macEntry.key);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryDelete
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryDelete)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid macEntryKeyPtr {entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                  macVlan->vlanId[100],
                                                  macVlan->macAddr[00:1A:FF:FF:FF:FF]}
         For eArch devices (except bobcat a0) the function will be called twice:
             1) entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E
             2) entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
    Expected: GT_OK.
    1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.3. Call function with valid macEntryKeyPtr {entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                                  ipMcast->vlanId[100],
                                                  ipMcast->sip[123.23.34.5],
                                                  ipMacst [123.45.67.8]}.
    Expected: GT_OK.
    1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.5. Call function with null macEntryKeyPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call function with out of range macEntryKeyPtr->entryType
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call function with out of range macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                            entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
    Expected: GT_OUT_OF_RANGE.
    1.8. Call function with out of range macEntryKeyPtr->key->macVlan->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                            entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS                  st = GT_OK;


    GT_U8                      dev;
    CPSS_MAC_ENTRY_EXT_KEY_STC macEntry;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid macEntryKeyPtr {entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                          macVlan->vlanId[100],
                                                          macVlan->macAddr[00:1A:FF:FF:FF:FF]}
            Expected: GT_OK.
        */
        macEntry.vid1               = 123; /* relevant for eArch devices */

        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.macVlan.vlanId = 100;

        macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
                      GT_BAD_PARAM - for other devices
         */
        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
            1.3. Call function with valid macEntryKeyPtr {entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                                          ipMcast->vlanId[100],
                                                          ipMcast->sip[123.23.34.5],
                                                          ipMacst->dip[123.45.67.8]}.
            Expected: GT_OK.
        */
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.ipMcast.vlanId = 100;

        macEntry.key.ipMcast.sip[0] = 123;
        macEntry.key.ipMcast.sip[1] = 23;
        macEntry.key.ipMcast.sip[2] = 34;
        macEntry.key.ipMcast.sip[3] = 5;

        macEntry.key.ipMcast.dip[0] = 123;
        macEntry.key.ipMcast.dip[1] = 45;
        macEntry.key.ipMcast.dip[2] = 67;
        macEntry.key.ipMcast.dip[3] = 8;

        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
           1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E
           Expected: GT_OK        - for eArch except bobcat a0.
                     GT_BAD_PARAM - for other devices
         */
        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);

        if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev) && !PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /* wait for acknowledge that the set was done */
            utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /*
            1.5. Call function with null macEntryKeyPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryDelete(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

  /*
            1.6. Call function with wrong enum values macEntryKeyPtr->entryType
                                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryDelete
                                                        (dev, &macEntry),
                                                        macEntry.entryType);

        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.7. Call function with out of range
            macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.ipMcast.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, macEntryPtr->key->ipMcast->vlanId = %d",
                                     dev, macEntry.key.ipMcast.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

        /*
            1.8. Call function with out of range
            macEntryKeyPtr->key->macVlan->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.ipMcast.vlanId     = 100;
        macEntry.key.macVlan.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                     "%d, macEntryPtr->key->macVlan->vlanId = %d",
                                     dev, macEntry.key.macVlan.vlanId);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

    }

    macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.macVlan.vlanId = 100;

    macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryDelete(dev, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with index [0],
                            skip [GT_FALSE],
                            macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                              macVlan->vlanId[100],
                                              macVlan-> macAddr[00:1A:FF:FF:FF:FF]},
                                              dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                            devPort {devNum [devNum],
                                                                     portNum [0]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},
                                              isStatic [GT_FALSE],
                                              daCommand [CPSS_MAC_TABLE_FRWRD_E],
                                              saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                                              daRoute [GT_FALSE],
                                              mirrorToRxAnalyzerPortEn [GT_FALSE],
                                              sourceID [0],
                                              userDefined [0],
                                              daQosIndex [0],
                                              saQosIndex [0],
                                              daSecurityLevel [0],
                                              saSecurityLevel [0],
                                              appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input was - check by fields (only valid).
    1.3. Call function with index [1],
                            skip [GT_FALSE]
                            and valid macEntryPtr {key{CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E,
                                                       ipMcast->vlanId[100],
                                                       ipMcast->sip={123.12.12.3},
                                                       ipMcast->dip={123.34.67.8}},
                            dstInterface {type[CPSS_INTERFACE_VID_E],
                                          devPort {devNum [devNum],
                                                   portNum [0]},
                                          trunkId [0],
                                          vidx [10],
                                          vlanId [100]},
                            isStatic [GT_FALSE],
                            daCommand [CPSS_MAC_TABLE_INTERV_E],
                            saCommand [CPSS_MAC_TABLE_CNTL_E],
                            daRoute [GT_FALSE],
                            mirrorToRxAnalyzerPortEn [GT_FALSE],
                            sourceID [BIT_5-1],
                            userDefined [0],
                            daQosIndex [30],
                            saQosIndex [101],
                            daSecurityLevel [10],
                            saSecurityLevel [40],
                            appSpecificCpuCode [GT_TRUE]}
    Expected: GT_OK.
    1.4. Call cpssDxChBrgFdbMacEntryRead with index [1]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input
    1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_PORT_E]
                   and other - the same as in 1.3.
    Expected: NON GT_OK (IP MC support only VIDX and VID).
    1.6. Call function with index [2],
                            skip [GT_FALSE]
                            and valid macEntryPtr{key{CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E,
                                                      ipMcast->vlanId[100],
                                                      ipMcast->sip={123.12.12.3},
                                                      ipMcast->dip = {123.34.67.8}},
                             dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                           devPort {devNum [devNum],
                                                    portNum [0]},
                                           trunkId [0],
                                           vidx [10],
                                           vlanId [100]},
                             isStatic [GT_FALSE],
                             daCommand [CPSS_MAC_TABLE_SOFT_DROP_E],
                             saCommand [CPSS_MAC_TABLE_DROP_E],
                             daRoute [GT_TRUE],
                             mirrorToRxAnalyzerPortEn [GT_TRUE],
                             sourceID [0],
                             userDefined [0],
                             daQosIndex [0],
                             saQosIndex [0],
                             daSecurityLevel [0],
                             saSecurityLevel [0],
                             appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.7. Call cpssDxChBrgFdbMacEntryRead with index [2]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input
    1.8. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                   and other - the same as in 1.6.
    Expected: NON GT_OK (IP MC support only VIDX and VID).
    1.9. Call function with out of range macEntryPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                            key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E].
    Expected: GT_BAD_PARAM.
    1.10. Call function with index [3]
                             and skip [GT_FALSE]
                             and valid macEntryPtr{key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                                        macVlan->vlanId[100],
                                                        macVlan->macAddr[00:1A:FF:FF:FF:FF]},
                             dstInterface {type[CPSS_INTERFACE_PORT_E],
                                           devPort {devNum [devNum],
                                                    portNum [0]},
                                           trunkId [0],
                                           vidx [10],
                                           vlanId [100]},
                             isStatic [GT_FALSE],
                             daCommand [CPSS_MAC_TABLE_FRWRD_E],
                             saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                             daRoute [GT_FALSE],
                             mirrorToRxAnalyzerPortEn [GT_FALSE],
                             sourceID [0],
                             userDefined [0],
                             daQosIndex [0],
                             saQosIndex [0],
                             daSecurityLevel [0],
                             saSecurityLevel [0],
                             appSpecificCpuCode [GT_FALSE]}.
    Expected: GT_OK.
    1.11. Call cpssDxChBrgFdbMacEntryRead with index [3]
                                               and non-NULL pointers.
    1.12. Call function with out of range index [macTableSize(dev)]
                             and other params from 1.10.
    Expected: NOT GT_OK.
    1.13. Call function with null macEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.14. Call function with out of range key->entryType
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.15. Call function with out of range dstInterface->type
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.16. Call function with out of range macEntryPtr->key->macVlan->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS]
                             and other params from 1.10.
    Expected: GT_BAD_PARAM.
    1.17. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.18. Call function with out of range dstInterface->devPort->hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.10.
    Expected: NON GT_OK.
    1.19. Call function with out of range dstInterface->trunkId [128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.20. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.21. Call function with out of range dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                             and dstInterface->type [CPSS_INTERFACE_VID_E].
    Expected: NON GT_OK.
    1.22. Call function with out of range macEntryPtr->daCommand
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.23. Call function with out of range macEntryPtr->saCommand
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.24. Call with userDefined [16] and other params from 1.10.
    Expected: NOT GT_OK.
    1.25. Call with daQosIndex [8] and other params from 1.10.
    Expected: NOT GT_OK.
    1.26. Call with saQosIndex [8] and other params from 1.10.
    Expected: NOT GT_OK.
    1.27. Call with daSecurityLevel [8] and other params from 1.10.
    Expected: NOT GT_OK.
    1.28. Call with saSecurityLevel [8] and other params from 1.10.
    Expected: NOT GT_OK.
    1.29. Call with sourceID [32] and other params from 1.10.
    Expected: NOT GT_OK.
    1.30. Call cpssDxChBrgFdbMacEntryInvalidate with index [0/ 1/ 2 / 3] to invalidate all changes
    Expected: GT_OK.
*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    GT_U32                  index       = 0;
    GT_BOOL                 skip        = GT_FALSE;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    GT_BOOL                 validGet    = GT_FALSE;
    GT_BOOL                 skipGet     = GT_FALSE;
    GT_BOOL                 agedGet     = GT_FALSE;
    GT_HW_DEV_NUM           HwDevNumGet   = 0;
    CPSS_MAC_ENTRY_EXT_STC  entryGet;
    GT_BOOL                 isEqual     = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0],
                            skip [GT_FALSE],
                            macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                              macVlan->vlanId[100],
                                              macVlan-> macAddr[00:1A:FF:FF:FF:FF]},
                                              dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                            devPort {devNum [devNum],
                                                                     portNum [0]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},
                                              isStatic [GT_FALSE],
                                              daCommand [CPSS_MAC_TABLE_FRWRD_E],
                                              saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                                              daRoute [GT_FALSE],
                                              mirrorToRxAnalyzerPortEn [GT_FALSE],
                                              sourceID [0],
                                              userDefined [0],
                                              daQosIndex [0],
                                              saQosIndex [0],
                                              daSecurityLevel [0],
                                              saSecurityLevel [0],
                                              appSpecificCpuCode [GT_FALSE]}.
            Expected: GT_OK.
        */
        index = 0;
        skip  = GT_FALSE;

        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId = 100;

        macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum  = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* allow next code to succeed with allowed command */
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /*
            1.2. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                                      and non-NULL pointers.
            Expected: GT_OK and the same parameters as input was - check by fields (only valid).
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        /* Verifying struct fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                   "get another skipPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                   "get another macEntryPtr->key.entryType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.macVlan.vlanId,
                                     entryGet.key.key.macVlan.vlanId,
                   "get another macEntryPtr->key.key.macVlan.vlanId than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.macVlan.macAddr,
                               (GT_VOID*) &entryGet.key.key.macVlan.macAddr,
                                sizeof(macEntry.key.key.macVlan.macAddr))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.key.macVlan.macAddr than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.trunkId,
                                     entryGet.dstInterface.trunkId,
                   "get another macEntryPtr->dstInterface.trunkId than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.isStatic, entryGet.isStatic,
                   "get another macEntryPtr->isStatic than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daCommand, entryGet.daCommand,
                   "get another macEntryPtr->daCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saCommand, entryGet.saCommand,
                   "get another macEntryPtr->saCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daRoute, entryGet.daRoute,
                   "get another macEntryPtr->daRoute than was set: %d", dev);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     entryGet.saMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     entryGet.daMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.mirrorToRxAnalyzerPortEn,
                                     entryGet.mirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->mirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, entryGet.sourceID,
                   "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined, entryGet.userDefined,
                   "get another macEntryPtr->userDefined than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daQosIndex, entryGet.daQosIndex,
                   "get another macEntryPtr->daQosIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saQosIndex, entryGet.saQosIndex,
                   "get another macEntryPtr->saQosIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel, entryGet.daSecurityLevel,
                   "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel, entryGet.saSecurityLevel,
                   "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.appSpecificCpuCode, entryGet.appSpecificCpuCode,
                   "get another macEntryPtr->appSpecificCpuCode than was set: %d", dev);

        /*
            1.3. Call function with index [1], skip [GT_FALSE]
                            and valid macEntryPtr {key{CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E,
                                                       ipMcast->vlanId[100],
                                                       ipMcast->sip={123.12.12.3},
                                                       ipMcast->dip={123.34.67.8}},
                            dstInterface {type[CPSS_INTERFACE_VID_E],
                                          devPort {devNum [devNum],
                                                   portNum [0]},
                                          trunkId [0],
                                          vidx [10],
                                          vlanId [100]},
                            isStatic [GT_FALSE],
                            daCommand [CPSS_MAC_TABLE_INTERV_E],
                            saCommand [CPSS_MAC_TABLE_CNTL_E],
                            daRoute [GT_FALSE],
                            mirrorToRxAnalyzerPortEn [GT_FALSE],
                            sourceID [BIT_5-1],
                            userDefined [0],
                            daQosIndex [30],
                            saQosIndex [101],
                            daSecurityLevel [0],
                            saSecurityLevel [7],
                            appSpecificCpuCode [GT_TRUE]}
            Expected: GT_OK.
        */
        index = 1;
        skip  = GT_FALSE;

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = 100;

        macEntry.key.key.ipMcast.sip[0] = 123;
        macEntry.key.key.ipMcast.sip[1] = 12;
        macEntry.key.key.ipMcast.sip[2] = 12;
        macEntry.key.key.ipMcast.sip[3] = 3;

        macEntry.key.key.ipMcast.dip[0] = 123;
        macEntry.key.key.ipMcast.dip[1] = 34;
        macEntry.key.key.ipMcast.dip[2] = 67;
        macEntry.key.key.ipMcast.dip[3] = 8;

        macEntry.dstInterface.type            = CPSS_INTERFACE_VID_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 0;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = BIT_5-1;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 4;
        macEntry.saQosIndex               = 5;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 7;
        macEntry.appSpecificCpuCode       = GT_TRUE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saSecurityLevel          = 7;
                macEntry.daQosIndex               = 4;
                macEntry.saQosIndex               = 5;

                note: macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E; not relevant to IPMC
            */
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.saSecurityLevel          = 0;
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.saQosIndex               = 0;
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.daQosIndex               = 0;
        }

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /*
            1.4. Call cpssDxChBrgFdbMacEntryRead with index [1]
                                                      and non-NULL pointers.
            Expected: GT_OK and the same parameters as input
        */
        index = 1;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        /* Verifying struct fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                   "get another skipPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                   "get another macEntryPtr->key.entryType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.ipMcast.vlanId,
                                     entryGet.key.key.ipMcast.vlanId,
                   "get another macEntryPtr->key.key.ipMcast.vlanId than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipMcast.dip,
                               (GT_VOID*) &entryGet.key.key.ipMcast.dip,
                                sizeof(macEntry.key.key.ipMcast.dip))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.key.ipMcast.dip than was set: %d", dev);
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipMcast.sip,
                               (GT_VOID*) &entryGet.key.key.ipMcast.sip,
                                sizeof(macEntry.key.key.ipMcast.sip))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.key.ipMcast.sip than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.vlanId,
                                     entryGet.dstInterface.vlanId,
                   "get another macEntryPtr->dstInterface.vlanId than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.isStatic, entryGet.isStatic,
                   "get another macEntryPtr->isStatic than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daCommand, entryGet.daCommand,
                   "get another macEntryPtr->daCommand than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daRoute, entryGet.daRoute,
                   "get another macEntryPtr->daRoute than was set: %d", dev);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     entryGet.saMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     entryGet.daMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.mirrorToRxAnalyzerPortEn,
                                         entryGet.mirrorToRxAnalyzerPortEn,
                "get another macEntryPtr->mirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined, entryGet.userDefined,
                   "get another macEntryPtr->userDefined than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daQosIndex, entryGet.daQosIndex,
                   "get another macEntryPtr->daQosIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saQosIndex, entryGet.saQosIndex,
                   "get another macEntryPtr->saQosIndex than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel, entryGet.daSecurityLevel,
                   "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel, entryGet.saSecurityLevel,
                   "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.appSpecificCpuCode,
                                        entryGet.appSpecificCpuCode,
                   "get another macEntryPtr->appSpecificCpuCode than was set: %d", dev);

        /*
            1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_PORT_E]
                           and other - the same as in 1.3.
            Expected: NON GT_OK (IP MC support only VIDX and VID).
        */
        macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->key.entryType = %d, macEntryPtr->dstInterface->type = %d",
                                         dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /*
            1.6. Call function with index [2],
                            skip [GT_FALSE]
                            and valid macEntryPtr{key{CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E,
                                                      ipMcast->vlanId[100],
                                                      ipMcast->sip={123.12.12.3},
                                                      ipMcast->dip = {123.34.67.8}},
                             dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                           devPort {devNum [devNum],
                                                    portNum [0]},
                                           trunkId [0],
                                           vidx [10],
                                           vlanId [100]},
                             isStatic [GT_FALSE],
                             daCommand [CPSS_MAC_TABLE_SOFT_DROP_E],
                             saCommand [CPSS_MAC_TABLE_DROP_E],
                             daRoute [GT_TRUE],
                             mirrorToRxAnalyzerPortEn [GT_TRUE],
                             sourceID [0],
                             userDefined [0],
                             daQosIndex [0],
                             saQosIndex [0],
                             daSecurityLevel [0],
                             saSecurityLevel [0],
                             appSpecificCpuCode [GT_FALSE]}.
            Expected: GT_OK.
        */
        index = 2;
        skip  = GT_FALSE;

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
        macEntry.key.key.ipMcast.vlanId = 100;

        macEntry.key.key.ipMcast.sip[0] = 123;
        macEntry.key.key.ipMcast.sip[1] = 12;
        macEntry.key.key.ipMcast.sip[2] = 12;
        macEntry.key.key.ipMcast.sip[3] = 3;

        macEntry.key.key.ipMcast.dip[0] = 123;
        macEntry.key.key.ipMcast.dip[1] = 34;
        macEntry.key.key.ipMcast.dip[2] = 67;
        macEntry.key.key.ipMcast.dip[3] = 8;

        macEntry.dstInterface.type            = CPSS_INTERFACE_VID_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_SOFT_DROP_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_DROP_E;
        macEntry.daRoute                  = GT_TRUE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.saMirrorToRxAnalyzerPortEn= GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn= GT_TRUE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.saMirrorToRxAnalyzerPortEn= GT_TRUE;
                macEntry.daMirrorToRxAnalyzerPortEn= GT_TRUE;

                note: macEntry.saCommand                = CPSS_MAC_TABLE_DROP_E; not relevant to IPMC
            */
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.saMirrorToRxAnalyzerPortEn= GT_FALSE;
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.daMirrorToRxAnalyzerPortEn= GT_FALSE;
        }

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /*
            1.7. Call cpssDxChBrgFdbMacEntryRead with index [2]
                                                      and non-NULL pointers.
            Expected: GT_OK and the same parameters as input
        */
        index = 2;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        /* Verifying struct fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                   "get another skipPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                   "get another macEntryPtr->key.entryType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.ipMcast.vlanId,
                                     entryGet.key.key.ipMcast.vlanId,
                   "get another macEntryPtr->key.key.ipMcast.vlanId than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipMcast.dip,
                               (GT_VOID*) &entryGet.key.key.ipMcast.dip,
                                sizeof(macEntry.key.key.ipMcast.dip))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.key.ipMcast.dip than was set: %d", dev);
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipMcast.sip,
                               (GT_VOID*) &entryGet.key.key.ipMcast.sip,
                               sizeof(macEntry.key.key.ipMcast.sip))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.key.ipMcast.sip than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.vlanId, entryGet.dstInterface.vlanId,
                   "get another macEntryPtr->dstInterface.vlanId than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.isStatic, entryGet.isStatic,
                   "get another macEntryPtr->isStatic than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daCommand, entryGet.daCommand,
                   "get another macEntryPtr->daCommand than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daRoute, entryGet.daRoute,
                   "get another macEntryPtr->daRoute than was set: %d", dev);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     entryGet.saMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     entryGet.daMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.mirrorToRxAnalyzerPortEn,
                                     entryGet.mirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->mirrorToRxAnalyzerPortEn than was set: %d", dev);
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined, entryGet.userDefined,
                   "get another macEntryPtr->userDefined than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daQosIndex, entryGet.daQosIndex,
                   "get another macEntryPtr->daQosIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saQosIndex, entryGet.saQosIndex,
                   "get another macEntryPtr->saQosIndex than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel, entryGet.daSecurityLevel,
                   "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel, entryGet.saSecurityLevel,
                   "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.appSpecificCpuCode, entryGet.appSpecificCpuCode,
                   "get another macEntryPtr->appSpecificCpuCode than was set: %d", dev);

        /*
            1.8. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                           and other - the same as in 1.6.
            Expected: NON GT_OK (IP MC support only VIDX and VID).
        */
        macEntry.dstInterface.type = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, macEntryPtr->key.entryType = %d, macEntryPtr->dstInterface->type = %d",
                                         dev, macEntry.key.entryType, macEntry.dstInterface.type);

        /*
            1.9. Call function with out of range
                macEntryPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E].
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.key.ipMcast.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
        macEntry.dstInterface.type      = CPSS_INTERFACE_VID_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, macEntryPtr->key->key->ipMcast->vlanId = %d",
                                     dev, macEntry.key.key.ipMcast.vlanId);

        /*
            1.10. Call function with index [3]
                             and skip [GT_FALSE]
                             and valid macEntryPtr{key {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                                        macVlan->vlanId[100],
                                                        macVlan->macAddr[00:1A:FF:FF:FF:FF]},
                             dstInterface {type[CPSS_INTERFACE_PORT_E],
                                           devPort {devNum [devNum],
                                                    portNum [0]},
                                           trunkId [0],
                                           vidx [10],
                                           vlanId [100]},
                             isStatic [GT_FALSE],
                             daCommand [CPSS_MAC_TABLE_FRWRD_E],
                             saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                             daRoute [GT_FALSE],
                             mirrorToRxAnalyzerPortEn [GT_FALSE],
                             sourceID [0],
                             userDefined [0],
                             daQosIndex [0],
                             saQosIndex [0],
                             daSecurityLevel [0],
                             saSecurityLevel [0],
                             appSpecificCpuCode [GT_FALSE]}.
            Expected: GT_OK.
        */
        index = 3;
        skip  = GT_FALSE;

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId = 100;

        macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 0;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* allow next code to succeed with allowed command */
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /*
            1.11. Call cpssDxChBrgFdbMacEntryRead with index [3]
                                                       and non-NULL pointers.
        */
        index = 3;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        /* Verifying struct fields */
        UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                   "get another skipPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                   "get another macEntryPtr->key.entryType than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.macVlan.vlanId,
                                     entryGet.key.key.macVlan.vlanId,
                   "get another macEntryPtr->key.key.macVlan.vlanId than was set: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.macVlan.macAddr,
                               (GT_VOID*) &entryGet.key.key.macVlan.macAddr,
                                sizeof(macEntry.key.key.macVlan.macAddr))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                   "get another macEntryPtr->key.macVlan.key.macAddr than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.hwDevNum,
                                     entryGet.dstInterface.devPort.hwDevNum,
                   "get another macEntryPtr->dstInterface.devPort.hwDevNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.portNum,
                                     entryGet.dstInterface.devPort.portNum,
                   "get another macEntryPtr->dstInterface.devPort.portNum than was set: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.isStatic, entryGet.isStatic,
                   "get another macEntryPtr->isStatic than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daCommand, entryGet.daCommand,
                   "get another macEntryPtr->daCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saCommand, entryGet.saCommand,
                   "get another macEntryPtr->saCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daRoute, entryGet.daRoute,
                   "get another macEntryPtr->daRoute than was set: %d", dev);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     entryGet.saMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     entryGet.daMirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.mirrorToRxAnalyzerPortEn,
                                     entryGet.mirrorToRxAnalyzerPortEn,
                   "get another macEntryPtr->mirrorToRxAnalyzerPortEn than was set: %d", dev);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, entryGet.sourceID,
                   "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined, entryGet.userDefined,
                   "get another macEntryPtr->userDefined than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daQosIndex, entryGet.daQosIndex,
                   "get another macEntryPtr->daQosIndex than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saQosIndex, entryGet.saQosIndex,
                   "get another macEntryPtr->saQosIndex than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel, entryGet.daSecurityLevel,
                   "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel, entryGet.saSecurityLevel,
                   "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        if(PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E)
           UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.appSpecificCpuCode, entryGet.appSpecificCpuCode,
                   "get another macEntryPtr->appSpecificCpuCode than was set: %d", dev);

        /*
            1.12. Call function with out of range index [macTableSize(dev)]
                                     and other params from 1.10.
            Expected: NOT GT_OK.
        */
        st = prvUtfMacTableSize(dev, &index);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfMacTableSize: %d", dev);

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.13. Call function with null macEntryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macEntryPtr = NULL", dev);

        /*
            1.14. Call function with wrong enum values key->entryType
                             and other parameters the same as in 1.10.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                        (dev, index, skip, &macEntry),
                                                        macEntry.key.entryType);

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        /*
            1.15. Call function with wrong enum values dstInterface->type
                                     and other parameters the same as in 1.10.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                        (dev, index, skip, &macEntry),
                                                        macEntry.dstInterface.type);

        macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

        /*
            1.16. Call function with out of range
                macEntryPtr->key->macVlan->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS]
                                     and other params from 1.10.
            Expected: GT_OUT_OF_RANGE.
        */
        macEntry.key.key.macVlan.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                        "%d, macEntryPtr->key->key->macVlan->vlanId = %d",
                                     dev, macEntry.key.key.macVlan.vlanId);

        /*
            1.17. Call function with out of range
                dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.key.key.macVlan.vlanId       = 100;
        macEntry.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, macEntryPtr->dstInterface.devPort.portNum = %d",
                                         dev, macEntry.dstInterface.devPort.portNum);

        /*
            1.18. Call function with out of range
                        dstInterface->devPort->hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                                     dstInterface->type = CPSS_INTERFACE_PORT_E
                                     and other parameters the same as in 1.10.
            Expected: NON GT_OK.
        */
        macEntry.dstInterface.devPort.portNum = BRG_FDB_VALID_PHY_PORT_CNS;
        macEntry.dstInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                        "%d, macEntryPtr->dstInterface->devPort->hwDevNum = %d",
                                     dev, macEntry.dstInterface.devPort.hwDevNum);

        /*
            1.19. Call function with out of range dstInterface->trunkId [128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.dstInterface.devPort.hwDevNum = dev;
        macEntry.dstInterface.trunkId        = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.type           = CPSS_INTERFACE_TRUNK_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, macEntryPtr->dstInterface->trunkId = %d",
                                         dev, macEntry.dstInterface.trunkId);

        /*
            1.20. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.dstInterface.trunkId = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx    = MAX_VIDX_MAC(dev);
        macEntry.dstInterface.type    = CPSS_INTERFACE_VIDX_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, macEntryPtr->dstInterface->vidx = %d",
                                         dev, macEntry.dstInterface.vidx);

        /*
            1.21. Call function with out of range
                    dstInterface->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                                     and dstInterface->type [CPSS_INTERFACE_VID_E].
            Expected: NON GT_OK.
        */
        macEntry.dstInterface.vidx   = 10;
        macEntry.dstInterface.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.dstInterface.type   = CPSS_INTERFACE_VID_E;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vlanId = %d",
                                     dev, macEntry.dstInterface.vlanId);

        /*
            1.22. Call function with wrong enum values macEntryPtr->daCommand
                                     and other parameters the same as in 1.10.
            Expected: GT_BAD_PARAM.
        */
        macEntry.dstInterface.vlanId = 100;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                        (dev, index, skip, &macEntry),
                                                        macEntry.daCommand);

        macEntry.daCommand     = CPSS_MAC_TABLE_INTERV_E;

        /*
            1.23. Call function with wrong enum values macEntryPtr->saCommand
                                     and other parameters the same as in 1.10.
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                        (dev, index, skip, &macEntry),
                                                        macEntry.saCommand);

        macEntry.saCommand  = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* allow next code to succeed with allowed command */
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }
        /*
            1.24. Call with userDefined [16] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.userDefined = 16;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->userDefined = %d",
                                     dev, macEntry.userDefined);

        macEntry.userDefined = 0;

        /*
            1.25. Call with daQosIndex [8] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.daQosIndex = 8;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daQosIndex = %d",
                                     dev, macEntry.daQosIndex);

        macEntry.daQosIndex = 0;

        /*
            1.26. Call with saQosIndex [8] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.saQosIndex = 8;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saQosIndex = %d",
                                     dev, macEntry.saQosIndex);

        macEntry.saQosIndex = 0;

        /*
            1.27. Call with daSecurityLevel [8] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.daSecurityLevel = 8;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->daSecurityLevel = %d",
                                     dev, macEntry.daSecurityLevel);

        macEntry.daSecurityLevel = 0;

        /*
            1.28. Call with saSecurityLevel [8] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.saSecurityLevel = 8;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->saSecurityLevel = %d",
                                     dev, macEntry.saSecurityLevel);

        macEntry.saSecurityLevel = 0;

        /*
            1.29. Call with sourceID [32] and other params from 1.10.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID = 32;

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->sourceID = %d",
                                     dev, macEntry.sourceID);

        macEntry.sourceID = 0;

        /*
            1.30. Call cpssDxChBrgFdbMacEntryInvalidate with
                index [0/ 1/ 2 / 3] to invalidate all changes
            Expected: GT_OK.
        */

        /* Call with index [0] */
        index = 0;
        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

        /* Call with index [1] */
        index = 1;

        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

        /* Call with index [2] */
        index = 2;

        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

        /* Call with index [3] */
        index = 3;

        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);
    }

    index = 0;
    skip  = GT_FALSE;

    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId = 100;

    macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.devPort.hwDevNum  = dev;
    macEntry.dstInterface.devPort.portNum = 0;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.vidx            = 10;
    macEntry.dstInterface.vlanId          = 100;

    macEntry.isStatic                 = GT_FALSE;
    macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
    macEntry.daRoute                  = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.sourceID                 = 0;
    macEntry.userDefined              = 0;
    macEntry.daQosIndex               = 0;
    macEntry.saQosIndex               = 0;
    macEntry.daSecurityLevel          = 0;
    macEntry.saSecurityLevel          = 0;
    macEntry.appSpecificCpuCode       = GT_FALSE;
    macEntry.age                      = GT_TRUE;
    macEntry.spUnknown                = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_U8                   *associatedDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with index [0],
                   non-null validPtr,
                   non-null skipPtr,
                   non-null agedPtr,
                   non-null associatedDevNumPtr
                   and non-null entryPtr.
    Expected: GT_OK.
    1.2. Call function with out of range index [macTableSize(dev)].
    Expected: NOT GT_OK.
    1.3. Call with index [0],
                   null validPtr [NULL],
                   non-null skipPtr,
                   non-null agedPtr,
                   non-null associatedDevNumPtr
                   and non-null entryPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with index [0],
                   non-null validPtr,
                   null skipPtr [NULL],
                   non-null agedPtr,
                   non-null associatedDevNumPtr
                   and non-null entryPtr.
    Expected: GT_BAD_PTR.
    1.5. Call with index [0],
                   non-null validPtr,
                   non-null skipPtr,
                   null agedPtr [NULL],
                   non-null associatedDevNumPtr
                   and non-null entryPtr.
    Expected: GT_BAD_PTR.
    1.6. Call with index [0],
                   non-null validPtr,
                   non-null skipPtr,
                   non-null agedPtr,
                   null associatedDevNumPtr [NULL]
                   and non-null entryPtr.
    Expected: GT_BAD_PTR.
    1.7. Call with index [0],
                   non-null validPtr,
                   non-null skipPtr,
                   non-null agedPtr,
                   non-null associatedDevNumPtr
                   and null entryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with non-null msgRatePtr
                   and null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS              st        = GT_OK;

    GT_U8                  dev;
    GT_U32                 index     = 0;
    GT_U32                 tableSize = 0;
    GT_BOOL                valid     = GT_FALSE;
    GT_BOOL                skip      = GT_FALSE;
    GT_BOOL                aged      = GT_FALSE;
    GT_HW_DEV_NUM          hwDevNum    = 0;
    CPSS_MAC_ENTRY_EXT_STC entry;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0],
                           non-null validPtr,
                           non-null skipPtr,
                           non-null agedPtr,
                           non-null associatedDevNumPtr
                           and non-null entryPtr.
            Expected: GT_OK.
        */
        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, &hwDevNum, &entry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* Getting table size */
        st = prvUtfMacTableSize(dev, &tableSize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfMacTableSize: %d", dev);

        /*
            1.2. Call function with out of range index [macTableSize(dev)].
            Expected: NOT GT_OK.
        */
        index = tableSize;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, &hwDevNum, &entry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.3. Call with index [0],
                           null validPtr [NULL],
                           non-null skipPtr,
                           non-null agedPtr,
                           non-null associatedDevNumPtr
                           and non-null entryPtr.
            Expected: GT_BAD_PTR.
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, NULL, &skip,
                                        &aged, &hwDevNum, &entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, validPtr = NULL", dev);

        /*
            1.4. Call with index [0],
                           non-null validPtr,
                           null skipPtr [NULL],
                           non-null agedPtr,
                           non-null associatedDevNumPtr
                           and non-null entryPtr.
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, NULL,
                                        &aged, &hwDevNum, &entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, skipPtr = NULL", dev);

        /*
            1.5. Call with index [0],
                           non-null validPtr,
                           non-null skipPtr,
                           null agedPtr [NULL],
                           non-null associatedDevNumPtr
                           and non-null entryPtr.
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        NULL, &hwDevNum, &entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, agedPtr = NULL", dev);

        /*
            1.6. Call with index [0],
                           non-null validPtr,
                           non-null skipPtr,
                           non-null agedPtr,
                           null associatedDevNumPtr [NULL]
                           and non-null entryPtr.
            Expected: GT_BAD_PTR.
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, NULL, &entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, associatedDevNumPtr = NULL", dev);

        /*
            1.7. Call with index [0],
                           non-null validPtr,
                           non-null skipPtr,
                           non-null agedPtr,
                           non-null associatedDevNumPtr
                           and null entryPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, &hwDevNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, entryPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, &hwDevNum, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &valid, &skip,
                                        &aged, &hwDevNum, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_U32        index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryInvalidate)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with index [0].
    Expected: GT_OK.
    1.2. Call with out of range index [macTableSize(dev)].
    Expected: NOT GT_OK.
*/
    GT_STATUS  st        = GT_OK;

    GT_U8      dev;
    GT_U32     index     = 0;
    GT_U32     tableSize = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0].
            Expected: GT_OK.
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* Getting table size */
        st = prvUtfMacTableSize(dev, &tableSize);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfMacTableSize: %d", dev);

        /*
            1.2. Call with out of range index [macTableSize(dev)].
            Expected: NOT GT_OK.
        */
        index = tableSize;

        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);
    }

    index = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRouteEntryWrite)
{
/*
      ITERATE_DEVICES (DxChx)
    1.1. Call function with index [0],
                            skip [GT_FALSE],
                            and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[1.2.3.4],
                                                    ipv4Unicast->vrfId[0]},
                                               dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                             devPort {devNum [devNum],
                                                                      portNum [1]},
                                                             trunkId [10],
                                                             vidx [10],
                                                             vlanId [100]},

                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                              CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]
                                               }.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input was - check by fields (only valid).
    1.3. Call function with index [1],
                            skip [GT_FALSE]
                            and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E,
                                                    ipv6Unicast->dip = {1112:1314::1516:1718}- not used in data entry ,
                                                    ipv6Unicast->vrfId[0]},
                                                dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                                              devPort {devNum [devNum],
                                                                       portNum [1]},
                                                              trunkId [0],
                                                              vidx [10],
                                                              vlanId [100]},
                                                ttlHopLimitDecEnable[GT_TRUE]
                                                ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                ingressMirror[GT_TRUE]
                                                ingressMirrorToAnalyzerIndex[2]
                                                qosProfileMarkingEnable[GT_TRUE]
                                                qosProfileIndex[3]
                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                countSet[1]
                                                trapMirrorArpBcEnable[GT_TRUE]
                                                dipAccessLevel[5]
                                                ICMPRedirectEnable[GT_TRUE]
                                                mtuProfileIndex[1]
                                                isTunnelStart[GT_TRUE]
                                                nextHopVlanId[6]
                                                nextHopARPPointer[7]
                                                nextHopTunnelPointer[8]

                                                nextHopDataBankNumber[1] - not used in data entry
                                                scopeCheckingEnable[GT_TRUE]
                                                CPSS_IP_SITE_ID_ENT     siteId[CPSS_IP_SITE_ID_EXTERNAL_E]
    Expected: GT_OK.
    1.4. Call cpssDxChBrgFdbMacEntryRead with index [1]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input
    1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_PORT_E]
                   and other - the same as in 1.3.
    Expected: GT_OK
    1.6. Call function with index [2],
                            skip [GT_FALSE]
                            and valid macEntryPtr{key{CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E,
                                                  ipv6Unicast->dip = {1112:1314::1516:1718},
                                                  ipv6Unicast->vrfId[0]} - not used in data entry}
    Expected: GT_OK.
    1.7. Call cpssDxChBrgFdbMacEntryRead with index [2]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input
    1.8. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                   and other - the same as in 1.3.
    Expected: GT_OK
    1.9. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                       key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
    Expected: GT_OUT_OF_RANGE.
    1.10. Call function with index [3]
                             and skip [GT_FALSE]
                             and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                    ipv4Unicast->dip[5.6.7.8],
                                                    ipv4Unicast->vrfId[0]},
                                                    dstInterface {type[CPSS_INTERFACE_PORT_E],
                                                    devPort {devNum [devNum],
                                                        portNum [0]},
                                                    trunkId [0],
                                                    vidx [10],
                                                    vlanId [100]},

                                                    ttlHopLimitDecEnable[GT_FALSE]
                                                    ttlHopLimDecOptionsExtChkByPass[GT_FALSE]
                                                    ingressMirror[GT_FALSE]
                                                    ingressMirrorToAnalyzerIndex[1]
                                                    qosProfileMarkingEnable[GT_FALSE]
                                                    qosProfileIndex[1]
                                                    qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E]
                                                    modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E]
                                                    modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E]
                                                    countSet[1]
                                                    trapMirrorArpBcEnable[GT_FALSE]
                                                    dipAccessLevel[1]
                                                    ICMPRedirectEnable[GT_FALSE]
                                                    mtuProfileIndex[0]
                                                    isTunnelStart[GT_FALSE]
                                                    nextHopVlanId[1]
                                                    nextHopARPPointer[1]
                                                    nextHopTunnelPointer[1]}.
    Expected: GT_OK.
    1.11. Call cpssDxChBrgFdbMacEntryRead with index [3]
                                               and non-NULL pointers.
    1.12. Call function with null macEntryPtr [NULL].
    Expected: GT_BAD_PTR.
    1.13. Call function with out of range key->entryType
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.14. Call function with out of range dstInterface->type
                             and other parameters the same as in 1.10.
    Expected: GT_BAD_PARAM.
    1.15. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS]
          key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E] and other params from 1.10.
    Expected: GT_OUT_OF_RANGE.
    1.16. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.17. Call function with out of range dstInterface->devPort->hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                             dstInterface->type = CPSS_INTERFACE_PORT_E
                             and other parameters the same as in 1.10.
    Expected: NON GT_OK.
    1.18. Call function with out of range dstInterface->trunkId [128],
                             dstInterface->type = CPSS_INTERFACE_TRUNK_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.19. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                             dstInterface->type = CPSS_INTERFACE_VIDX_E
                             and other parameters the same as in 1.10.
    Expected: NOT GT_OK.
    1.20. Call function with out of range dstInterface->vlanId [5]
                         and dstInterface->type [CPSS_INTERFACE_VID_E].
    Expected: GT_OK, vladId will e set to 0xFFF.
    1.21. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                             and other parameters the same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.22. Call function with out of range macEntryPtr->qosProfileIndex[128],
                             and other parameters the same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
    1.23. Call with out of range qosPrecedence and other params from 1.1.
    Expected: NOT GT_OK.
    1.24. Call with out of range dipAccessLevel [64] and other params from 1.1.
    Expected: NOT GT_OK.
    1.25. Call with with out of mtuProfileIndex [8] and other params from 1.1.
    Expected: NOT GT_OK.
    1.26. Call with with out of nextHopVlanId [8192] and other params from 1.1.
    Expected: NOT GT_OK.
    1.27. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
    Expected: NOT GT_OK.
    1.28. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
    Expected: NOT GT_OK.
    1.29. Call with with out of countSet [7] and other params from 1.1.
    Expected: NOT GT_OK.
    1.30. Call with with out of siteId and other params from 1.2.
    Expected: NOT GT_OK.
    1.31. Call cpssDxChBrgFdbMacEntryInvalidate with index [0/ 1/ 2 / 3] to invalidate all changes
    Expected: GT_OK.
    1.32 Call with out of range multipathPointer and other params from 1.1.
    Expected: NOT GT_OK.
    1.33 Call with out of range pointerType and other params from 1.1.
    Expected: NOT GT_OK.
*/
    GT_STATUS               st          = GT_OK;

    GT_U8                   dev;
    GT_U32                  index       = 0;
    GT_BOOL                 skip        = GT_FALSE;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    GT_BOOL                 validGet    = GT_FALSE;
    GT_BOOL                 skipGet     = GT_FALSE;
    GT_BOOL                 agedGet     = GT_FALSE;
    GT_HW_DEV_NUM           HwDevNumGet   = 0;
    CPSS_MAC_ENTRY_EXT_STC  entryGet;
    GT_BOOL                 isEqual     = GT_FALSE;
    GT_U32                  notAppFamilyBmp = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with index [0],
                                    skip [GT_FALSE],
                                    and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                            ipv4Unicast->dip[1.2.3.4],
                                                            ipv4Unicast->vrfId[0]},
                                                       dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                                                     devPort {devNum [devNum],
                                                                              portNum [1]},
                                                                     trunkId [10],
                                                                     vidx [10],
                                                                     vlanId [100]},

                                                        ttlHopLimitDecEnable[GT_TRUE]
                                                        ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                        ingressMirror[GT_TRUE]
                                                        ingressMirrorToAnalyzerIndex[2]
                                                        qosProfileMarkingEnable[GT_TRUE]
                                                        qosProfileIndex[3]
                                                        qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/
                                                                      CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                        modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                                  CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                                  CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                        modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E,
                                                                  CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E,
                                                                  CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                        countSet[1]
                                                        trapMirrorArpBcEnable[GT_TRUE]
                                                        dipAccessLevel[5]
                                                        ICMPRedirectEnable[GT_TRUE]
                                                        mtuProfileIndex[1]
                                                        isTunnelStart[GT_TRUE]
                                                        nextHopVlanId[6]
                                                        nextHopARPPointer[7]
                                                        nextHopTunnelPointer[8]
                                                       }.
            Expected: GT_OK.
        */
        index = 0;
        skip  = GT_FALSE;

        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        macEntry.key.key.ipv4Unicast.vrfId = 0;

        macEntry.key.key.ipv4Unicast.dip[0] = 1;
        macEntry.key.key.ipv4Unicast.dip[1] = 2;
        macEntry.key.key.ipv4Unicast.dip[2] = 3;
        macEntry.key.key.ipv4Unicast.dip[3] = 4;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.hwDevNum  = dev;
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.age                      = GT_TRUE;


        macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
        /* ttlHopLimitDecEnable and ttlHopLimDecOptionsExtChkByPass
            can't be both enabled */
        macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_FALSE;
        macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
        macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
        macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.qosProfileIndex=3;
        macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
        macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        macEntry.fdbRoutingInfo.countSet=1;
        macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.dipAccessLevel=5;
        macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
        macEntry.fdbRoutingInfo.mtuProfileIndex=1;
        macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
        macEntry.fdbRoutingInfo.nextHopVlanId=6;
        macEntry.fdbRoutingInfo.nextHopARPPointer=7;
        macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* entry should fail due to
                macEntry.fdbRoutingInfo.dipAccessLevel=5;
                macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                macEntry.fdbRoutingInfo.qosProfileIndex=3;
                macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
                macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
                macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
            */
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.fdbRoutingInfo.dipAccessLevel=0;
            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
            macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            macEntry.fdbRoutingInfo.qosProfileIndex=0;
            macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.ingressMirror=GT_FALSE;

            macEntry.fdbRoutingInfo.routingType=CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E;
        }

        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);

        if(!UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, index, skip);
        }
        else
        {

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

            /*
                1.2. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                                          and non-NULL pointers.
                Expected: GT_OK and the same parameters as input was - check by fields (only valid).
            */
            index = 0;

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.ipv4Unicast.vrfId,
                                         entryGet.key.key.ipv4Unicast.vrfId,
                       "get another macEntryPtr->key.key.ipv4Unicast.vrfId than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipv4Unicast.dip,
                                   (GT_VOID*) &entryGet.key.key.ipv4Unicast.dip,
                                    sizeof(macEntry.key.key.ipv4Unicast.dip))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macEntryPtr->key.key.ipv4Unicast.dipr than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                       "get another macEntryPtr->dstInterface.type than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.trunkId,
                                         entryGet.dstInterface.trunkId,
                       "get another macEntryPtr->dstInterface.trunkId than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimitDecEnable, entryGet.fdbRoutingInfo.ttlHopLimitDecEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimitDecEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass, entryGet.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirror, entryGet.fdbRoutingInfo.ingressMirror,
                       "get another macEntryPtr->fdbRoutingInfo.ingressMirror than was set: %d", dev);
            if(macEntry.fdbRoutingInfo.ingressMirror == GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex, entryGet.fdbRoutingInfo.ingressMirrorToAnalyzerIndex,
                           "get another macEntryPtr->fdbRoutingInfo.ingressMirrorToAnalyzerIndex than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileMarkingEnable, entryGet.fdbRoutingInfo.qosProfileMarkingEnable,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileMarkingEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileIndex, entryGet.fdbRoutingInfo.qosProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosPrecedence, entryGet.fdbRoutingInfo.qosPrecedence,
                       "get another macEntryPtr->fdbRoutingInfo.qosPrecedence than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyUp, entryGet.fdbRoutingInfo.modifyUp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyUp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyDscp, entryGet.fdbRoutingInfo.modifyDscp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyDscp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.countSet, entryGet.fdbRoutingInfo.countSet,
                       "get another macEntryPtr->fdbRoutingInfo.countSet than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.trapMirrorArpBcEnable, entryGet.fdbRoutingInfo.trapMirrorArpBcEnable,
                       "get another macEntryPtr->fdbRoutingInfo.trapMirrorArpBcEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.dipAccessLevel, entryGet.fdbRoutingInfo.dipAccessLevel,
                       "get another macEntryPtr->fdbRoutingInfo.dipAccessLevel than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ICMPRedirectEnable, entryGet.fdbRoutingInfo.ICMPRedirectEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ICMPRedirectEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.mtuProfileIndex, entryGet.fdbRoutingInfo.mtuProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.mtuProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.isTunnelStart, entryGet.fdbRoutingInfo.isTunnelStart,
                       "get another macEntryPtr->fdbRoutingInfo.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopVlanId, entryGet.fdbRoutingInfo.nextHopVlanId,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, entryGet.fdbRoutingInfo.nextHopARPPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopARPPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopTunnelPointer, entryGet.fdbRoutingInfo.nextHopTunnelPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopTunnelPointer than was set: %d", dev);
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.routingType, entryGet.fdbRoutingInfo.routingType,
                            "get another macEntryPtr->fdbRoutingInfo.routingType than was set: %d", dev);
            }

            /*
                1.3. Call function with index [1],
                                        skip [GT_FALSE]
                                        and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E,
                                                                ipv6Unicast->dip = {1112:1314::1516:1718}- not used in data entry ,
                                                                ipv6Unicast->vrfId[0]},
                                                            dstInterface {type[CPSS_INTERFACE_VIDX_E],
                                                                          devPort {devNum [devNum],
                                                                                   portNum [8]},
                                                                          trunkId [0],
                                                                          vidx [10],
                                                                          vlanId [100]},
                                                            ttlHopLimitDecEnable[GT_TRUE]
                                                            ttlHopLimDecOptionsExtChkByPass[GT_TRUE]
                                                            ingressMirror[GT_TRUE]
                                                            ingressMirrorToAnalyzerIndex[2]
                                                            qosProfileMarkingEnable[GT_TRUE]
                                                            qosProfileIndex[3]
                                                            qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E]
                                                            modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                            modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E]
                                                            countSet[1]
                                                            trapMirrorArpBcEnable[GT_TRUE]
                                                            dipAccessLevel[5]
                                                            ICMPRedirectEnable[GT_TRUE]
                                                            mtuProfileIndex[1]
                                                            isTunnelStart[GT_TRUE]
                                                            nextHopVlanId[6]
                                                            nextHopARPPointer[7]
                                                            nextHopTunnelPointer[8]

                                                            nextHopDataBankNumber[1] - not used in data entry
                                                            scopeCheckingEnable[GT_TRUE]
                                                            CPSS_IP_SITE_ID_ENT     siteId[CPSS_IP_SITE_ID_EXTERNAL_E]
                Expected: GT_OK.
            */

            index = 1;
            skip  = GT_FALSE;

            cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;
            macEntry.key.key.ipv6Unicast.vrfId = 0;

            macEntry.dstInterface.type            = CPSS_INTERFACE_VIDX_E;
            macEntry.dstInterface.devPort.hwDevNum  = dev;
            macEntry.dstInterface.devPort.portNum = 8;
            macEntry.dstInterface.trunkId         = 10;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.vidx            = 10;
            macEntry.dstInterface.vlanId          = 100;

            macEntry.age                      = GT_TRUE;

            /* ttlHopLimitDecEnable and ttlHopLimDecOptionsExtChkByPass
                can't be both enabled */
            macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_TRUE;
            macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
            macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
            macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
            macEntry.fdbRoutingInfo.qosProfileIndex=3;
            macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
            macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
            macEntry.fdbRoutingInfo.countSet=1;
            macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
            macEntry.fdbRoutingInfo.dipAccessLevel=5;
            macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
            macEntry.fdbRoutingInfo.mtuProfileIndex=1;
            macEntry.fdbRoutingInfo.isTunnelStart=GT_FALSE;
            macEntry.fdbRoutingInfo.nextHopVlanId=6;
            macEntry.fdbRoutingInfo.nextHopARPPointer=7;
            macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;

            macEntry.fdbRoutingInfo.scopeCheckingEnable=GT_TRUE;
            macEntry.fdbRoutingInfo.siteId=CPSS_IP_SITE_ID_EXTERNAL_E;

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* entry should fail due to
                    macEntry.fdbRoutingInfo.dipAccessLevel=5;
                    macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                    macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                    macEntry.fdbRoutingInfo.qosProfileIndex=3;
                    macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
                    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
                    macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
                    macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
                */
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.dipAccessLevel=0;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.qosProfileIndex=0;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_FALSE;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_FALSE;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.ingressMirror=GT_FALSE;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            }

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

            /*
                1.4. Call cpssDxChBrgFdbMacEntryRead with index [1]
                                                          and non-NULL pointers.
                Expected: GT_OK and the same parameters as input
            */
            index = 1;

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.ipv6Unicast.vrfId,
                                         entryGet.key.key.ipv6Unicast.vrfId,
                       "get another macEntryPtr->key.key.ipv4Unicast.vrfId than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                       "get another macEntryPtr->dstInterface.type than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.vidx,
                                         entryGet.dstInterface.vidx,
                       "get another macEntryPtr->dstInterface.vidx than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimitDecEnable, entryGet.fdbRoutingInfo.ttlHopLimitDecEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimitDecEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass, entryGet.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirror, entryGet.fdbRoutingInfo.ingressMirror,
                       "get another macEntryPtr->fdbRoutingInfo.ingressMirror than was set: %d", dev);
            if(macEntry.fdbRoutingInfo.ingressMirror == GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex, entryGet.fdbRoutingInfo.ingressMirrorToAnalyzerIndex,
                           "get another macEntryPtr->fdbRoutingInfo.ingressMirrorToAnalyzerIndex than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileMarkingEnable, entryGet.fdbRoutingInfo.qosProfileMarkingEnable,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileMarkingEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileIndex, entryGet.fdbRoutingInfo.qosProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosPrecedence, entryGet.fdbRoutingInfo.qosPrecedence,
                       "get another macEntryPtr->fdbRoutingInfo.qosPrecedence than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyUp, entryGet.fdbRoutingInfo.modifyUp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyUp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyDscp, entryGet.fdbRoutingInfo.modifyDscp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyDscp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.countSet, entryGet.fdbRoutingInfo.countSet,
                       "get another macEntryPtr->fdbRoutingInfo.countSet than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.trapMirrorArpBcEnable, entryGet.fdbRoutingInfo.trapMirrorArpBcEnable,
                       "get another macEntryPtr->fdbRoutingInfo.trapMirrorArpBcEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.dipAccessLevel, entryGet.fdbRoutingInfo.dipAccessLevel,
                       "get another macEntryPtr->fdbRoutingInfo.dipAccessLevel than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ICMPRedirectEnable, entryGet.fdbRoutingInfo.ICMPRedirectEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ICMPRedirectEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.mtuProfileIndex, entryGet.fdbRoutingInfo.mtuProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.mtuProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.isTunnelStart, entryGet.fdbRoutingInfo.isTunnelStart,
                       "get another macEntryPtr->fdbRoutingInfo.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopVlanId, entryGet.fdbRoutingInfo.nextHopVlanId,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopARPPointer, entryGet.fdbRoutingInfo.nextHopARPPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopARPPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, entryGet.fdbRoutingInfo.nextHopTunnelPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopTunnelPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.scopeCheckingEnable, entryGet.fdbRoutingInfo.scopeCheckingEnable,
                       "get another macEntryPtr->fdbRoutingInfo.scopeCheckingEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.siteId, entryGet.fdbRoutingInfo.siteId,
                       "get another macEntryPtr->fdbRoutingInfo.siteId than was set: %d", dev);

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                macEntry.fdbRoutingInfo.routingType = CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E;
                macEntry.fdbRoutingInfo.multipathPointer = BIT_15-1;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

                st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.routingType, entryGet.fdbRoutingInfo.routingType,
                            "get another macEntryPtr->fdbRoutingInfo.routingType than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.multipathPointer, entryGet.fdbRoutingInfo.multipathPointer,
                            "get another macEntryPtr->fdbRoutingInfo.multipathPointer than was set: %d", dev);

                /* restore MAC entry params */
                macEntry.fdbRoutingInfo.routingType=CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E;
                macEntry.fdbRoutingInfo.multipathPointer = 0;
            }

            /*
                1.5. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_PORT_E]
                       and other - the same as in 1.3.
                Expected: GT_OK
            */
            macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, macEntryPtr->key.entryType = %d, macEntryPtr->dstInterface->type = %d",
                                             dev, macEntry.key.entryType, macEntry.dstInterface.type);

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.portNum,
                                     entryGet.dstInterface.devPort.portNum,
                   "get another macEntryPtr->dstInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.hwDevNum,
                                     entryGet.dstInterface.devPort.hwDevNum,
                   "get another macEntryPtr->dstInterface.devPort.hwDevNum than was set: %d", dev);


            /*
                 1.6. Call function with index [2],
                                        skip [GT_FALSE]
                                        and valid macEntryPtr{key{CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E,
                                                              ipv6Unicast->dip = {1112:1314::1516:1718},
                                                              ipv6Unicast->vrfId[0]} - not used in data entry}
                Expected: GT_OK.
            */
            index = 2;
            skip  = GT_FALSE;

            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;

            macEntry.key.key.ipv6Unicast.dip[0] = 0x11;
            macEntry.key.key.ipv6Unicast.dip[1] = 0x12;
            macEntry.key.key.ipv6Unicast.dip[2] = 0x13;
            macEntry.key.key.ipv6Unicast.dip[3] = 0x14;
            macEntry.key.key.ipv6Unicast.dip[4] = 0;
            macEntry.key.key.ipv6Unicast.dip[5] = 0;
            macEntry.key.key.ipv6Unicast.dip[6] = 0;
            macEntry.key.key.ipv6Unicast.dip[7] = 0;
            macEntry.key.key.ipv6Unicast.dip[8] = 0;
            macEntry.key.key.ipv6Unicast.dip[9] = 0;
            macEntry.key.key.ipv6Unicast.dip[10] =0;
            macEntry.key.key.ipv6Unicast.dip[11] =0;
            macEntry.key.key.ipv6Unicast.dip[12] =0x15;
            macEntry.key.key.ipv6Unicast.dip[13] =0x16;
            macEntry.key.key.ipv6Unicast.dip[14] =0x17;
            macEntry.key.key.ipv6Unicast.dip[15] =0x18;

            macEntry.fdbRoutingInfo.nextHopDataBankNumber=1;

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* entry should fail due to
                    macEntry.fdbRoutingInfo.nextHopDataBankNumber=1;
                */
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.nextHopDataBankNumber=0;
            }


            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

            /*
                1.7. Call cpssDxChBrgFdbMacEntryRead with index [2]
                                                          and non-NULL pointers.
                Expected: GT_OK and the same parameters as input
            */
            index = 2;

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                GT_U32 _3msBytes_mask = 0x000003;/*128-106=22 --> 24-22=2 --> mask or 2 bits */
                GT_U32 _3msBytes_get;
                GT_U32 _3msBytes_set;
                /* the entry in the HW hold only 106 LSBits of the IPv6 address */
                /* so compare with mask the 24MSBits (3 bytes) and then exact compare the other 13 bytes */
                _3msBytes_set =
                    ((GT_U32)macEntry.key.key.ipv6Unicast.dip[0] << (2*8)) |
                    ((GT_U32)macEntry.key.key.ipv6Unicast.dip[1] << (1*8)) |
                    ((GT_U32)macEntry.key.key.ipv6Unicast.dip[2] << (0*8)) ;

                _3msBytes_get =
                    ((GT_U32)entryGet.key.key.ipv6Unicast.dip[0] << (2*8)) |
                    ((GT_U32)entryGet.key.key.ipv6Unicast.dip[1] << (1*8)) |
                    ((GT_U32)entryGet.key.key.ipv6Unicast.dip[2] << (0*8)) ;

                isEqual = ((_3msBytes_set & _3msBytes_mask) == _3msBytes_get) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "get another bits 104..105 in macEntryPtr->key.key.ipMcast.dip than was set: %d", dev);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipv6Unicast.dip[3],
                                       (GT_VOID*) &entryGet.key.key.ipv6Unicast.dip[3],
                                        sizeof(macEntry.key.key.ipv6Unicast.dip)-3)) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "get another bits 0..103(13 LSBytes)  macEntryPtr->key.key.ipMcast.dip than was set: %d", dev);
            }
            else
            {
                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipv6Unicast.dip,
                                       (GT_VOID*) &entryGet.key.key.ipv6Unicast.dip,
                                        sizeof(macEntry.key.key.ipv6Unicast.dip))) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                           "get another macEntryPtr->key.key.ipMcast.dip than was set: %d", dev);
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopDataBankNumber,
                                         entryGet.fdbRoutingInfo.nextHopDataBankNumber,
                                         "get another macEntryPtr->fdbRoutingInfo.nextHopDataBankNumber than was set: %d", dev);



            /*
                Call function with out of range nextHopDataBankNumber[17]
                Expected: GT_OUT_OF_RANGE.
            */
            macEntry.fdbRoutingInfo.nextHopDataBankNumber=17;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* any value differ from 0 is error */
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d,  macEntry.fdbRoutingInfo.nextHopDataBankNumber = %d",
                                             dev,  macEntry.fdbRoutingInfo.nextHopDataBankNumber);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "%d,  macEntry.fdbRoutingInfo.nextHopDataBankNumber = %d",
                                             dev,  macEntry.fdbRoutingInfo.nextHopDataBankNumber);
            }

            macEntry.fdbRoutingInfo.nextHopDataBankNumber=1;
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                macEntry.fdbRoutingInfo.nextHopDataBankNumber=0;
            }

            /*
               1.8. Call with macEntryPtr->dstInterface->type [CPSS_INTERFACE_TRUNK_E]
                          and other - the same as in 1.3.
            */
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;
            macEntry.key.key.ipv6Unicast.vrfId = 0;
            macEntry.dstInterface.type = CPSS_INTERFACE_TRUNK_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "%d, macEntryPtr->key.entryType = %d, macEntryPtr->dstInterface->type = %d",
                                             dev, macEntry.key.entryType, macEntry.dstInterface.type);

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                   "get another macEntryPtr->dstInterface.type than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.trunkId,
                                     entryGet.dstInterface.trunkId,
                   "get another macEntryPtr->dstInterface.trunkId than was set: %d", dev);

            /*
                1.9. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                                   key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
                Expected: GT_OUT_OF_RANGE.
            */
            macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                            "%d, macEntryPtr->key->key->ipv4Unicast.vrfId = %d",
                                         dev, macEntry.key.key.ipv4Unicast.vrfId);

            /*
                1.10. Call function with index [3]
                                         and skip [GT_FALSE]
                                         and valid macEntryPtr {key {CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E,
                                                                ipv4Unicast->dip[5.6.7.8],
                                                                ipv4Unicast->vrfId[0]},
                                                                dstInterface {type[CPSS_INTERFACE_PORT_E],
                                                                devPort {devNum [devNum],
                                                                    portNum [0]},
                                                                trunkId [0],
                                                                vidx [10],
                                                                vlanId [100]},

                                                                ttlHopLimitDecEnable[GT_FALSE]
                                                                ttlHopLimDecOptionsExtChkByPass[GT_FALSE]
                                                                ingressMirror[GT_FALSE]
                                                                ingressMirrorToAnalyzerIndex[1]
                                                                qosProfileMarkingEnable[GT_FALSE]
                                                                qosProfileIndex[1]
                                                                qosPrecedence[CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E]
                                                                modifyUp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E]
                                                                modifyDscp[ CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E]
                                                                countSet[1]
                                                                trapMirrorArpBcEnable[GT_FALSE]
                                                                dipAccessLevel[1]
                                                                ICMPRedirectEnable[GT_FALSE]
                                                                mtuProfileIndex[0]
                                                                isTunnelStart[GT_FALSE]
                                                                nextHopVlanId[1]
                                                                nextHopARPPointer[1]
                                                                nextHopTunnelPointer[1]}.
                Expected: GT_OK.
            */
            index = 3;
            skip  = GT_FALSE;

            cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
            macEntry.key.key.ipv4Unicast.vrfId = 0;

            macEntry.key.key.ipv4Unicast.dip[0] = 5;
            macEntry.key.key.ipv4Unicast.dip[1] = 6;
            macEntry.key.key.ipv4Unicast.dip[2] = 7;
            macEntry.key.key.ipv4Unicast.dip[3] = 8;


            macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
            macEntry.dstInterface.devPort.hwDevNum  = dev;
            macEntry.dstInterface.devPort.portNum = 0;
            macEntry.dstInterface.trunkId         = 0;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.vidx            = 10;
            macEntry.dstInterface.vlanId          = 100;

            macEntry.age                      = GT_TRUE;

            macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_FALSE;
            macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
            macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=1;
            macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.qosProfileIndex=1;
            macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
            macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
            macEntry.fdbRoutingInfo.countSet=1;
            macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.dipAccessLevel=1;
            macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_FALSE;
            macEntry.fdbRoutingInfo.mtuProfileIndex=0;
            macEntry.fdbRoutingInfo.isTunnelStart=GT_FALSE;
            macEntry.fdbRoutingInfo.nextHopVlanId=1;
            macEntry.fdbRoutingInfo.nextHopARPPointer=1;
            macEntry.fdbRoutingInfo.nextHopTunnelPointer=1;

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* entry should fail due to
                    macEntry.fdbRoutingInfo.dipAccessLevel=1;
                    macEntry.fdbRoutingInfo.qosProfileIndex=1;
                    macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
                */
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.dipAccessLevel=0;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.qosProfileIndex=0;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);
                macEntry.fdbRoutingInfo.ingressMirror=GT_FALSE;
            }

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

            /*
                1.11. Call cpssDxChBrgFdbMacEntryRead with index [3]
                                                           and non-NULL pointers.
            */
            index = 3;

            st = utfCpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet, &agedGet, &HwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.ipv4Unicast.vrfId,
                                         entryGet.key.key.ipv4Unicast.vrfId,
                       "get another macEntryPtr->key.key.ipv4Unicast.vrfId than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.ipv4Unicast.dip,
                                   (GT_VOID*) &entryGet.key.key.ipv4Unicast.dip,
                                    sizeof(macEntry.key.key.ipv4Unicast.dip))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macEntryPtr->key.macVlan.key.ipv4Unicast.dip than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                       "get another macEntryPtr->dstInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.hwDevNum,
                                         entryGet.dstInterface.devPort.hwDevNum,
                       "get another macEntryPtr->dstInterface.devPort.hwDevNum than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.devPort.portNum,
                                         entryGet.dstInterface.devPort.portNum,
                       "get another macEntryPtr->dstInterface.devPort.portNum than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimitDecEnable, entryGet.fdbRoutingInfo.ttlHopLimitDecEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimitDecEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass, entryGet.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass,
                       "get another macEntryPtr->fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirror, entryGet.fdbRoutingInfo.ingressMirror,
                       "get another macEntryPtr->fdbRoutingInfo.ingressMirror than was set: %d", dev);
            if(macEntry.fdbRoutingInfo.ingressMirror == GT_TRUE)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex, entryGet.fdbRoutingInfo.ingressMirrorToAnalyzerIndex,
                           "get another macEntryPtr->fdbRoutingInfo.ingressMirrorToAnalyzerIndex than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileMarkingEnable, entryGet.fdbRoutingInfo.qosProfileMarkingEnable,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileMarkingEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosProfileIndex, entryGet.fdbRoutingInfo.qosProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.qosProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.qosPrecedence, entryGet.fdbRoutingInfo.qosPrecedence,
                       "get another macEntryPtr->fdbRoutingInfo.qosPrecedence than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyUp, entryGet.fdbRoutingInfo.modifyUp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyUp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.modifyDscp, entryGet.fdbRoutingInfo.modifyDscp,
                       "get another macEntryPtr->fdbRoutingInfo.modifyDscp than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.countSet, entryGet.fdbRoutingInfo.countSet,
                       "get another macEntryPtr->fdbRoutingInfo.countSet than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.trapMirrorArpBcEnable, entryGet.fdbRoutingInfo.trapMirrorArpBcEnable,
                       "get another macEntryPtr->fdbRoutingInfo.trapMirrorArpBcEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.dipAccessLevel, entryGet.fdbRoutingInfo.dipAccessLevel,
                       "get another macEntryPtr->fdbRoutingInfo.dipAccessLevel than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.ICMPRedirectEnable, entryGet.fdbRoutingInfo.ICMPRedirectEnable,
                       "get another macEntryPtr->fdbRoutingInfo.ICMPRedirectEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.mtuProfileIndex, entryGet.fdbRoutingInfo.mtuProfileIndex,
                       "get another macEntryPtr->fdbRoutingInfo.mtuProfileIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.isTunnelStart, entryGet.fdbRoutingInfo.isTunnelStart,
                       "get another macEntryPtr->fdbRoutingInfo.isTunnelStart than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopVlanId, entryGet.fdbRoutingInfo.nextHopVlanId,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopVlanId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.fdbRoutingInfo.nextHopARPPointer, entryGet.fdbRoutingInfo.nextHopARPPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopARPPointer than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0, entryGet.fdbRoutingInfo.nextHopTunnelPointer,
                       "get another macEntryPtr->fdbRoutingInfo.nextHopTunnelPointer than was set: %d", dev);


            /*
                1.12. Call function with null macEntryPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            index = 0;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macEntryPtr = NULL", dev);

            /*
                1.13. Call function with wrong enum values key->entryType
                                 and other parameters the same as in 1.10.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                            (dev, index, skip, &macEntry),
                                                            macEntry.key.entryType);

            macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;

            /*
                1.14. Call function with wrong enum values dstInterface->type
                                         and other parameters the same as in 1.10.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntryWrite
                                                            (dev, index, skip, &macEntry),
                                                            macEntry.dstInterface.type);

            macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

            /*
                1.15. Call function with out of range macEntryPtr->key->ipv4Unicast->vrfId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                      key->entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E] and other params from 1.10.
                Expected: GT_OUT_OF_RANGE.
            */
            macEntry.key.key.ipv4Unicast.vrfId = PRV_CPSS_MAX_NUM_VLANS_CNS;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                            "%d, macEntryPtr.key.key.ipv4Unicast.vrfId = %d",
                                         dev, macEntry.key.key.ipv4Unicast.vrfId);

            macEntry.key.key.ipv4Unicast.vrfId = 0;

            /*
                1.16. Call function with out of range dstInterface->devPort->portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                                         dstInterface->type = CPSS_INTERFACE_PORT_E
                                         and other parameters the same as in 1.10.
                Expected: NOT GT_OK.
            */
            macEntry.dstInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);
            macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, macEntryPtr->dstInterface.devPort.portNum = %d",
                                             dev, macEntry.dstInterface.devPort.portNum);

            /*
                1.17. Call function with out of range dstInterface->devPort->hwDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                                         dstInterface->type = CPSS_INTERFACE_PORT_E
                                         and other parameters the same as in 1.10.
                Expected: NON GT_OK.
            */
            macEntry.dstInterface.devPort.portNum = BRG_FDB_VALID_PHY_PORT_CNS;
            macEntry.dstInterface.devPort.hwDevNum  = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
            macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, macEntryPtr->dstInterface->devPort->hwDevNum = %d",
                                         dev, macEntry.dstInterface.devPort.hwDevNum);

            /*
                1.18. Call function with out of range dstInterface->trunkId [128],
                                         dstInterface->type = CPSS_INTERFACE_TRUNK_E
                                         and other parameters the same as in 1.10.
                Expected: NOT GT_OK.
            */
            macEntry.dstInterface.devPort.hwDevNum = dev;
            macEntry.dstInterface.trunkId        = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.type           = CPSS_INTERFACE_TRUNK_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, macEntryPtr->dstInterface->trunkId = %d",
                                             dev, macEntry.dstInterface.trunkId);

            /*
                1.19. Call function with out of range dstInterface->vidx [maxMeGrInd(dev)],
                                     dstInterface->type = CPSS_INTERFACE_VIDX_E
                                     and other parameters the same as in 1.10.
                Expected: NOT GT_OK.
            */
            macEntry.dstInterface.trunkId = 10;
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.vidx    = MAX_VIDX_MAC(dev);
            macEntry.dstInterface.type    = CPSS_INTERFACE_VIDX_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                "%d, macEntryPtr->dstInterface->vidx = %d",
                                             dev, macEntry.dstInterface.vidx);

            /*
                1.20. Call function with out of range dstInterface->vlanId [5]
                                     and dstInterface->type [CPSS_INTERFACE_VID_E].
                Expected: GT_OK, vladId will e set to 0xFFF.
            */
            macEntry.dstInterface.vidx   = 10;
            macEntry.dstInterface.vlanId = 5;
            macEntry.dstInterface.type   = CPSS_INTERFACE_VID_E;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->dstInterface->vlanId = %d",
                                         dev, macEntry.dstInterface.vlanId);

            /*
                1.21. Call function with out of range macEntryPtr->ingressMirrorToAnalyzerIndex[7]
                                     and other parameters the same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            if(macEntry.fdbRoutingInfo.ingressMirror == GT_TRUE)
            {
                macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=7;

                st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = %d",
                                             dev, macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex);

                macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
            }

            /*
                1.22. Call function with out of range macEntryPtr->qosProfileIndex[128],
                                     and other parameters the same as in 1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            macEntry.fdbRoutingInfo.qosProfileIndex=128;

            st = utfCpssDxChBrgFdbMacEntrySet(dev, &macEntry);
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* the fild not supported in any value != 0 */
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.qosProfileIndex = %d",
                                         dev, macEntry.fdbRoutingInfo.qosProfileIndex);

                macEntry.fdbRoutingInfo.qosProfileIndex=0;
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, macEntry.fdbRoutingInfo.qosProfileIndex = %d",
                                         dev, macEntry.fdbRoutingInfo.qosProfileIndex);


                macEntry.fdbRoutingInfo.qosProfileIndex=1;
            }

            /*
                1.23. Call with out of range qosPrecedence and other params from 1.1.
                Expected: NOT GT_OK.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                            (dev, &macEntry),
                                                            macEntry.fdbRoutingInfo.qosPrecedence);

            /*
                1.24. Call with out of range dipAccessLevel [64] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.dipAccessLevel=64;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntry.fdbRoutingInfo.dipAccessLevel=64; = %d",
                                         dev, macEntry.fdbRoutingInfo.dipAccessLevel);
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                macEntry.fdbRoutingInfo.dipAccessLevel=0;
            }
            else
            {
                macEntry.fdbRoutingInfo.dipAccessLevel=1;
            }

            /*
                1.25. Call with with out of mtuProfileIndex [8] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.mtuProfileIndex = 8;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->fdbRoutingInfo.mtuProfileIndex = %d",
                                         dev, macEntry.fdbRoutingInfo.mtuProfileIndex);

            macEntry.fdbRoutingInfo.mtuProfileIndex = 0;

            /*
                1.26. Call with with out of nextHopVlanId [8192] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.nextHopVlanId = 8192;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->fdbRoutingInfo.nextHopVlanId = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopVlanId);

            macEntry.fdbRoutingInfo.nextHopVlanId = 0;

            /*
                1.27. Call with with out of nextHopARPPointer [131072] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.isTunnelStart=GT_FALSE;
            macEntry.fdbRoutingInfo.nextHopARPPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp <= _128K ?
                                                _128K:
                                                PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->fdbRoutingInfo.nextHopARPPointer = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopARPPointer);

            macEntry.fdbRoutingInfo.nextHopARPPointer = 0;

            /*
                1.28. Call with with out of nextHopTunnelPointer [32768] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
            macEntry.fdbRoutingInfo.nextHopTunnelPointer=PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart <= _32K ?
                                                    _32K :
                                                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->fdbRoutingInfo.nextHopTunnelPointer = %d",
                                         dev, macEntry.fdbRoutingInfo.nextHopTunnelPointer);

            macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;

            /*
                1.29. Call with with out of countSet [7] and other params from 1.1.
                Expected: NOT GT_OK.
            */
            macEntry.fdbRoutingInfo.countSet = 7;

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, macEntryPtr->fdbRoutingInfo.countSet = %d",
                                         dev, macEntry.fdbRoutingInfo.countSet);

            macEntry.fdbRoutingInfo.countSet = 0;

            /*
                1.30. Call with with out of siteId and other params from 1.2.
                Expected: NOT GT_OK.
            */

            macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E;

            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                        (dev, &macEntry),
                                                        macEntry.fdbRoutingInfo.siteId);
            /*
                1.31. Call cpssDxChBrgFdbMacEntryInvalidate with
                    index [0/ 1/ 2 / 3] to invalidate all changes
                Expected: GT_OK.
            */

            /* Call with index [0] */
            index = 0;
            st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

            /* Call with index [1] */
            index = 1;

            st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

            /* Call with index [2] */
            index = 2;

            st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

            /* Call with index [3] */
            index = 3;

            st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, index);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, index);

            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* 1.32 Call with out of range multipathPointer and other params from 1.1.*/
                macEntry.fdbRoutingInfo.routingType=CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E;
                macEntry.fdbRoutingInfo.multipathPointer = BIT_15;
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "%d,  macEntry.fdbRoutingInfo.multipathPointer = %d",
                                dev,  macEntry.fdbRoutingInfo.multipathPointer);
                macEntry.fdbRoutingInfo.multipathPointer = 0;
                /*
                    1.33. Call with out of range pointerType and other params from 1.1.
                    Expected: NOT GT_OK.
                */
                UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacEntrySet
                                                                (dev, &macEntry),
                                                                macEntry.fdbRoutingInfo.routingType);
            }
        }
    }

    index = 0;
    skip  = GT_FALSE;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
    macEntry.key.key.ipv4Unicast.vrfId = 0;

    macEntry.key.key.ipv4Unicast.dip[0] = 1;
    macEntry.key.key.ipv4Unicast.dip[1] = 2;
    macEntry.key.key.ipv4Unicast.dip[2] = 3;
    macEntry.key.key.ipv4Unicast.dip[3] = 4;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.devPort.hwDevNum  = dev;
    macEntry.dstInterface.devPort.portNum = 0;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.hwDevNum  = dev;
    macEntry.dstInterface.vidx            = 10;
    macEntry.dstInterface.vlanId          = 100;

    macEntry.age                      = GT_TRUE;

    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass=GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror=GT_TRUE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex=2;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.qosProfileIndex=3;
    macEntry.fdbRoutingInfo.qosPrecedence=CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.modifyDscp=CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    macEntry.fdbRoutingInfo.countSet=1;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.dipAccessLevel=5;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable=GT_TRUE;
    macEntry.fdbRoutingInfo.mtuProfileIndex=1;
    macEntry.fdbRoutingInfo.isTunnelStart=GT_TRUE;
    macEntry.fdbRoutingInfo.nextHopVlanId=6;
    macEntry.fdbRoutingInfo.nextHopARPPointer=7;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer=8;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMaxLookupLenSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookupLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMaxLookupLenSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with lookupLen [4].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMaxLookupLenGet
    Expected: GT_OK and the same lookupLenPtr.
    1.3. Call function with out of range lookupLen [0, 36].
    Expected: NOT GT_OK.
    1.4. Call function with out of range lookupLen [5].
    Expected: NOT GT_OK.
*/
    GT_STATUS  st     = GT_OK;

    GT_U8      dev;
    GT_U32     len    = 0;
    GT_U32     lenGet = 0;
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChBrgFdbHashModeGet(dev,&hashMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, len);

        /*
            1.1. Call function with lookupLen [4].
            Expected: GT_OK.
        */
        if(hashMode == CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
        {
            len = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.numOfBanks;
        }
        else
        {
            len = 4;
        }

        st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, len);

        /*
            1.2. Call cpssDxChBrgFdbMaxLookupLenGet
            Expected: GT_OK and the same lookupLenPtr.
        */
        st = cpssDxChBrgFdbMaxLookupLenGet(dev, &lenGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMaxLookupLenGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(len, lenGet,
                       "get another lookupLenPtr than was set: %d", dev);

        /*
            1.3. Call function with out of range lookupLen [0, 36].
            Expected: NOT GT_OK.
        */

        /* Call with lookupLen [0] */
        len = 0;

        st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, len);

        /* Call with lookupLen [36] */
        len = 36;

        st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, len);

        /*
            1.4. Call function with out of range lookupLen [5].
            Expected: NOT GT_OK.
        */
        len = 5;

        st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, len);
    }

    len = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMaxLookupLenSet(dev, len);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMaxLookupLenGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *lookupLenPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMaxLookupLenGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null lookupLenPtr.
    Expected: GT_OK.
    1.2. Call with null lookupLenPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;

    GT_U8       dev;
    GT_U32      len = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null lookupLenPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbMaxLookupLenGet(dev, &len);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null lookupLenPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbMaxLookupLenGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMaxLookupLenGet(dev, &len);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMaxLookupLenGet(dev, &len);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacVlanLookupModeSet
(
    IN GT_U8            devNum,
    IN CPSS_MAC_VL_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacVlanLookupModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_IVL_EE / CPSS_SVL_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMacVlanLookupModeGet
    Expected: GT_OK and the same modePtr.
    1.3. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS          st      = GT_OK;


    GT_U8              dev;
    CPSS_MAC_VL_ENT    mode    = CPSS_IVL_E;
    CPSS_MAC_VL_ENT    modeGet = CPSS_IVL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_IVL_EE / CPSS_SVL_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_IVL_EE] */
        mode = CPSS_IVL_E;

        st = cpssDxChBrgFdbMacVlanLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbMacVlanLookupModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacVlanLookupModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_SVL_E] */
        mode = CPSS_SVL_E;

        st = cpssDxChBrgFdbMacVlanLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbMacVlanLookupModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacVlanLookupModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacVlanLookupModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_IVL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMacVlanLookupModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMacVlanLookupModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacVlanLookupModeGet
(
    IN  GT_U8            devNum,
    OUT CPSS_MAC_VL_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacVlanLookupModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS        st  = GT_OK;

    GT_U8            dev;
    CPSS_MAC_VL_ENT  mode = CPSS_IVL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMacVlanLookupModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAuMsgRateLimitSet
(
    IN GT_U8                       devNum,
    IN GT_U32                      msgRate,
    IN GT_BOOL                     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAuMsgRateLimitSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with msgRate [2 * (200 * coreClock / 200)], enable [GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbAuMsgRateLimitGet
    Expected: GT_OK and the same msgRatePtr and enablePtr.
    1.3. Call with out of range
    msgRate [(51200/200 + 1) * (200 * coreClock / 200)], enable [GT_TRUE].
    Expected: NOT GT_OK.
    1.4. Call with out of range
    msgRate [(51200/200 + 1) * (200 * coreClock / 200)], enable [GT_FALSE].
    Expected: GT_OK.
*/
    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_U32      rate      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      rateGet   = 0;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      coreClock = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting core clock */
        st = prvUtfCoreClockGet(dev, &coreClock);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCoreClockGet: %d", dev);

        /*
            1.1. Call with msgRate [1], enable [GT_TRUE].
            Expected: GT_OK.
        */
        rate   = 2 * (200 * coreClock / 200);
        enable = GT_TRUE;

        st = cpssDxChBrgFdbAuMsgRateLimitSet(dev, rate, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, rate, enable);

        /*
            1.2. Call cpssDxChBrgFdbAuMsgRateLimitGet
            Expected: GT_OK and the same msgRatePtr and enablePtr.
        */
        st = cpssDxChBrgFdbAuMsgRateLimitGet(dev,&rateGet,&enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAuMsgRateLimitGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another enablePtr than was set: %d", dev);

        /*
            1.3. Call with out of range msgRate [51200*devClock(dev)/200],
                   enable [GT_TRUE].
            Expected: NOT GT_OK.
        */
        rate   = (51200/200 + 1) * (200 * coreClock / 200);
        enable = GT_TRUE;

        st = cpssDxChBrgFdbAuMsgRateLimitSet(dev, rate, enable);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, rate, enable);

        /*
            1.4. Call with out of range msgRate [51000*devClock(dev)/200],
                           enable [GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbAuMsgRateLimitSet(dev, rate, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, rate, enable);
    }

    rate   = 200;
    enable = GT_TRUE;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAuMsgRateLimitSet(dev, rate, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAuMsgRateLimitSet(dev, rate, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAuMsgRateLimitGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *msgRatePtr,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAuMsgRateLimitGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null msgRatePtr and non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null msgRatePtr [NULL] and non-null enablePtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null msgRatePtr and null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32      rate   = 0;
    GT_BOOL     enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null msgRatePtr and non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAuMsgRateLimitGet(dev, &rate, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null msgRatePtr [NULL] and non-null enablePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuMsgRateLimitGet(dev, NULL, &enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.3. Call with non-null msgRatePtr and null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuMsgRateLimitGet(dev, &rate, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAuMsgRateLimitGet(dev, &rate, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAuMsgRateLimitGet(dev, &rate, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaMsgOnChainTooLongSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaMsgOnChainTooLongSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbNaMsgOnChainTooLongGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbNaMsgOnChainTooLongSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbNaMsgOnChainTooLongGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaMsgOnChainTooLongGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbNaMsgOnChainTooLongSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbNaMsgOnChainTooLongGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, &stateGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbNaMsgOnChainTooLongGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaMsgOnChainTooLongSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaMsgOnChainTooLongSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaMsgOnChainTooLongGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaMsgOnChainTooLongGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaMsgOnChainTooLongGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSpAaMsgToCpuSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSpAaMsgToCpuSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbSpAaMsgToCpuGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbSpAaMsgToCpuSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbSpAaMsgToCpuGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSpAaMsgToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbSpAaMsgToCpuSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbSpAaMsgToCpuGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSpAaMsgToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSpAaMsgToCpuSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSpAaMsgToCpuSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSpAaMsgToCpuGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSpAaMsgToCpuGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSpAaMsgToCpuGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAAandTAToCpuSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAAandTAToCpuSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbAAandTAToCpuGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbAAandTAToCpuSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAAandTAToCpuGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbAAandTAToCpuGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAAandTAToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbAAandTAToCpuSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAAandTAToCpuGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbAAandTAToCpuGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAAandTAToCpuGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAAandTAToCpuSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAAandTAToCpuSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAAandTAToCpuGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAAandTAToCpuGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAAandTAToCpuGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAAandTAToCpuGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAAandTAToCpuGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAAandTAToCpuGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_HASH_FUNC_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_MAC_HASH_FUNC_XOR_E /
                                  CPSS_MAC_HASH_FUNC_CRC_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbHashModeGet
    Expected: GT_OK and the same modePtr.
    1.3. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st      = GT_OK;

    GT_U8                       dev;
    CPSS_MAC_HASH_FUNC_MODE_ENT mode    = CPSS_MAC_HASH_FUNC_XOR_E;
    CPSS_MAC_HASH_FUNC_MODE_ENT modeGet = CPSS_MAC_HASH_FUNC_XOR_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_MAC_HASH_FUNC_XOR_E /
                                          CPSS_MAC_HASH_FUNC_CRC_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_MAC_HASH_FUNC_XOR_E] */
        mode = CPSS_MAC_HASH_FUNC_XOR_E;

        st = cpssDxChBrgFdbHashModeSet(dev, mode);
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

            /*
                1.2. Call cpssDxChBrgFdbHashModeGet
                Expected: GT_OK and the same modePtr.
            */
            st = cpssDxChBrgFdbHashModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbHashModeGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "get another modePtr than was set: %d", dev);
        }

        /* 1.1. Call with mode [CPSS_MAC_HASH_FUNC_CRC_E] */
        mode = CPSS_MAC_HASH_FUNC_CRC_E;

        st = cpssDxChBrgFdbHashModeSet(dev, mode);
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, mode);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

            /*
                1.2. Call cpssDxChBrgFdbHashModeGet
                Expected: GT_OK and the same modePtr.
            */
            st = cpssDxChBrgFdbHashModeGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbHashModeGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "get another modePtr than was set: %d", dev);
        }

        /* 1.1. Call with mode [CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E] */
        mode = CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;

        st = cpssDxChBrgFdbHashModeSet(dev, mode);
        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

            if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* restore last value */
                mode = CPSS_MAC_HASH_FUNC_CRC_E;

                st = cpssDxChBrgFdbHashModeSet(dev, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, mode);
        }


        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_MAC_HASH_FUNC_XOR_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbHashModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashModeGet
(
    IN  GT_U8                         devNum,
    OUT CPSS_MAC_HASH_FUNC_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st   = GT_OK;

    GT_U8                        dev;
    CPSS_MAC_HASH_FUNC_MODE_ENT  mode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbHashModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbHashModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgingTimeoutSet
(
    IN GT_U8  devNum,
    IN GT_U32 timeout
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgingTimeoutSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with timeout [300].
    Expected: GT_OK.
    1.2. Call function with timeout [615].
    Expected: NOT GT_OK for device with 200 MHZ.
    1.3. Call function with timeout [890].
    Expected: NOT GT_OK for device with 144 MHZ
              and NOT GT_OK for device with 200 MHZ.
    1.4. Call function with timeout [140, 143].
    Expected: GT_OK.
    1.5. Call cpssDxChBrgFdbAgingTimeoutGet
    Expected: GT_OK and timeoutPtr == 140.
    1.6. Call function with timeout [150].
    Expected: GT_OK.
    1.7. Call cpssDxChBrgFdbAgingTimeoutGet
    Expected: GT_OK and timeoutPtr == 140 for clock 144 MHZ
              and timeoutPtr == 150 for clock 200 MHZ.
    1.8. Call function with timeout [160].
    Expected: GT_OK.
    1.9. Call cpssDxChBrgFdbAgingTimeoutGet
    Expected: GT_OK and timeoutPtr == 154 for clock 144 MHZ
              and timeoutPtr == 160 for clock 200 MHZ.
*/
    GT_STATUS   st         = GT_OK;

    GT_U8       dev;
    GT_U32      timeout    = 0;
    GT_U32      timeoutGet = 0;
    GT_U32      coreClock  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with timeout [300].
            Expected: GT_OK.
        */
        timeout = 300;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /* Getting core clock */
        st = prvUtfCoreClockGet(dev, &coreClock);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfCoreClockGet: %d", dev);

        /*
            1.2. Call function with timeout [615].
            Expected: NOT GT_OK for device with 200 MHZ.
        */
        timeout = 615;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        if (200 == coreClock && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);
        }

        /*
            1.3. Call function with timeout [890].
            Expected: NOT GT_OK for device with 144 MHZ
                      and NOT GT_OK for device with 200 MHZ.
        */
        timeout = 890;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        if (((200 == coreClock) || (144 == coreClock)) && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);
        }

        /*
            1.4. Call function with timeout [140, 143].
            Expected: GT_OK.
        */

        /* timeout [140] */
        timeout = 140;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.5. Call cpssDxChBrgFdbAgingTimeoutGet
            Expected: GT_OK and timeoutPtr == 140.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutGet: %d", dev);

        if (((200 == coreClock) || (220 == coreClock)) && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(140, timeoutGet,
                           "get another timeoutPtr than was set: %d", dev);
        }

        /* timeout [143] */
        timeout = 143;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.5. Call cpssDxChBrgFdbAgingTimeoutGet
            Expected: GT_OK and timeoutPtr == 140.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutGet: %d", dev);

        if (((200 == coreClock) || (220 == coreClock)) && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(140, timeoutGet,
                           "get another timeoutPtr than was set: %d", dev);
        }

        /*
            1.6. Call function with timeout [150].
            Expected: GT_OK.
        */
        timeout = 150;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.7. Call cpssDxChBrgFdbAgingTimeoutGet
            Expected: GT_OK and timeoutPtr == 140 for clock 144 MHZ
                      and timeoutPtr == 150 for clock 200 MHZ.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutGet: %d", dev);
        if (200 == coreClock  && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(150, timeoutGet,
                       "get another timeoutPtr than was set: %d", dev);
        }
        else if (144 == coreClock  && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(140, timeoutGet,
                       "get another timeoutPtr than was set: %d", dev);
        }

        /*
            1.8. Call function with timeout [160].
            Expected: GT_OK.
        */
        timeout = 160;

        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.9. Call cpssDxChBrgFdbAgingTimeoutGet
            Expected: GT_OK and timeoutPtr == 154 for clock 144 MHZ
                      and timeoutPtr == 160 for clock 200 MHZ.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutGet: %d", dev);
        if (200 == coreClock  && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(160, timeoutGet,
                       "get another timeoutPtr than was set: %d", dev);
        }
        else if (144 == coreClock  && !PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(154, timeoutGet,
                       "get another timeoutPtr than was set: %d", dev);
        }
    }

    timeout = 300;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgingTimeoutSet(dev, timeout);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

static void doSleepWithTimeIndication(IN GT_U32     timeToSleep)
{
    GT_U32  granularity = 1000;
    GT_U32  ii;
    GT_U32  timePaddedAlready = 0;

    for(ii = 0 ;
        (timePaddedAlready + granularity) <= timeToSleep ;
        ii++ , timePaddedAlready += granularity)
    {
        cpssOsTimerWkAfter(granularity);

        PRV_UTF_LOG1_MAC(" %d " , ii+1);
        if(0x1F == (ii & 0x1F))
        {
            PRV_UTF_LOG0_MAC("\n");
        }
    }
    PRV_UTF_LOG0_MAC("\n");

    /* we need to sleep the 'leftover' from the granularity */
    if(timeToSleep > timePaddedAlready)
    {
        cpssOsTimerWkAfter(timeToSleep - timePaddedAlready);
    }

    PRV_UTF_LOG1_MAC("done sleep %d \n",timeToSleep);
}


UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgingTimeoutSet__auto_aging_functionality)
{
    GT_STATUS   st         = GT_OK;

    GT_U8       dev;
    GT_U32      numFdbEntries;
    CPSS_MAC_ENTRY_EXT_STC       macEntry;
    GT_U32      ii,retry,retryMax=100;
    GT_U32      timeout = 20 , timeoutGet;
    GT_U32      numOfValid,numOfSkip;
    GT_U32      sleep;
    GT_BOOL     isAuFifoUsed;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E);

    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        /* GM not supports aging daemon */
        GM_NOT_SUPPORT_THIS_TEST_MAC
    }

    cpssOsMemSet(&macEntry,0,sizeof(macEntry));
    /*set dynamic entries on trunk 1 */
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.portNum = 0;/* set port that for multi-port group associated with port group 0 */
    macEntry.dstInterface.trunkId = 1;
    macEntry.key.key.macVlan.macAddr.arEther[0] = 0x00;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0x11;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0x22;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0x33;
    macEntry.key.key.macVlan.macAddr.arEther[4] = 0x44;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0x55;
    /* set the entry as 'refeshed' that needs 2 aging cycles */
    macEntry.age    = GT_TRUE;

    /* test aging timeout by:
        aging time 20 sec

        part 1 : no aging before time
        ==============================
        add 1000 fdb entries (cause event spread in the FDB)
        trigger auto aging
        sleep 18 sec (1 cycle - 2 seconds)
        stop auto aging (set trigger aging)
        check no entry deleted.

        part 2 : all aging done in time
        ==============================
        add 1000 fdb entries (cause event spread in the FDB)
        trigger auto aging
        sleep 42 sec (2 cycles + 2 seconds)
        stop auto aging (set trigger aging)
        check ALL entries deleted.

    */


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPpHwImplementWaGet(dev, CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E, &isAuFifoUsed);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPpHwImplementWaGet: %d", dev);

        /* reduce number of entries for FIFO systems to avoid AU FIFO Full erratum */
        numFdbEntries = (isAuFifoUsed) ? 20 : 1000;

        macEntry.dstInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(dev);

        /* make sure that the FDB is empty prior to the start of the test */
        PRV_UTF_LOG0_MAC("flush FDB: make sure that the FDB is empty prior to the start of the test \n");
        prvTgfBrgFdbFlush(GT_TRUE);

        if(macEntry.age == GT_TRUE)/*dummy --> always true */
        {
            /* make sure that the appDemo will process the AA messages */
            appDemoSupportAaMessageSet(GT_TRUE);

            PRV_UTF_LOG0_MAC("set the aging without removal (lion,lion2 need it) \n");
            st = cpssDxChBrgFdbActionModeSet(dev, CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbActionModeSet: %d", dev);

            PRV_UTF_LOG0_MAC("Enable AA and TA for sending to the CPU \n");
            st = cpssDxChBrgFdbAAandTAToCpuSet(dev, GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbAAandTAToCpuSet: %d", dev);
        }
        else
        {
            PRV_UTF_LOG0_MAC("set the aging with removal (so appDemo not need to handle AA) \n");
            st = cpssDxChBrgFdbActionModeSet(dev, CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbActionModeSet: %d", dev);

            PRV_UTF_LOG0_MAC("Disable AA and TA from sending to the CPU \n");
            st = cpssDxChBrgFdbAAandTAToCpuSet(dev, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbAAandTAToCpuSet: %d", dev);
        }

        /* synch with the appDemo */
        AU_FDB_UPDATE_LOCK();

        PRV_UTF_LOG1_MAC("set the aging timeout [%d] \n",timeout);
        st = cpssDxChBrgFdbAgingTimeoutSet(dev,timeout);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutSet: %d", dev);

        PRV_UTF_LOG0_MAC("get the aging timeout \n");
        st = cpssDxChBrgFdbAgingTimeoutGet(dev,&timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgingTimeoutGet: %d", dev);
        if(timeoutGet != timeout)
        {
            PRV_UTF_LOG2_MAC("the timeout was set to [%d] (not [%d]) \n",timeoutGet,timeout);
        }
        /* use the actual time that we got (due to granularity issues)*/
        timeout = timeoutGet;

        PRV_UTF_LOG0_MAC("set the trigger mode [TRIG] \n");
        /* set the device in trigger aging --> to stop any aging that may be */
        st = cpssDxChBrgFdbMacTriggerModeSet(dev,CPSS_ACT_TRIG_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeSet: %d", dev);

        /* synch with the appDemo */
        AU_FDB_UPDATE_UNLOCK();

        retry = 0;

        /* add 1000 FDB entries (same mac , different vlan) */
        PRV_UTF_LOG1_MAC("add %d FDB entries (same mac , different vlan) \n",numFdbEntries);
        for(ii = 0 ; ii < numFdbEntries ; ii++)
        {
            macEntry.key.key.macVlan.vlanId = (GT_U16)ii;
            /* synch with the appDemo */
            AU_FDB_UPDATE_LOCK();
            st = utfCpssDxChBrgFdbMacEntrySet(dev,&macEntry);
            /* synch with the appDemo */
            AU_FDB_UPDATE_UNLOCK();
            if(st == GT_BAD_STATE && retry < retryMax)
            {
                ii--; /*allow retry of this entry again*/
                retry++;
                cpssOsTimerWkAfter(1);
                continue;
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "utfCpssDxChBrgFdbMacEntrySet: %d", dev);
            retry = 0;
        }

        PRV_UTF_LOG0_MAC("sleep 100 ms \n");
        cpssOsTimerWkAfter(100);

        /* set the device in auto aging */
        PRV_UTF_LOG0_MAC("set the trigger mode [AUTO] \n");
        st = cpssDxChBrgFdbMacTriggerModeSet(dev,CPSS_ACT_AUTO_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeSet: %d", dev);

        /* the timeout declared as 'single pass' of aging daemon, so after (timeout - 2) passes
           still all entries should exist  */
        sleep = (timeout - 2)*1000;
        PRV_UTF_LOG1_MAC("the timeout declared as 'single pass' of aging daemon, so after (timeout - 2) passes \n"
           "still all entries should exist --> sleep (%d) ms \n",sleep);
        doSleepWithTimeIndication(sleep);

        PRV_UTF_LOG0_MAC("set the trigger mode [TRIG] \n");
        /* set the device in trigger aging --> to stop any aging that may be */
        /* synch with the appDemo */
        AU_FDB_UPDATE_LOCK();
        st = cpssDxChBrgFdbMacTriggerModeSet(dev,CPSS_ACT_TRIG_E);
        /* synch with the appDemo */
        AU_FDB_UPDATE_UNLOCK();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeSet: %d", dev);

        PRV_UTF_LOG0_MAC("sleep 50 ms \n");
        cpssOsTimerWkAfter(50);

        /* get the number of valid,skipped entries in the FDB */
        PRV_UTF_LOG0_MAC("get the number of valid,skipped entries in the FDB \n");
        st = prvTgfBrgFdbPortGroupCount(dev,0,&numOfValid, &numOfSkip, NULL, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        PRV_UTF_LOG2_MAC("got numOfValid[%d] , numOfSkip[%d]  \n",numOfValid, numOfSkip);
        /* make sure that all entries are still valid  */
        UTF_VERIFY_EQUAL2_STRING_MAC(numFdbEntries, (numOfValid - numOfSkip),
                                     "prvTgfBrgFdbPortGroupCount number of FDB valid entries [%d] and not [%d] \n", numOfValid - numOfSkip , numFdbEntries);

        retry = 0;
        /* 'refresh' the 1000 FDB entries (same mac , different vlan) */
        PRV_UTF_LOG1_MAC("'refresh' the %d FDB entries (same mac , different vlan) \n",numFdbEntries);
        for(ii = 0 ; ii < numFdbEntries ; ii++)
        {
            macEntry.key.key.macVlan.vlanId = (GT_U16)ii;
            /* synch with the appDemo */
            AU_FDB_UPDATE_LOCK();
            st = utfCpssDxChBrgFdbMacEntrySet(dev,&macEntry);
            /* synch with the appDemo */
            AU_FDB_UPDATE_UNLOCK();
            if(st == GT_BAD_STATE && retry < retryMax)
            {
                ii--; /*allow retry of this entry again*/
                retry++;
                cpssOsTimerWkAfter(1);
                continue;
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "utfCpssDxChBrgFdbMacEntrySet: %d", dev);
            retry = 0;
        }

        PRV_UTF_LOG0_MAC("set the trigger mode [AUTO] \n");
        /* synch with the appDemo */
        AU_FDB_UPDATE_LOCK();
        st = cpssDxChBrgFdbMacTriggerModeSet(dev,CPSS_ACT_AUTO_E);
        /* synch with the appDemo */
        AU_FDB_UPDATE_UNLOCK();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeSet: %d", dev);

        /* sleep for (2*timeout) + 2 seconds the timeout ... and then check that ALL deleted */
        sleep = ((2*timeout) + 2) *1000;
        PRV_UTF_LOG1_MAC("sleep for (2*timeout) + 2 seconds the timeout ... and then check that ALL deleted --> sleep (%d) ms \n",
                    sleep);
        doSleepWithTimeIndication(sleep);

        /* set the device in trigger aging --> to stop any aging that may be */
        PRV_UTF_LOG0_MAC("set the trigger mode [TRIG] \n");
        /* synch with the appDemo */
        AU_FDB_UPDATE_LOCK();
        st = cpssDxChBrgFdbMacTriggerModeSet(dev,CPSS_ACT_TRIG_E);
        /* synch with the appDemo */
        AU_FDB_UPDATE_UNLOCK();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeSet: %d", dev);

        /* get the number of valid,skipped entries in the FDB */
        PRV_UTF_LOG0_MAC("get the number of valid,skipped entries in the FDB \n");
        st = prvTgfBrgFdbPortGroupCount(dev,0,&numOfValid, &numOfSkip, NULL, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        PRV_UTF_LOG2_MAC("got numOfValid[%d] , numOfSkip[%d]  \n",numOfValid, numOfSkip);
        /* make sure that all entries deleted */
        UTF_VERIFY_EQUAL2_STRING_MAC(0, (numOfValid - numOfSkip),
                                     "prvTgfBrgFdbPortGroupCount number of FDB valid entries [%d] and not [%d] \n", numOfValid - numOfSkip , 0);

        /* make sure that the FDB is empty at end of the test */
        /* removed to allow debug prvTgfBrgFdbFlush(GT_TRUE);*/
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgingTimeoutGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *timeoutPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgingTimeoutGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null timeoutPtr.
    Expected: GT_OK.
    1.2. Call with null timeoutPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS    st      = GT_OK;

    GT_U8        dev;
    GT_U32       timeout = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null timeoutPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null devTableBmpPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgingTimeoutGet(dev, &timeout);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSecureAgingSet
(
    IN GT_U8          devNum,
    IN GT_BOOL        enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSecureAgingSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbSecureAgingGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbSecureAgingSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbSecureAgingGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbSecureAgingGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAgingGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbSecureAgingSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbSecureAgingGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbSecureAgingGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAgingGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSecureAgingSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSecureAgingSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSecureAgingGet
(
    IN  GT_U8          devNum,
    OUT GT_BOOL        *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSecureAgingGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbSecureAgingGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSecureAgingGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSecureAgingGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSecureAgingGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSecureAutoLearnSet
(
    IN GT_U8                                devNum,
    IN CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSecureAutoLearnSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with mode [CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_TRAP_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_HARD_DROP_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
    Expected: GT_OK and the same modePtr.
    1.3. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                           st      = GT_OK;

    GT_U8                               dev;
    CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT mode    = CPSS_IVL_E;
    CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT modeGet = CPSS_IVL_E;



    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_TRAP_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_HARD_DROP_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_FORWARD_E /
                                  CPSS_MAC_SECURE_AUTO_LEARN_UNK_MIRROR_TO_CPU_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_UNK_TRAP_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_UNK_TRAP_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another mode than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_UNK_SOFT_DROP_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_UNK_HARD_DROP_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_UNK_HARD_DROP_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_UNK_FORWARD_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_UNK_FORWARD_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /*
                1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
                Expected: GT_OK and the same modePtr.
            */
            st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "get another modePtr than was set: %d", dev);
        }
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.1. Call with mode [CPSS_MAC_SECURE_AUTO_LEARN_UNK_MIRROR_TO_CPU_E] */
        mode = CPSS_MAC_SECURE_AUTO_LEARN_UNK_MIRROR_TO_CPU_E;

        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /*
                1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
                Expected: GT_OK and the same modePtr.
            */
            st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &modeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbSecureAutoLearnGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                           "get another modePtr than was set: %d", dev);
        }
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);


        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbSecureAutoLearnSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSecureAutoLearnSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSecureAutoLearnGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSecureAutoLearnGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st   = GT_OK;

    GT_U8                               dev;
    CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT mode = CPSS_MAC_SECURE_AUTO_LEARN_DISABLED_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSecureAutoLearnGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticTransEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticTransEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbStaticTransEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbStaticTransEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbStaticTransEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbStaticTransEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticTransEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbStaticTransEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbStaticTransEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbStaticTransEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticTransEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticTransEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticTransEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticTransEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticTransEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbStaticTransEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbStaticTransEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticTransEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticTransEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticDelEnable
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticDelEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with FDB deletion mode.
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbStaticDelEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT     mode    = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT     modeGet = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with delition mode CPSS_DXCH_BRG_FDB_DEL_MODE_ENT.
            Expected: GT_OK.
        */

        /* Call with enable [CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E] */
        mode = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;

        st = cpssDxChBrgFdbStaticDelEnable(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbStaticDelEnableGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbStaticDelEnableGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticDelEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E] */
        mode = CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E;

        st = cpssDxChBrgFdbStaticDelEnable(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.3. Call cpssDxChBrgFdbStaticDelEnableGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbStaticDelEnableGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticDelEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [CPSS_DXCH_BRG_FDB_DEL_MODE_STATIC_ONLY_E] */
        mode = CPSS_DXCH_BRG_FDB_DEL_MODE_STATIC_ONLY_E;

        st = cpssDxChBrgFdbStaticDelEnable(dev, mode);
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mode);

        /*
            1.4. Call cpssDxChBrgFdbStaticDelEnableGet
            Expected: GT_OK and the same modePtr.
        */
         st = cpssDxChBrgFdbStaticDelEnableGet(dev, &modeGet);
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChBrgFdbStaticDelEnableGet: %d", dev);
         /*Skip comparing deletion mode values if device is not eArch*/
         if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
         {
             UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                        "get another enablePtr than was set: %d", dev);
         }
    }

    mode = CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticDelEnable(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticDelEnable(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticDelEnableGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_BRG_FDB_DEL_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticDelEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_BRG_FDB_DEL_MODE_ENT     mode = CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbStaticDelEnableGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbStaticDelEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticDelEnableGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticDelEnableGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionsEnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionsEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbActionsEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbActionsEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbActionsEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionsEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbActionsEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbActionsEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbActionsEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionsEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionsEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionsEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionsEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionsEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionsEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionsEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionsEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbTrigActionStatusGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *actFinishedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbTrigActionStatusGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null actFinishedPtr.
    Expected: GT_OK.
    1.2. Call with null actFinishedPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null actFinishedPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbTrigActionStatusGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null actFinishedPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbTrigActionStatusGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbTrigActionStatusGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbTrigActionStatusGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacTriggerModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ACTION_MODE_ENT     mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacTriggerModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_ACT_TRIG_E/
                                  CPSS_ACT_AUTO_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMacTriggerModeGet
    Expected: GT_OK and the same modePtr.
    1.3. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                st      = GT_OK;


    GT_U8                    dev;
    CPSS_MAC_ACTION_MODE_ENT mode    = CPSS_ACT_TRIG_E;
    CPSS_MAC_ACTION_MODE_ENT modeGet = CPSS_ACT_TRIG_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_ACT_TRIG_E/
                                          CPSS_ACT_AUTO_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_ACT_TRIG_E] */
        mode = CPSS_ACT_TRIG_E;

        st = cpssDxChBrgFdbMacTriggerModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbMacTriggerModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbMacTriggerModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_ACT_AUTO_E] */
        mode = CPSS_ACT_AUTO_E;

        st = cpssDxChBrgFdbMacTriggerModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbMacTriggerModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbMacTriggerModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbMacTriggerModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbMacTriggerModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_ACT_TRIG_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMacTriggerModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMacTriggerModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacTriggerModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_ACTION_MODE_ENT     *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacTriggerModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                st   = GT_OK;

    GT_U8                    dev;
    CPSS_MAC_ACTION_MODE_ENT mode = CPSS_ACT_AUTO_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbMacTriggerModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbMacTriggerModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMacTriggerModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMacTriggerModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticOfNonExistDevRemove
(
    IN GT_U8                       devNum,
    IN GT_BOOL                     deleteStatic
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticOfNonExistDevRemove)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with deleteStatic [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbStaticOfNonExistDevRemoveGet
    Expected: GT_OK and the same deleteStaticPtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with deleteStatic [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with deleteStatic [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbStaticOfNonExistDevRemove(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbStaticOfNonExistDevRemoveGet
            Expected: GT_OK and the same deleteStaticPtr.
        */
        st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticOfNonExistDevRemoveGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another deleteStaticPtr than was set: %d", dev);

        /* Call with deleteStatic [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbStaticOfNonExistDevRemove(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbStaticOfNonExistDevRemoveGet
            Expected: GT_OK and the same deleteStaticPtr.
        */
        st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbStaticOfNonExistDevRemoveGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another deleteStaticPtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticOfNonExistDevRemove(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticOfNonExistDevRemove(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbStaticOfNonExistDevRemoveGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *deleteStaticPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbStaticOfNonExistDevRemoveGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null deleteStaticPtr.
    Expected: GT_OK.
    1.2. Call with null deleteStaticPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null deleteStaticPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null deleteStaticPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_E_ARCH_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbStaticOfNonExistDevRemoveGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDropAuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDropAuEnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbDropAuEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbDropAuEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbDropAuEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbDropAuEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbDropAuEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbDropAuEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbDropAuEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbDropAuEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbDropAuEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbDropAuEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDropAuEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDropAuEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDropAuEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbDropAuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbDropAuEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbDropAuEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDropAuEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  ageOutAllDevOnTrunkEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with ageOutAllDevOnTrunkEnable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet
    Expected: GT_OK and the same ageOutAllDevOnTrunkEnablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ageOutAllDevOnTrunkEnable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with ageOutAllDevOnTrunkEnable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet
            Expected: GT_OK and the same ageOutAllDevOnTrunkEnablePtr.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another ageOutAllDevOnTrunkEnablePtr than was set: %d", dev);

        /* Call with ageOutAllDevOnTrunkEnable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet
            Expected: GT_OK and the same ageOutAllDevOnTrunkEnablePtr.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another ageOutAllDevOnTrunkEnablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *ageOutAllDevOnTrunkEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null ageOutAllDevOnTrunkEnablePtr.
    Expected: GT_OK.
    1.2. Call with null ageOutAllDevOnTrunkEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null ageOutAllDevOnTrunkEnablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null ageOutAllDevOnTrunkEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  ageOutAllDevOnNonTrunkEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with ageOutAllDevOnNonTrunkEnable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet
    Expected: GT_OK and the same ageOutAllDevOnNonTrunkEnablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with ageOutAllDevOnNonTrunkEnable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with ageOutAllDevOnNonTrunkEnable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet
            Expected: GT_OK and the same ageOutAllDevOnNonTrunkEnablePtr.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another ageOutAllDevOnNonTrunkEnablePtr than was set: %d", dev);

        /* Call with ageOutAllDevOnNonTrunkEnable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet
            Expected: GT_OK and the same ageOutAllDevOnNonTrunkEnablePtr.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another ageOutAllDevOnNonTrunkEnablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *ageOutAllDevOnNonTrunkEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null ageOutAllDevOnNonTrunkEnablePtr.
    Expected: GT_OK.
    1.2. Call with null ageOutAllDevOnNonTrunkEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null ageOutAllDevOnNonTrunkEnablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null ageOutAllDevOnNonTrunkEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionTransplantDataSet
(
    IN GT_U8                    devNum,
    IN CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    IN CPSS_INTERFACE_INFO_STC  *newInterfacePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionTransplantDataSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [100]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                         trunkId [2],
                                         vidx [0],
                                         vlanId [0]}.
    Expected: GT_OK.
    1.2. Call function oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [100]},
                       newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [100]}.
    Expected: GT_OK.
    1.3. Call cpssDxChBrgFdbActionTransplantDataGet.
    Expected: GT_OK and the same oldInterfacePtr and newInterfacePtr.
    1.4. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_VID_E /
                                             CPSS_INTERFACE_VIDX_E] (not supported),
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: NOT GT_OK.
    1.5. Call function oldInterfacePtr {type[CPSS_INTERFACE_VID_E /
                                             CPSS_INTERFACE_VIDX_E] (not supported),
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: NOT GT_OK.
    1.6. Call function oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        out of range trunkId [MAX_TRUNK_ID_CNS = 128],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: NOT GT_OK.
    1.7. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [MAX_TRUNK_ID_CNS = 128],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: GT_OK.
    1.8. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        out of range devPort{
                                            out of range devNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                                                             portNum [0]},
                                        trunkId [1],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: NOT GT_OK.
    1.9. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        out of range devPort {devNum [2],
                                        out of range  portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS]},
                                        trunkId [1], vidx [0], vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
    Expected: NOT GT_OK.
    1.10. Call function with oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                              out of range devPort {devNum [2],
                                              out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]},
                                              trunkId [1],
                                              vidx [0],
                                              vlanId [0]},
                             newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                              devPort {devNum [devNum],
                                                       portNum [0]},
                                              trunkId [0],
                                              vidx [0],
                                              vlanId [0]}.
    Expected: GT_OK.
    1.11. Call function with null oldInterfacePtr [NULL]
                             and valid newInterfacePtr
    Expected: GT_BAD_PTR.
    1.12. Call function with valid non-null oldInterfacePtr
                             and null newInterfacePtr[NULL].
    Expected: GT_BAD_PTR.
    1.13. Call function with out of range oldInterfacePtr->type
                             and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.14. Call function with out of range newInterfacePtr->type
                             and other params from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;


    CPSS_INTERFACE_INFO_STC oldInterface;
    CPSS_INTERFACE_INFO_STC newInterface;
    CPSS_INTERFACE_INFO_STC oldIfGet;
    CPSS_INTERFACE_INFO_STC newIfGet;

    cpssOsBzero((GT_VOID*) &oldInterface, sizeof(oldInterface));
    cpssOsBzero((GT_VOID*) &newInterface, sizeof(newInterface));
    cpssOsBzero((GT_VOID*) &oldIfGet, sizeof(oldIfGet));
    cpssOsBzero((GT_VOID*) &newIfGet, sizeof(newIfGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                                devPort {devNum [devNum],
                                                         portNum [0]},
                                                trunkId [0],
                                                vidx [0],
                                                vlanId [100]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                         trunkId [2],
                                         vidx [0],
                                         vlanId [0]}.
            Expected: GT_OK.
        */
        oldInterface.type            = CPSS_INTERFACE_PORT_E;
        oldInterface.devPort.hwDevNum  = dev;
        oldInterface.devPort.portNum = 0;
        oldInterface.trunkId         = 0;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);
        oldInterface.vidx            = 0;
        oldInterface.vlanId          = 100;

        newInterface.type            = CPSS_INTERFACE_TRUNK_E;
        newInterface.devPort.hwDevNum  = dev;
        newInterface.devPort.portNum = 0;
        newInterface.trunkId         = 2;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(newInterface.trunkId);
        newInterface.vidx            = 0;
        newInterface.vlanId          = 0;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                                 trunkId [0],
                                                 vidx [0],
                                                 vlanId [100]},
                                newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                                 devPort {devNum [devNum],
                                                          portNum [0]},
                                                 trunkId [0],
                                                 vidx [0],
                                                 vlanId [100]}.
            Expected: GT_OK.
        */
        oldInterface.type            = CPSS_INTERFACE_TRUNK_E;
        oldInterface.devPort.hwDevNum  = dev;
        oldInterface.devPort.portNum = 0;
        oldInterface.trunkId         = 0;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);
        oldInterface.vidx            = 0;
        oldInterface.vlanId          = 100;

        newInterface.type            = CPSS_INTERFACE_PORT_E;
        newInterface.devPort.hwDevNum  = dev;
        newInterface.devPort.portNum = 0;
        newInterface.trunkId         = 0;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(newInterface.trunkId);
        newInterface.vidx            = 0;
        newInterface.vlanId          = 100;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.3. Call cpssDxChBrgFdbActionTransplantDataGet.
            Expected: GT_OK and the same oldInterfacePtr and newInterfacePtr.
        */
        st = cpssDxChBrgFdbActionTransplantDataGet(dev, &oldIfGet, &newIfGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionTransplantDataGet: %d", dev);

        /* Verifying oldInterfacePtr */
        UTF_VERIFY_EQUAL1_STRING_MAC(oldInterface.type, oldIfGet.type,
                       "get another oldInterfacePtr->type than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(oldInterface.trunkId, oldIfGet.trunkId,
                       "get another oldInterfacePtr->trunkId than was set: %d", dev);

        /* Verifying newInterfacePtr */
        UTF_VERIFY_EQUAL1_STRING_MAC(newInterface.type, newIfGet.type,
                       "get another newInterfacePtr->type than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(newInterface.devPort.portNum, newIfGet.devPort.portNum,
                       "get another newInterfacePtr->devPort.portNum than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(newInterface.devPort.hwDevNum, newIfGet.devPort.hwDevNum,
                       "get another newInterfacePtr->devPort.hwDevNum than was set: %d", dev);

        /*
            1.4. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_VID_E /
                                             CPSS_INTERFACE_VIDX_E] (not supported),
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: NOT GT_OK.
        */

        /* Call with newInterfacePtr->type[CPSS_INTERFACE_VID_E] */
        oldInterface.type = CPSS_INTERFACE_PORT_E;
        newInterface.type = CPSS_INTERFACE_VID_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, newInterfacePtr->type = %d", dev, newInterface.type);

        /* Call with newInterfacePtr->type[CPSS_INTERFACE_VIDX_E] */
        newInterface.type = CPSS_INTERFACE_VIDX_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, newInterfacePtr->type = %d", dev, newInterface.type);

        /*
            1.5. Call function oldInterfacePtr {type[CPSS_INTERFACE_VID_E /
                                             CPSS_INTERFACE_VIDX_E] (not supported),
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: NOT GT_OK.
        */

        /* Call with oldInterfacePtr->type[CPSS_INTERFACE_VID_E] */
        oldInterface.type = CPSS_INTERFACE_VID_E;
        newInterface.type = CPSS_INTERFACE_TRUNK_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, oldInterfacePtr->type = %d", dev, oldInterface.type);

        /* Call with oldInterfacePtr->type[CPSS_INTERFACE_VIDX_E] */
        oldInterface.type = CPSS_INTERFACE_VIDX_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                            "%d, oldInterfacePtr->type = %d", dev, oldInterface.type);

        /*
            1.6. Call function oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        out of range trunkId [MAX_TRUNK_ID_CNS = 128],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: NOT GT_OK.
        */
        oldInterface.type    = CPSS_INTERFACE_TRUNK_E;
        oldInterface.trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);

        newInterface.type    = CPSS_INTERFACE_PORT_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, oldInterfacePtr->type = %d, oldInterfacePtr->trunkId = %d",
                                        dev, oldInterface.type, oldInterface.trunkId);

        /*
            1.7. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [MAX_TRUNK_ID_CNS = 128],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: GT_OK.
        */
        oldInterface.type    = CPSS_INTERFACE_PORT_E;
        oldInterface.trunkId = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, oldInterfacePtr->type = %d, oldInterfacePtr->trunkId = %d",
                                    dev, oldInterface.type, oldInterface.trunkId);

        /*
            1.8. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                      out of range devPort{out of range devNum[PRV_CPSS_MAX_PP_DEVICES_CNS],
                                                             portNum [0]},
                                        trunkId [1],
                                        vidx [0],
                                        vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        devPort {devNum [devNum],
                                                 portNum [0]},
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: NOT GT_OK.
        */
        oldInterface.type           = CPSS_INTERFACE_PORT_E;
        oldInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);
        oldInterface.trunkId        = 1;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);

        newInterface.type = CPSS_INTERFACE_TRUNK_E;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, oldInterfacePtr->type = %d, oldInterfacePtr->devPort->hwDevNum = %d",
                                     dev, oldInterface.type, oldInterface.devPort.hwDevNum);

        /*
            1.9. Call function oldInterfacePtr {type[CPSS_INTERFACE_PORT_E],
                                        out of range devPort {devNum [2],
                                        out of range  portNum[UTF_CPSS_PP_MAX_PORTS_NUM_CNS]},
                                        trunkId [1], vidx [0], vlanId [0]},
                       newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                        trunkId [0],
                                        vidx [0],
                                        vlanId [0]}.
            Expected: NOT GT_OK.
        */
        oldInterface.type            = CPSS_INTERFACE_PORT_E;
        oldInterface.devPort.hwDevNum  = dev;
        oldInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, oldInterfacePtr->type = %d, oldInterfacePtr->devPort->portNum = %d",
                                     dev, oldInterface.type, oldInterface.devPort.portNum);

        /*
            1.10. Call function with oldInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                              out of range devPort {devNum [2],
                                               out of range portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]},
                                              trunkId [1],
                                              vidx [0],
                                              vlanId [0]},
                             newInterfacePtr {type[CPSS_INTERFACE_TRUNK_E],
                                              devPort {devNum [devNum],
                                                       portNum [0]},
                                              trunkId [0],
                                              vidx [0],
                                              vlanId [0]}.
            Expected: GT_OK.
        */
        oldInterface.type            = CPSS_INTERFACE_TRUNK_E;
        oldInterface.devPort.portNum = UTF_CPSS_PP_MAX_PORT_AS_DATA_NUM_CNS(dev);

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, oldInterfacePtr->type = %d, oldInterfacePtr->devPort->portNum = %d",
                                     dev, oldInterface.type, oldInterface.devPort.portNum);

        /*
            1.11. Call function with null oldInterfacePtr [NULL]
                             and valid newInterfacePtr
            Expected: GT_BAD_PTR.
        */
        oldInterface.devPort.portNum = BRG_FDB_VALID_PHY_PORT_CNS;

        st = cpssDxChBrgFdbActionTransplantDataSet(dev, NULL, &newInterface);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, oldInterfacePtr = NULL", dev);

        /*
            1.12. Call function with valid non-null oldInterfacePtr
                             and null newInterfacePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, newInterfacePtr = NULL", dev);

        /*
            1.13. Call function with wrong enum values oldInterfacePtr->type
                             and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbActionTransplantDataSet
                                                        (dev, &oldInterface, &newInterface),
                                                        oldInterface.type);

        /*
            1.14. Call function with wrong enum values newInterfacePtr->type
                             and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbActionTransplantDataSet
                                                        (dev, &oldInterface, &newInterface),
                                                        newInterface.type);
    }

    oldInterface.type            = CPSS_INTERFACE_PORT_E;
    oldInterface.devPort.hwDevNum  = dev;
    oldInterface.devPort.portNum = 0;
    oldInterface.trunkId         = 0;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(oldInterface.trunkId);
    oldInterface.vidx            = 0;
    oldInterface.vlanId          = 100;

    newInterface.type            = CPSS_INTERFACE_TRUNK_E;
    newInterface.devPort.hwDevNum  = dev;
    newInterface.devPort.portNum = 0;
    newInterface.trunkId         = 2;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(newInterface.trunkId);
    newInterface.vidx            = 0;
    newInterface.vlanId          = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionTransplantDataSet(dev, &oldInterface, &newInterface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionTransplantDataGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    OUT CPSS_INTERFACE_INFO_STC  *newInterfacePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionTransplantDataGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null oldInterfacePtr and non-null newInterfacePtr.
    Expected: GT_OK.
    1.2. Call with null oldInterfacePtr [NULL] and non-null newInterfacePtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null oldInterfacePtr and null newInterfacePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_INTERFACE_INFO_STC oldInterface;
    CPSS_INTERFACE_INFO_STC newInterface;

    cpssOsBzero((GT_VOID*) &oldInterface, sizeof(oldInterface));
    cpssOsBzero((GT_VOID*) &newInterface, sizeof(newInterface));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null oldInterfacePtr and non-null newInterfacePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionTransplantDataGet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null oldInterfacePtr [NULL] and non-null newInterfacePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionTransplantDataGet(dev, NULL, &newInterface);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, oldInterfacePtr = NULL", dev);

        /*
            1.3. Call with non-null oldInterfacePtr and null newInterfacePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionTransplantDataGet(dev, &oldInterface, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, newInterfacePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionTransplantDataGet(dev, &oldInterface, &newInterface);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionTransplantDataGet(dev, &oldInterface, &newInterface);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbFromCpuAuMsgStatusGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *completedPtr,
    OUT GT_BOOL  *succeededPtr
    INOUT   GT_U32  *entryOffsetPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbFromCpuAuMsgStatusGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null completedPtr, non-null succeededPtr and
        non-null entryOffsetPtr.
    Expected: GT_OK.
    1.2. Call with null completedPtr [NULL] and non-null succeededPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null completedPtr and null succeededPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with non-null completedPtr, non-null succeededPtr and
        null entryOffsetPtr[NULL].
    Expected: GT_OK.
*/

    GT_U8       dev         = 0;
    GT_BOOL     completed = GT_FALSE;
    GT_BOOL     succeeded = GT_FALSE;
    GT_U32      entryOffset = 0;
    GT_STATUS   st        = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null completedPtr, non-null succeededPtr and
                non-null entryOffsetPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, &succeeded, &entryOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null completedPtr [NULL] and non-null succeededPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, NULL, &succeeded, &entryOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, completedPtr = NULL", dev);

        /*
            1.3. Call with non-null completedPtr and null succeededPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, NULL, &entryOffset);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, completedPtr = NULL", dev);
        /*
            1.4. Call with non-null completedPtr, non-null succeededPtr and
                null entryOffsetPtr[NULL].
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, &succeeded, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, &succeeded, &entryOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, &completed, &succeeded,  &entryOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveVlanSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   vlanMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveVlanSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with vlanId [1] and vlanMask [2].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveVlanGet
    Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
    1.3. Call function with vlanId [0] and vlanMask [0].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveVlanGet
    Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
    1.4. Call function with vlanId [0x1FFF] and vlanMask [0x1FFF].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveVlanGet
    Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
    1.5. Call function with out of range vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                            and vlanMask [1].
    Expected: GT_OUT_OF_RANGE.
    1.6. Call function with vlanId [2]
                            and out of range vlanMask [PRV_CPSS_MAX_NUM_VLANS_CNS].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st          = GT_OK;

    GT_U8       dev;
    GT_U16      vlanId      = 0;
    GT_U16      vlanMask    = 0;
    GT_U16      vlanIdGet   = 0;
    GT_U16      vlanMaskGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with vlanId [1]
                                    and vlanMask [2].
            Expected: GT_OK.
        */
        vlanId   = 1;
        vlanMask = 2;

        st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, vlanMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveVlanGet
            Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
        */
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanIdGet, &vlanMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionActiveVlanGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                       "get another vlanIdPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanMask, vlanMaskGet,
                       "get another vlanMaskPtr than was set: %d", dev);

        /*
            1.3. Call function with vlanId [100]
                                    and vlanMask [0].
            Expected: GT_OK.
        */
        vlanId   = 100;
        vlanMask = 0;

        st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, vlanMask);


        /*
            1.2. Call cpssDxChBrgFdbActionActiveVlanGet
            Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
        */
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanIdGet, &vlanMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionActiveVlanGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                       "get another vlanIdPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(vlanMask, vlanMaskGet,
                       "get another vlanMaskPtr than was set: %d", dev);


        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /*
                1.4. Call function with vlanId [0x1FFF]
                                        and vlanMask [0x1FFF].
                Expected: GT_OK.
            */
            vlanId   = 0x1FFF;
            vlanMask = 0x1FFF;

            st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, vlanMask);


            /*
                1.2. Call cpssDxChBrgFdbActionActiveVlanGet
                Expected: GT_OK and the same vlanIdPtr and vlanMaskPtr.
            */
            st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanIdGet, &vlanMaskGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbActionActiveVlanGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanId, vlanIdGet,
                           "get another vlanIdPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(vlanMask, vlanMaskGet,
                           "get another vlanMaskPtr than was set: %d", dev);
        }


        /*
            1.5. Call function with out of range vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                                    and vlanMask [1].
            Expected: GT_OUT_OF_RANGE.
        */
        vlanId   = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        vlanMask = 1;

        st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, vlanId);

        /*
            1.6. Call function with vlanId [2]
                                    and out of range vlanMask [PRV_CPSS_MAX_NUM_VLANS_CNS].
            Expected: GT_OUT_OF_RANGE.
        */
        vlanId   = 2;
        vlanMask = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OUT_OF_RANGE, st, "%d, vlanMask = %d", dev ,vlanMask);
    }

    vlanId   = 1;
    vlanMask = 2;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveVlanSet(dev, vlanId, vlanMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveVlanGet
(
    IN  GT_U8    devNum,
    OUT GT_U16   *vlanIdPtr,
    OUT GT_U16   *vlanMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveVlanGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null vlanIdPtr
                   and non-null vlanMaskPtr.
    Expected: GT_OK.
    1.2. Call with null vlanIdPtr [NULL]
                   and non-null vlanMaskPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null vlanIdPtr
                   and null vlanMaskPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_U16      vlanId   = 0;
    GT_U16      vlanMask = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null vlanIdPtr
                   and non-null vlanMaskPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanId, &vlanMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null vlanIdPtr [NULL]
                   and non-null vlanMaskPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, NULL, &vlanMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanIdPtr = NULL", dev);

        /*
            1.3. Call with non-null vlanIdPtr
                           and null vlanMaskPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, vlanMaskPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanId, &vlanMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveVlanGet(dev, &vlanId, &vlanMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveDevSet
(
    IN GT_U8    devNum,
    IN GT_U32   actDev,
    IN GT_U32   actDevMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveDevSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with actDev [0, 31]
                            and actDevMask [1, 31].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveDevGet
    Expected: GT_OK and the same actDevPtr and actDevMaskPtr.
    1.3. Call function with out of range actDev [32]
                            and actDevMask [1].
    Expected: NOT GT_OK.
    1.4. Call function with actDev [2] and out of range actDevMask [32].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st            = GT_OK;

    GT_U8       dev;
    GT_U32      actDev        = 0;
    GT_U32      actDevMask    = 0;
    GT_U32      actDevGet     = 0;
    GT_U32      actDevMaskGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with actDev [0, 31]
                            and actDevMask [1, 31].
            Expected: GT_OK.
        */

        /* Call with actDev [0] and actDevMask [1] */
        actDev     = 0;
        actDevMask = 1;
        if(sysGenGlobalInfo.supportDualDeviceId)
        {
            actDev     *= 2;
            actDevMask *= 2;

            /* LSBit of pattern must be 0 when mask of this bit is 1*/
            actDev     &= (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev) - 1);
            actDevMask &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev);
        }
        st = cpssDxChBrgFdbActionActiveDevSet(dev, actDev, actDevMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, actDev, actDevMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveDevGet
            Expected: GT_OK and the same actDevPtr and actDevMaskPtr.
        */
        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDevGet, &actDevMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionActiveDevGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actDev, actDevGet,
                       "get another actDevPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actDevMask, actDevMaskGet,
                       "get another actDevMaskPtr than was set: %d", dev);

        /* Call with actDev [31] and actDevMask [31] */
        actDev     = 31;
        actDevMask = 31;

        if(sysGenGlobalInfo.supportDualDeviceId)
        {
            actDev     *= 2;
            actDevMask *= 2;

            /* LSBit of pattern must be 0 when mask of this bit is 1*/
            actDev     &= (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev) - 1);
            actDevMask &= PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev);
        }

        st = cpssDxChBrgFdbActionActiveDevSet(dev, actDev, actDevMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, actDev, actDevMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveDevGet
            Expected: GT_OK and the same actDevPtr and actDevMaskPtr.
        */
        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDevGet, &actDevMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionActiveDevGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actDev, actDevGet,
                       "get another actDevPtr than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actDevMask, actDevMaskGet,
                       "get another actDevMaskPtr than was set: %d", dev);

        /*
            1.3. Call function with out of range actDev [32]
                                    and actDevMask [1].
            Expected: NOT GT_OK.
        */
        actDev     = 32;
        actDevMask = 1;

        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDevGet, &actDevMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, actDev);

        /*
            1.4. Call function with actDev [2] and out of range actDevMask [32].
            Expected: NOT GT_OK.
        */
        actDev     = 2;
        actDevMask = 32;

        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDevGet, &actDevMaskGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, actDevMask = %d", dev, actDevMask);
    }

    actDev     = 0;
    actDevMask = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveDevSet(dev, actDev, actDevMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveDevSet(dev, actDev, actDevMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveDevGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actDevPtr,
    OUT GT_U32   *actDevMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveDevGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null actDevPtr
                   and non-null actDevMaskPtr.
    Expected: GT_OK.
    1.2. Call with null actDevPtr [NULL]
                   and non-null actDevMaskPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null actDevPtr
                   and null actDevMaskPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st         = GT_OK;

    GT_U8       dev;
    GT_U32      actDev     = 0;
    GT_U32      actDevMask = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null actDevPtr and non-null actDevMaskPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDev, &actDevMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null actDevPtr [NULL] and non-null actDevMaskPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveDevGet(dev, NULL, &actDevMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actDevPtr = NULL", dev);

        /*
            1.3. Call with non-null actDevPtr and null actDevMaskPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actDevMaskPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDev, &actDevMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveDevGet(dev, &actDev, &actDevMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveInterfaceSet
(
    IN GT_U8   devNum,
    IN GT_U32  actIsTrunk,
    IN GT_U32  actIsTrunkMask,
    IN GT_U32  actTrunkPort,
    IN GT_U32  actTrunkPortMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveInterfaceSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with actIsTrunk [1],
                   actIsTrunkMask [1],
                   actTrunkPort[1],
                   actTrunkPortMask[0x7F]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveInterfaceGet
    Expected: GT_OK and the same actIsTrunkPtr,
                                 actIsTrunkMaskPtr,
                                 actTrunkPortPtr
                                 and actTrunkPortMaskPtr.
    1.3. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   actTrunkPort[1],
                   actTrunkPortMask[0x7F]
    Expected: GT_OK.
    1.4. Call cpssDxChBrgFdbActionActiveInterfaceGet
    Expected: GT_OK and the same actIsTrunkPtr,
                                 actTrunkPortPtr
                                 and actTrunkPortMaskPtr.
    1.5. Call with actIsTrunk [1],
                   actIsTrunkMask [1],
                   out of range actTrunkPort[MAX_TRUNK_ID_CNS =128],
                   actTrunkPortMask[0x7F]
    Expected: NOT GT_OK.
    1.6. Call with actIsTrunk [1],
                   actIsTrunkMask [1],
                   actTrunkPort[1],
                   out of range actTrunkPortMask[0x8F]
    Expected: NOT GT_OK.
    1.7. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   out of range port ID actTrunkPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                   actTrunkPortMask [0x7F]
    Expected: NOT GT_OK.
    1.8. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   actTrunkPort[1],
                   out of range actTrunkPortMask[0x8F]
    Expected: NOT GT_OK.
    1.9. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   actTrunkPort [0],
                   actTrunkPortMask[0]
    Expected: GT_OK.
*/
    GT_STATUS   st                  = GT_OK;

    GT_U8       dev;
    GT_U32      actIsTrunk          = 0;
    GT_U32      actIsTrunkMask      = 0;
    GT_U32      actTrunkPort        = 0;
    GT_U32      actTrunkPortMask    = 0;
    GT_U32      actIsTrunkGet       = 0;
    GT_U32      actIsTrunkMaskGet   = 0;
    GT_U32      actTrunkPortGet     = 0;
    GT_U32      actTrunkPortMaskGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with actIsTrunk [1],
                           actIsTrunkMask [1],
                           actTrunkPort[1],
                           actTrunkPortMask[0x7F]
            Expected: GT_OK.
        */
        actIsTrunk       = 1;
        actIsTrunkMask   = 1;
        actTrunkPort     = 1;
        actTrunkPortMask = 0x7F;

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveInterfaceGet
            Expected: GT_OK and the same actIsTrunkPtr,
                                         actIsTrunkMaskPtr,
                                         actTrunkPortPtr
                                         and actTrunkPortMaskPtr.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunkGet, &actIsTrunkMaskGet,
                                                    &actTrunkPortGet, &actTrunkPortMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionActiveInterfaceGet: %d", dev);

        /* Verifying parameters */
        UTF_VERIFY_EQUAL1_STRING_MAC(actIsTrunk, actIsTrunkGet,
                       "get another actIsTrunk than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actIsTrunkMask, actIsTrunkMaskGet,
                       "get another actIsTrunkMask than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actTrunkPort, actTrunkPortGet,
                       "get another actTrunkPort than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(actTrunkPortMask, actTrunkPortMaskGet,
                       "get another actTrunkPortMask than was set: %d", dev);

        /*
            1.3. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   actTrunkPort[1],
                   actTrunkPortMask[0x7F]
            Expected: GT_OK.
        */
        actIsTrunk       = 0;
        actIsTrunkMask   = 0;
        actTrunkPort     = 1;
        actTrunkPortMask = 0x7F;

        if(sysGenGlobalInfo.supportDualDeviceId == GT_TRUE)
        {
            /* when 'isTunk' is don't care that mask of the trunk/port must be
               limited to 6 bits , because trunk can come from 2 hemispheres */
            actTrunkPortMask &= 0x3F;
        }

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);

#if 0
        /* NEXT code not needed since the &= 0x3F from the 5 lines above */
        if(sysGenGlobalInfo.supportDualDeviceId)
        {
            /* the mask 0x7F is not supported for dual devices systems
               when actIsTrunkMask is don't care */
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_STATE, st, dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        }
        else
#endif /*0*/
        {
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
            /*
                1.4. Call cpssDxChBrgFdbActionActiveInterfaceGet
                Expected: GT_OK and the same actIsTrunkPtr,
                                             actTrunkPortPtr
                                             and actTrunkPortMaskPtr.
            */
            st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunkGet, &actIsTrunkMaskGet,
                                                        &actTrunkPortGet, &actTrunkPortMaskGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                           "cpssDxChBrgFdbActionActiveInterfaceGet: %d", dev);

            /* Verifying parameters */
            UTF_VERIFY_EQUAL1_STRING_MAC(actIsTrunk, actIsTrunkGet,
                           "get another actIsTrunkPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(actIsTrunkMask, actIsTrunkMaskGet,
                           "get another actIsTrunkMaskPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(actTrunkPort, actTrunkPortGet,
                           "get another actTrunkPortPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(actTrunkPortMask, actTrunkPortMaskGet,
                           "get another actTrunkPortMaskPtr than was set: %d", dev);
        }

        /*
            1.5. Call with out of range actTrunkPort[MAX_TRUNK_ID_CNS =128],
                           actIsTrunkMask [1],
                           actIsTrunk [1],
                           actTrunkPortMask[0x7F]
            Expected: NOT GT_OK.
        */
        actTrunkPort       = MAX_PORT_TRUNK_MAC(dev);
        actIsTrunkMask      = 1;
        actIsTrunk          = 1;
        actTrunkPortMask    = 0x7F;

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actTrunkPort = %d", dev, actIsTrunk);

        /*
            1.6. Call with actIsTrunk [1],
                            actIsTrunkMask [0x1],
                           actTrunkPort[1],
                           out of range actTrunkPortMask[0x8F]
            Expected: NOT GT_OK.
        */
        actIsTrunk       = 1;
        actIsTrunkMask   = 1;
        actTrunkPort     = 1;
        actTrunkPortMask = MAX_PORT_TRUNK_MAC(dev);

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actTrunkPortMask = %d", dev, actIsTrunkMask);

        /*
            1.7. Call with actIsTrunk [0],
                   actIsTrunkMask [0],
                   out of range port ID actTrunkPort [UTF_CPSS_PP_MAX_PORTS_NUM_CNS],
                   actTrunkPortMask [0x7F]
            Expected: NOT GT_OK.
        */
        actIsTrunk       = 0;
        actIsTrunkMask   = 0;
        actTrunkPort     = MAX_PORT_TRUNK_MAC(dev);
        actTrunkPortMask = 0x7F;

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actTrunkPort = %d", dev, actTrunkPort);

        /*
            1.8. Call with actIsTrunk [0],
                           actIsTrunkMask [0],
                           actTrunkPort[1],
                           out of range actTrunkPortMask[0x8F]
            Expected: NOT GT_OK.
        */
        actIsTrunk       = 0;
        actIsTrunkMask   = 0;
        actTrunkPort     = 1;
        actTrunkPortMask = MAX_PORT_TRUNK_MAC(dev);

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, actTrunkPortMask = %d", dev, actTrunkPortMask);

        /*
            1.9. Call with actIsTrunk [0],
                           actIsTrunkMask [0],
                           actTrunkPort [0],
                           actTrunkPortMask[0]
            Expected: GT_OK.
        */
        actIsTrunk       = 0;
        actIsTrunkMask   = 0;
        actTrunkPort     = 0;
        actTrunkPortMask = 0;

        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
    }

    actIsTrunk       = 1;
    actIsTrunkMask   = 1;
    actTrunkPort     = 1;
    actTrunkPortMask = 0x7F;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveInterfaceSet(dev, actIsTrunk, actIsTrunkMask,
                                                    actTrunkPort, actTrunkPortMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveInterfaceGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *actIsTrunkPtr,
    OUT GT_U32  *actIsTrunkMaskPtr,
    OUT GT_U32  *actTrunkPortPtr,
    OUT GT_U32  *actTrunkPortMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveInterfaceGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null actIsTrunkPtr,
                   non-null actIsTrunkMaskPtr,
                   non-null actTrunkPortPtr,
                   non-null actTrunkPortMaskPtr.
    Expected: GT_OK.
    1.2. Call with null actIsTrunkPtr [NULL],
                   non-null actIsTrunkMaskPtr,
                   non-null actTrunkPortPtr,
                   non-null actTrunkPortMaskPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null actIsTrunkPtr,
                   null actIsTrunkMaskPtr [NULL],
                   non-null actTrunkPortPtr,
                   non-null actTrunkPortMaskPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with non-null actIsTrunkPtr,
                   non-null actIsTrunkMaskPtr,
                   null actTrunkPortPtr [NULL],
                   non-null actTrunkPortMaskPtr.
    Expected: GT_BAD_PTR.
    1.5. Call with non-null actIsTrunkPtr,
                   non-null actIsTrunkMaskPtr,
                   non-null actTrunkPortPtr,
                   null actTrunkPortMaskPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st               = GT_OK;

    GT_U8       dev;
    GT_U32      actIsTrunk       = 0;
    GT_U32      actIsTrunkMask   = 0;
    GT_U32      actTrunkPort     = 0;
    GT_U32      actTrunkPortMask = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null actIsTrunkPtr,
                           non-null actIsTrunkMaskPtr,
                           non-null actTrunkPortPtr,
                           non-null actTrunkPortMaskPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, &actIsTrunkMask,
                                                    &actTrunkPort, &actTrunkPortMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null actIsTrunkPtr [NULL],
                           non-null actIsTrunkMaskPtr,
                           non-null actTrunkPortPtr,
                           non-null actTrunkPortMaskPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, NULL, &actIsTrunkMask,
                                                    &actTrunkPort, &actTrunkPortMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actIsTrunkPtr = NULL", dev);

        /*
            1.3. Call with non-null actIsTrunkPtr,
                           null actIsTrunkMaskPtr [NULL],
                           non-null actTrunkPortPtr,
                           non-null actTrunkPortMaskPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, NULL,
                                                    &actTrunkPort, &actTrunkPortMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actIsTrunkMaskPtr = NULL", dev);

        /*
            1.4. Call with non-null actIsTrunkPtr,
                           non-null actIsTrunkMaskPtr,
                           null actTrunkPortPtr [NULL],
                           non-null actTrunkPortMaskPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, &actIsTrunkMask,
                                                    NULL, &actTrunkPortMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actTrunkPortPtr = NULL", dev);

        /*
            1.5. Call with non-null actIsTrunkPtr,
                           non-null actIsTrunkMaskPtr,
                           non-null actTrunkPortPtr,
                           null actTrunkPortMaskPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, &actIsTrunkMask,
                                                    &actTrunkPort, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actTrunkPortMaskPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, &actIsTrunkMask,
                                                    &actTrunkPort, &actTrunkPortMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveInterfaceGet(dev, &actIsTrunk, &actIsTrunkMask,
                                                    &actTrunkPort, &actTrunkPortMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbUploadEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbUploadEnableSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionsEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbUploadEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbUploadEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbUploadEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbUploadEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbUploadEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbUploadEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbUploadEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbUploadEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbUploadEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbUploadEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbUploadEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbUploadEnableGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbUploadEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbUploadEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbUploadEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbUploadEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbTrigActionStart
(
    IN GT_U8                        devNum,
    IN CPSS_FDB_ACTION_MODE_ENT     mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbTrigActionStart)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E /
                                  CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E].
    Expected: GT_OK.
    1.2. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;


    CPSS_FDB_ACTION_MODE_ENT mode        = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;
    GT_BOOL                  isCompleted = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isCompleted = GT_FALSE;

        /*
            1.1. Call function with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E /
                                          CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E] */
        mode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

        st = cpssDxChBrgFdbTrigActionStart(dev, mode);
        switch(PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode)
        {
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E:
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, mode);
                break;
            default:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

                /* wait that triggered action is completed */
                while (isCompleted != GT_TRUE)
                {
                    if(GT_TRUE == prvUtfReduceLogSizeFlagGet())
                    {
                        /* minimize log by use sleep to reduce number of pooling */
                        cpssOsTimerWkAfter(20);
                    }

                    st = cpssDxChBrgFdbTrigActionStatusGet(dev, &isCompleted);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChBrgFdbTrigActionStatusGet: %d", dev);
                }

                break;
        }

        /* 1.1. Call with mode [CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E] */
        mode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;

        st = cpssDxChBrgFdbTrigActionStart(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /* wait that triggered action is completed */
        isCompleted = GT_FALSE;

        while (isCompleted != GT_TRUE)
        {
            if(GT_TRUE == prvUtfReduceLogSizeFlagGet())
            {
                /* minimize log by use sleep to reduce number of pooling */
                cpssOsTimerWkAfter(20);
            }

            st = cpssDxChBrgFdbTrigActionStatusGet(dev, &isCompleted);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbTrigActionStatusGet: %d", dev);
        }

        /*
            1.2. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbTrigActionStart
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbTrigActionStart(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbTrigActionStart(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_FDB_ACTION_MODE_ENT     mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E /
                                  CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E /
                                  CPSS_FDB_ACTION_DELETING_E /
                                  CPSS_FDB_ACTION_TRANSPLANTING_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionModeGet
    Expected: GT_OK and the same modePtr.
    1.3. Call function with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                st      = GT_OK;


    GT_U8                    dev;
    CPSS_FDB_ACTION_MODE_ENT mode    = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;
    CPSS_FDB_ACTION_MODE_ENT modeGet = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E /
                                          CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E /
                                          CPSS_FDB_ACTION_DELETING_E /
                                          CPSS_FDB_ACTION_TRANSPLANTING_E].
            Expected: GT_OK.
        */

        /* 1.1. Call with mode [CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E] */
        mode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

        st = cpssDxChBrgFdbActionModeSet(dev, mode);
        switch(PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode)
        {
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E:
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, mode);
                break;
            default:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

                /*
                    1.2. Call cpssDxChBrgFdbActionModeGet
                    Expected: GT_OK and the same modePtr.
                */
                st = cpssDxChBrgFdbActionModeGet(dev, &modeGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                     "cpssDxChBrgFdbActionModeGet: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                     "get another modePtr than was set: %d", dev);
                break;
        }

        /* 1.1. Call with mode [CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E] */
        mode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;

        st = cpssDxChBrgFdbActionModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbActionModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbActionModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_FDB_ACTION_DELETING_E] */
        mode = CPSS_FDB_ACTION_DELETING_E;

        st = cpssDxChBrgFdbActionModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbActionModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbActionModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbActionModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with mode [CPSS_FDB_ACTION_TRANSPLANTING_E] */
        mode = CPSS_FDB_ACTION_TRANSPLANTING_E;

        st = cpssDxChBrgFdbActionModeSet(dev, mode);
        switch(PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode)
        {
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E:
            case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_LINKED_E:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, mode);
                break;
            default:
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);
        /*
            1.2. Call cpssDxChBrgFdbActionModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbActionModeGet(dev, &modeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
             "cpssDxChBrgFdbActionModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeGet,
            "get another modePtr than was set: %d", dev);
                break;
        }

        /*
            1.3. Call function with wrong enum values mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbActionModeSet
                                                        (dev, mode),
                                                        mode);
    }

    mode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_FDB_ACTION_MODE_ENT     *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                st   = GT_OK;

    GT_U8                    dev;
    CPSS_FDB_ACTION_MODE_ENT mode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null modePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacTriggerToggle
(
    IN GT_U8  devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacTriggerToggle)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChBrgFdbMacTriggerToggle(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* wait for the devices to finish */
    prvTgfBrgFdbActionDoneWaitForAllDev(GT_TRUE);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbMacTriggerToggle(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMacTriggerToggle(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfAuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAuMsgBlockGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null pointers, numOfAuPtr [2].
    Expected: GT_OK.
    1.2. Call with null numOfAuPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with null auMessagesPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_U32                          numOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC     auMessages[2];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&auMessages, sizeof(auMessages));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers, numOfAuPtr [2].
            Expected: GT_OK.
        */
        numOfAu = 2;

        st = cpssDxChBrgFdbAuMsgBlockGet(dev, &numOfAu, auMessages);
        st = (GT_NO_MORE == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numOfAu);

        /*
            1.2. Call with null numOfAuPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuMsgBlockGet(dev, NULL, auMessages);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfAuPtr = NULL", dev);

        /*
            1.3. Call with null auMessagesPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuMsgBlockGet(dev, &numOfAu, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, auMessagesPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAuMsgBlockGet(dev, &numOfAu, auMessages);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAuMsgBlockGet(dev, &numOfAu, auMessages);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbFuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfFuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *fuMessagesPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbFuMsgBlockGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-null pointers, numOfAuPtr [2].
    Expected: GT_OK.
    1.2. Call with null numOfAuPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with null auMessagesPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8                           dev;
    GT_U32                          numOfFu;
    CPSS_MAC_UPDATE_MSG_EXT_STC     fuMessages[2];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    cpssOsBzero((GT_VOID*)&fuMessages, sizeof(fuMessages));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers, numOfFuPtr [2].
            Expected: GT_OK.
        */
        numOfFu = 2;

        st = cpssDxChBrgFdbFuMsgBlockGet(dev, &numOfFu, fuMessages);
        st = (GT_NO_MORE == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, numOfFu);

        /*
            1.2. Call with null numOfFuPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbFuMsgBlockGet(dev, NULL, fuMessages);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, numOfFuPtr = NULL", dev);

        /*
            1.3. Call with null fuMessagesPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbFuMsgBlockGet(dev, &numOfFu, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, fuMessagesPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbFuMsgBlockGet(dev, &numOfFu, fuMessages);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbFuMsgBlockGet(dev, &numOfFu, fuMessages);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbTrunkAgingModeSet
(
    IN GT_U8                         devNum,
    IN CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT  trunkAgingMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbTrunkAgingModeSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with mode [CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E/
                                  CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E].
    Expected: GT_OK.
    1.2. Call function cpssDxChBrgFdbTrunkAgingModeGet with non NULL trunkAgingModePtr.
    Expected: GT_OK and trunkAgingMode the same as was set.
    1.3. Call function with out of range trunkAgingMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st    = GT_OK;


    GT_U8                       dev;
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT trunkAgingMode;
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT trunkAgingModeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode [CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E/
                                          CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E].
            Expected: GT_OK.
        */

        /* 1.1. for trunkAgingMode == CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E */
        trunkAgingMode = CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E;

        st = cpssDxChBrgFdbTrunkAgingModeSet(dev, trunkAgingMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkAgingMode);

        /*
            1.2. Call function cpssDxChBrgFdbTrunkAgingModeGet with non NULL trunkAgingModePtr.
            Expected: GT_OK and trunkAgingMode the same as was set.
        */
        /* 1.2. for trunkAgingMode == CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E */
        st = cpssDxChBrgFdbTrunkAgingModeGet(dev, &trunkAgingModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbTrunkAgingModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkAgingMode, trunkAgingModeGet,
                       "get another trunkAgingMode than was set: %d", dev);

        /* 1.1. for trunkAgingMode == CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E */
        trunkAgingMode = CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E;

        st = cpssDxChBrgFdbTrunkAgingModeSet(dev, trunkAgingMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, trunkAgingMode);

        /*
            1.2. Call function cpssDxChBrgFdbTrunkAgingModeGet with non NULL trunkAgingModePtr.
            Expected: GT_OK and trunkAgingMode the same as was set.
        */
        /* 1.2. for trunkAgingMode == CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E */
        st = cpssDxChBrgFdbTrunkAgingModeGet(dev, &trunkAgingModeGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbTrunkAgingModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(trunkAgingMode, trunkAgingModeGet,
                       "get another trunkAgingMode than was set: %d", dev);

        /*
            1.3. Call with wrong enum values trunkAgingMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbTrunkAgingModeSet
                                                        (dev, trunkAgingMode),
                                                        trunkAgingMode);
    }

    trunkAgingMode = CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbTrunkAgingModeSet(dev, trunkAgingMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbTrunkAgingModeSet(dev, trunkAgingMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbTrunkAgingModeGet
(
    IN GT_U8                        devNum,
    OUT CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT *trunkAgingModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbTrunkAgingModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with non NULL trunkAgingModePtr.
    Expected: GT_OK.
    1.2. Call function with trunkAgingModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st        = GT_OK;

    GT_U8                       dev;
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT trunkAgingModeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non NULL trunkAgingModePtr.
        Expected: GT_OK. */

        st = cpssDxChBrgFdbTrunkAgingModeGet(dev, &trunkAgingModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call function with trunkAgingModePtr [NULL].
        Expected: GT_BAD_PTR. */

        st = cpssDxChBrgFdbTrunkAgingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbTrunkAgingModeGet(dev, &trunkAgingModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbTrunkAgingModeGet(dev, &trunkAgingModeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeBitDaRefreshEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeBitDaRefreshEnableSet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionsEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeBitDaRefreshEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeBitDaRefreshEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbAgeBitDaRefreshEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbAgeBitDaRefreshEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeBitDaRefreshEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAgeBitDaRefreshEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAgeBitDaRefreshEnableGet)
{
/*
    ITERATE_DEVICES (xCat and above)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAgeBitDaRefreshEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbAuqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbAuqFuqMessagesNumberGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E] and non-null pointers.
    Expected: GT_OK.
    1.2. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_FU_E] and  non-null pointers.
    Expected: GT_OK for cheetah2 and above and not GT_OK for cheetah.
    1.3. Call with wrong queueType and non-null pointers.
    Expected: GT_BAD_PARAM.
    1.4. Call with null numOfAuPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call with null endOfQueueReachedPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;


    GT_U8       dev;

    CPSS_DXCH_FDB_QUEUE_TYPE_ENT queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;
    GT_U32                       numOfAuPtr;
    GT_BOOL                      endOfQueueReachedPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E] and  non-null pointers.
            Expected: GT_OK.
        */
        queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;

        st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType, &numOfAuPtr, &endOfQueueReachedPtr);
        if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(dev) == 0) ||
           (PRV_CPSS_DXCH_PP_MAC(dev)->errata.info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.enabled == GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
        }
        else
        {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_FU_E] and  non-null pointers.
            Expected: GT_OK for cheetah2 and above and not GT_OK for cheetah.
        */
        queueType = CPSS_DXCH_FDB_QUEUE_TYPE_FU_E;

        st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType, &numOfAuPtr, &endOfQueueReachedPtr);

        if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(dev) == 0) ||
           (PRV_CPSS_DXCH_PP_MAC(dev)->errata.info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.enabled == GT_TRUE))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_INITIALIZED, st, dev);
        }
        else
        {
        if(PRV_CPSS_PP_MAC(dev)->devFamily > CPSS_PP_FAMILY_CHEETAH_E)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        }

        /*
            1.3. Call with wrong enum values queueType and non-null pointers.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbAuqFuqMessagesNumberGet
                                                        (dev, queueType, &numOfAuPtr, &endOfQueueReachedPtr),
                                                        queueType);

        /*
            1.4. Call with null numOfAuPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType, NULL, &endOfQueueReachedPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.5. Call with null endOfQueueReachedPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType, &numOfAuPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType,
                                            &numOfAuPtr, &endOfQueueReachedPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbAuqFuqMessagesNumberGet(dev, queueType,
                                            &numOfAuPtr, &endOfQueueReachedPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutedLearningEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutedLearningEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionsEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* Call with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChBrgFdbRoutedLearningEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbRoutedLearningEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbRoutedLearningEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChBrgFdbRoutedLearningEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChBrgFdbRoutedLearningEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbRoutedLearningEnableGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                       "get another enablePtr than was set: %d", dev);
    }

    state = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutedLearningEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutedLearningEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbRoutedLearningEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbRoutedLearningEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbRoutedLearningEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbInit
(
    IN  GT_U8   devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st    = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct dev.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbInit(dev);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_BOOL      age
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryAgeBitSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with index [0] and age [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call with out of range index.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32      index = 0;
    GT_BOOL     age = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0] and age [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        index = 0;
        age = GT_FALSE;

        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        age = GT_TRUE;

        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        index = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb - 1;
        age = GT_FALSE;

        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        age = GT_TRUE;

        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range index.
            Expected: GT_OUT_OF_RANGE.
        */
        index = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        index = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryAgeBitSet(dev, index, age);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntryStatusGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with index [0] and non-null pointers.
    Expected: GT_OK.
    1.2. Call with out of range index.
    Expected: GT_OUT_OF_RANGE.
    1.3. Call with wrong validPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong skipPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32      index = 0;
    GT_BOOL     valid;
    GT_BOOL     skip;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0] and non-null pointers.
            Expected: GT_OK.
        */
        index = 0;

        st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, &valid, &skip);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range index.
            Expected: GT_OUT_OF_RANGE.
        */
        index = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

        st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, &valid, &skip);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        index = 0;

        /*
            1.3. Call with wrong validPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, NULL, &skip);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong skipPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, &valid, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, &valid, &skip);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = utfCpssDxChBrgFdbMacEntryStatusGet(dev, index, &valid, &skip);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbQueueFullGet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_BOOL *isFullPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbQueueFullGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E /
                              CPSS_DXCH_FDB_QUEUE_TYPE_FU_E]
                              and non-null isFullPtr.
    Expected: GT_OK.
    1.2. Call with wrong queueType.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong isFullPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;
    GT_BOOL     isFull;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E /
                                      CPSS_DXCH_FDB_QUEUE_TYPE_FU_E]
                                      and non-null isFullPtr.
            Expected: GT_OK.
        */
        queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;

        st = cpssDxChBrgFdbQueueFullGet(dev, queueType, &isFull);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        queueType = CPSS_DXCH_FDB_QUEUE_TYPE_FU_E;

        st = cpssDxChBrgFdbQueueFullGet(dev, queueType, &isFull);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enum values queueType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbQueueFullGet
                                                        (dev, queueType, &isFull),
                                                        queueType);
        queueType = CPSS_DXCH_FDB_QUEUE_TYPE_FU_E;

        /*
            1.3. Call with wrong isFull [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbQueueFullGet(dev, queueType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbQueueFullGet(dev, queueType, &isFull);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbQueueFullGet(dev, queueType, &isFull);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbQueueRewindStatusGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *rewindPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbQueueRewindStatusGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null rewindPtr.
    Expected: GT_OK.
    1.2. Call with null rewindPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     rewind = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null rewindPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbQueueRewindStatusGet(dev, &rewind);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null rewindPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbQueueRewindStatusGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbQueueRewindStatusGet(dev, &rewind);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbQueueRewindStatusGet(dev, &rewind);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSecondaryAuMsgBlockGet
(
    IN      GT_U8                                    devNum,
    INOUT   GT_U32                                   *numOfAuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC              *auMessagesPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSecondaryAuMsgBlockGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with numOfAuPtr [1] and non-null auMessagesPtr.
    Expected: GT_OK.
    1.2. Call with null numOfAuPtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with null auMessagesPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32                       numOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC  auMessages[1];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&auMessages, sizeof(auMessages));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with numOfAuPtr [1] and non-null auMessagesPtr.
            Expected: GT_OK.
        */
        numOfAu = 1;

        st = cpssDxChBrgFdbSecondaryAuMsgBlockGet(dev, &numOfAu, auMessages);
        if (GT_NO_MORE == st)
        {
            continue;
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with null numOfAuPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSecondaryAuMsgBlockGet(dev, NULL, auMessages);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.3. Call with null auMessagesPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSecondaryAuMsgBlockGet(dev, &numOfAu, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSecondaryAuMsgBlockGet(dev, &numOfAu, auMessages);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSecondaryAuMsgBlockGet(dev, &numOfAu, auMessages);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbTriggerAuQueueWa
(
    IN      GT_U8                                    devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbTriggerAuQueueWa)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct devNum.
    Expected: GT_OK.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct devNum.
            Expected: GT_NOT_SUPPORTED - CPSS does not supports the API
                      Only PSS over CPSS supports it.
        */
        st = cpssDxChBrgFdbTriggerAuQueueWa(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbTriggerAuQueueWa(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbTriggerAuQueueWa(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntrySet
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntrySet)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntrySet */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntrySet);
}
/* run the test of : cpssDxChBrgFdbMacEntrySet in multi-hash-mode
   NOTE: relevant to SIP5 devices only
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMacEntrySet_Multi_hash_mode)
{
    multiHashMode = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntrySet */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntrySet);

    /* restore */
    multiHashMode = GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupQaSend
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupQaSend)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbPortGroupQaSend */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbQaSend);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryDelete
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryDelete)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryDelete */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryDelete);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryWrite)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryWrite */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryWrite);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryStatusGet)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryStatusGet */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryStatusGet);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_U8                   *associatedDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryRead)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryRead */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryRead);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32        index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryInvalidate)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryInvalidate */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryInvalidate);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
(
    IN  GT_U8    devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_PORT_GROUPS_BMP  *completedPortGroupsBmpPtr,
    OUT GT_PORT_GROUPS_BMP  *succeededPortGroupsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1. Call with non-null completedPtr and non-null succeededPtr.
    Expected: GT_OK.
    1.2. Call with null completedPtr [NULL] and non-null succeededPtr.
    Expected: GT_BAD_PTR.
    1.3. Call with non-null completedPtr and null succeededPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_PORT_GROUPS_BMP  completed;
    GT_PORT_GROUPS_BMP  succeeded;
    GT_U32    portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);
            /*
                1.1. Call with non-null completedPtr and non-null succeededPtr.
                Expected: GT_OK.
            */

        if( PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E ==
                PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode )
        {
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev, portGroupsBmp, &completed, &succeeded);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null completedPtr [NULL] and non-null succeededPtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev, portGroupsBmp,  NULL, &succeeded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, completedPtr = NULL", dev);

        /*
            1.3. Call with non-null completedPtr and null succeededPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev, portGroupsBmp, &completed, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, completedPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev,
                    portGroupsBmp, &completed, &succeeded);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev, portGroupsBmp, &completed, &succeeded);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(dev, portGroupsBmp, &completed, &succeeded);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32       index,
    IN GT_BOOL      age
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupMacEntryAgeBitSet)
{
    usePortGroupsBmp = GT_TRUE;
    /* call test of cpssDxChBrgFdbMacEntryAgeBitSet */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbMacEntryAgeBitSet);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChTestFdbHash
(
    IN GT_U8                        devNum
)
*/
static void prvCpssDxChTestFdbHash(IN GT_U8 dev)
{
    CPSS_MAC_ENTRY_EXT_KEY_STC  macEntryKey;
    GT_STATUS                   st = GT_OK;

    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        /* SIP6 not supports the below API */
    }
    else
    if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
    {
        /* disable using of vid1 field of FDB entry */
        (void)cpssDxChBrgFdbVid1AssignmentEnableSet(dev, GT_TRUE);
    }

    /* set the entry type to be MAC Address */
    macEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

#ifdef ASIC_SIMULATION
    cpssOsPrintf("prvCpssDxChTestFdbHash: start MAC hash check \n");
#endif /*ASIC_SIMULATION*/

    utfGeneralStateMessageSave(0,"MAC hash check");

    /* test FDB Hash for MAC Address */
    UT_FDB_DUMP(("prvCpssDxChTestFdbHash: start MAC hash check\n"));
    st = prvUtTestFdbHashCases(dev, &macEntryKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifdef ASIC_SIMULATION
    cpssOsPrintf("prvCpssDxChTestFdbHash: start IPV4_MCAST hash check \n");
#endif /*ASIC_SIMULATION*/

    utfGeneralStateMessageSave(0,"IPV4_MCAST hash check");

    /* set the entry type to be IP Address */
    macEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;

    /* test FDB Hash for IP Address */
    UT_FDB_DUMP(("prvCpssDxChTestFdbHash: start IPV4_MCAST hash check\n"));
    st = prvUtTestFdbHashCases(dev, &macEntryKey);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* SIP6 not supports the below API */
        }
        else
        {
            /* enable using of vid1 field of FDB entry */
            st = cpssDxChBrgFdbVid1AssignmentEnableSet(dev, GT_TRUE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*********** test FDB Hash for MAC Address (double tag key) ***********/
#ifdef ASIC_SIMULATION
        cpssOsPrintf("prvCpssDxChTestFdbHash: start MAC hash check (double tag key) \n");
#endif /*ASIC_SIMULATION*/
        PRV_UTF_LOG0_MAC("prvCpssDxChTestFdbHash: start MAC hash check (double tag key) \n");
        utfGeneralStateMessageSave(0,"MAC_FID_VID1 hash check");

        macEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;

        st = prvUtTestFdbHashCases(dev, &macEntryKey);
        if(st != GT_OK)
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* SIP6 not supports the below API */
            }
            else
            {
                /* disable using of vid1 field of FDB entry */
                (void)cpssDxChBrgFdbVid1AssignmentEnableSet(dev, GT_FALSE);
            }
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
#ifdef ASIC_SIMULATION
            cpssOsPrintf("prvCpssDxChTestFdbHash: IPV4_MCAST in double tag key is not support since no vid1 in the entry \n");
#endif /*ASIC_SIMULATION*/
            PRV_UTF_LOG0_MAC("prvCpssDxChTestFdbHash: IPV4_MCAST in double tag key is not support since no vid1 in the entry \n");
        }
        else
        {
            /********** test FDB Hash for IP MC Address (double tag key) **********/
#ifdef ASIC_SIMULATION
            cpssOsPrintf("prvCpssDxChTestFdbHash: start IPV4_MCAST hash check (double tag key) \n");
#endif /*ASIC_SIMULATION*/
            PRV_UTF_LOG0_MAC("prvCpssDxChTestFdbHash: start IPV4_MCAST hash check (double tag key) \n");
            utfGeneralStateMessageSave(0,"IPV4_MCAST_FID_VID1 hash check");

            macEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;

            st = prvUtTestFdbHashCases(dev, &macEntryKey);
            /* disable using of vid1 field of FDB entry */
            (void)cpssDxChBrgFdbVid1AssignmentEnableSet(dev, GT_FALSE);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
    }

#ifdef ASIC_SIMULATION
    /* indicate end of function */
    cpssOsPrintf("prvCpssDxChTestFdbHash: end \n");
#endif /*ASIC_SIMULATION*/

}



/* FDB flag for reduced factor */
static GT_U32   fdbHashIterationsReducedFactor = 1;

#define LAST_FDB_SIZE_CNS        0xFFFFFFFF
#define NO_MODIFY_FDB_SIZE_CNS   0xEEEEEEEE
static GT_U32  dummyFdbSizesArr[]   = {NO_MODIFY_FDB_SIZE_CNS,LAST_FDB_SIZE_CNS};
static GT_U32  ch3FdbSizesArr[]     = {_16K,_32K,LAST_FDB_SIZE_CNS};
static GT_U32  lionFdbSizesArr[]    = {_16K,_32K,LAST_FDB_SIZE_CNS};
static GT_U32  xcatFdbSizesArr[]    = {_8K,_16K,LAST_FDB_SIZE_CNS};
static GT_U32  lion2FdbSizesArr[]   = {_16K,_32K,_64K,LAST_FDB_SIZE_CNS};
static GT_U32  bobcat2FdbSizesArr[] = {_32K,_64K,_128K ,LAST_FDB_SIZE_CNS};
static GT_U32  caelumFdbSizesArr[]  = {_32K,_64K,LAST_FDB_SIZE_CNS};
static GT_U32  cetusFdbSizesArr[]   = {_32K,LAST_FDB_SIZE_CNS};
static GT_U32  bobcat3FdbSizesArr[] = {_8K,_16K,_32K,_64K,_128K,_256K,LAST_FDB_SIZE_CNS};
static GT_U32  aldrin2FdbSizesArr[] = {_8K,_16K,_32K,_64K,_128K,LAST_FDB_SIZE_CNS};

/* check if need to test this FDB size */
static GT_BOOL  isNeedToTestFdbSize
(
    IN GT_U8    dev,
    IN GT_U32   fdbSize
)
{
    GT_U32   *fdbSizePtr;
    GT_U32      ii;
    GT_BOOL isSupported = GT_FALSE;
    GT_U32  numSupportedSizes = 0;

    switch(PRV_CPSS_PP_MAC(dev)->devFamily)
    {
        default : /*ch1,2*/
            fdbSizePtr = dummyFdbSizesArr;
            break;
        case CPSS_PP_FAMILY_CHEETAH3_E:
            fdbSizePtr = ch3FdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            fdbSizePtr = xcatFdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_LION_E:
            fdbSizePtr = lionFdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            fdbSizePtr = lion2FdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(PRV_CPSS_PP_MAC(dev)->devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                fdbSizePtr = bobcat2FdbSizesArr;
            }
            else
            {
                switch(PRV_CPSS_PP_MAC(dev)->devType)
                {
                    case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                        fdbSizePtr = cetusFdbSizesArr;
                        break;
                    default:
                        fdbSizePtr = caelumFdbSizesArr;
                        break;
                }
            }
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E :
            fdbSizePtr = bobcat3FdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E :
            fdbSizePtr = aldrin2FdbSizesArr;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E :
            /* currently Falcon as BC3 */
            CPSS_TBD_BOOKMARK_FALCON
            fdbSizePtr = bobcat3FdbSizesArr;
            break;
    }

    for(ii = 0 ; fdbSizePtr[ii] != LAST_FDB_SIZE_CNS ;ii++)
    {
        if(fdbSize != NO_MODIFY_FDB_SIZE_CNS)
        {
            numSupportedSizes++;
        }

        if(fdbSize == fdbSizePtr[ii])
        {
            isSupported =  GT_TRUE;
        }
    }

    if(isSupported == GT_FALSE &&/* support for ch1,2 */
       fdbSize == PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb)
    {
        fdbHashIterationsReducedFactor = 1;
        /* current FDB size need to be tested */
        return GT_TRUE;
    }

    fdbHashIterationsReducedFactor = numSupportedSizes;

    return isSupported;
}

/* check if the specific FDB size needed to be tested for the system .
if yes --> test it
if no  -> skip it
*/
static void testFdbHash(IN GT_U32    fdbSize)
{
    GT_U8       dev;
    GT_STATUS   st;
    GT_U32   origFdb;

    GT_BOOL wasTested = GT_FALSE;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(GT_FALSE == isNeedToTestFdbSize(dev,fdbSize))
        {
            continue;
        }

        /* save the the FDB size */
        origFdb = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            /* update the FDB size */
            st = prvCpssDxChBrgFdbSizeSet(dev,fdbSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        utfGeneralStateMessageSave(10,"fdb size [%d]",fdbSize);
        osPrintf("testFdbHash: start FDB size [%d] check \n",fdbSize);
        /* run the test under the specific FDB size */
        prvCpssDxChTestFdbHash(dev);

        if(PRV_CPSS_PP_MAC(dev)->devFamily >= CPSS_PP_FAMILY_CHEETAH3_E)
        {
            /* restore fdb size */
            st = prvCpssDxChBrgFdbSizeSet(dev,origFdb);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        wasTested = GT_TRUE;
    }

    if(wasTested == GT_FALSE)
    {
        /* state that test was skipped */
        SKIP_TEST_MAC;
    }
}

UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_8K  ){testFdbHash(  _8K);}
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_16K ){testFdbHash( _16K);}
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_32K ){testFdbHash( _32K);}
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_64K ){testFdbHash( _64K);}
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_128K){testFdbHash(_128K);}
UTF_TEST_CASE_MAC(prvCpssDxChTestFdbHash_256K){testFdbHash(_256K);}
/* function that can be called from the terminal to test all modes one after the other */
void prvCpssDxChTestFdbHash_runAll(void)
{
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_8K   );
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_16K  );
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_32K  );
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_64K  );
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_128K );
    UTF_TEST_CALL_MAC(prvCpssDxChTestFdbHash_256K );
}

/*----------------------------------------------------------------------------*/
/*
    Test function for filling FDB table.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFillFdbTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_FDB_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in FDB table.
         Call cpssDxChBrgFdbMacEntryWrite with index [0..numEntries-1],
                                               skip [GT_FALSE]
                                               and valid macEntryPtr.
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgFdbMacEntryWrite with index [numEntries],
                                               skip [GT_FALSE]
                                               and valid macEntryPtr.
    Expected: NOT GT_OK.
    1.4. Read all entries in FDB table and compare with original.
         Call cpssDxChBrgFdbMacEntryRead with index [0..numEntries-1],
         and non-NULL validPtr, skipPtr, agedPtr, associatedDevNumPtr, entryPtr.
    Expected: GT_OK and the same entries as was written.
    1.5. Try to read entry with index out of range.
         Call cpssDxChBrgFdbMacEntryRead with index [numEntries],
         and non-NULL validPtr, skipPtr, agedPtr, associatedDevNumPtr, entryPtr.
    Expected: NOT GT_OK.
    1.6. Delete all entries in FDB table.
         Call cpssDxChBrgFdbMacEntryInvalidate with index [0..numEntries-1],
    Expected: GT_OK.
    1.7. Try to delete entry with index out of range.
         Call cpssDxChBrgFdbMacEntryInvalidate with index [numEntries],
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL                 isEqual    = GT_FALSE;
    GT_U32                  numEntries = 0;
    GT_U32                  iTemp      = 0;
    GT_BOOL                 skip       = GT_FALSE;
    CPSS_MAC_ENTRY_EXT_STC  macEntry;

    GT_BOOL                 validGet  = GT_FALSE;
    GT_BOOL                 skipGet   = GT_FALSE;
    GT_BOOL                 agedGet   = GT_FALSE;
    GT_HW_DEV_NUM           hwDevNumGet = 0;
    CPSS_MAC_ENTRY_EXT_STC  entryGet;
    GT_U32                  step = 1;
    /*defined as 'variable' to allow fine tuning changes during debug. */
    GT_U32   gmFillMaxIterations = _1K;
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT sip6FdbMacEntryMuxingMode = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E;
    static GT_U32  sip6udbMask[CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E] = {
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E -                                            */
     /* 12 bits of 'TAG_1_VID'                                                                       */
     0,
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E -                                               */
     /* 12 bits of 'SRC_ID'                                                                          */
     0,
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E -                      */
     /* 10 bits of UDB (bits 1..10) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'  */
     0x3FF,
     /* 1 bit of 'SRC_ID' (bit 0)                                                                    */
     /* 1 bit of 'DA_ACCESS_LEVEL'                                                                   */
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_4_AND_4_SRC_ID_3_DA_ACCESS_LEVEL_E -                 */
     /* 4 bits of UDB (bits 1..4) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'    */
     /* and 4 bits of UDB (bits 7..10)                                                               */
     /* 3 bits of 'SRC_ID' (bit 0..2)                                                                */
     /* 1 bit of 'DA_ACCESS_LEVEL'                                                                   */
     0x3FF - (BIT_5 | BIT_4)
    };
    GT_U32  used_sip6udbMask = 0;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));
    cpssOsBzero((GT_VOID*) &entryGet, sizeof(entryGet));

    /* Fill the entry for FDB table */
    skip  = GT_FALSE;
    macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId = 100;

    macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

    macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
    macEntry.dstInterface.trunkId         = 10;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
    macEntry.dstInterface.vidx            = 0;
    macEntry.dstInterface.vlanId          = 0;

    macEntry.isStatic                     = GT_FALSE;
    macEntry.daCommand                    = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                    = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
    macEntry.daRoute                      = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
    macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
    macEntry.sourceID                     = 0;
    macEntry.userDefined                  = 0;
    macEntry.daQosIndex                   = 0;
    macEntry.saQosIndex                   = 0;
    macEntry.daSecurityLevel              = 0;
    macEntry.saSecurityLevel              = 0;
    macEntry.appSpecificCpuCode           = GT_FALSE;
    macEntry.age                          = GT_TRUE;
    macEntry.spUnknown                    = GT_FALSE;

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            st = cpssDxChBrgFdbMacEntryMuxingModeGet(dev,
                &sip6FdbMacEntryMuxingMode);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgFdbMacEntryMuxingModeGet: %d", dev);

            if(sip6FdbMacEntryMuxingMode < CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E )
            {
                used_sip6udbMask = sip6udbMask[sip6FdbMacEntryMuxingMode];
            }

            /* allow next code to succeed with allowed command */
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_FDB_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in FDB table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* make every entry unique */
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                macEntry.dstInterface.trunkId = (GT_TRUNK_ID)((iTemp+1) & 0xFFF);
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
                macEntry.userDefined          = (iTemp+1) & used_sip6udbMask;
            }
            else
            {
                macEntry.dstInterface.trunkId = (GT_TRUNK_ID)(1 + iTemp % 127);
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
                macEntry.userDefined          = iTemp % 0xF;
                macEntry.daQosIndex           = iTemp % 7;
                macEntry.saQosIndex           = iTemp % 7;
                macEntry.daSecurityLevel      = iTemp % 7;
                macEntry.saSecurityLevel      = iTemp % 7;
            }

            if(macEntry.dstInterface.trunkId == 0)
            {
                /* trunkId must not be 0 */
                macEntry.dstInterface.trunkId = 1;
            }

            st = utfCpssDxChBrgFdbMacEntryWrite(dev, iTemp, skip, &macEntry);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryWrite: %d, %d, %d", dev, iTemp, skip);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = utfCpssDxChBrgFdbMacEntryWrite(dev, numEntries, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbMacEntryWrite: %d, %d, %d", dev, numEntries, skip);

        /* 1.4. Read all entries in FDB table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            /* restore unique entry before compare */
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                macEntry.dstInterface.trunkId = (GT_TRUNK_ID)((iTemp+1) & 0xFFF);
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
                macEntry.userDefined          = (iTemp+1) & used_sip6udbMask;
            }
            else
            {
                macEntry.dstInterface.trunkId = (GT_TRUNK_ID)(1 + iTemp % 127);
                CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
                macEntry.userDefined          = iTemp % 0xF;
                macEntry.daQosIndex           = iTemp % 7;
                macEntry.saQosIndex           = iTemp % 7;
                macEntry.daSecurityLevel      = iTemp % 7;
                macEntry.saSecurityLevel      = iTemp % 7;
            }

            if(macEntry.dstInterface.trunkId == 0)
            {
                /* trunkId must not be 0 */
                macEntry.dstInterface.trunkId = 1;
            }

            st = utfCpssDxChBrgFdbMacEntryRead(dev, iTemp, &validGet, &skipGet, &agedGet, &hwDevNumGet, &entryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, iTemp);

            /* Verifying struct fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(skip, skipGet,
                       "get another skipPtr than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.entryType, entryGet.key.entryType,
                       "get another macEntryPtr->key.entryType than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.key.macVlan.vlanId,
                                         entryGet.key.key.macVlan.vlanId,
                       "get another macEntryPtr->key.key.macVlan.vlanId than was set: %d", dev);

            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &macEntry.key.key.macVlan.macAddr,
                                   (GT_VOID*) &entryGet.key.key.macVlan.macAddr,
                                    sizeof(macEntry.key.key.macVlan.macAddr))) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                       "get another macEntryPtr->key.key.macVlan.macAddr than was set: %d", dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, entryGet.dstInterface.type,
                       "get another macEntryPtr->dstInterface.type than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.trunkId,
                                         entryGet.dstInterface.trunkId,
                       "get another macEntryPtr->dstInterface.trunkId than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.isStatic, entryGet.isStatic,
                       "get another macEntryPtr->isStatic than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daCommand, entryGet.daCommand,
                       "get another macEntryPtr->daCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saCommand, entryGet.saCommand,
                       "get another macEntryPtr->saCommand than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daRoute, entryGet.daRoute,
                       "get another macEntryPtr->daRoute than was set: %d", dev);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                         entryGet.saMirrorToRxAnalyzerPortEn,
                       "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                         entryGet.daMirrorToRxAnalyzerPortEn,
                       "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d", dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.mirrorToRxAnalyzerPortEn,
                                         entryGet.mirrorToRxAnalyzerPortEn,
                       "get another macEntryPtr->mirrorToRxAnalyzerPortEn than was set: %d", dev);
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, entryGet.sourceID,
                       "get another macEntryPtr->sourceID than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined, entryGet.userDefined,
                       "get another macEntryPtr->userDefined than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daQosIndex, entryGet.daQosIndex,
                       "get another macEntryPtr->daQosIndex than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saQosIndex, entryGet.saQosIndex,
                       "get another macEntryPtr->saQosIndex than was set: %d", dev);

            if(PRV_CPSS_PP_MAC(dev)->devFamily > CPSS_PP_FAMILY_CHEETAH_E)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel, entryGet.daSecurityLevel,
                           "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel, entryGet.saSecurityLevel,
                           "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.appSpecificCpuCode, entryGet.appSpecificCpuCode,
                           "get another macEntryPtr->appSpecificCpuCode than was set: %d", dev);
            }
        }

        /* 1.5. Try to read entry with index out of range. */
        st = utfCpssDxChBrgFdbMacEntryRead(dev, numEntries, &validGet, &skipGet, &agedGet, &hwDevNumGet, &entryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, numEntries);

        /* 1.6. Delete all entries in FDB table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, iTemp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, iTemp);
        }

        /* 1.7. Try to delete entry with index out of range. */
        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, numEntries);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashByParamsCalc
(
    IN  CPSS_MAC_HASH_FUNC_MODE_ENT        hashMode,
    IN  CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC *hashParamsPtr,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC        *macEntryKeyPtr,
    OUT GT_U32                            *hashPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashByParamsCalc)
{
/*
    1.1. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                 macVlan->vlanId[100],
                                                 macVlan->macAddr[00:1A:FF:FF:FF:FF]}
    and enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2  Same as 1.1 except wrong enum values for macEntryKeyPtr->entryType
    Expected: GT_BAD_PARAM
    1.3  Same as 1.1 except wrong enum values for hashMode
    Expected: GT_BAD_PARAM
    1.4. Call function with null macEntryKeyPtr [NULL].
    Expected: GT_BAD_PTR.
    1.5. Call function with null hashParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.6. Call function with null hashPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                         st = GT_OK;
    GT_U32                            hash;
    CPSS_MAC_ENTRY_EXT_KEY_STC        macEntry;
    CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC hashParams;
    CPSS_MAC_HASH_FUNC_MODE_ENT       hashMode;

    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));
    cpssOsBzero((GT_VOID*) &hashParams, sizeof(hashParams));

    /*
        1.1. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
        macVlan->vlanId[100], macVlan->macAddr[00:1A:FF:FF:FF:FF]} and enable [GT_FALSE / GT_TRUE].
        Expected: GT_OK.
    */

    macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.macVlan.vlanId = 100;

    macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

    hashParams.fid16BitHashEn = GT_FALSE;
    hashParams.size = CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E;
    hashMode = CPSS_MAC_HASH_FUNC_CRC_E;

    st = cpssDxChBrgFdbHashByParamsCalc(hashMode, &hashParams, &macEntry, &hash);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, hash);

    /*
        1.2. Call function with wrong enum values for macEntryKeyPtr >entryType
        Expected: GT_BAD_PARAM.
    */

    UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashByParamsCalc
                                                      (hashMode, &hashParams, &macEntry, &hash),
                                                      macEntry.entryType);

    /*
        1.3. Call function with wrong enum values for hashMode
        Expected: GT_BAD_PARAM.
    */

    UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashByParamsCalc
                                                      (hashMode, &hashParams, &macEntry, &hash),
                                                      hashMode);

    /*
        1.4. Call function with null macEntryKeyPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    st = cpssDxChBrgFdbHashByParamsCalc(hashMode, &hashParams, NULL, &hash);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "macEntryPtr %p, NULL", &macEntry);

    /*
        1.5. Call function with null hashParamsPtr [NULL].
        Expected: GT_BAD_PTR.
    */

    st = cpssDxChBrgFdbHashByParamsCalc(hashMode, NULL, &macEntry, &hash);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "hashParamsPtr %p, NULL", &hashParams);

    /*
       1.6. Call function with null hashPtr [NULL].
       Expected: GT_BAD_PTR.
    */
    st = cpssDxChBrgFdbHashByParamsCalc(hashMode, &hashParams, &macEntry, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "hashPtr %p, NULL", &hash);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashCalc
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC *macEntryKeyPtr
    OUT GT_U32                     *hashPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashCalc)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                                 macVlan->vlanId[100],
                                                 macVlan->macAddr[00:1A:FF:FF:FF:FF]}
    Expected: GT_OK.
    1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.3. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                                 ipMcast->vlanId[100],
                                                 ipMcast->sip[123.23.34.5],
                                                 ipMacst [123.45.67.8]}.
    Expected: GT_OK.
    1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.5. Call function with valid macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E],
                                                 ipMcast->vlanId[100],
                                                 ipMcast->sip[123.23.34.5],
                                                 ipMacst [123.45.67.8]}.
    Expected: GT_OK.
    1.6. Same as 1.5 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.7. Call function with null macEntryKeyPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call function with wrong enum values macEntryKeyPtr >entryType
                            and other parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call function with out of range
         macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                         entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
    Expected: GT_BAD_PARAM.
    1.10. Call function with out of range
         macEntryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                            entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: GT_BAD_PARAM.
    1.11. Call function with null hashPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                  st = GT_OK;

    GT_U8                      dev;
    CPSS_MAC_ENTRY_EXT_KEY_STC macEntry;
    GT_U32                     hash;
    GT_STATUS                  expected;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with valid
                 macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                          macVlan->vlanId[100],
                                          macVlan->macAddr[00:1A:FF:FF:FF:FF]}
            Expected: GT_OK.
        */
        macEntry.vid1               = 123; /* relevant for eArch devices */
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.macVlan.vlanId = 100;

        macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
          1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
          Expected: GT_OK        - for eArch except bobcat a0.
                    GT_BAD_PARAM - for other devices
        */

        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
        if (PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.hashMode ==
            CPSS_MAC_HASH_FUNC_XOR_E)
        {
          expected = GT_BAD_PARAM;
        }
        else
        {
        expected = (PRV_CPSS_SIP_5_10_CHECK_MAC(dev)) ?
            GT_OK : GT_BAD_PARAM;
        }
        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /*
            1.3. Call function with valid
                 macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                                ipMcast->vlanId[100],
                                ipMcast->sip[123.23.34.5],
                                ipMacst->dip[123.45.67.8]}.
            Expected: GT_OK.
        */
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.ipMcast.vlanId = 100;

        macEntry.key.ipMcast.sip[0] = 123;
        macEntry.key.ipMcast.sip[1] = 23;
        macEntry.key.ipMcast.sip[2] = 34;
        macEntry.key.ipMcast.sip[3] = 5;

        macEntry.key.ipMcast.dip[0] = 123;
        macEntry.key.ipMcast.dip[1] = 45;
        macEntry.key.ipMcast.dip[2] = 67;
        macEntry.key.ipMcast.dip[3] = 8;

        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
                      GT_BAD_PARAM - for other devices
        */
        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
        if (PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.hashMode ==
            CPSS_MAC_HASH_FUNC_CRC_E)
        {
            expected = GT_NOT_SUPPORTED;
        }
        else
        {
            expected = GT_BAD_PARAM;
        }
        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /*
            1.5. Call function with valid
                 macEntryKeyPtr{entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E],
                                ipMcast->vlanId[100],
                                ipMcast->sip[123.23.34.5],
                                ipMacst->dip[123.45.67.8]}.
            Expected: GT_OK.
        */
        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;

        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.6. Same as 1.5 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
                      GT_BAD_PARAM - for other devices
        */
        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E;
        if (PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.hashMode ==
            CPSS_MAC_HASH_FUNC_CRC_E)
        {
          expected = GT_NOT_SUPPORTED;
        }
        else
        {
        expected = GT_BAD_PARAM;
        }
        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /*
            1.7. Call function with null macEntryKeyPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashCalc(dev, NULL, &hash);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);

        /*
            1.8. Call function with wrong enum values macEntryKeyPtr->entryType
                                    and other parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashCalc
                                                        (dev, &macEntry, &hash),
                                                        macEntry.entryType);

        macEntry.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;

        /*
            1.9. Call function with out of range
                 macEntryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
            Expected: GT_BAD_PARAM.
        */
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        macEntry.key.ipMcast.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);

        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                     "%d, macEntryPtr->key->ipMcast->vlanId = %d",
                                     dev, macEntry.key.ipMcast.vlanId);

        /*
            1.10. Call function with out of range
                 macEntryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                                    entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: GT_BAD_PARAM.
        */
        macEntry.key.ipMcast.vlanId = 100;
        macEntry.key.macVlan.vlanId = UTF_CPSS_PP_MAX_VLAN_NUM_CNS(dev);
        macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                                     "%d, macEntryPtr->key->macVlan->vlanId = %d",
                                     dev, macEntry.key.macVlan.vlanId);

        /*
            1.11. Call function with null hashPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    macEntry.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.macVlan.vlanId = 100;

    macEntry.key.macVlan.macAddr.arEther[0] = 0x0;
    macEntry.key.macVlan.macAddr.arEther[1] = 0x1A;
    macEntry.key.macVlan.macAddr.arEther[2] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[3] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[4] = 0xFF;
    macEntry.key.macVlan.macAddr.arEther[5] = 0xFF;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgFdbHashCalc(dev, &macEntry, &hash);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaMsgVid1EnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaMsgVid1EnableSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbNaMsgVid1EnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

        enable = GT_TRUE;

        st = cpssDxChBrgFdbNaMsgVid1EnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaMsgVid1EnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaMsgVid1EnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbNaMsgVid1EnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbNaMsgVid1EnableGet)
{

/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st    = GT_OK;
    GT_BOOL     enable;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbNaMsgVid1EnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdb16BitFidHashEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdb16BitFidHashEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdb16BitFidHashEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgFdb16BitFidHashEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChBrgFdb16BitFidHashEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdb16BitFidHashEnableGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);

        /*
            1.1. Call with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        enable = GT_FALSE;

        st = cpssDxChBrgFdb16BitFidHashEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChBrgFdb16BitFidHashEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdb16BitFidHashEnableGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdb16BitFidHashEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdb16BitFidHashEnableSet(devNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdb16BitFidHashEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdb16BitFidHashEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum,  &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &enable);

        /*
            1.2. Call function with NULL enablePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdb16BitFidHashEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveUserDefinedSet
(
    IN GT_U8    devNum,
    IN GT_U32   actUerDefined,
    IN GT_U32   actUerDefinedMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveUserDefinedSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with actUerDefined [0 / maxActUerDefined/2 / maxActUerDefined],
                   actUerDefinedMask [0 / maxActUerDefined/2 / maxActUerDefined].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbActionActiveUserDefinedGet with non NULL pointers.
    Expected: GT_OK and the same actUerDefined, actUerDefinedMask.
    1.3. Call with out of range actUerDefined [maxActUerDefined + 1]
                   and others valid params.
    Expected: NOT GT_OK.
    1.4. Call with out of range actUerDefinedMask [maxActUerDefined + 1]
                   and others valid params.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;

    GT_U32      maxActUerDefined     = 0;
    GT_U32      actUerDefined        = 0;
    GT_U32      actUerDefinedGet     = 0;
    GT_U32      actUerDefinedMask    = 0;
    GT_U32      actUerDefinedMaskGet = 0;
    GT_U32      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* get max action active UerDefined */
        maxActUerDefined = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FDB_USER_DEFINED_MAC(devNum);

        /*
            1.1. Call with actUerDefined [0 / maxActUerDefined/2 / maxActUerDefined],
                           actUerDefinedMask [0 / maxActUerDefined/2 / maxActUerDefined].
            Expected: GT_OK.
        */

        /* call with actUerDefined = 0 */
        actUerDefined     = 0;
        actUerDefinedMask = 0;

        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, actUerDefined, actUerDefinedMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveUserDefinedGet with non NULL pointers.
            Expected: GT_OK and the same actUerDefined, actUerDefinedMask.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefinedGet,
                                                      &actUerDefinedMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbActionActiveUserDefinedGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefined, actUerDefinedGet,
                   "get another actUerDefined than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefinedMask, actUerDefinedMaskGet,
                   "get another actUerDefinedMask than was set: %d", devNum);

        /* call with actUerDefined = maxActUerDefined/2 */
        actUerDefined     = maxActUerDefined / 2;
        actUerDefinedMask = maxActUerDefined / 2;

        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, actUerDefined, actUerDefinedMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveUserDefinedGet with non NULL pointers.
            Expected: GT_OK and the same actUerDefined, actUerDefinedMask.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefinedGet,
                                                      &actUerDefinedMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbActionActiveUserDefinedGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefined, actUerDefinedGet,
                   "get another actUerDefined than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefinedMask, actUerDefinedMaskGet,
                   "get another actUerDefinedMask than was set: %d", devNum);

        /* call with actUerDefined = maxActUerDefined */
        actUerDefined     = maxActUerDefined;
        actUerDefinedMask = maxActUerDefined;

        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, actUerDefined, actUerDefinedMask);

        /*
            1.2. Call cpssDxChBrgFdbActionActiveUserDefinedGet with non NULL pointers.
            Expected: GT_OK and the same actUerDefined, actUerDefinedMask.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefinedGet,
                                                      &actUerDefinedMaskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbActionActiveUserDefinedGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefined, actUerDefinedGet,
                   "get another actUerDefined than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(actUerDefinedMask, actUerDefinedMaskGet,
                   "get another actUerDefinedMask than was set: %d", devNum);

        /*
            1.3. Call with out of range actUerDefined [maxActUerDefined + 1]
                           and others valid params.
            Expected: NOT GT_OK.
        */
        actUerDefined = maxActUerDefined + 1;

        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, actUerDefined, actUerDefinedMask);

        /* restore values */
        actUerDefined = 0;

        /*
            1.4. Call with out of range actUerDefinedMask [maxActUerDefined + 1]
                           and others valid params.
            Expected: NOT GT_OK.
        */

        actUerDefinedMask = maxActUerDefined + 1;

        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, actUerDefined, actUerDefinedMask);
    }

    actUerDefined     = 0;
    actUerDefinedMask = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                      actUerDefinedMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveUserDefinedSet(devNum, actUerDefined,
                                                  actUerDefinedMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbActionActiveUserDefinedGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actUerDefinedPtr,
    OUT GT_U32   *actUerDefinedMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbActionActiveUserDefinedGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with not NULL actUerDefinedPtr,
                   not NULL actUerDefinedMaskPtr.
    Expected: GT_OK.
    1.2. Call with NULL actUerDefinedPtr,
                   and others valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL actUerDefinedMaskPtr,
                   and others valid params.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;

    GT_U32      actUerDefined     = 0;
    GT_U32      actUerDefinedMask = 0;
    GT_U32      notAppFamilyBmp = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,  notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with not NULL actUerDefinedPtr,
                           not NULL actUerDefinedMaskPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefined,
                                                      &actUerDefinedMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with NULL actUerDefinedPtr,
                           and others valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, NULL, &actUerDefinedMask);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actUerDefinedPtr = NULL", devNum);

        /*
            1.3. Call with NULL actUerDefinedMaskPtr,
                           and others valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefined, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actUerDefinedMaskPtr = NULL", devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefined,
                                                      &actUerDefinedMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbActionActiveUserDefinedGet(devNum, &actUerDefined,
                                                  &actUerDefinedMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDaLookupAnalyzerIndexSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDaLookupAnalyzerIndexSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enbale [GT_FALSE / GT_TRUE],
                   index [0 / 3 / 6].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbDaLookupAnalyzerIndexGet with non NULL pointers.
    Expected: GT_OK and the same enable, index.
    1.3. Call with enbale [GT_FALSE],
                   out of range index [7] (not relevant).
    Expected: GT_OK.
    1.4. Call with enbale [GT_TRUE]
                   out of range index [7].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      index     = 0;
    GT_U32      indexGet  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with enbale [GT_FALSE / GT_TRUE],
                           index [0 / 3 / 6].
            Expected: GT_OK.
        */

        /* call with index = 0 */
        enable = GT_FALSE;
        index  = 0;

        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbDaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbDaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /* call with index = 3 */
        enable = GT_TRUE;
        index  = 3;

        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbDaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbDaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /* call with index = 6 */
        index = 6;

        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbDaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbDaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /*
            1.3. Call with enbale [GT_FALSE],
                           out of range index [7] (not relevant).
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        index  = 7;

        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /* restore values */
        index = 0;

        /*
            1.4. Call with enbale [GT_TRUE]
                           out of range index [7].
            Expected: NOT GT_OK.
        */
        enable = GT_TRUE;
        index  = 7;

        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);
    }

    enable = GT_FALSE;
    index  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbDaLookupAnalyzerIndexGet
(
    GT_U8       devNum,
    GT_BOOL     *enablePtr,
    GT_U32      *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbDaLookupAnalyzerIndexGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with not NULL enablePtr,
                   not NULL indexPtr.
    Expected: GT_OK.
    1.2. Call with NULL enablePtr,
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL indexPtr,
                   and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;
    GT_U32      index  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr,
                           not NULL indexPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum,  &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with NULL enablePtr,
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, NULL, &index);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);

        /*
            1.3. Call with NULL indexPtr,
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enable, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, indexPtr = NULL", devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbDaLookupAnalyzerIndexGet(devNum, &enable, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbLearnPrioritySet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with learnPriority [CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E /
                                    CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbLearnPriorityGet and not NULL pointers.
    Expected: GT_OK and the same learnPriority.
    1.1.3. Call with wrong learnPriority enum values
                           and other valid params.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    GT_PORT_NUM portNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriority    = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriorityGet = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with learnPriority [CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E /
                                                CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E].
                Expected: GT_OK.
            */

            /* call with learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E */
            learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

            st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, learnPriority);

            /*
                1.1.2. Call cpssDxChBrgFdbLearnPriorityGet and not NULL pointers.
                Expected: GT_OK and the same learnPriority.
            */
            st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriorityGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbLearnPrioritySet: %d, %d", devNum, portNum);

            /* verify values */
            UTF_VERIFY_EQUAL2_STRING_MAC(learnPriority, learnPriorityGet,
                       "get another learnPriority than was set: %d, %d", devNum, portNum);

            /* call with learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E */
            learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E;

            st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, learnPriority);

            /*
                1.1.2. Call cpssDxChBrgFdbLearnPriorityGet and not NULL pointers.
                Expected: GT_OK and the same learnPriority.
            */
            st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriorityGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbLearnPrioritySet: %d, %d", devNum, portNum);

            /* verify values */
            UTF_VERIFY_EQUAL2_STRING_MAC(learnPriority, learnPriorityGet,
                       "get another learnPriority than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call with wrong learnPriority enum values
                                       and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbLearnPrioritySet
                                (devNum, portNum, learnPriority),
                                learnPriority);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    portNum = 0;
    learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learnPriority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbLearnPriorityGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call with not NULL learnPriorityPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL learnPriorityPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    GT_PORT_NUM portNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    learnPriority = CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL learnPriorityPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call function with NULL learnPriorityPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, learnPriorityPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    portNum = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriority);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMaxLengthSrcIdEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMaxLengthSrcIdEnableSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbMaxLengthSrcIdEnableGet with not NULL pointers.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable = GT_TRUE */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChBrgFdbMaxLengthSrcIdEnableGet with not NULL pointers.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMaxLengthSrcIdEnableGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);

        /* call with enable = GT_FALSE */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChBrgFdbMaxLengthSrcIdEnableGet with not NULL pointers.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMaxLengthSrcIdEnableGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet(devNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMaxLengthSrcIdEnableGet
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                             *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMaxLengthSrcIdEnableGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call function with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call function with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, &enable);

        /*
            1.2. Call function with NULL enablePtr.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMaxLengthSrcIdEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbMessagesQueueManagerInfoGet
(
    IN  GT_U8   devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMessagesQueueManagerInfoGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with valid devNum.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with valid devNum.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbMessagesQueueManagerInfoGet(devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbMessagesQueueManagerInfoGet(devNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbMessagesQueueManagerInfoGet(devNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupQueueFullGet
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_DXCH_FDB_QUEUE_TYPE_ENT queueType,
    OUT GT_PORT_GROUPS_BMP          *isFullPortGroupsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupQueueFullGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E /
                              CPSS_DXCH_FDB_QUEUE_TYPE_FU_E],
                   non-null isFullPortGroupsBmpPtr.
    Expected: GT_OK.
    1.2. Call with wrong queueType enum values
                   and other valid params.
    Expected: GT_BAD_PARAM.
    1.3. Call with isFullPortGroupsBmpPtr [NULL]
                   and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st            = GT_OK;
    GT_U8               dev           = 0;
    GT_U32              portGroupId   = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;

    CPSS_DXCH_FDB_QUEUE_TYPE_ENT queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;
    GT_PORT_GROUPS_BMP           isFullPortGroupsBmp = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            if(PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E ==
               PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode)
            {
                portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            }

            /*
                1.1. Call with queueType [CPSS_DXCH_FDB_QUEUE_TYPE_AU_E /
                                          CPSS_DXCH_FDB_QUEUE_TYPE_FU_E],
                               non-null isFullPortGroupsBmpPtr.
                Expected: GT_OK.
            */

            /* call with queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E */
            queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;

            st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, &isFullPortGroupsBmp);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, queueType);

            /* call with queueType = CPSS_DXCH_FDB_QUEUE_TYPE_FU_E */
            queueType = CPSS_DXCH_FDB_QUEUE_TYPE_FU_E;

            st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, &isFullPortGroupsBmp);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, queueType);

            /*
                1.2. Call with wrong queueType enum values
                               and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbPortGroupQueueFullGet
                                (dev, portGroupsBmp, queueType, &isFullPortGroupsBmp),
                                queueType);

            /*
                1.3. Call with isFullPortGroupsBmpPtr [NULL]
                               and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, isFullPortGroupsBmpPtr = NULL", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, &isFullPortGroupsBmp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    queueType = CPSS_DXCH_FDB_QUEUE_TYPE_AU_E;
    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, &isFullPortGroupsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortGroupQueueFullGet(dev, portGroupsBmp, queueType, &isFullPortGroupsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortGroupTrigActionStatusGet
(
    IN  GT_U8    devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_PORT_GROUPS_BMP  *actFinishedPortGroupsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortGroupTrigActionStatusGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1. Call with non-null actFinishedPortGroupsBmpPtr.
    Expected: GT_OK.
    1.2. Call with actFinishedPortGroupsBmpPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st            = GT_OK;
    GT_U8               dev           = 0;
    GT_U32              portGroupId   = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;

    GT_PORT_GROUPS_BMP  actFinishedPortGroupsBmp = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* initialize port group */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            if(PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E ==
               PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.fdbMode)
            {
                portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            }

            /*
                1.1. Call with non-null actFinishedPortGroupsBmpPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbPortGroupTrigActionStatusGet(dev, portGroupsBmp, &actFinishedPortGroupsBmp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);

            /*
                1.2. Call with actFinishedPortGroupsBmpPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbPortGroupTrigActionStatusGet(dev, portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, actFinishedPortGroupsBmpPtr = NULL", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChBrgFdbPortGroupTrigActionStatusGet(dev, portGroupsBmp, &actFinishedPortGroupsBmp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortGroupTrigActionStatusGet(dev, portGroupsBmp, &actFinishedPortGroupsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortGroupTrigActionStatusGet(dev, portGroupsBmp, &actFinishedPortGroupsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaLookupAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable,
    IN GT_U32  index
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaLookupAnalyzerIndexSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with enbale [GT_FALSE / GT_TRUE],
                   index [0 / 3 / 6].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbSaLookupAnalyzerIndexGet with non NULL pointers.
    Expected: GT_OK and the same enable, index.
    1.3. Call with enbale [GT_FALSE],
                   out of range index [7] (not relevant).
    Expected: GT_OK.
    1.4. Call with enbale [GT_TRUE]
                   out of range index [7].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_U32      index     = 0;
    GT_U32      indexGet  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with enbale [GT_FALSE / GT_TRUE],
                           index [0 / 3 / 6].
            Expected: GT_OK.
        */

        /* call with index = 0 */
        enable = GT_FALSE;
        index  = 0;

        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbSaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbSaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /* call with index = 3 */
        enable = GT_TRUE;
        index  = 3;

        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbSaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbSaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /* call with index = 6 */
        index = 6;

        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /*
            1.2. Call cpssDxChBrgFdbSaLookupAnalyzerIndexGet with non NULL pointers.
            Expected: GT_OK and the same enable, index.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enableGet, &indexGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbSaLookupAnalyzerIndexGet: %d", devNum);

        /* verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", devNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(index, indexGet,
                   "get another index than was set: %d", devNum);

        /*
            1.3. Call with enbale [GT_FALSE],
                           out of range index [7] (not relevant).
            Expected: GT_OK.
        */
        enable = GT_FALSE;
        index  = 7;

        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);

        /* restore values */
        index = 0;

        /*
            1.4. Call with enbale [GT_TRUE]
                           out of range index [7].
            Expected: NOT GT_OK.
        */
        enable = GT_TRUE;
        index  = 7;

        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, enable, index);
    }

    enable = GT_FALSE;
    index  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, enable, index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaLookupAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaLookupAnalyzerIndexGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with not NULL enablePtr,
                   not NULL indexPtr.
    Expected: GT_OK.
    1.2. Call with NULL enablePtr,
                   and other valid params.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL indexPtr,
                   and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_BOOL     enable = GT_FALSE;
    GT_U32      index  = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr,
                           not NULL indexPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum,  &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with NULL enablePtr,
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, NULL, &index);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", devNum);

        /*
            1.3. Call with NULL indexPtr,
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enable, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, indexPtr = NULL", devNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enable, &index);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaLookupAnalyzerIndexGet(devNum, &enable, &index);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           userGroup
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbUserGroupSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with userGroup [0 / 5 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbUserGroupGet with not NULL pointers.
    Expected: GT_OK and the same userGroup.
    1.1.3. Call with out of range of userGroup [8].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    GT_PORT_NUM portNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      userGroup    = 0;
    GT_U32      userGroupGet = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with userGroup [0 / 5 / 7].
                Expected: GT_OK.
            */

            /* call with userGroup = 0 */
            userGroup = 0;

            st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, userGroup);

            /*
                1.1.2. Call cpssDxChBrgFdbUserGroupGet with not NULL pointers.
                Expected: GT_OK and the same userGroup.
            */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroupGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbUserGroupGet: %d, %d", devNum, portNum);

            /* verify values */
            UTF_VERIFY_EQUAL2_STRING_MAC(userGroup, userGroupGet,
                       "get another userGroup than was set: %d, %d", devNum, portNum);

            /* call with userGroup = 5 */
            userGroup = 5;

            st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, userGroup);

            /*
                1.1.2. Call cpssDxChBrgFdbUserGroupGet with not NULL pointers.
                Expected: GT_OK and the same userGroup.
            */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroupGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbUserGroupGet: %d, %d", devNum, portNum);

            /* verify values */
            UTF_VERIFY_EQUAL2_STRING_MAC(userGroup, userGroupGet,
                       "get another userGroup than was set: %d, %d", devNum, portNum);

            /* call with userGroup = 7 */
            userGroup = 7;

            st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, userGroup);

            /*
                1.1.2. Call cpssDxChBrgFdbUserGroupGet with not NULL pointers.
                Expected: GT_OK and the same userGroup.
            */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroupGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbUserGroupGet: %d, %d", devNum, portNum);

            /* verify values */
            UTF_VERIFY_EQUAL2_STRING_MAC(userGroup, userGroupGet,
                       "get another userGroup than was set: %d, %d", devNum, portNum);

            /*
                1.1.3. Call with out of range of userGroup [8].
                Expected: NOT GT_OK.
            */
            userGroup = 16;

            st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }

        userGroup = 0;

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    portNum = 0;
    userGroup = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbUserGroupSet(devNum, portNum, userGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbUserGroupGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *userGroupPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbUserGroupGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (SIP5)
    1.1.1. Call function with not NULL userGroupPtr.
    Expected: GT_OK.
    1.1.2. Call function with NULL userGroupPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st      = GT_OK;
    GT_U8       devNum  = 0;
    GT_PORT_NUM portNum = 0;
    GT_U32      notAppFamilyBmp = 0;

    GT_U32      userGroup = 0;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call function with not NULL userGroupPtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, &userGroup);

            /*
                1.1.2. Call function with NULL userGroupPtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, userGroupPtr = NULL", devNum, portNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active portNum */
            st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);

        st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU portNum number.                                         */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }
    }

    portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroup);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashCrcMultiResultsCalc
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC  *entryKeyPtr,
    IN  GT_U32                      multiHashStartBankIndex,
    IN  GT_U32                      numOfBanks,
    OUT GT_U32                      crcMultiHashArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashCrcMultiResultsCalc)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
            entryKeyPtr{
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                key->macVlan->vlanId[100],
            },
            multiHashStartBankIndex[0],
            numOfBanks[16],
            valid value of crcMultiHashArrPtr.
    Expected: GT_OK.
    1.2. Call function with
            entryKeyPtr{
                vid1[123],
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E],
                key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                key->macVlan->vlanId[100],
            },
            multiHashStartBankIndex[0],
            numOfBanks[16],
            valid value of crcMultiHashArrPtr.
    Expected: GT_BAD_PARAM - on bobcat a0
              GT_OK        - on all other devices.
    1.3. Call function with valid
            entryKeyPtr{
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                key->ipMcast->sip[123.23.34.5],
                key->ipMacst->dip[123.45.67.8]
                key->ipMcast->vlanId[100],
            },
            multiHashStartBankIndex[5],
            numOfBanks[11],
            valid value of crcMultiHashArrPtr.
    Expected: GT_OK.
    1.4. Call function with valid
            entryKeyPtr{
                vid1[123],
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E],
                key->ipMcast->sip[123.23.34.5],
                key->ipMacst->dip[123.45.67.8]
                key->ipMcast->vlanId[100],
            },
            multiHashStartBankIndex[5],
            numOfBanks[11],
            valid value of crcMultiHashArrPtr.
    Expected: GT_BAD_PARAM - on bobcat a0
              GT_OK        - on all other devices.
    1.5. Call function with out of range enum value
        entryKeyPtr->entryType and other valid parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call function with out of range
        entryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
        entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
    Expected: NOT GT_OK.
    1.7. Call function with out of range
        entryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
        entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: GT_OUT_OF_RANGE.
    1.8. Call function with out of range entryKeyPtr[NULL] and other valid
        parameters the same as in 1.1.
    Expected: GT_BAD_PTR.
    1.9. Call function with out of range multiHashStartBankIndex[16] and
        other valid parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.10. Call function with out of range numOfBanks[17] and
        other valid parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.11. Call function with out of range crcMultiHashArrPtr[NULL] and
        other valid parameters the same as in 1.1.
    Expected: GT_BAD_PTR.

    1.12. Call function with valid
            entryKeyPtr{
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E],
                key.ipv4Unicast.dip[123.45.67.8]
                key.ipv4Unicast.vrfId[100],
            },
            multiHashStartBankIndex[0],
            numOfBanks[16],
            valid value of crcMultiHashArrPtr.
    Expected: GT_OK.
    1.13. Call function with out of range
            key.ipv4Unicast.vrfId [4096],
            entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: NOT GT_OK.
    1.14. Call function with valid
            entryKeyPtr{
            entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E],
            key.ipv6Unicast.dip[11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26]
            key.ipv6Unicast.vrfId[100],
            },
            multiHashStartBankIndex[0],
            numOfBanks[16],
            valid value of crcMultiHashArrPtr.
    Expected: GT_OK.

    1.15. Call function with out of range
            key.ipv6Unicast.vrfId [4096],
            entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
    Expected: NOT GT_OK.

*/

    GT_STATUS                   st                          = GT_OK;
    GT_U8                       dev                         = 0;
    CPSS_MAC_ENTRY_EXT_KEY_STC  entryKey                    = {0};
    GT_U32                      multiHashStartBankIndex     = 0;
    GT_U32                      numOfBank                   = 0;
    GT_U32                      crcMultiHashArr[NUM_BANKS_MULTI_HASH_CNS];
    GT_U32                      i = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                    entryKeyPtr{
                        entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                        key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                        key->macVlan->vlanId[100],
                    },
                    multiHashStartBankIndex[0],
                    numOfBanks[16],
                    valid value of crcMultiHashArrPtr.
            Expected: GT_OK.
        */
        entryKey.vid1 = 123;
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        entryKey.key.macVlan.macAddr.arEther[0] = 0x00;
        entryKey.key.macVlan.macAddr.arEther[1] = 0x1A;
        entryKey.key.macVlan.macAddr.arEther[2] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[3] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[4] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[5] = 0xFF;
        entryKey.key.macVlan.vlanId = 100;

        multiHashStartBankIndex = 0;
        numOfBank = 16;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call function with
                    entryKeyPtr{
                        vid1[123],
                        entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E],
                        key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                        key->macVlan->vlanId[100],
                    },
                    multiHashStartBankIndex[0],
                    numOfBanks[16],
                    valid value of crcMultiHashArrPtr.
            Expected: GT_BAD_PARAM - on bobcat a0
                      GT_OK        - on all other devices.

         */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        if (!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.3. Call function with valid
                    entryKeyPtr{
                        entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                        key->ipMcast->sip[123.23.34.5],
                        key->ipMacst->dip[123.45.67.8]
                        key->ipMcast->vlanId[100],
                    },
                    multiHashStartBankIndex[15],
                    numOfBanks[16],
                    valid value of crcMultiHashArrPtr.
            Expected: GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        entryKey.key.ipMcast.sip[0] = 123;
        entryKey.key.ipMcast.sip[1] = 23;
        entryKey.key.ipMcast.sip[2] = 34;
        entryKey.key.ipMcast.sip[3] = 5;
        entryKey.key.ipMcast.dip[0] = 123;
        entryKey.key.ipMcast.dip[1] = 45;
        entryKey.key.ipMcast.dip[2] = 67;
        entryKey.key.ipMcast.dip[3] = 8;
        entryKey.key.ipMcast.vlanId = 100;

        multiHashStartBankIndex = 5;
        numOfBank = 11;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call function with valid
                    entryKeyPtr{
                        vid1[123],
                        entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E],
                        key->ipMcast->sip[123.23.34.5],
                        key->ipMacst->dip[123.45.67.8]
                        key->ipMcast->vlanId[100],
                    },
                    multiHashStartBankIndex[5],
                    numOfBanks[11],
                    valid value of crcMultiHashArrPtr.
            Expected: GT_BAD_PARAM - on bobcat a0
                      GT_OK        - on all other devices.
         */

        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        if (!PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        multiHashStartBankIndex = 5;
        numOfBank = 12;
        /*5+12 > 16 --> ERROR */
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.5. Call function with out of range enum value
                entryKeyPtr->entryType and other valid parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashCrcMultiResultsCalc
                            (dev, &entryKey, multiHashStartBankIndex, numOfBank, crcMultiHashArr),
                            entryKey.entryType);

        /*
            1.6. Call function with out of range
                entryKeyPtr->key->macVlan->vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS],
                entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: NOT GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        entryKey.key.macVlan.macAddr.arEther[0] = 0x00;
        entryKey.key.macVlan.macAddr.arEther[1] = 0x1A;
        entryKey.key.macVlan.macAddr.arEther[2] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[3] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[4] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[5] = 0xFF;
        entryKey.key.macVlan.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        entryKey.key.macVlan.vlanId = 100;

        /*
            1.7. Call function with out of range
                entryKeyPtr->key->macVlan->vlanId[PRV_CPSS_MAX_NUM_VLANS_CNS],
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E].
            Expected: NOT GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        entryKey.key.ipMcast.sip[0] = 123;
        entryKey.key.ipMcast.sip[1] = 23;
        entryKey.key.ipMcast.sip[2] = 34;
        entryKey.key.ipMcast.sip[3] = 5;
        entryKey.key.ipMcast.dip[0] = 123;
        entryKey.key.ipMcast.dip[1] = 45;
        entryKey.key.ipMcast.dip[2] = 67;
        entryKey.key.ipMcast.dip[3] = 8;
        entryKey.key.ipMcast.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        entryKey.key.ipMcast.vlanId = 100;

        /*
            1.8. Call function with out of range entryKeyPtr[NULL] and other valid
                parameters the same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, NULL,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.9. Call function with out of range multiHashStartBankIndex[16] and
                other valid parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */
        multiHashStartBankIndex = 16;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        multiHashStartBankIndex = 15;

        /*
            1.10. Call function with out of range numOfBanks[17] and
                other valid parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */
        numOfBank = 17;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        numOfBank = 16;

        /*
            1.11. Call function with out of range crcMultiHashArrPtr[NULL] and
                other valid parameters the same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
        1.1. Call function with valid
                entryKeyPtr{
                    entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E],
                    key.ipv4Unicast.dip[123.45.67.8]
                    key.ipv4Unicast.vrfId[100],
                },
                multiHashStartBankIndex[0],
                numOfBanks[16],
                valid value of crcMultiHashArrPtr.
        Expected: GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E;
        entryKey.key.ipv4Unicast.dip[0] = 123;
        entryKey.key.ipv4Unicast.dip[1] = 45;
        entryKey.key.ipv4Unicast.dip[2] = 67;
        entryKey.key.ipv4Unicast.dip[3] = 8;
        entryKey.key.ipv4Unicast.vrfId = 100;

        multiHashStartBankIndex = 0;
        numOfBank = 16;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.13. Call function with out of range
                key.ipv4Unicast.vrfId [4096],
                entryType [CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E].
            Expected: NOT GT_OK.
        */
        entryKey.key.ipv4Unicast.vrfId = 4096;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        /*
        1.14. Call function with valid
                entryKeyPtr{
                    entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E],
                    key.ipv6Unicast.dip[11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26]
                    key.ipv6Unicast.vrfId[100],
                },
                multiHashStartBankIndex[0],
                numOfBanks[16],
                valid value of crcMultiHashArrPtr.
        Expected: GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E;

        for (i = 0; i < 16; i++)
        {
            entryKey.key.ipv6Unicast.dip[i] = (GT_U8)(11+i);
        }
        entryKey.key.ipv6Unicast.vrfId = 19;
        multiHashStartBankIndex = 0;
        numOfBank = 16;

        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.15. Call function with out of range
                key.ipv6Unicast.vrfId [4096],
                entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E].
            Expected: NOT GT_OK.
        */
        entryKey.key.ipv6Unicast.vrfId = 4096;
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }


    /* restore valid values */
    entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    entryKey.key.macVlan.macAddr.arEther[0] = 0x00;
    entryKey.key.macVlan.macAddr.arEther[1] = 0x1A;
    entryKey.key.macVlan.macAddr.arEther[2] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[3] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[4] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[5] = 0xFF;
    entryKey.key.macVlan.vlanId = 100;

    multiHashStartBankIndex = 15;
    numOfBank = 16;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                                   multiHashStartBankIndex,
                                                   numOfBank, crcMultiHashArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev, &entryKey,
                                               multiHashStartBankIndex,
                                               numOfBank, crcMultiHashArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashRequestSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC  *entryKeyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashRequestSend)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with
            entryKeyPtr{
                vid1 = 123,
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                key->macVlan->vlanId[100],
            }.
    Expected: GT_OK.
    1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.3. Call function with
            entryKeyPtr{
                vid1 = 123,
                entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                key->ipMcast->sip[123.23.34.5],
                key->ipMacst->dip[123.45.67.8]
                key->ipMcast->vlanId[100],
            }.
    Expected: GT_OK.
    1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
    Expected: GT_OK        - for eArch except bobcat a0.
              GT_BAD_PARAM - for other devices
    1.5. Call function with out of range enum value
        entryKeyPtr->entryType and other valid parameters the same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call function with out of range
        entryKeyPtr->key->ipMcast->vlanId[PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(dev)+1].
    Expected: NOT GT_OK.
    1.7. Call function with out of range entryKeyPtr[NULL] and other valid
        parameters the same as in 1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                          = GT_OK;
    GT_U8                       dev                         = 0;
    CPSS_MAC_ENTRY_EXT_KEY_STC  entryKey                    = {0};
    GT_STATUS                   expected                    = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                    entryKeyPtr{
                        entryType[CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                        key->macVlan->macAddr[00:1A:FF:FF:FF:FF]
                        key->macVlan->vlanId[100],
                    }.
            Expected: GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        entryKey.key.macVlan.macAddr.arEther[0] = 0x00;
        entryKey.key.macVlan.macAddr.arEther[1] = 0x1A;
        entryKey.key.macVlan.macAddr.arEther[2] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[3] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[4] = 0xFF;
        entryKey.key.macVlan.macAddr.arEther[5] = 0xFF;
        entryKey.key.macVlan.vlanId = 100;

        st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, GT_FALSE);
        /*
            1.2. Same as 1.1 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
            GT_BAD_PARAM - for other devices
        */
        expected = PRV_CPSS_SIP_5_10_CHECK_MAC(dev) ?
            GT_OK : GT_BAD_PARAM;
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E;

        st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, GT_FALSE);

        /*
            1.3. Call function with
                entryKeyPtr{
                    entryType[CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E],
                    key->ipMcast->sip[123.23.34.5],
                    key->ipMacst->dip[123.45.67.8]
                    key->ipMcast->vlanId[100],
                }.
            Expected: GT_OK.
        */
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
        entryKey.key.ipMcast.sip[0] = 123;
        entryKey.key.ipMcast.sip[1] = 23;
        entryKey.key.ipMcast.sip[2] = 34;
        entryKey.key.ipMcast.sip[3] = 5;
        entryKey.key.ipMcast.dip[0] = 123;
        entryKey.key.ipMcast.dip[1] = 45;
        entryKey.key.ipMcast.dip[2] = 67;
        entryKey.key.ipMcast.dip[3] = 8;
        entryKey.key.ipMcast.vlanId = 100;

        st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait for acknowledge that the set was done */
        utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev, GT_FALSE);

        /*
            1.4. Same as 1.3 except entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E
            Expected: GT_OK        - for eArch except bobcat a0.
            GT_BAD_PARAM - for other devices
        */

        expected = PRV_CPSS_SIP_5_10_CHECK_MAC(dev)  && !PRV_CPSS_SIP_6_CHECK_MAC(dev)?
            GT_OK : GT_BAD_PARAM;
        entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E;

        st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
        UTF_VERIFY_EQUAL1_PARAM_MAC(expected, st, dev);

        /*
            1.5. Call function with out of range enum value
                entryKeyPtr->entryType and other valid parameters the same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbHashRequestSend
                            (dev, &entryKey),
                            entryKey.entryType);

        if(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev) < 0xFFFF) /* check that we can cast to GT_U16 after '+1' */
        {
            /*
                1.6. Call function with out of range
                    entryKeyPtr->key->macVlan->vlanId [PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC+1].
                Expected: NOT GT_OK.
            */
            entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
            entryKey.key.macVlan.vlanId = (GT_U16)(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1);

            st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
            entryKey.key.ipMcast.vlanId = (GT_U16)(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_FID_MAC(dev)+1);

            st = cpssDxChBrgFdbHashRequestSend(dev, &entryKey);
            UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* restore 'mac addr' state */
            entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        }

        entryKey.key.macVlan.vlanId = 100;

        /*
            1.7. Call function with out of range entryKeyPtr[NULL] and
                other valid parameters the same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbHashRequestSend(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* restore valid values */
    entryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    entryKey.key.macVlan.macAddr.arEther[0] = 0x00;
    entryKey.key.macVlan.macAddr.arEther[1] = 0x1A;
    entryKey.key.macVlan.macAddr.arEther[2] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[3] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[4] = 0xFF;
    entryKey.key.macVlan.macAddr.arEther[5] = 0xFF;
    entryKey.key.macVlan.vlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashRequestSend(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbHashRequestSend(dev, NULL);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbHashResultsGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       multiHashStartBankIndex,
    IN  GT_U32       numOfBanks,
    OUT GT_U32       crcMultiHashArr[],
    INOUT GT_U32     *xorHashPtr,
    INOUT GT_U32     *crcHashPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbHashResultsGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call function with
            multiHashStartBankIndex[0, 5, 15],
            numOfBanks[1, 7, 16],
            valid value of crcMultiHashArrPtr,
            valid value of xorHashPtr,
            valid value of crcHashPtr.
    Expected: GT_OK.
    1.2. Call function with out of range multiHashStartBankIndex[16] and
        other valid parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call function with out of range numOfBanks[0] and
        other valid parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.4. Call function with out of range numOfBanks[17] and
        other valid parameters the same as in 1.1.
    Expected: NOT GT_OK.
    1.5. Call function with out of range crcMultiHashArrPtr[NULL] and
        other valid parameters the same as in 1.1.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                          = GT_OK;
    GT_U8                       dev                         = 0;
    GT_U32                      multiHashStartBankIndex     = 0;
    GT_U32                      numOfBanks                  = 0;
    GT_U32                      crcMultiHashArr[16]         = {0};
    GT_U32                      xorHash                     = 0;
    GT_U32                      crcHash                     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with
                multiHashStartBankIndex[0, 5, 15],
                numOfBanks[14, 7, 0],
                valid value of crcMultiHashArrPtr,
                valid value of xorHashPtr,
                valid value of crcHashPtr.
            Expected: GT_OK.
        */

        /* call with multiHashStartBankIndex = 0 */
        multiHashStartBankIndex = 0;
        numOfBanks = 14;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with multiHashStartBankIndex = 5 */
        multiHashStartBankIndex = 5;
        numOfBanks = 7;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with multiHashStartBankIndex = 15 */
        multiHashStartBankIndex = 15;
        numOfBanks = 0;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        numOfBanks = 1;

        /*
            1.2. Call function with out of range multiHashStartBankIndex[16]
                and other valid parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */

        multiHashStartBankIndex = 16;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        multiHashStartBankIndex = 15;

        /*
            1.3. Call function with out of range numOfBanks[0] and
                other valid parameters the same as in 1.1.
            Expected: GT_OK.
        */
        numOfBanks = 0;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        numOfBanks = 16;

        /*
            1.4. Call function with out of range numOfBanks[17] and
                other valid parameters the same as in 1.1.
            Expected: NOT GT_OK.
        */
        numOfBanks = 17;

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, crcMultiHashArr,
                                          &xorHash, &crcHash);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        numOfBanks = 16;

        /*
            1.5. Call function with out of range crcMultiHashArrPtr[NULL] and
                other valid parameters the same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, NULL,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    multiHashStartBankIndex = 15;
    numOfBanks = 16;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                          numOfBanks, NULL,
                                          &xorHash, &crcHash);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/

    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgFdbHashResultsGet(dev, multiHashStartBankIndex,
                                      numOfBanks, NULL,
                                      &xorHash, &crcHash);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbCrcHashUpperBitsModeSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call function with mode
                [CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,
                 CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E,
                 CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbCrcHashUpperBitsModeGet
    Expected: GT_OK and the same mode.
    1.3. Call function with out of range enum value mode.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                       st      = GT_OK;
    GT_U8                                           dev     = 0;
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT  mode    = 0;
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT  modeRet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with mode
                        [CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,
                         CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E,
                         CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E].
            Expected: GT_OK.
        */

        /* call with mode[CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E] */
        mode = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;

        st = cpssDxChBrgFdbCrcHashUpperBitsModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbCrcHashUpperBitsModeGet
            Expected: GT_OK and the same mode.
        */
        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &modeRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                     "got another mode than was set: %d",
                                     dev);

        /* call with mode[CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E] */
        mode = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_FID_E;

        st = cpssDxChBrgFdbCrcHashUpperBitsModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbCrcHashUpperBitsModeGet
            Expected: GT_OK and the same mode.
        */
        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &modeRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                     "got another mode than was set: %d",
                                     dev);

        /* call with mode[CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E] */
        mode = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E;

        st = cpssDxChBrgFdbCrcHashUpperBitsModeSet(dev, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mode);

        /*
            1.2. Call cpssDxChBrgFdbSecureAutoLearnGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &modeRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(mode, modeRet,
                                     "got another mode than was set: %d",
                                     dev);

        /*
            1.3. Call function with out of range enum value mode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbCrcHashUpperBitsModeSet
                            (dev, mode),
                            mode);
    }

    mode = CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbCrcHashUpperBitsModeSet(dev, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbCrcHashUpperBitsModeSet(dev, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbCrcHashUpperBitsModeGet
(
    IN  GT_U8                                            devNum,
    OUT  CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbCrcHashUpperBitsModeGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with invalid modePtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                       st      = GT_OK;
    GT_U8                                           dev     = 0;
    CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT  mode    = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null modePtr.
            Expected: GT_OK.
        */

        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with invalid modePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbCrcHashUpperBitsModeGet(dev, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbBankCounterValueGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          bankIndex,
    OUT GT_U32                                          *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbBankCounterValueGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with bankIndex[0, 5, 15] and non-null valuePtr.
    Expected: GT_OK.
    1.2. Call with out of range bankIndex[16] and other valid values the
        same as in 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range valuePtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                                       st          = GT_OK;
    GT_U8                                           dev         = 0;
    GT_U32                                          bankIndex   = 0;
    GT_U32                                          value       = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with bankIndex[0, 5, 15] and non-null valuePtr.
            Expected: GT_OK.
        */

        /* call with multiHashStartBankIndex = 0 */
        bankIndex = 0;

        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with bankIndex = 5 */
        bankIndex = 5;

        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with bankIndex = 15 */
        bankIndex = 15;

        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with out of range bankIndex[16] and other valid values the
                same as in 1.1.
            Expected: NOT GT_OK.
        */
        bankIndex = 16;

        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore valid values */
        bankIndex = 0;

        /*
            1.3. Call with out of range valuePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbBankCounterValueGet(dev, bankIndex, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbBankCounterUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       bankIndex,
    IN GT_BOOL                      incOrDec
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbBankCounterUpdate)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with bankIndex[0, 5, 15], incOrDec[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
    1.2. Call with out of range bankIndex[16] and other valid values the
        same as in 1.1.
    Expected: NOT GT_OK.
*/

    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_U32          bankIndex   = 0;
    GT_BOOL         incOrDec    = GT_FALSE;
    GT_U32          prevValue;
    GT_U32          newValue;
    GT_U32          expectedValue;
    GT_U32          ii;
    CPSS_MAC_HASH_FUNC_MODE_ENT  mode;
    GT_U32          maxIterations = 500;

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
    {
        maxIterations = 10;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with bankIndex[0, 5, 15], incOrDec[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /***************************************/
        /* set the device into XOR / CRC       */
        /***************************************/

        /* call with multiHashStartBankIndex = 0 */
        bankIndex = 0;

        for(mode = CPSS_MAC_HASH_FUNC_XOR_E; mode <= CPSS_MAC_HASH_FUNC_CRC_E; mode++)
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* the device not supports those modes */
                break;
            }

            for(incOrDec = GT_FALSE ; incOrDec <= GT_TRUE ; incOrDec++)
            {
                st = cpssDxChBrgFdbHashModeSet(dev,mode);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* wait until PP is NOT busy */
                utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

                st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* in XOR/CRC mode the bank counters are also changed */
                expectedValue =
                    (incOrDec == GT_FALSE)
                        ? ((prevValue > 0) ? (prevValue - 1) : 0)
                        : (prevValue + 1);

                UTF_VERIFY_EQUAL1_PARAM_MAC(expectedValue, newValue, dev);
            }
        }


        /***************************************/
        /* set the device into multi hash mode */
        /***************************************/
        mode = CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
        st = cpssDxChBrgFdbHashModeSet(dev,mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with multiHashStartBankIndex = 0 */
        bankIndex = 0;
        incOrDec = GT_FALSE;

        st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* wait until PP is NOT busy */
        utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

        st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(prevValue == 0)
        {
            /* we asked for decrement , but the previous value was 0 , so new
               value must be 0 too (+ interrupt should be generated 'ERROR')*/
            UTF_VERIFY_EQUAL1_PARAM_MAC(0, newValue, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC((prevValue - 1), newValue, dev);
        }


        incOrDec = GT_TRUE;

        for(ii = 0; ii < maxIterations ; ii++)
        {
            /* call with bankIndex = 5 */
            bankIndex = 5;

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait until PP is NOT busy */
            utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

            st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC((prevValue + 1), newValue, dev);

            /* call with bankIndex = 15 */
            bankIndex = 15;

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait until PP is NOT busy */
            utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

            st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC((prevValue + 1), newValue, dev);
        }

        incOrDec = GT_FALSE;

        for(ii = 0; ii < maxIterations ; ii++)
        {
            /* call with bankIndex = 5 */
            bankIndex = 5;

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait until PP is NOT busy */
            utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

            st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC((prevValue - 1), newValue, dev);

            /* call with bankIndex = 15 */
            bankIndex = 15;

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&prevValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* wait until PP is NOT busy */
            utfCpssDxChBrgFdbBankCounterUpdateStatusGet(dev);

            st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            st = cpssDxChBrgFdbBankCounterValueGet(dev,bankIndex,&newValue);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC((prevValue - 1), newValue, dev);
        }

        /*
            1.2. Call with out of range bankIndex[16] and other valid
                values the same as in 1.1.
            Expected: NOT GT_OK.
        */
        bankIndex = 16;

        st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* restore valid values */
        bankIndex = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbBankCounterUpdate(dev, bankIndex, incOrDec);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbBankCounterUpdateStatusGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                     *isFinishedPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbBankCounterUpdateStatusGet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with non-null isFinishedPtr.
    Expected: GT_OK.
    1.2. Call with invalid isFinishedPtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS       st          = GT_OK;
    GT_U8           dev         = 0;
    GT_BOOL         isFinished  = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null isFinishedPtr.
            Expected: GT_OK.
        */

        st = cpssDxChBrgFdbBankCounterUpdateStatusGet(dev, &isFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with invalid isFinishedPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbBankCounterUpdateStatusGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbBankCounterUpdateStatusGet(dev, &isFinished);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbBankCounterUpdateStatusGet(dev, &isFinished);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortVid1LearningEnableSet
(
    IN GT_U8        devNum,
    IN GT_U8        port,
    IN GT_BOOL      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortVid1LearningEnableSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgFdbPortVid1LearningEnableGet with not NULL enablePtr
    Expected: GT_OK
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable    = GT_FALSE;
    GT_BOOL        enableGet = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgFdbPortVid1LearningEnableGet with
                       not NULL enablePtr
                Expected: GT_OK
            */
            st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgFdbPortVid1LearningEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);

            /* iterate with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChBrgFdbPortVid1LearningEnableGet with
                       not NULL enablePtr
                Expected: GT_OK
            */
            st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                  "cpssDxChBrgFdbPortVid1LearningEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                          "got another enable than was set: %d, %d", dev, port);
        }

        /* set correct values*/
        enable = GT_FALSE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_FALSE;
    port   = BRG_FDB_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortVid1LearningEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbPortVid1LearningEnableGet
(
    IN  GT_U8        devNum,
    IN  GT_U8        port,
    OUT GT_BOOL      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbPortVid1LearningEnableGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS      st  = GT_OK;
    GT_U8          dev;
    GT_PORT_NUM    port;
    GT_U32         notAppFamilyBmp = 0;
    GT_BOOL        enable = GT_FALSE;


    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available physical ports
        */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
                 for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.4. For active device check that function returns GT_OK
                 for CPU port number.
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = BRG_FDB_VALID_PHY_PORT_CNS;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbPortVid1LearningEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbVid1AssignmentEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbVid1AssignmentEnableSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call function with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbVid1AssignmentEnableGet
    Expected: GT_OK and the same enablePtr.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with enable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChBrgFdbVid1AssignmentEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /* Verifying values */
        st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbVid1AssignmentEnableGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "get another enable than was set: %d", dev);

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChBrgFdbVid1AssignmentEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChBrgFdbVid1AssignmentEnableGet
            Expected: GT_OK and the same enablePtr.
        */
        st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbVid1AssignmentEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                    "get another enable than was set: %d", dev);
    }

    enable = GT_TRUE;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbVid1AssignmentEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbVid1AssignmentEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbVid1AssignmentEnableGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbVid1AssignmentEnableGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.2. Call with null enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_BOOL     enable = GT_FALSE;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    /* SIP6 not support it */
    notAppFamilyBmp |= UTF_CPSS_PP_ALL_SIP6_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"%d, enablePtr = NULL",dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbVid1AssignmentEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbMuxedFields)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.  SrcID field in FDB table is 9b     (src_id_length_in_fdb = "0")
        disable using of VID1 in FDB entry (vid1_assignment_mode = "0")
        Call cpssDxChBrgFdbMacEntryWrite with
                index [0],
                skip [GT_FALSE],
                macEntryPtr {
                    key {entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                         macVlan->vlanId[100],
                         macVlan-> macAddr[00:1A:FF:FF:FF:FF]},
                    dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                  devPort{devNum [dev],
                                          portNum [0]},
                                  trunkId [10],
                                  vidx [10],
                                  vlanId [100]},
                    isStatic [GT_FALSE],
                    daCommand [CPSS_MAC_TABLE_FRWRD_E],
                    saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                    daRoute [GT_FALSE],
                    mirrorToRxAnalyzerPortEn [GT_FALSE],
                    sourceID [0 / 0x1A5 / BIT_9-1],
                    userDefined [0 / 0xA5 / BIT_8-1],
                    daQosIndex [0],
                    saQosIndex [0],
                    daSecurityLevel [0 / 5 / 7],
                    saSecurityLevel [0 / 5 / 7],
                    appSpecificCpuCode [GT_FALSE],
                    saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE]}.
    Expected: GT_OK.
    2.  SrcID field in FDB table is 12b    (src_id_length_in_fdb = "1")
        disable using of VID1 in FDB entry (vid1_assignment_mode = "0")
        Call cpssDxChBrgFdbMacEntryWrite with
                macEntryPtr {
                    sourceID [0 / 0xA5A / BIT_12-1],
                    userDefined [0 / 0x1A / BIT_5-1],
                    saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE]
                    and other parameters the same as in 1.}.
    Expected: GT_OK.
    3.  SrcID field in FDB table is 9b    (src_id_length_in_fdb = "0")
        enable using of VID1 in FDB entry (vid1_assignment_mode = "1")
        Call cpssDxChBrgFdbMacEntryWrite with
                macEntryPtr {
                    sourceID [0 / 0x1A / BIT_6-1],
                    userDefined [0 / 0xA5 / BIT_8-1],
                    saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    vid1 [0 / 0xA5A / BIT_12-1]
                    and other parameters the same as in 1.}.
    Expected: GT_OK.
    4.  SrcID field in FDB table is 12b   (src_id_length_in_fdb = "1")
        enable using of VID1 in FDB entry (vid1_assignment_mode = "1")
        Call cpssDxChBrgFdbMacEntryWrite with
                macEntryPtr {
                    sourceID [0 / 5 / BIT_6-1],
                    userDefined [0 / 0x1A / BIT_5-1],
                    saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                    vid1 [0 / 0xA5A / BIT_12-1]
                    and other parameters the same as in 1.}.
    Expected: GT_OK.
    5. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                              and non-NULL pointers.
    Expected: GT_OK and the same parameters as input was.
    6.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range sourceID [8]
                                       and other parameters the same as in 1.
    Expected: NOT GT_OK.
    7.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range userDefined [BIT_8]
                                       and other parameters the same as in 1.
    Expected: NOT GT_OK.
    8.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range sourceID [0x40]
                                       and other parameters the same as in 2.
    Expected: NOT GT_OK.
    9.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range userDefined [BIT_5]
                                       and other parameters the same as in 2.
    Expected: NOT GT_OK.
    10. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range sourceID [8]
                                       and other parameters the same as in 3.
    Expected: NOT GT_OK.
    11. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range userDefined [BIT_8]
                                       and other parameters the same as in 3.
    Expected: NOT GT_OK.
    12. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range vid1 [BIT_12]
                                       and other parameters the same as in 3.
    Expected: NOT GT_OK.
    13. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range sourceID [0x40]
                                       and other parameters the same as in 4.
    Expected: NOT GT_OK.
    14. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range userDefined [BIT_5]
                                       and other parameters the same as in 4.
    Expected: NOT GT_OK.
    15. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                       out of range vid1 [BIT_12]
                                       and other parameters the same as in 4.
    Expected: NOT GT_OK.
    16. Call cpssDxChBrgFdbMacEntryWrite with
                                       daSecurityLevel [5],
                                       saSecurityLevel [5]
                                       and other parameters the same as in 4.
    Expected: GT_OK and daSecurityLevel [0], saSecurityLevel [0].
    17. Call cpssDxChBrgFdbMacEntryInvalidate to invalidate all changes.
       Disable configuration for Src ID 12 bits and using of VID1
       in FDB entry
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_U32      index = 0;
    GT_BOOL     skip  = GT_FALSE;
    GT_BOOL     srcIdLengthInFDB;
    GT_BOOL     tag1VidFdbEn;
    GT_BOOL     validGet    = GT_FALSE;
    GT_BOOL     skipGet     = GT_FALSE;
    GT_BOOL     agedGet     = GT_FALSE;
    GT_HW_DEV_NUM    hwDevNumGet   = 0;
    GT_BOOL     isValidTest = GT_FALSE;

    CPSS_MAC_ENTRY_EXT_STC    macEntry;
    CPSS_MAC_ENTRY_EXT_STC    macEntryGet;

    cpssOsBzero((GT_VOID*) &macEntry, sizeof(macEntry));
    cpssOsBzero((GT_VOID*) &macEntryGet, sizeof(macEntryGet));

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* sip6 not supported -- add it to 'not applicable' */
    UTF_SIP6_ADD_TO_FAMILY_BMP_MAC(notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isValidTest = GT_TRUE;
        /*
            1.  SrcID field in FDB table is 9b     (src_id_length_in_fdb = "0")
                disable using of VID1 in FDB entry (vid1_assignment_mode = "0")
                Call cpssDxChBrgFdbMacEntryWrite with
                        index [0],
                        skip [GT_FALSE],
                        macEntryPtr {
                            key {entryType [CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E],
                                 macVlan->vlanId[100],
                                 macVlan-> macAddr[00:1A:FF:FF:FF:FF]},
                            dstInterface {type[CPSS_INTERFACE_TRUNK_E],
                                          devPort{devNum [dev],
                                                  portNum [0]},
                                          trunkId [10],
                                          vidx [10],
                                          vlanId [100]},
                            isStatic [GT_FALSE],
                            daCommand [CPSS_MAC_TABLE_FRWRD_E],
                            saCommand [CPSS_MAC_TABLE_MIRROR_TO_CPU_E],
                            daRoute [GT_FALSE],
                            mirrorToRxAnalyzerPortEn [GT_FALSE],
                            sourceID [0 / 0x1A5 / BIT_9-1],
                            userDefined [0 / 0xA5 / BIT_8-1],
                            daQosIndex [0],
                            saQosIndex [0],
                            daSecurityLevel [0],
                            saSecurityLevel [0],
                            appSpecificCpuCode [GT_FALSE],
                            saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE]}.
            Expected: GT_OK.
        */
        /* Call with
                sourceID [0],
                userDefined [0],
                saMirrorToRxAnalyzerPortEn [GT_FALSE],
                daMirrorToRxAnalyzerPortEn [GT_FALSE],
                and other parameters the same as in 1.
        */
        srcIdLengthInFDB = GT_FALSE;
        tag1VidFdbEn     = GT_FALSE;
                /* Disable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Disable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
        index = 0;
        skip  = GT_FALSE;

        macEntry.key.entryType          = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId = 100;

        macEntry.key.key.macVlan.macAddr.arEther[0] = 0x0;
        macEntry.key.key.macVlan.macAddr.arEther[1] = 0x1A;
        macEntry.key.key.macVlan.macAddr.arEther[2] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[3] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[4] = 0xFF;
        macEntry.key.key.macVlan.macAddr.arEther[5] = 0xFF;

        macEntry.dstInterface.type            = CPSS_INTERFACE_TRUNK_E;
        macEntry.dstInterface.devPort.hwDevNum  = dev;
        macEntry.dstInterface.devPort.portNum = 0;
        macEntry.dstInterface.trunkId         = 10;
        CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
        macEntry.dstInterface.vidx            = 10;
        macEntry.dstInterface.vlanId          = 100;

        macEntry.isStatic                 = GT_FALSE;
        macEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
        macEntry.saCommand                = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
        macEntry.daRoute                  = GT_FALSE;
        macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.daQosIndex               = 0;
        macEntry.saQosIndex               = 0;
        macEntry.daSecurityLevel          = 0;
        macEntry.saSecurityLevel          = 0;
        macEntry.appSpecificCpuCode       = GT_FALSE;
        macEntry.age                      = GT_TRUE;
        macEntry.spUnknown                = GT_FALSE;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* allow next code to succeed with allowed command */
            macEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
        }

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel,
                                     macEntryGet.daSecurityLevel,
              "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel,
                                     macEntryGet.saSecurityLevel,
              "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);

        /*      sourceID [0x1A5],
                userDefined [0xA5],
                saMirrorToRxAnalyzerPortEn [GT_TRUE],
                daMirrorToRxAnalyzerPortEn [GT_TRUE],
                and other parameters the same as in 1.
        */
        macEntry.daSecurityLevel            = 5;
        macEntry.saSecurityLevel            = 5;
        macEntry.sourceID                   = 0x1A5;
        macEntry.userDefined                = 0xA5;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel,
                                     macEntryGet.daSecurityLevel,
              "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel,
                                     macEntryGet.saSecurityLevel,
              "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*      sourceID [BIT_9-1],
                userDefined [BIT_8-1],
                and other parameters the same as in 1.
        */
        macEntry.daSecurityLevel            = 7;
        macEntry.saSecurityLevel            = 7;
        macEntry.sourceID                   = BIT_9-1;
        macEntry.userDefined                = BIT_8-1;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daSecurityLevel,
                                     macEntryGet.daSecurityLevel,
              "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saSecurityLevel,
                                     macEntryGet.saSecurityLevel,
              "get another macEntryPtr->saSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*
            2.  SrcID field in FDB table is 12b    (src_id_length_in_fdb = "1")
                disable using of VID1 in FDB entry (vid1_assignment_mode = "0")
                Call cpssDxChBrgFdbMacEntryWrite with
                        macEntryPtr {
                            sourceID [0 / 0xA5A / BIT_12-1],
                            userDefined [0 / 0x1A / BIT_5-1],
                            saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE]
                            and other parameters the same as in 1.}.
            Expected: GT_OK.
        */
        /* Call with
                sourceID [0],
                userDefined [0],
                saMirrorToRxAnalyzerPortEn [GT_FALSE],
                daMirrorToRxAnalyzerPortEn [GT_FALSE],
                and other parameters the same as in 1.
        */
        srcIdLengthInFDB = GT_TRUE;
        tag1VidFdbEn     = GT_FALSE;
                /* Enable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Disable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);

        macEntry.sourceID                 = 0;
        macEntry.userDefined              = 0;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);

        /*      sourceID [0xA5A],
                userDefined [0x1A],
                saMirrorToRxAnalyzerPortEn [GT_TRUE],
                daMirrorToRxAnalyzerPortEn [GT_TRUE],
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = 0xA5A;
        macEntry.userDefined                = 0x1A;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*      sourceID [BIT_12-1],
                userDefined [BIT_5-1],
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = BIT_12-1;
        macEntry.userDefined                = BIT_5-1;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                        &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                              "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*
            3.  SrcID field in FDB table is 9b    (src_id_length_in_fdb = "0")
                enable using of VID1 in FDB entry (vid1_assignment_mode = "1")
                Call cpssDxChBrgFdbMacEntryWrite with
                        macEntryPtr {
                            sourceID [0 / 0x1A / BIT_6-1],
                            userDefined [0 / 0xA5 / BIT_8-1],
                            saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            vid1 [0 / 0xA5A / BIT_12-1]
                            and other parameters the same as in 1.}.
            Expected: GT_OK.
        */
        /* Call with
                sourceID [0],
                userDefined [0],
                saMirrorToRxAnalyzerPortEn [GT_FALSE],
                daMirrorToRxAnalyzerPortEn [GT_FALSE],
                vid1 [0]
                and other parameters the same as in 1.
        */
        srcIdLengthInFDB = GT_FALSE;
        tag1VidFdbEn     = GT_TRUE;
                /* Disable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Enable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);

        macEntry.sourceID                   = 0;
        macEntry.userDefined                = 0;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.key.vid1                   = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                        &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                              "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);

        /*      sourceID [0x1A],
                userDefined [0xA5],
                saMirrorToRxAnalyzerPortEn [GT_TRUE],
                daMirrorToRxAnalyzerPortEn [GT_TRUE],
                vid1 [0xA5A5A5A5]
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = 0x1A;
        macEntry.userDefined                = 0xA5;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.key.vid1                       = 0xA5A;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*      sourceID [BIT_6-1],
                userDefined [BIT_8-1],
                vid1 [BIT_12-1]
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = BIT_6-1;
        macEntry.userDefined                = BIT_8-1;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.key.vid1                       = BIT_12-1;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*
            4.  SrcID field in FDB table is 12b   (src_id_length_in_fdb = "1")
                enable using of VID1 in FDB entry (vid1_assignment_mode = "1")
                Call cpssDxChBrgFdbMacEntryWrite with
                        macEntryPtr {
                            sourceID [0 / 5 / BIT_6-1],
                            userDefined [0 / 0x1A / BIT_5-1],
                            saMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            daMirrorToRxAnalyzerPortEn [GT_FALSE / GT_TRUE],
                            vid1 [0 / 0xA5A / BIT_12-1]
                            and other parameters the same as in 1.}.
            Expected: GT_OK.
        */
        /* Call with
                sourceID [0],
                userDefined [0],
                saMirrorToRxAnalyzerPortEn [GT_FALSE],
                daMirrorToRxAnalyzerPortEn [GT_FALSE],
                vid1 [0]
                and other parameters the same as in 1.
        */
        srcIdLengthInFDB = GT_TRUE;
        tag1VidFdbEn     = GT_TRUE;
                /* Enable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Enable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);

        macEntry.sourceID                   = 0;
        macEntry.userDefined                = 0;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_FALSE;
        macEntry.key.vid1                   = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);

        /*      sourceID [5],
                userDefined [0x1A],
                saMirrorToRxAnalyzerPortEn [GT_TRUE],
                daMirrorToRxAnalyzerPortEn [GT_TRUE],
                vid1 [0xA5A]
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = 5;
        macEntry.userDefined                = 0x1A;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.key.vid1                   = 0xA5A;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
                         "get another macEntryPtr->key.vid1 than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*      sourceID [BIT_6-1],
                userDefined [BIT_5-1],
                vid1 [BIT_12-1]
                and other parameters the same as in 1.
        */
        macEntry.sourceID                   = BIT_6-1;
        macEntry.userDefined                = BIT_5-1;
        macEntry.saMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.daMirrorToRxAnalyzerPortEn = GT_TRUE;
        macEntry.key.vid1                   = BIT_12-1;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /*
            5. Call cpssDxChBrgFdbMacEntryRead with index [0]
                                                      and non-NULL pointers.
            Expected: GT_OK and the same parameters as input was.
        */
        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                           &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.key.vid1, macEntryGet.key.vid1,
                         "get another macEntryPtr->key.vid1 than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.saMirrorToRxAnalyzerPortEn,
                                     macEntryGet.saMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->saMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.daMirrorToRxAnalyzerPortEn,
                                     macEntryGet.daMirrorToRxAnalyzerPortEn,
         "get another macEntryPtr->daMirrorToRxAnalyzerPortEn than was set: %d",
                                     dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.sourceID, macEntryGet.sourceID,
                     "get another macEntryPtr->sourceID than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.userDefined,
                                     macEntryGet.userDefined,
                  "get another macEntryPtr->userDefined than was set: %d", dev);
        /*
            6.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range sourceID [BIT_9]
                                        and other parameters the same as in 1.
            Expected: NOT GT_OK.
        */
        srcIdLengthInFDB = GT_FALSE;
        tag1VidFdbEn     = GT_FALSE;
                /* Disable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Disable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
        macEntry.sourceID    = BIT_9;
        macEntry.userDefined = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

       /*
            7.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range userDefined [BIT_8]
                                        and other parameters the same as in 1.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = BIT_8;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            8.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range sourceID [BIT_12]
                                        and other parameters the same as in 2.
            Expected: NOT GT_OK.
        */
        srcIdLengthInFDB = GT_TRUE;
        tag1VidFdbEn     = GT_FALSE;
                /* Enable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Disable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
        macEntry.sourceID    = BIT_12;
        macEntry.userDefined = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            9.  Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range userDefined [BIT_5]
                                        and other parameters the same as in 2.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = BIT_5;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            10. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range sourceID [BIT_6]
                                        and other parameters the same as in 3.
            Expected: NOT GT_OK.
        */
        srcIdLengthInFDB = GT_FALSE;
        tag1VidFdbEn     = GT_TRUE;
                /* Disable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Enable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
        macEntry.sourceID    = BIT_6;
        macEntry.userDefined = 0;
        macEntry.key.vid1        = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            11. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range userDefined [BIT_8]
                                        and other parameters the same as in 3.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = BIT_8;
        macEntry.key.vid1    = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            12. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range vid1 [BIT_12]
                                        and other parameters the same as in 3.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = 0;
        macEntry.key.vid1    = BIT_12;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            13. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range sourceID [BIT_6]
                                        and other parameters the same as in 4.
            Expected: NOT GT_OK.
        */
        srcIdLengthInFDB = GT_TRUE;
        tag1VidFdbEn     = GT_TRUE;
                /* Enable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Enable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
        macEntry.sourceID    = BIT_6;
        macEntry.userDefined = 0;
        macEntry.key.vid1    = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            14. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range userDefined [BIT_5]
                                        and other parameters the same as in 4.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = BIT_5;
        macEntry.key.vid1    = 0;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            15. Call cpssDxChBrgFdbMacEntryWrite/cpssDxChBrgFdbMacEntrySet with
                                        out of range vid1 [BIT_12]
                                        and other parameters the same as in 4.
            Expected: NOT GT_OK.
        */
        macEntry.sourceID    = 0;
        macEntry.userDefined = 0;
        macEntry.key.vid1    = BIT_12;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChBrgFdbMacEntrySet(dev, &macEntry);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        macEntry.key.vid1 = 0;

        /*
            16. Call cpssDxChBrgFdbMacEntryWrite with
                                        daSecurityLevel [5],
                                        saSecurityLevel [5]
                                        and other parameters the same as in 4.
            Expected: GT_OK and daSecurityLevel [0], saSecurityLevel [0].
        */
        macEntry.daSecurityLevel = 5;
        macEntry.saSecurityLevel = 5;

        st = cpssDxChBrgFdbMacEntryWrite(dev, index, skip, &macEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, index, skip);

        /* Verifying struct fields */
        st = cpssDxChBrgFdbMacEntryRead(dev, index, &validGet, &skipGet,
                                        &agedGet, &hwDevNumGet, &macEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                              "cpssDxChBrgFdbMacEntryRead: %d, %d", dev, index);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, macEntryGet.daSecurityLevel,
              "get another macEntryPtr->daSecurityLevel than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, macEntryGet.saSecurityLevel,
              "get another macEntryPtr->saSecurityLevel than was set: %d", dev);

        /*
            17. Call cpssDxChBrgFdbMacEntryInvalidate to invalidate all changes.
                  Disable configuration for Src ID 12 bits and using of VID1
                  in FDB entry
            Expected: GT_OK.
        */
        srcIdLengthInFDB = GT_FALSE;
        tag1VidFdbEn     = GT_FALSE;
                /* Disable configuration for Src ID 12 bits */
        st = cpssDxChBrgFdbMaxLengthSrcIdEnableSet (dev, srcIdLengthInFDB);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbMaxLengthSrcIdEnableSet: %d, %d",
                                dev, srcIdLengthInFDB);
                /* Disable using of VID1 in FDB entry */
        st = cpssDxChBrgFdbVid1AssignmentEnableSet (dev, tag1VidFdbEn);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChBrgFdbVid1AssignmentEnableSet: %d, %d",
                                dev, tag1VidFdbEn);
    }

    if(isValidTest == GT_FALSE)
    {
        /* register the test as skipped */
        SKIP_TEST_MAC
    }

}

/**
* @internal prvUtfCoreClockGet function
* @endinternal
*
* @brief   This routine returns core clock per device.
*
* @param[in] dev                      -  device id
*
* @param[out] coreClockPtr             -  (pointer to) core clock
*                                      GT_OK           -  Get core clock was OK
*                                      GT_BAD_PARAM    -  Invalid device id
*                                      GT_BAD_PTR      -  Null pointer
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS prvUtfCoreClockGet
(
    IN  GT_U8       dev,
    OUT GT_U32      *coreClockPtr
)
{
    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(coreClockPtr);

    /* check if dev active and from DxChx family */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    *coreClockPtr = PRV_CPSS_PP_MAC(dev)->coreClock;

    return(GT_OK);
}

/**
* @internal prvUtfMacTableSize function
* @endinternal
*
* @brief   This routine returns mac table size per device.
*
* @param[in] dev                      -  device id
*
* @param[out] tableSizePtr             - (pointer to) mac table size
*                                      GT_OK           - Get core clock was OK
*                                      GT_BAD_PARAM    - Invalid device id
*                                      GT_BAD_PTR      - Null pointer
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS prvUtfMacTableSize
(
    IN  GT_U8       dev,
    OUT GT_U32      *tableSizePtr
)
{
    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(tableSizePtr);

    /* check if dev active and from DxChx family */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    *tableSizePtr = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

    return(GT_OK);
}

/**
* @internal prvUtTestFdbHashCases function
* @endinternal
*
* @brief   This routine test all cases of CPSS FDB Hash function, according
*         to entry type.
* @param[in] dev                      -  device id
* @param[in] macKeyPtr                - pointer to mac entry key (entry type)
*/
static GT_STATUS prvUtTestFdbHashCases
(
    IN GT_U8 dev,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr
)
{
    GT_STATUS   st;
    CPSS_MAC_VL_ENT macVlanModesArr[] = {CPSS_SVL_E,CPSS_IVL_E};
    GT_U32  numOfMacVlanModes = sizeof(macVlanModesArr) / sizeof(macVlanModesArr[0]);
    CPSS_MAC_HASH_FUNC_MODE_ENT hashFuncModesArr[] =
            {CPSS_MAC_HASH_FUNC_XOR_E,
             CPSS_MAC_HASH_FUNC_CRC_E,
             CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E};
    GT_U32  numOfHashFuncModes = sizeof(hashFuncModesArr) / sizeof(hashFuncModesArr[0]);
    GT_U32  ii,jj,kk;
    GT_BOOL fdb16BitFidHashEnable;
    GT_BOOL useMultiHash = GT_FALSE;
    GT_U32  isDoubleTagKey    = 0; /* if 1 use double tag FDB Lookup key mode */
    GT_U32  skipIteration     = 0;

    char*       hashFuncModeStr[/*numOfHashFuncModes*/]=
        {"XOR","CRC","CRC multi hash"};
    char*       macVlanModeStr[/*numOfMacVlanModes*/]=
        {"SVL","IVL"};


    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(macKeyPtr);

    /* check if dev active and from DxChx family */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    isDoubleTagKey =
        (macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E   ||
         macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E ||
         macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E);

    /* Double Lookup Key mode is relevant for bobcat2 b0 and above only */
    if (isDoubleTagKey && !PRV_CPSS_SIP_5_10_CHECK_MAC(dev))
    {
        /* do nothing */
        return GT_OK;
    }

    for(kk = 0 ; kk < 2 ; kk++)
    {
        fdb16BitFidHashEnable = (0 == kk? GT_FALSE : GT_TRUE);

        /* check if we need to skip this iteration */
        skipIteration =
            /* legacy devices don't support 16b FID */
            (GT_TRUE == fdb16BitFidHashEnable  &&
             !UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            ||
            /* device with FDB > 64 must always work with
               fdb16BitFidHashEnable = GT_TRUE */
            (GT_FALSE == fdb16BitFidHashEnable &&
             PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb > _64K)
            ||
            /* FID is always 16b if double key FDB lookup key mode is used*/
            (isDoubleTagKey && GT_FALSE == fdb16BitFidHashEnable);

        if (skipIteration)
        {
            continue;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            st = cpssDxChBrgFdb16BitFidHashEnableSet(dev,fdb16BitFidHashEnable);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdb16BitFidHashEnableSet));
                return st;
            }
        }

        for(ii = 0; ii < numOfMacVlanModes; ii++)
        {
            /* double tag key can be used in IVL mode only */
            if (isDoubleTagKey && macVlanModesArr[ii] != CPSS_IVL_E)
            {
                continue;
            }

            /* set vlan mode to SVL/IVL */
            st = cpssDxChBrgFdbMacVlanLookupModeSet(dev, macVlanModesArr[ii]);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbMacVlanLookupModeSet));
                return st;
            }

            for(jj = 0; jj < numOfHashFuncModes; jj++)
            {
                if(PRV_CPSS_SIP_6_CHECK_MAC(dev) &&
                    hashFuncModesArr[jj] != CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
                {
                    /* sip6 device not supports this mode */
                    continue;
                }


                useMultiHash = (hashFuncModesArr[jj] ==
                                CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E) ?
                    GT_TRUE : GT_FALSE;

                skipIteration =
                    /* XOR and CRC hash are not used in double lookup key mode */
                    (isDoubleTagKey &&
                     hashFuncModesArr[jj] != CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E)
                    ||
                    /* only eArch devices support multi hash mode */
                    (GT_TRUE == useMultiHash &&
                     !UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev));

                if (skipIteration)
                {
                    continue;
                }

                /* set hash mode to XOR/CRC/multi-hash */
                st = cpssDxChBrgFdbHashModeSet(dev, hashFuncModesArr[jj]);
                if (st != GT_OK)
                {
                    utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbHashModeSet));
                    return st;
                }

                utfGeneralStateMessageSave(1,"hashFuncMode = %s ",hashFuncModeStr[jj]);
                utfGeneralStateMessageSave(2,"macVlanMode = %s ",macVlanModeStr[ii]);
                utfGeneralStateMessageSave(3,"fdb16BitFidHashEnable = %d ",fdb16BitFidHashEnable);

                /* test FDB Hash */
                PRV_UTF_LOG3_MAC("prvUtTestFdbHashCases: start %s %s fdb16BitFidHashEnable[%d]check\n",
                                hashFuncModeStr[jj],macVlanModeStr[ii],fdb16BitFidHashEnable);
                st = prvUtTestFdbHash(dev,
                    macKeyPtr,
                    useMultiHash,
                    fdb16BitFidHashEnable);

                UT_FDB_DUMP(("prvUtTestFdbHashCases: end \n"));

                if (st != GT_OK)
                {
                    return st;
                }
            }/*jj*/
        }/*ii*/
    }/*kk*/


    return GT_OK;
}


/**
* @internal fdbHashSingleEntryCheck function
* @endinternal
*
* @brief   This routine test CPSS FDB Hash function, according to entry type.
*
* @param[in] dev                      -  device id
* @param[in] entryPtr                 - pointer to FDB entry
* @param[in] calculatedHashIndex      - calculated hash index
* @param[in] invalidate               - indication to  the FDB entry
*/
static GT_STATUS fdbHashSingleEntryCheck
(
    IN GT_U8                       dev,
    IN CPSS_MAC_ENTRY_EXT_STC      *entryPtr,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC  *origMacKeyPtr,
    IN GT_U32                      calculatedHashIndex,
    IN GT_BOOL                     invalidate
)
{
    GT_STATUS   st;
    GT_BOOL                     valid;
    GT_BOOL                     skip;
    GT_BOOL                     aged;
    GT_HW_DEV_NUM               associatedHwDevNum = 0;
    GT_U32                      tableLength = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;/* FDB table size */
    CPSS_MAC_ENTRY_EXT_STC      entryForRead;

    cpssOsMemSet(&entryForRead, 0, sizeof(entryForRead));
    /* add the entry to FDB table */
    st = utfCpssDxChBrgFdbMacEntrySet(dev, entryPtr);
    if (st != GT_OK)
    {
        utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntrySet));
        return st;
    }

    /* wait for acknowledge that the set was done */
    utfCpssDxChBrgFdbFromCpuAuMsgStatusGet(dev,GT_TRUE);

    /* Read the entry by previously calculated hash index */
    st = utfCpssDxChBrgFdbMacEntryRead(dev, calculatedHashIndex, &valid, &skip, &aged,
                                    &associatedHwDevNum, &entryForRead);
    if (st != GT_OK)
    {
        utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryRead));
        return st;
    }

    /* check if the entry exists, if yes check that there is match between the keys */
    st = prvUtCheckMacEntry(valid, skip, &entryForRead, origMacKeyPtr);
    if (st != GT_OK)
    {
        utfGeneralStateMessageSave(7,"FAILED [%s]",STR(prvUtCheckMacEntry));

        PRV_UTF_LOG3_MAC(
            "index=%d, valid=%d, skip=%d",
            calculatedHashIndex, valid, skip);

        UT_FDB_DUMP(("prvUtCheckMacEntry: failed in hashIndex[%d] \n",
                     calculatedHashIndex));
        return st;
    }

    if(invalidate == GT_TRUE)
    {
        /* remove the entry from FDB */
        st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, calculatedHashIndex);
        if (st != GT_OK)
        {
            utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryInvalidate));
            return st;
        }

        if ((UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            && (valid == GT_TRUE) && (skip == GT_FALSE))
        {
            /* decrement Leaned MACs Counter */
            st = cpssDxChBrgFdbBankCounterUpdate(
                dev, (calculatedHashIndex / (tableLength / 16))/*bankIndex*/, GT_FALSE /*incOrDec*/);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbBankCounterUpdate));
                return st;
            }
        }

    }

    return GT_OK;
}

/**
* @internal prvUtTestFdbHash function
* @endinternal
*
* @brief   This routine test CPSS FDB Hash function, according to entry type.
*
* @param[in] dev                      -  device id
* @param[in] macKeyPtr                - pointer to mac entry key (entry type)
* @param[in] useMultiHash             - indication to use multi hash calculations.
*/
static GT_STATUS prvUtTestFdbHash
(
    IN GT_U8 dev,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC *macKeyPtr,
    IN GT_BOOL    useMultiHash,
    IN GT_BOOL    fdb16BitFidHashEnable
)
{
    GT_U32                      hashIndex;
    CPSS_MAC_ENTRY_EXT_STC      fdbEntry;
    GT_U32                      bitArray[3];
    GT_U32                      deltaWord0 = 0x10624d;
    GT_U32                      deltaWord1 = 0x10624;
    GT_U32                      deltaWord2 = 0x1;
    GT_U32                      i;
    GT_STATUS   st;
    GT_U32                      tableLength = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;/* FDB table size */
    static GT_U32               crcMultiHashArr[NUM_BANKS_MULTI_HASH_CNS];/*array for multiple hash results */
    static GT_U32               crcMultiHashByParamsArr[NUM_BANKS_MULTI_HASH_CNS];/*array for multiple hash results of by params func*/
    GT_U32                      numIterations;/* number of iteration to do to check hashing */
    GT_U32                      multiHashIndex;/* temporary index into crcMultiHashArr[] */
    GT_U32                      maxMultiHash = NUM_BANKS_MULTI_HASH_CNS;/* max valid values in crcMultiHashArr[] */
    static CPSS_MAC_ENTRY_EXT_STC dummyEntry;
    CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC  hashParams;
    CPSS_MAC_HASH_FUNC_MODE_ENT        hashMode;
    GT_U32 hashIndexByParams;
    GT_U32  isSip5;      /*flag of supporting SIP 5    devices*/
    GT_U32  isSip5_15;   /*flag of supporting SIP 5_15 devices*/
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT sip6FdbMacEntryMuxingMode = CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E;
    static GT_U32  sip6Vid1Mask[CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E] = {
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E -                                            */
     /* 12 bits of 'TAG_1_VID'                                                                       */
     0xfff,
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E -                                               */
     /* 12 bits of 'SRC_ID'                                                                          */
     0,
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E -                      */
     /* 10 bits of UDB (bits 1..10) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'  */
     0,
     /* 1 bit of 'SRC_ID' (bit 0)                                                                    */
     /* 1 bit of 'DA_ACCESS_LEVEL'                                                                   */
     /* CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_4_AND_4_SRC_ID_3_DA_ACCESS_LEVEL_E -                 */
     /* 4 bits of UDB (bits 1..4) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'    */
     /* and 4 bits of UDB (bits 7..10)                                                               */
     /* 3 bits of 'SRC_ID' (bit 0..2)                                                                */
     /* 1 bit of 'DA_ACCESS_LEVEL'                                                                   */
     0
    };
    GT_U32  used_sip6Vid1Mask = 0;

    /* initialize mac entry */
    cpssOsMemSet(&fdbEntry,0,sizeof(CPSS_MAC_ENTRY_EXT_STC));
    cpssOsMemSet(&dummyEntry,0,sizeof(CPSS_MAC_ENTRY_EXT_STC));

    /* initialize bit array - to build dummyEntry.key */
    bitArray[0] = 0xFFFFFFFF;
    bitArray[1] = 0xFFFFFFFF;
    bitArray[2] = 0xFFFFFFFF;
    prvUtBuildMacAddrKeyFromBitArray(dev,bitArray, &dummyEntry.key, fdb16BitFidHashEnable);
    dummyEntry.dstInterface.type = CPSS_INTERFACE_VID_E;

    /* initialize bit array */
    bitArray[0] = 0;
    bitArray[1] = 0;
    bitArray[2] = 0;

    isSip5    = PRV_CPSS_SIP_5_CHECK_MAC(dev);
    isSip5_15 = PRV_CPSS_SIP_5_15_CHECK_MAC(dev);

    /* initialize hashParams structure */
    hashParams.vlanMode = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.vlanMode;
    hashParams.crcHashUpperBitsMode = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.crcHashUpperBitsMode;
    hashParams.size = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.size;
    hashParams.fid16BitHashEn = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.fid16BitHashEn;

    if(hashParams.size < CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E)
    {
        /* BC2 behave different than Lion2,ch1,2,3,xcat1,2,3 and Bobk */
        hashParams.useZeroInitValueForCrcHash = (isSip5 & (!isSip5_15)) ?
            GT_TRUE : GT_FALSE;
    }
    else
    {
        /* Bobk behave different than Lion2 and Bc2 */
        hashParams.useZeroInitValueForCrcHash = (!isSip5_15) ? GT_TRUE : GT_FALSE;
    }

    hashMode = PRV_CPSS_DXCH_PP_MAC(dev)->bridge.fdbHashParams.hashMode;

    if (useMultiHash == GT_FALSE)
    {
        /* default for HW test */
        numIterations = tableLength;
    }
    else
    {
        /* default for HW test - divided to 32 only to save time */
        numIterations = tableLength / 32;
    }
#ifdef ASIC_SIMULATION
    /* reduce number of iterations in simulation */
    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        #ifdef _WIN32
        numIterations = 8;
        #else
        numIterations = 128;
        #endif
    }
    else
    {
        numIterations = _1K;
    }

    numIterations /= fdbHashIterationsReducedFactor;

    if(useMultiHash == GT_TRUE)
    {
        /* to reduce the number of iterations when multi-hash is used */
        /* because each multi hash is doing maxMultiHash iterations */
        numIterations /= maxMultiHash;
    }

    if(numIterations < 8)
    {
        /* set minimal number of iterations*/
        numIterations = 8;
    }
#else  /*! ASIC_SIMULATION*/
    numIterations /= fdbHashIterationsReducedFactor;
#endif /* ! ASIC_SIMULATION*/
    if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
    {
        numIterations = 2;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        st = cpssDxChBrgFdbMacEntryMuxingModeGet(dev,
            &sip6FdbMacEntryMuxingMode);
        if(st != GT_OK)
        {
            utfGeneralStateMessageSave(4,"FAILED [%s]",STR(cpssDxChBrgFdbMacEntryMuxingModeGet));
            return st;
        }

        if(sip6FdbMacEntryMuxingMode < CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E )
        {
            used_sip6Vid1Mask = sip6Vid1Mask[sip6FdbMacEntryMuxingMode];
        }
    }

    PRV_UTF_LOG1_MAC("\n ***** test will perform %d iterations ***** \n", numIterations);

    /* build MAC Address from bit array */
    for (i = 0; i < numIterations; i++)
    {
        utfGeneralStateMessageSave(4,"iteration = %d",i);

#ifdef ASIC_SIMULATION
        if((i & 0x0FF) == 0)/* every 256 iterations */
        {
            /* show that the function alive */
            utfPrintKeepAlive();
        }
#endif /*ASIC_SIMULATION*/

        prvUtBuildMacAddrKeyFromBitArray(dev,bitArray, macKeyPtr,fdb16BitFidHashEnable);

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* the vid support depends on the muxing mode */
            dummyEntry.key.vid1 &= used_sip6Vid1Mask;
            macKeyPtr->vid1     &= used_sip6Vid1Mask;
        }

        fdbEntry.key = *macKeyPtr;

        /* Set destination interface type to be VID */
        fdbEntry.dstInterface.type = CPSS_INTERFACE_VID_E;

        if(useMultiHash == GT_FALSE)
        {
            /* calculate the entry hash index by CPSS hash algorithm */
            st = cpssDxChBrgFdbHashCalc(dev, macKeyPtr, &hashIndex);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbHashCalc));
                return st;
            }

            utfGeneralStateMessageSave(5,"hashIndex = 0x%5.5x",hashIndex);

            /* check this single entry */
            st = fdbHashSingleEntryCheck(dev,&fdbEntry,macKeyPtr,hashIndex,GT_TRUE);


            if (st != GT_OK)
            {
                return st;
            }

            st = cpssDxChBrgFdbHashByParamsCalc(hashMode, &hashParams, macKeyPtr, &hashIndexByParams);

            /*check hashIndex cpssDxChBrgFdbHashByParamsCalc and cpssDxChBrgFdbHashCalc*/
            if( hashIndex != hashIndexByParams )
            {
                utfGeneralStateMessageSave(7,"FAILED:results of cpssDxChBrgFdbHashCalc[0x%5.5x] and cpssDxChBrgFdbHashByParamsCalc[0x%5.5x] not the same"
                                           ,hashIndex, hashIndexByParams);
                return st;
            }
        }
        else
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev) &&
               ((macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E) ||
                (macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E)))
            {
                /*sip*/
                GT_U32  sip32 = (macKeyPtr->key.ipMcast.sip[0] << 24) |
                          (macKeyPtr->key.ipMcast.sip[1] << 16) |
                          (macKeyPtr->key.ipMcast.sip[2] <<  8) |
                          (macKeyPtr->key.ipMcast.sip[3] <<  0) ;
                GT_U32   vlanId = macKeyPtr->key.ipMcast.vlanId;

                if(prvCpssDxchFalconCiderVersionGet() <= PRV_CPSS_DXCH_FALCON_CIDER_VERSION_18_02_22_E)
                {
                    if((vlanId & 1) != ((sip32 >> 26) & 1))
                    {
                        /* JIRA : MT-358 - in the MC-IP update message : Bit 77 is part of 2 fields (non-muxed fields) : fid and sip[26:0] */
                        macKeyPtr->key.ipMcast.vlanId ^= 1;/* flip bit 0 to allow check other problems in the entry */
                        fdbEntry.key.key.ipMcast.vlanId = macKeyPtr->key.ipMcast.vlanId;
                    }
                }
            }

            /* we need to calculate the multiple hash results */
            st = cpssDxChBrgFdbHashCrcMultiResultsCalc(dev,
                        macKeyPtr,
                        0,/*multiHashStartBankIndex*/
                        NUM_BANKS_MULTI_HASH_CNS,/*numOfBanks*/
                        crcMultiHashArr);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbHashCrcMultiResultsCalc));
                return st;
            }

            st = cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(&hashParams,
                        macKeyPtr,
                        0,/*multiHashStartBankIndex*/
                        NUM_BANKS_MULTI_HASH_CNS,/*numOfBanks*/
                        crcMultiHashByParamsArr);
            if (st != GT_OK)
            {
                utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc));
                return st;
            }

            /* occupy all 16 possible entries */
            for(multiHashIndex = 0 ; multiHashIndex  < maxMultiHash; multiHashIndex++)
            {
                hashIndex = crcMultiHashArr[multiHashIndex];
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, hashIndex ,GT_FALSE,&dummyEntry);
                if (st != GT_OK)
                {
                    utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryWrite));
                    return st;
                }
            }

            /* allow to check that ALL 16 hash results that we calculated
                for the entry are ok. */
            for(multiHashIndex = 0 ; multiHashIndex  < maxMultiHash; multiHashIndex++)
            {
                hashIndex = crcMultiHashArr[multiHashIndex];

                utfGeneralStateMessageSave(5,"hashIndex = 0x%5.5x",hashIndex);
                utfGeneralStateMessageSave(6,"multiHashIndex = %d",multiHashIndex);

                /* free the alone place to cause it be selected by HW getting NA message from CPU */
                st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, hashIndex);
                if (st != GT_OK)
                {
                    utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryInvalidate));
                    return st;
                }

                /* check this single entry - writing entry via NA message from CPU */
                /* and reading by hashIndex                                        */
                st = fdbHashSingleEntryCheck(dev,&fdbEntry,macKeyPtr,hashIndex,GT_FALSE);
                if (st != GT_OK)
                {
                    return st;
                }

                /* override the checked entry by dummy entry */
                st = utfCpssDxChBrgFdbMacEntryWrite(dev, hashIndex ,GT_FALSE,&dummyEntry);
                if (st != GT_OK)
                {
                    utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryWrite));
                    return st;
                }
            }

            /* now we are ready to invalidate the entries */
            for(multiHashIndex = 0 ; multiHashIndex  < maxMultiHash; multiHashIndex++)
            {
                hashIndex = crcMultiHashArr[multiHashIndex];

                utfGeneralStateMessageSave(4,"hashIndex = 0x%5.5x",hashIndex);
                utfGeneralStateMessageSave(5,"multiHashIndex = %d",multiHashIndex);
                /* remove the dummy entries from the FDB */
                st = utfCpssDxChBrgFdbMacEntryInvalidate(dev, hashIndex);
                if (st != GT_OK)
                {
                    utfGeneralStateMessageSave(7,"FAILED [%s]",STR(utfCpssDxChBrgFdbMacEntryInvalidate));
                    return st;
                }

                /* decrement the bank counter ... since counter incremented by the
                   PP during utfCpssDxChBrgFdbMacEntrySet(...) */
                if (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
                {
                    /* decrement Leaned MACs Counter */
                    st = cpssDxChBrgFdbBankCounterUpdate(
                        dev, (hashIndex / (tableLength / 16))/*bankIndex*/, GT_FALSE /*incOrDec*/);
                    if (st != GT_OK)
                    {
                        utfGeneralStateMessageSave(7,"FAILED [%s]",STR(cpssDxChBrgFdbBankCounterUpdate));
                        return st;
                    }
                }
            }
            /*now we check results cpssDxChBrgFdbHashCrcMultiResultsCalc and cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc   */
            for(multiHashIndex = 0 ; multiHashIndex  < maxMultiHash; multiHashIndex++)
            {
                if (crcMultiHashArr[multiHashIndex] != crcMultiHashByParamsArr[multiHashIndex])
                {
                    utfGeneralStateMessageSave(7,"FAILED results of cpssDxChBrgFdbHashCrcMultiResultsCalc and cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc not the same");
                    return st;
                }
            }
        }

        /* increase bit array */
        bitArray[0] += deltaWord0;
        bitArray[1] += deltaWord1;
        bitArray[2] += deltaWord2;
    }

    return GT_OK;
}

/**
* @internal prvUtBuildMacAddrKeyFromBitArray function
* @endinternal
*
* @brief   This routine build MAC entry key from bit array, according to entry type.
*
* @param[in] bitArray[3]              - bit array
* @param[in,out] macKeyPtr                - mac entry key (entry type)
* @param[in] fdb16BitFidHashEnable    - indication to use 16 bit of the FID (vlanId)
* @param[in,out] macKeyPtr                - mac entry key
*                                      COMMENTS:
*                                      None.
*/
static void prvUtBuildMacAddrKeyFromBitArray
(
    IN GT_U8                            dev,
    IN  GT_U32                          bitArray[3],
    INOUT CPSS_MAC_ENTRY_EXT_KEY_STC    *macKeyPtr,
    IN GT_BOOL    fdb16BitFidHashEnable
)
{
    GT_U32  fidMask = fdb16BitFidHashEnable ?
        PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(dev) :/*currently CPSS uses vid check*/
        0x0FFF;

    if (macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E ||
        macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E)
    {
        /* build MAC Address */
        macKeyPtr->key.macVlan.macAddr.arEther[0] = (GT_U8)(bitArray[0] & 0xFF);
        macKeyPtr->key.macVlan.macAddr.arEther[1] = (GT_U8)((bitArray[0] >> 8) & 0xFF);
        macKeyPtr->key.macVlan.macAddr.arEther[2] = (GT_U8)((bitArray[0] >> 16) & 0xFF);
        macKeyPtr->key.macVlan.macAddr.arEther[3] = (GT_U8)((bitArray[0] >> 24) & 0xFF);
        macKeyPtr->key.macVlan.macAddr.arEther[4] = (GT_U8)(bitArray[1] & 0xFF);
        macKeyPtr->key.macVlan.macAddr.arEther[5] = (GT_U8)((bitArray[1] >> 8) & 0xFF);
        /* Vlan ID */
        macKeyPtr->key.macVlan.vlanId = (GT_U16)((bitArray[1] >> 16) & fidMask);
        /* VID1  */
        macKeyPtr->vid1 = (macKeyPtr->key.macVlan.vlanId + 5) & 0xFFF;

        utfGeneralStateMessageSave(11,"mac addr : [%x:%x:%x:%x:%x:%x]",
            macKeyPtr->key.macVlan.macAddr.arEther[0],
            macKeyPtr->key.macVlan.macAddr.arEther[1],
            macKeyPtr->key.macVlan.macAddr.arEther[2],
            macKeyPtr->key.macVlan.macAddr.arEther[3],
            macKeyPtr->key.macVlan.macAddr.arEther[4],
            macKeyPtr->key.macVlan.macAddr.arEther[5]
            );
        utfGeneralStateMessageSave(12,"----");
        utfGeneralStateMessageSave(13,"vlanId [%d]",macKeyPtr->key.macVlan.vlanId);
    }
    else
    {
        /* build IP Address*/
        /* Source IP */
        macKeyPtr->key.ipMcast.sip[0] = (GT_U8)(bitArray[0] & 0xFF);
        macKeyPtr->key.ipMcast.sip[1] = (GT_U8)((bitArray[0] >> 8) & 0xFF);
        macKeyPtr->key.ipMcast.sip[2] = (GT_U8)((bitArray[0] >> 16) & 0xFF);
        macKeyPtr->key.ipMcast.sip[3] = (GT_U8)((bitArray[0] >> 24) & 0xFF);
        utfGeneralStateMessageSave(11,"SIP [%d.%d.%d.%d]",
            macKeyPtr->key.ipMcast.sip[0],
            macKeyPtr->key.ipMcast.sip[1],
            macKeyPtr->key.ipMcast.sip[2],
            macKeyPtr->key.ipMcast.sip[3]
        );
        /* Destination IP */
        macKeyPtr->key.ipMcast.dip[0] = (GT_U8)(bitArray[1] & 0xFF);
        macKeyPtr->key.ipMcast.dip[1] = (GT_U8)((bitArray[1] >> 8) & 0xFF);
        macKeyPtr->key.ipMcast.dip[2] = (GT_U8)((bitArray[1] >> 16) & 0xFF);
        macKeyPtr->key.ipMcast.dip[3] = (GT_U8)((bitArray[1] >> 24) & 0xFF);
        utfGeneralStateMessageSave(12,"DIP [%d.%d.%d.%d]",
            macKeyPtr->key.ipMcast.dip[0],
            macKeyPtr->key.ipMcast.dip[1],
            macKeyPtr->key.ipMcast.dip[2],
            macKeyPtr->key.ipMcast.dip[3]
        );
        /* Vlan ID */
        macKeyPtr->key.ipMcast.vlanId = (GT_U16)(bitArray[2] & fidMask);
        utfGeneralStateMessageSave(13,"vlanId [%d]",macKeyPtr->key.macVlan.vlanId);
        /* VID1 */
        macKeyPtr->vid1 = (macKeyPtr->key.ipMcast.vlanId + 3) & 0xFFF;
    }
    utfGeneralStateMessageSave(14,"vid1 [%d]",macKeyPtr->vid1);

}

/**
* @internal prvUtCheckMacEntry function
* @endinternal
*
* @brief   This routine checks if the entry exists in FDB in the index created
*         by CPSS, if yes check that there a match between entries keys.
* @param[in] valid                    -  entry validity
* @param[in] skip                     -  entry  bit
*                                      entry       - mac entry
* @param[in] macKeyPtr                - mac entry key
*
* @param[out] macKeyPtr                - mac entry key
*                                      GT_OK           - the test was ok
*                                      GT_FAIL         - on fail
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS prvUtCheckMacEntry
(
    IN GT_BOOL                      valid,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC      *entryPtr,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC  *macKeyPtr
)
{

    if (valid == GT_FALSE)
    {
        return GT_FAIL;
    }

    if (skip == GT_TRUE)
    {
        return GT_FAIL;
    }
    if (macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E ||
        macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E)
    {
        /* compare MAC Address entries */
        if ((entryPtr->key.key.macVlan.macAddr.arEther[0] !=
            macKeyPtr->key.macVlan.macAddr.arEther[0]) ||
            (entryPtr->key.key.macVlan.macAddr.arEther[1] !=
            macKeyPtr->key.macVlan.macAddr.arEther[1]) ||
            (entryPtr->key.key.macVlan.macAddr.arEther[2] !=
            macKeyPtr->key.macVlan.macAddr.arEther[2]) ||
            (entryPtr->key.key.macVlan.macAddr.arEther[3] !=
            macKeyPtr->key.macVlan.macAddr.arEther[3]) ||
            (entryPtr->key.key.macVlan.macAddr.arEther[4] !=
            macKeyPtr->key.macVlan.macAddr.arEther[4]) ||
            (entryPtr->key.key.macVlan.macAddr.arEther[5] !=
            macKeyPtr->key.macVlan.macAddr.arEther[5]) ||
            (entryPtr->key.key.macVlan.vlanId !=
             macKeyPtr->key.macVlan.vlanId))
        {
            return GT_FAIL;
        }
    }
    else
    {
        /* compare MAC Address entries */
        if ((entryPtr->key.key.ipMcast.sip[0] != macKeyPtr->key.ipMcast.sip[0]) ||
            (entryPtr->key.key.ipMcast.sip[1] != macKeyPtr->key.ipMcast.sip[1]) ||
            (entryPtr->key.key.ipMcast.sip[2] != macKeyPtr->key.ipMcast.sip[2]) ||
            (entryPtr->key.key.ipMcast.sip[3] != macKeyPtr->key.ipMcast.sip[3]) ||
            (entryPtr->key.key.ipMcast.dip[0] != macKeyPtr->key.ipMcast.dip[0]) ||
            (entryPtr->key.key.ipMcast.dip[1] != macKeyPtr->key.ipMcast.dip[1]) ||
            (entryPtr->key.key.ipMcast.dip[2] != macKeyPtr->key.ipMcast.dip[2]) ||
            (entryPtr->key.key.ipMcast.dip[3] != macKeyPtr->key.ipMcast.dip[3]) ||
            (entryPtr->key.key.ipMcast.vlanId != macKeyPtr->key.ipMcast.vlanId))
        {
            return GT_FAIL;
        }
    }
    if ((macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E ||
         macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E ||
         macKeyPtr->entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E)
        &&
        entryPtr->key.vid1 != macKeyPtr->vid1)
        {
            return GT_FAIL;
        }


    return GT_OK;
}

#ifdef ASIC_SIMULATION
GT_STATUS utfMaxRetryOnSleep0Set
(
    IN GT_U32   maxRetry
)
{
    utfMaxRetryOnSleep0 = maxRetry;
    return GT_OK;
}
#endif /*ASIC_SIMULATION*/


/*----------------------------------------------------------------------------*/
/* helper for wait until PP is ready by calling*/
static GT_VOID utfCpssDxChBrgFdbBankCounterUpdateStatusGet
(
    IN GT_U8    devNum
)
{
    GT_STATUS   st              = GT_OK;
    GT_BOOL     isFinished      = GT_FALSE;

#ifdef ASIC_SIMULATION
GT_U32  sleepTime = 0;
GT_U32  maxRetry = utfMaxRetryOnSleep0;
GT_U32  totalMaxRetry = utfMaxRetryOnSleep1;
#endif/*ASIC_SIMULATION*/


    st = cpssDxChBrgFdbBankCounterUpdateStatusGet(devNum, &isFinished);

#ifdef ASIC_SIMULATION
    if(utfMaxRetryOnSleep0 == 0)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, isFinished, devNum);
        if(GT_TRUE != isFinished)
        {
            return;
        }
    }
#endif/*ASIC_SIMULATION*/

    do{

#ifdef ASIC_SIMULATION
        if ((sleepTime == 0) && maxRetry)
        {
            /* do max retries with 'busy wait' but with option to other tasks
               to also run --> reduce run time */
            maxRetry--;
        }
        else if (maxRetry == 0)
        {
            sleepTime = 1;
        }

        totalMaxRetry--;
        if(totalMaxRetry == 0)
        {
            PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbBankCounterUpdateStatusGet Timed out after [%d] seconds \n", 10);
        }
        else if((totalMaxRetry % 1000) == 0)
        {
            /* show the wait process , every 1 seconds */
            PRV_UTF_LOG0_MAC("@");
        }

        osTimerWkAfter(sleepTime);
#endif /*ASIC_SIMULATION*/

        st = cpssDxChBrgFdbBankCounterUpdateStatusGet(devNum, &isFinished);
        if(st != GT_OK)
        {
            PRV_UTF_LOG1_MAC("[TGF]: utfCpssDxChBrgFdbBankCounterUpdateStatusGet FAILED, rc = [%d] \n", st);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            return;
        }
    } while (isFinished == GT_FALSE);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaLookupSkipModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaLookupSkipModeSet)
{
/*
    1. Go over all active devices.
    1.1. Call function with saLookupMode [CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E / CPSS_DXCH_FDB_SA_MATCHING_MODE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgFdbSaLookupSkipModeGet
    Expected: GT_OK and the same saLookupModePtr.
    1.3. Call function with out of range saLookupMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS          st      = GT_OK;
    GT_U8              dev;
    CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupMode = CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E;
    CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupModeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with saLookupMode [CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E / CPSS_DXCH_FDB_SA_MATCHING_MODE_E]
            Expected: GT_OK.
        */

        /* 1.1. Call with saLookupMode [CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E] */
        saLookupMode = CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E;

        st = cpssDxChBrgFdbSaLookupSkipModeSet(dev, saLookupMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, saLookupMode);

        /*
            1.2. Call cpssDxChBrgFdbSaLookupSkipModeGet
            Expected: GT_OK and the same modePtr.
        */
        st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, &saLookupModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSaLookupSkipModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(saLookupMode, saLookupModeGet,
                       "get another modePtr than was set: %d", dev);

        /* 1.1. Call with aLookupMode [CPSS_DXCH_FDB_SA_MATCHING_MODE_E] */
        saLookupMode = CPSS_DXCH_FDB_SA_MATCHING_MODE_E;

        st = cpssDxChBrgFdbSaLookupSkipModeSet(dev, saLookupMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, saLookupMode);

        /*
            1.2. Call cpssDxChBrgFdbSaLookupSkipModeGet
            Expected: GT_OK and the same saLookupMode.
        */
        st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, &saLookupModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgFdbSaLookupSkipModeGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(saLookupModeGet, saLookupModeGet,
                       "get another modePtr than was set: %d", dev);

        /*
            1.3. Call function with wrong enum values saLookupMode.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbSaLookupSkipModeSet
                                                        (dev, saLookupMode),
                                                        saLookupMode);
    }

    saLookupMode = CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaLookupSkipModeSet(dev, saLookupMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaLookupSkipModeSet(dev, saLookupMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaLookupSkipModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaLookupSkipModeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null modePtr.
    Expected: GT_OK.
    1.2. Call with null modePtr [NULL].
    Expected: GT_BAD_PTR.
    2. For not-active devices and devices from non-applicable family
    2.1 check that function returns GT_BAD_PARAM
    3. Call function with out of bound value for device id.
    3.1 check that function returns GT_BAD_PARAM
*/
    GT_STATUS        st  = GT_OK;

    GT_U8            dev;
    CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null saLookupModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, &saLookupMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null saLookupModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, &saLookupMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaLookupSkipModeGet(dev, &saLookupMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaDaCmdDropModeSet
(
    IN  GT_U8                  devNum,
    IN  CPSS_MAC_TABLE_CMD_ENT saDropCmdMode,
    IN  CPSS_MAC_TABLE_CMD_ENT daDropCmdMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaDaCmdDropModeSet)
{
    GT_STATUS              st      = GT_OK;
    GT_U8                  dev;
    CPSS_MAC_TABLE_CMD_ENT saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
    CPSS_MAC_TABLE_CMD_ENT daDropCmdMode = CPSS_MAC_TABLE_DROP_E;
    CPSS_MAC_TABLE_CMD_ENT saDropCmdModeGet;
    CPSS_MAC_TABLE_CMD_ENT daDropCmdModeGet;
    CPSS_MAC_TABLE_CMD_ENT saDropCmdModeRestore;
    CPSS_MAC_TABLE_CMD_ENT daDropCmdModeRestore;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save modes to restore after test */
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(
            dev, &saDropCmdModeRestore, &daDropCmdModeRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChBrgFdbSaDaCmdDropModeGet failed");

        /* first set and check */
        saDropCmdMode = CPSS_MAC_TABLE_SOFT_DROP_E;
        daDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        st = cpssDxChBrgFdbSaDaCmdDropModeSet(dev, saDropCmdMode, daDropCmdMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, saDropCmdMode, daDropCmdMode);
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(
            dev, &saDropCmdModeGet, &daDropCmdModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChBrgFdbSaDaCmdDropModeGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            saDropCmdMode, saDropCmdModeGet, "get value differs from set value");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            daDropCmdMode, daDropCmdModeGet, "get value differs from set value");

        /* second set and check */
        saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        daDropCmdMode = CPSS_MAC_TABLE_SOFT_DROP_E;
        st = cpssDxChBrgFdbSaDaCmdDropModeSet(dev, saDropCmdMode, daDropCmdMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, saDropCmdMode, daDropCmdMode);
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(
            dev, &saDropCmdModeGet, &daDropCmdModeGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChBrgFdbSaDaCmdDropModeGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            saDropCmdMode, saDropCmdModeGet, "get value differs from set value");
        UTF_VERIFY_EQUAL0_STRING_MAC(
            daDropCmdMode, daDropCmdModeGet, "get value differs from set value");

        /* restore modes after test */
        st = cpssDxChBrgFdbSaDaCmdDropModeSet(
            dev, saDropCmdModeRestore, daDropCmdModeRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "cpssDxChBrgFdbSaDaCmdDropModeSet failed");

        /*
            1.3. Call function with wrong enum values saLookupMode.
            Expected: GT_BAD_PARAM.
        */
        saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        daDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        UTF_ENUMS_CHECK_MAC(
            cpssDxChBrgFdbSaDaCmdDropModeSet(dev, saDropCmdMode, daDropCmdMode),
            saDropCmdMode);
        saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        daDropCmdMode = CPSS_MAC_TABLE_DROP_E;
        UTF_ENUMS_CHECK_MAC(
            cpssDxChBrgFdbSaDaCmdDropModeSet(dev, saDropCmdMode, daDropCmdMode),
            daDropCmdMode);
    }

    saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
    daDropCmdMode = CPSS_MAC_TABLE_DROP_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaDaCmdDropModeSet(
            dev, saDropCmdMode, daDropCmdMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaDaCmdDropModeSet(
        dev, saDropCmdMode, daDropCmdMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbSaDaCmdDropModeGet
(
    IN  GT_U8                  devNum,
    OUT CPSS_MAC_TABLE_CMD_ENT *saDropCmdModePtr,
    OUT CPSS_MAC_TABLE_CMD_ENT *daDropCmdModePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbSaDaCmdDropModeGet)
{
    GT_STATUS              st      = GT_OK;
    GT_U8                  dev;
    CPSS_MAC_TABLE_CMD_ENT saDropCmdMode = CPSS_MAC_TABLE_DROP_E;
    CPSS_MAC_TABLE_CMD_ENT daDropCmdMode = CPSS_MAC_TABLE_DROP_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(dev, &saDropCmdMode, &daDropCmdMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with null saLookupModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(dev, NULL, &daDropCmdMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(dev, &saDropCmdMode, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgFdbSaDaCmdDropModeGet(dev, &saDropCmdMode, &daDropCmdMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbSaDaCmdDropModeGet(dev, &saDropCmdMode, &daDropCmdMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgFdb suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgFdb)
    /* run this test before other tests may modify the 'needed' configurations */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgingTimeoutSet__auto_aging_functionality)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaToCpuPerPortSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortLearnStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortLearnStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaToCpuPerPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaStormPreventSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaStormPreventGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDeviceTableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDeviceTableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntrySet_Multi_hash_mode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRouteEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbQaSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRouteEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMaxLookupLenSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMaxLookupLenGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacVlanLookupModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacVlanLookupModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAuMsgRateLimitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAuMsgRateLimitGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaMsgOnChainTooLongSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaMsgOnChainTooLongGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSpAaMsgToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSpAaMsgToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAAandTAToCpuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAAandTAToCpuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgingTimeoutSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgingTimeoutGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSecureAgingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSecureAgingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSecureAutoLearnSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSecureAutoLearnGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticTransEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticTransEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticDelEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticDelEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionsEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionsEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbTrigActionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacTriggerModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacTriggerModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticOfNonExistDevRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbStaticOfNonExistDevRemoveGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDropAuEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDropAuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionTransplantDataSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionTransplantDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbFromCpuAuMsgStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveVlanSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveVlanGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveDevSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveDevGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveInterfaceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveInterfaceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbUploadEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbUploadEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbTrigActionStart)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacTriggerToggle)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAuMsgBlockGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbFuMsgBlockGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbTrunkAgingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbTrunkAgingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeBitDaRefreshEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAgeBitDaRefreshEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbAuqFuqMessagesNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutedLearningEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbRoutedLearningEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryAgeBitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMacEntryStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbQueueFullGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbQueueRewindStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSecondaryAuMsgBlockGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbTriggerAuQueueWa)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashByParamsCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaMsgVid1EnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbNaMsgVid1EnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdb16BitFidHashEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdb16BitFidHashEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMaxLengthSrcIdEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMaxLengthSrcIdEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMessagesQueueManagerInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaLookupAnalyzerIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaLookupAnalyzerIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDaLookupAnalyzerIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbDaLookupAnalyzerIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveUserDefinedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbActionActiveUserDefinedGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbLearnPrioritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbLearnPriorityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbUserGroupSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbUserGroupGet)

    /* portGroup tests */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupQaSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryInvalidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupMacEntryAgeBitSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupTrigActionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortGroupQueueFullGet)

    /* Test filling FDB Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFillFdbTable)
    /* start FDB hash tests */
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_8K    )
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_16K   )
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_32K   )
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_64K   )
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_128K  )
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChTestFdbHash_256K  )
    /* end FDB hash tests */

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashCrcMultiResultsCalc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashRequestSend)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbHashResultsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbCrcHashUpperBitsModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbCrcHashUpperBitsModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbBankCounterValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbBankCounterUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbBankCounterUpdateStatusGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortVid1LearningEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbPortVid1LearningEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbVid1AssignmentEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbVid1AssignmentEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbMuxedFields)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaLookupSkipModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaLookupSkipModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaDaCmdDropModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbSaDaCmdDropModeGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChBrgFdb)


