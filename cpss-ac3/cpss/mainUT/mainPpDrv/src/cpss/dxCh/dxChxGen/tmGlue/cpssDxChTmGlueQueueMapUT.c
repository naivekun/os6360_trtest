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
* @file cpssDxChTmGlueQueueMapUT.c
*
* @brief Unit tests for Traffic Manager Glue
* Queue Mapping API.
*
* @version   7
********************************************************************************
*/
/* includes */

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueQueueMap.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */

#define PRV_TGF_NOT_TM_DEV_CNS \
    (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | \
    UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E)

#ifdef IS_64BIT_OS
        #define ADDR_STAMP_CNS (void*)0xFEFEFEFEFEFEFEFE
#else
        #define ADDR_STAMP_CNS (void*)0xFEFEFEFE
#endif /* IS_64BIT_OS */

#define REP_ADDR_STAMP_MAC(_arr, _member, _address)              \
    {                                                            \
        GT_U32 __dim = (sizeof(_arr) / sizeof(_arr[0]));         \
        GT_U32 __i;                                              \
        for (__i = 0; (__i < __dim); __i++)                      \
        {                                                        \
            if (_arr[__i]._member == ADDR_STAMP_CNS)             \
            {                                                    \
                _arr[__i]._member = _address;                    \
            }                                                    \
        }                                                        \
    }

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapBypassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   bypass
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapBypassEnableSet)
{
/*
    ITERATE_TM_DEVICES
    1.1. Call function with bypass [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call function cpssDxChTmGlueQueueMapBypassEnableSet with not NULL bypassPtr
         and other params from 1.1.
    Expected: GT_OK and the same bypass as was set.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     bypass = GT_FALSE;
    GT_BOOL     bypassGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with blockNum [0 / (CNC_BLOCKS_NUM - 1)],
                                    client [CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E /
                                            CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E],
                                    updateEnable [GT_FALSE / GT_TRUE].
            Expected: GT_OK.
        */
        /* iterate with  bypass = GT_TRUE */
        bypass = GT_TRUE;

        st = cpssDxChTmGlueQueueMapBypassEnableSet(dev, bypass);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bypass);

        st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, &bypassGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(bypass, bypassGet);

        /* iterate with  bypass = GT_FALSE */
        bypass = GT_FALSE;

        st = cpssDxChTmGlueQueueMapBypassEnableSet(dev, bypass);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bypass);

        st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, &bypassGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(bypass, bypassGet);

    }

    bypass = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapBypassEnableSet(dev, bypass);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapBypassEnableSet(dev, bypass);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapBypassEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *bypassPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapBypassEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call function with not NULL bypassPtr.
    Expected: GT_OK.
    1.2. Call function with bypassPtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     bypass = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not NULL bypassPtr.
            Expected: GT_OK.
        */

        st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, &bypass);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.4. Call function with updateEnablePtr [NULL]
                                    and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, &bypass);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapBypassEnableGet(dev, &bypass);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapBitSelectTableEntrySet
(
    IN  GT_U8                                              devNum,
    IN  GT_U32                                             entryIndex,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapBitSelectTableEntrySet)
{
    GT_STATUS                                          st = GT_OK;
    GT_U8                                              dev;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   entryGet;
    GT_U32                                             i;
    GT_32                                              compareRc;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   entry;
    struct
    {
        GT_BOOL                                             checkByGet;
        GT_STATUS                                           exspectedRc;
        GT_U32                                              entryIndex;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC*   entryPtr;
        GT_U32                                              queueIdBase;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT     selectType0;
        GT_U32                                              bitSelector0;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, 1, ADDR_STAMP_CNS, 2,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E, 3},
        /* entry index out of table size */
        {GT_FALSE, GT_OUT_OF_RANGE, 0x0FFFFFFF, ADDR_STAMP_CNS, 2,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E, 3},
        /* null pointer */
        {GT_FALSE,   GT_BAD_PTR, 1, NULL, 2,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E, 3},
        /* out of range queueIdBase */
        {GT_FALSE,   GT_OUT_OF_RANGE, 1, ADDR_STAMP_CNS, 0x0FFFFFFF,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E, 3},
        /* wrong selectType0 */
        {GT_FALSE,   GT_BAD_PARAM, 1, ADDR_STAMP_CNS, 2,
            (CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT)0xFF, 3},
        /* out of range  bitSelector0*/
        {GT_FALSE,   GT_OUT_OF_RANGE, 1, ADDR_STAMP_CNS, 2,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E, 0x0FFFFFFF},
        /* correct parameters - last test must leave enctry contents correct */
        {GT_TRUE,   GT_OK, 0, ADDR_STAMP_CNS, 0,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E, 0}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, entryPtr, &entry);

    cpssOsMemSet(&entry, 0, sizeof(entry));
    cpssOsMemSet(&entryGet, 0, sizeof(entryGet));

    /* prepare entry, parts will be updated */
    entry.queueIdBase = 0;
    for (i = 0; (i < 14); i++)
    {
        entry.bitSelectArr[i].selectType =
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E;
        entry.bitSelectArr[i].bitSelector = i;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            entry.queueIdBase                 = paramOfCase[i].queueIdBase;
            entry.bitSelectArr[0].selectType  = paramOfCase[i].selectType0;
            entry.bitSelectArr[0].bitSelector = paramOfCase[i].bitSelector0;

            st = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(
                dev, paramOfCase[i].entryIndex, paramOfCase[i].entryPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapBitSelectTableEntryGet(
                dev, paramOfCase[i].entryIndex, &entryGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            compareRc = cpssOsMemCmp(
                paramOfCase[i].entryPtr, &entryGet, sizeof(entryGet));
            UTF_VERIFY_EQUAL1_PARAM_MAC(0, compareRc, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(
            dev, 0, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(
        dev, 0, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapBitSelectTableEntryGet
(
    IN  GT_U8                                              devNum,
    IN  GT_U32                                             entryIndex,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   *entryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapBitSelectTableEntryGet)
{
    GT_STATUS                                          st = GT_OK;
    GT_U8                                              dev;
    GT_U32                                             i;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC   entry;
    struct
    {
        GT_STATUS                                           exspectedRc;
        GT_U32                                              entryIndex;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC*   entryPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, 1, ADDR_STAMP_CNS},
        /* entry index out of table size */
        {GT_OUT_OF_RANGE, 0x0FFFFFFF, ADDR_STAMP_CNS},
        /* null pointer */
        {GT_BAD_PTR, 1, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, entryPtr, &entry);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapBitSelectTableEntryGet(
                dev, paramOfCase[i].entryIndex, paramOfCase[i].entryPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapBitSelectTableEntryGet(
            dev, 0, &entry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapBitSelectTableEntryGet(
        dev, 0, &entry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet
(
    IN  GT_U8           devNum,
    IN  GT_HW_DEV_NUM   targetHwDevId,
    IN  GT_U32          entryIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     entryIndexGet;
    GT_U32     i;

    struct
    {
        GT_BOOL       checkByGet;
        GT_STATUS     exspectedRc;
        GT_HW_DEV_NUM targetHwDevId;
        GT_U32        entryIndex;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, 1, 2},
        /* bad target device ID */
        {GT_FALSE, GT_OUT_OF_RANGE, 0x0FFFFFFF, 2},
        /* out of range index */
        {GT_FALSE, GT_OUT_OF_RANGE, 0, 0x0FFFFFFF}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet(
                dev, paramOfCase[i].targetHwDevId, paramOfCase[i].entryIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet(
                dev, paramOfCase[i].targetHwDevId, &entryIndexGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].entryIndex, entryIndexGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet(
            dev, 0, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet(
        dev, 0, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet
(
    IN  GT_U8           devNum,
    IN  GT_HW_DEV_NUM   targetHwDevId,
    OUT GT_U32          *entryIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     i;

    GT_U32 entryIndex;
    struct
    {
        GT_STATUS     exspectedRc;
        GT_HW_DEV_NUM targetHwDevId;
        GT_U32        *entryIndexPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, 1, ADDR_STAMP_CNS},
        /* bad target device ID */
        {GT_OUT_OF_RANGE, 0x0FFFFFFF, ADDR_STAMP_CNS},
        /* null pointer */
        {GT_BAD_PTR, 0, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, entryIndexPtr, &entryIndex);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet(
                dev, paramOfCase[i].targetHwDevId, paramOfCase[i].entryIndexPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet(
            dev, 0, &entryIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet(
        dev, 0, &entryIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapCpuCodeToTcMapSet
(
    IN  GT_U8                      devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT   cpuCode,
    IN  GT_U32                     tmTc
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapCpuCodeToTcMapSet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     tmTcGet;
    GT_U32     i;

    struct
    {
        GT_BOOL                    checkByGet;
        GT_STATUS                  exspectedRc;
        CPSS_NET_RX_CPU_CODE_ENT   cpuCodeSet;
        CPSS_NET_RX_CPU_CODE_ENT   cpuCodeGet;
        GT_U32                     tmTc;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_NET_FIRST_USER_DEFINED_E, CPSS_NET_FIRST_USER_DEFINED_E, 1},
        /* correct parameters - fill all table, check 0-th entry */
        {GT_TRUE,   GT_OK, CPSS_NET_ALL_CPU_OPCODES_E, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, 2},
        /* bad CPU code */
        {GT_FALSE, GT_BAD_PARAM, 0x0FFFFFFF, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, 3},
        /* out of range index */
        {GT_FALSE, GT_OUT_OF_RANGE, CPSS_NET_FIRST_USER_DEFINED_E, CPSS_NET_FIRST_USER_DEFINED_E, 0x0FFFFFFF}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapCpuCodeToTcMapSet(
                dev, paramOfCase[i].cpuCodeSet, paramOfCase[i].tmTc);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapCpuCodeToTcMapGet(
                dev, paramOfCase[i].cpuCodeGet, &tmTcGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].tmTc, tmTcGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapCpuCodeToTcMapSet(
            dev, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapCpuCodeToTcMapSet(
        dev, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapCpuCodeToTcMapGet
(
    IN  GT_U8                      devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT   cpuCode,
    OUT GT_U32                     *tmTcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapCpuCodeToTcMapGet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     tmTcGet;
    GT_U32     i;

    GT_U32     tmTc;
    struct
    {
        GT_STATUS                  exspectedRc;
        CPSS_NET_RX_CPU_CODE_ENT   cpuCode;
        GT_U32                     *tmTcPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, CPSS_NET_FIRST_USER_DEFINED_E, ADDR_STAMP_CNS},
        /* bad CPU code */
        {GT_BAD_PARAM, 0x0FFFFFFF, ADDR_STAMP_CNS},
        /* null pointer */
        {GT_BAD_PTR, CPSS_NET_FIRST_USER_DEFINED_E, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, tmTcPtr, &tmTc);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapCpuCodeToTcMapGet(
                dev, paramOfCase[i].cpuCode, paramOfCase[i].tmTcPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapCpuCodeToTcMapGet(
            dev, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, &tmTcGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapCpuCodeToTcMapGet(
        dev, CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E, &tmTcGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapTcToTcMapSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand,
    IN  GT_BOOL                     isUnicast,
    IN  GT_U32                      tc,
    IN  GT_U32                      tmTc
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapTcToTcMapSet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     tmTcGet;
    GT_U32     i;

    struct
    {
        GT_BOOL                     checkByGet;
        GT_STATUS                   exspectedRc;
        CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand;
        GT_BOOL                     isUnicast;
        GT_U32                      tc;
        GT_U32                      tmTc;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E, GT_FALSE, 1, 9},
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 2, 10},
        /* Not supported packetDsaCommand */
        {GT_FALSE, GT_BAD_PARAM, CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, GT_FALSE, 1, 9},
        /* wrong tc */
        {GT_FALSE, GT_BAD_PARAM, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 9, 9},
        /* out of range  tmTc */
        {GT_FALSE, GT_OUT_OF_RANGE, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 3, 16}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapTcToTcMapSet(
                dev, paramOfCase[i].packetDsaCommand, paramOfCase[i].isUnicast,
                paramOfCase[i].tc, paramOfCase[i].tmTc);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapTcToTcMapGet(
                dev, paramOfCase[i].packetDsaCommand, paramOfCase[i].isUnicast,
                paramOfCase[i].tc, &tmTcGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].tmTc, tmTcGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapTcToTcMapSet(
            dev, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_FALSE, 0, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapTcToTcMapSet(
        dev, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_FALSE, 0, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapTcToTcMapGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand,
    IN  GT_BOOL                     isUnicast,
    IN  GT_U32                      tc,
    OUT GT_U32                      *tmTcPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapTcToTcMapGet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     i;

    GT_U32     tmTcGet;
    struct
    {
        GT_STATUS                   exspectedRc;
        CPSS_DXCH_NET_DSA_CMD_ENT   packetDsaCommand;
        GT_BOOL                     isUnicast;
        GT_U32                      tc;
        GT_U32                      *tmTcPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E, GT_FALSE, 1, ADDR_STAMP_CNS},
        /* correct parameters */
        {GT_OK, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 2, ADDR_STAMP_CNS},
        /* Not supported packetDsaCommand */
        {GT_BAD_PARAM, CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, GT_FALSE, 1, ADDR_STAMP_CNS},
        /* wrong tc */
        {GT_BAD_PARAM, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 9, ADDR_STAMP_CNS},
        /* NULL  tmTcPtr */
        {GT_BAD_PTR, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_TRUE, 3, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, tmTcPtr, &tmTcGet);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapTcToTcMapGet(
                dev, paramOfCase[i].packetDsaCommand, paramOfCase[i].isUnicast,
                paramOfCase[i].tc, paramOfCase[i].tmTcPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapTcToTcMapGet(
            dev, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_FALSE, 0, &tmTcGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapTcToTcMapGet(
        dev, CPSS_DXCH_NET_DSA_CMD_FORWARD_E, GT_FALSE, 0, &tmTcGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapEgressPolicerForceSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   queueId,
    IN  GT_BOOL  forceMeter,
    IN  GT_BOOL  forceCounting
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapEgressPolicerForceSet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_BOOL    forceMeterGet;
    GT_BOOL    forceCountingGet;
    GT_U32     i;

    struct
    {
        GT_BOOL    checkByGet;
        GT_STATUS  exspectedRc;
        GT_U32     queueId;
        GT_BOOL    forceMeter;
        GT_BOOL    forceCounting;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, 1, GT_FALSE, GT_FALSE},
        /* correct parameters */
        {GT_TRUE,   GT_OK, 2, GT_TRUE, GT_TRUE},
        /* correct parameters */
        {GT_TRUE,   GT_OK, 3, GT_FALSE, GT_TRUE},
        /* wrong queueId */
        {GT_FALSE, GT_BAD_PARAM,  0x0FFFFFFF, GT_FALSE, GT_FALSE}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapEgressPolicerForceSet(
                dev, paramOfCase[i].queueId,
                paramOfCase[i].forceMeter, paramOfCase[i].forceCounting);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapEgressPolicerForceGet(
                dev, paramOfCase[i].queueId,
                &forceMeterGet, &forceCountingGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].forceMeter, forceMeterGet, i);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].forceCounting, forceCountingGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapEgressPolicerForceSet(
            dev, 0, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapEgressPolicerForceSet(
        dev, 0, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapEgressPolicerForceGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   queueId,
    OUT GT_BOOL  *forceMeterPtr,
    OUT GT_BOOL  *forceCountingPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapEgressPolicerForceGet)
{
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U32     i;

    GT_BOOL    forceMeter;
    GT_BOOL    forceCounting;
    struct
    {
        GT_STATUS  exspectedRc;
        GT_U32     queueId;
        GT_BOOL    *forceMeterPtr;
        GT_BOOL    *forceCountingPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, 1, ADDR_STAMP_CNS, ADDR_STAMP_CNS},
        /* wrong queueId */
        {GT_BAD_PARAM,  0x0FFFFFFF, ADDR_STAMP_CNS, ADDR_STAMP_CNS},
        /* NULL pointer for forceMeter */
        {GT_BAD_PTR, 1, NULL, ADDR_STAMP_CNS},
        /* NULL pointer for forceCounting */
        {GT_BAD_PTR, 1, ADDR_STAMP_CNS, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, forceMeterPtr, &forceMeter);
    REP_ADDR_STAMP_MAC(paramOfCase, forceCountingPtr, &forceCounting);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapEgressPolicerForceGet(
                dev, paramOfCase[i].queueId,
                paramOfCase[i].forceMeterPtr, paramOfCase[i].forceCountingPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapEgressPolicerForceGet(
            dev, 0, &forceMeter, &forceCounting);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapEgressPolicerForceGet(
        dev, 0, &forceMeter, &forceCounting);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapToCpuModeSet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   toCpuMode,
    IN  GT_U32                                        toCpuSelectorIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapToCpuModeSet)
{
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   toCpuModeGet;
    GT_U32                                        toCpuSelectorIndexGet;
    GT_U32                                        i;

    struct
    {
        GT_BOOL                                       checkByGet;
        GT_STATUS                                     exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   toCpuMode;
        GT_U32                                        toCpuSelectorIndex;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E, 7},
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_USE_TO_CPU_CONFIG_E, 8},
        /* wrong toCpuMode */
        {GT_FALSE,   GT_BAD_PARAM, 0xFF, 10},
        /* out-of-range  toCpuSelectorIndex*/
        {GT_FALSE, GT_OUT_OF_RANGE, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E, 0x0FFFFFFF}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapToCpuModeSet(
                dev, paramOfCase[i].toCpuMode, paramOfCase[i].toCpuSelectorIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapToCpuModeGet(
                dev, &toCpuModeGet, &toCpuSelectorIndexGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].toCpuMode, toCpuModeGet, i);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].toCpuSelectorIndex, toCpuSelectorIndexGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapToCpuModeSet(
            dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapToCpuModeSet(
        dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapToCpuModeGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   *toCpuModePtr,
    OUT GT_U32                                        *toCpuSelectorIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapToCpuModeGet)
{
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_U32                                        i;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   toCpuModeGet;
    GT_U32                                        toCpuSelectorIndexGet;
    struct
    {
        GT_STATUS                                     exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT   *toCpuModePtr;
        GT_U32                                        *toCpuSelectorIndexPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, ADDR_STAMP_CNS, ADDR_STAMP_CNS},
        /* NULL pointer toCpuModePtr*/
        {GT_BAD_PTR, NULL, ADDR_STAMP_CNS},
        /* NULL pointer toCpuSelectorIndexPtr*/
        {GT_BAD_PTR, ADDR_STAMP_CNS, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, toCpuModePtr, &toCpuModeGet);
    REP_ADDR_STAMP_MAC(paramOfCase, toCpuSelectorIndexPtr, &toCpuSelectorIndexGet);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapToCpuModeGet(
                dev, paramOfCase[i].toCpuModePtr, paramOfCase[i].toCpuSelectorIndexPtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapToCpuModeGet(
            dev, &toCpuModeGet, &toCpuSelectorIndexGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapToCpuModeGet(
        dev, &toCpuModeGet, &toCpuSelectorIndexGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapMcModeSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT   mcMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapMcModeSet)
{
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT mcModeGet;
    GT_U32                                  i;

    struct
    {
        GT_BOOL                                 checkByGet;
        GT_STATUS                               exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT mcMode;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_REGULAR_E},
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_FORCE_EPCL_CONFIG_E},
        /* wrong mcMode */
        {GT_FALSE,   GT_BAD_PARAM, 0xFF}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapMcModeSet(
                dev, paramOfCase[i].mcMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapMcModeGet(
                dev, &mcModeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].mcMode, mcModeGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapMcModeSet(
            dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_REGULAR_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapMcModeSet(
        dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_REGULAR_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapMcModeGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT   *mcModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapMcModeGet)
{
    GT_STATUS                               st = GT_OK;
    GT_U8                                   dev;
    GT_U32                                  i;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT mcModeGet;
    struct
    {
        GT_BOOL                                 checkByGet;
        GT_STATUS                               exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT *mcModePtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, ADDR_STAMP_CNS},
        /* NULL mcModePtr */
        {GT_FALSE,  GT_BAD_PTR, NULL}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, mcModePtr, &mcModeGet);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapMcModeGet(
                dev, paramOfCase[i].mcModePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapMcModeGet(
            dev, &mcModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapMcModeGet(
        dev, &mcModeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapSelectorTableAccessModeSet
(
    IN  GT_U8                                                 devNum,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapSelectorTableAccessModeSet)
{
    GT_STATUS                                             st = GT_OK;
    GT_U8                                                 dev;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  modeGet;
    GT_U32                                                i;

    struct
    {
        GT_BOOL                                               checkByGet;
        GT_STATUS                                             exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  mode;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_LOCAL_TARGET_PORT_E},
        /* correct parameters */
        {GT_TRUE,   GT_OK, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_TARGET_DEV_MAP_E},
        /* wrong mcMode */
        {GT_FALSE,   GT_BAD_PARAM, 0xFF}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapSelectorTableAccessModeSet(
                dev, paramOfCase[i].mode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

            if (paramOfCase[i].checkByGet == GT_FALSE)
            {
                continue;
            }

            st = cpssDxChTmGlueQueueMapSelectorTableAccessModeGet(
                dev, &modeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].mode, modeGet, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapSelectorTableAccessModeSet(
            dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_LOCAL_TARGET_PORT_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapSelectorTableAccessModeSet(
        dev, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_LOCAL_TARGET_PORT_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapSelectorTableAccessModeGet
(
    IN  GT_U8                                                 devNum,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapSelectorTableAccessModeGet)
{
    GT_STATUS                                             st = GT_OK;
    GT_U8                                                 dev;
    GT_U32                                                i;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  modeGet;
    struct
    {
        GT_STATUS                                             exspectedRc;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT  *modePtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, ADDR_STAMP_CNS},
        /* NULL modePtr */
        {GT_BAD_PTR, NULL},
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, modePtr, &modeGet);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases); i++)
        {
            st = cpssDxChTmGlueQueueMapSelectorTableAccessModeGet(
                dev, paramOfCase[i].modePtr);
            UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapSelectorTableAccessModeGet(
            dev, &modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapSelectorTableAccessModeGet(
        dev, &modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet
(
    IN  GT_U8                                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                                     physicalPort,
    IN  CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  *lengthOffsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet)
{
    GT_STATUS                                                st = GT_OK;
    GT_U8                                                    dev;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  lengthOffsetGet;
    GT_U32                                                   i;
    GT_PHYSICAL_PORT_NUM                                     port;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  lengthOffset;
    struct
    {
        GT_BOOL                                                     checkByGet;
        GT_STATUS                                                   exspectedRc;
        GT_PHYSICAL_PORT_NUM                                        physicalPort;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC     *lengthOffsetPtr;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT offsetCmd;
        GT_U32                                                      offsetValue;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_TRUE,   GT_OK, 0, ADDR_STAMP_CNS,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E, 1},
        /* correct parameters */
        {GT_TRUE,   GT_OK, 0, ADDR_STAMP_CNS,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_SUBTRACT_E, 127},
        /* wrong command */
        {GT_FALSE,   GT_BAD_PARAM, 0, ADDR_STAMP_CNS,
            0x0FFFFFFF, 0},
        /* wrong command */
        {GT_FALSE,   GT_BAD_PARAM, 0, ADDR_STAMP_CNS,
            0x0FFFFFFF, 0},
        /* out of range offset */
        {GT_FALSE,   GT_OUT_OF_RANGE, 0, ADDR_STAMP_CNS,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E, 0x0FFFFFFF},
        /* out of range offset */
        {GT_FALSE,   GT_OUT_OF_RANGE, 0, ADDR_STAMP_CNS,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E, 128},
        /* NULL lengthOffsetPtr */
        {GT_FALSE,   GT_BAD_PTR, 0, NULL,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E, 0},
        /* wrong port */
        {GT_FALSE,   GT_BAD_PARAM, 0x0FFFFFFF, ADDR_STAMP_CNS,
            CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E, 0},
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, lengthOffsetPtr, &lengthOffset);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases - 1); i++)
        {
            lengthOffset.offsetCmd   = paramOfCase[i].offsetCmd;
            lengthOffset.offsetValue = paramOfCase[i].offsetValue;

            st = prvUtfNextPhyPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* For all active devices go over all available physical ports.*/
            while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
            {

                st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet(
                                dev, port, paramOfCase[i].lengthOffsetPtr);
                UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

                if (paramOfCase[i].checkByGet == GT_FALSE)
                {
                    continue;
                }

                st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
                                                dev, port, &lengthOffsetGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

                UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].offsetCmd, lengthOffsetGet.offsetCmd, i);
                UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].offsetValue, lengthOffsetGet.offsetValue, i);
            }
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet(
                                dev, port, paramOfCase[i].lengthOffsetPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);

        if (paramOfCase[i].checkByGet == GT_FALSE)
        {
            continue;
        }

        st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
                                                dev, port, &lengthOffsetGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, i);

        UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].offsetCmd, lengthOffsetGet.offsetCmd, i);
        UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].offsetValue, lengthOffsetGet.offsetValue, i);
    }

    port = 0;

    /* set valid values */
    lengthOffset.offsetCmd   =
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E;
    lengthOffset.offsetValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet(
            dev, 0, &lengthOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet(
        dev, 0, &lengthOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet
(
    IN  GT_U8                                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                                     physicalPort,
    OUT CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  *lengthOffsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet)
{
    GT_STATUS                                                st = GT_OK;
    GT_U8                                                    dev;
    GT_U32                                                   i;
    GT_PHYSICAL_PORT_NUM                                     port;

    CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC  lengthOffset;
    struct
    {
        GT_STATUS                                                   exspectedRc;
        GT_PHYSICAL_PORT_NUM                                        physicalPort;
        CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC     *lengthOffsetPtr;
    } paramOfCase[] =
    {
        /* correct parameters */
        {GT_OK, 0, ADDR_STAMP_CNS},
        /* NULL lengthOffsetPtr */
        {GT_BAD_PTR, 0, NULL},
        /* wrong port */
        {GT_BAD_PARAM, 0x0FFFFFFF, ADDR_STAMP_CNS}
    };

    GT_U32 numOfCases = sizeof(paramOfCase) / sizeof(paramOfCase[0]);

    REP_ADDR_STAMP_MAC(paramOfCase, lengthOffsetPtr, &lengthOffset);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (i = 0; (i < numOfCases-1); i++)
        {
            st = prvUtfNextPhyPortReset(&port, dev);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* For all active devices go over all available physical ports.*/
            while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
            {
                st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
                                    dev, port, paramOfCase[i].lengthOffsetPtr);
                UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
            }
        }

        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
                          dev, port, paramOfCase[i].lengthOffsetPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(paramOfCase[i].exspectedRc, st, i);
    }

    port = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, PRV_TGF_NOT_TM_DEV_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
            dev, port, &lengthOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet(
        dev, port, &lengthOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTmGlueQueueMap suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTmGlueQueueMap)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapBitSelectTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapBitSelectTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapCpuCodeToTcMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapCpuCodeToTcMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapTcToTcMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapTcToTcMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapEgressPolicerForceSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapEgressPolicerForceGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapToCpuModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapToCpuModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapMcModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapMcModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapSelectorTableAccessModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapSelectorTableAccessModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTmGlueQueueMap)

