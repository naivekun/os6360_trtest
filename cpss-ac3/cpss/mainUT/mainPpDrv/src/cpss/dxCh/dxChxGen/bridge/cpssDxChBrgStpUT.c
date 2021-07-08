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
* @file cpssDxChBrgStpUT.c
*
* @brief Unit tests for cpssDxChBrgStp, that provides
* Spanning Tree protocol facility DxCh CPSS implementation.
*
* @version   21
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpStateSet
(
    IN GT_U8                    dev,
    IN GT_U8                    port,
    IN GT_U16                   stpId,
    IN CPSS_STP_STATE_ENT       state
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpStateSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with stpId [0 / max - 1]
    and state [CPSS_STP_DISABLED_E / CPSS_STP_LRN_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgStpStateGet with the same params.
    Expected: GT_OK and the same state.
    1.1.3. Call function with out of range stpId [max].
    Expected: NON GT_OK.
    1.1.3. Call function with wrong enum values state.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port;
    GT_U16                  stpId = 0;
    CPSS_STP_STATE_ENT      state = CPSS_STP_DISABLED_E;
    CPSS_STP_STATE_ENT      stateGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call function with stpId [0 / max - 1]
                and state [CPSS_STP_DISABLED_E / CPSS_STP_LRN_E].
                Expected: GT_OK.
            */
            /* call with stpId[0] and state[CPSS_STP_DISABLED_E] */
            stpId = 0;
            state = CPSS_STP_DISABLED_E;

            st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, stpId, state);

            /*
                1.1.2. Call cpssDxChBrgStpStateGet with the same params.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL3_STRING_MAC(state, stateGet,
                "get another state than was set: %d, %d, %d", dev, port, stpId);

            /* call with stpId[max] and state[CPSS_STP_LRN_E] */
            stpId = (GT_U16)(PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum - 1);
            state = CPSS_STP_LRN_E;

            st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, stpId, state);

            /* call get func */
            st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            UTF_VERIFY_EQUAL3_STRING_MAC(state, stateGet,
                    "get another state than was set: %d, %d, %d", dev, port, stpId);

            /*
                1.1.3. Call function with out of range stpId [max].
                Expected: NON GT_OK.
            */
            stpId = (GT_U16)PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum;

            st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stpId);

            stpId = 0; /* restore */

            /*
                1.1.3. Call function with wrong enum values state.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgStpStateSet
                            (dev, port, stpId, state),
                            state);
        }

        stpId = 0;
        state = CPSS_STP_LRN_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port like other ports */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    stpId = 0;
    state = CPSS_STP_LRN_E;
    port  = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpStateSet(dev, port, stpId, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpStateGet
(
    IN GT_U8                    dev,
    IN GT_U8                    port,
    IN GT_U16                   stpId,
    OUT CPSS_STP_STATE_ENT      *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpStateGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call function with stpId [0 - max]
    Expected: GT_OK.
    1.1.2. Call function with out of range stpId [max].
    Expected: NON GT_OK.
    1.1.3. Call function with wrong statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS           st = GT_OK;

    GT_U8               dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U16              stpId = 0;
    CPSS_STP_STATE_ENT  stateGet;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 16;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,
                    PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum,
                    gmFillMaxIterations,step);
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            for(stpId = 0; stpId < PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum; stpId+=step)
            {
                /*
                    1.1.1. Call function with stpId [0 - max]
                    Expected: GT_OK.
                */
                st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call function with out of range stpId [max].
                Expected: NON GT_OK.
            */
            stpId = (GT_U16)PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum;

            st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stpId);

            stpId = 0; /* restore */

            /*
                1.1.3. Call function with wrong statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgStpStateGet(dev, port, stpId, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, stpId);

            if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
            {
                /* limit number of ports due to loop on STG ! */
                port += 11;
            }
        }

        stpId = 0;
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the range is '256 physical ports' */
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /* cpu port like other ports */
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    stpId = 0;
    port  = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpStateGet(dev, port, stpId, &stateGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpEntryGet
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    OUT GT_U32      stpEntryPtr[CPSS_DXCH_STG_ENTRY_SIZE_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpEntryGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with stpId [0] and non-NULL stpEntrPtr.
    Expected: GT_OK.
    1.2. Call function with out of range
         stpId [PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum]
         and non-NULL stpEntrPtr.
    Expected: NON GT_OK.
    1.3. Call function with stpId [0] and stpEntrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32      stpId = 0; /* GT_U32 type used to fix Tornado vxWorks compiler error */
    GT_U32      stpEntryArr [CPSS_DXCH_STG_ENTRY_SIZE_CNS];

    cpssOsBzero((GT_VOID*) stpEntryArr, sizeof(stpEntryArr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with stpId [0] and non-NULL stpEntrPtr.
            Expected: GT_OK.
        */
        stpId = 0;
        st = cpssDxChBrgStpEntryGet(dev, (GT_U16)stpId, stpEntryArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stpId);

        /*
            1.2. Call function with out of range
            stpId [PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum]
            and non-NULL stpEntrPtr.
            Expected: NON GT_OK.
        */
        stpId = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum;
        st = cpssDxChBrgStpEntryGet(dev, (GT_U16)stpId, stpEntryArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stpId);

        stpId = 0; /* restore */

        /*
            1.3. Call function with stpId [0] and stpEntrPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChBrgStpEntryGet(dev, (GT_U16)stpId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, stpId);
    }

    stpId = 0;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpEntryGet(dev, (GT_U16)stpId, stpEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpEntryGet(dev, (GT_U16)stpId, stpEntryArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpEntryWrite
(
    IN  GT_U8       dev,
    IN  GT_U16      stpId,
    IN  GT_U32      stpEntryPtr[CPSS_DXCH_STG_ENTRY_SIZE_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpEntryWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call function with stpId [0] and stpEntrPtr [{12,1213}] .
    Expected: GT_OK.
    1.2. Call function cpssDxChBrgStpEntryGet with stpId [0] and non-NULL stpEntrPtr.
    Expected: GT_OK and stpEntry the same as just written.
    1.3. Call function with out of range
         stpId [PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum]
         and non-NULL stpEntrPtr.
    Expected: NON GT_OK.
    1.4. Call function with stpId [0] and stpEntrPtr [NULL] .
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U16      stpId = 0;
    GT_U32      stpEntryArr [CPSS_DXCH_STG_ENTRY_SIZE_CNS];
    GT_U32      retStpEntryArr [CPSS_DXCH_STG_ENTRY_SIZE_CNS];
    GT_U32      validPorts[3+7];
    GT_U32      ii,wordNum,stpValue;

    GT_BOOL     failureWas = GT_FALSE;

    cpssOsBzero ((GT_VOID*)stpEntryArr, sizeof(stpEntryArr));
    cpssOsBzero ((GT_VOID*)retStpEntryArr, sizeof(retStpEntryArr));

    stpEntryArr [0] = 12;/*ports : 0,2 */
    stpEntryArr [1] = 1213;/*ports : 16,18,20,22 */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get ports with MAC */
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
            0  /*start port */, &validPorts[0] , 3/* 3 ports  0-2  */);
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
            16 /*start port */, &validPorts[3] , 7/* 7 ports 16-22 */);

        cpssOsMemSet(stpEntryArr,0,sizeof(stpEntryArr));
        for(ii = 0 ; ii < (3+7) ; ii++)
        {
            if(validPorts[ii] == PRV_TGF_INVALID_VALUE_CNS)
            {
                continue;
            }

            wordNum = validPorts[ii] / 16; /* 16 ports in word */

            stpValue = 1 + (ii % 3);

            stpEntryArr[wordNum] |=  stpValue  << (2*(validPorts[ii] % 16));
        }

        /*
            1.1. Call function with stpId [0] and stpEntrPtr [{12,1213}].
            Expected: GT_OK.
        */
        stpId = 0;

        st = cpssDxChBrgStpEntryWrite(dev, stpId, stpEntryArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, stpId);

        /*
            1.2. Call function cpssDxChBrgStpEntryGet with stpId [0] and
            non-NULL stpEntrPtr.
            Expected: GT_OK and stpEntry the same as just written.
        */
        st = cpssDxChBrgStpEntryGet(dev, stpId, retStpEntryArr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgStpEntryGet: %d, %d", dev, stpId);

        failureWas = (0 == cpssOsMemCmp((GT_VOID*)stpEntryArr,
                                        (GT_VOID*)retStpEntryArr,
                                      sizeof(stpEntryArr))) ? GT_FALSE : GT_TRUE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, failureWas,
            "get another stpEntry than was set: %d, %d", dev, stpId);

        /*
            1.3. Call function with out of range
            stpId [PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum]
            and non-NULL stpEntrPtr.
            Expected: NON GT_OK.
        */
        stpId = (GT_U16)PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum;

        st = cpssDxChBrgStpEntryWrite(dev, stpId, stpEntryArr);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, stpId);

        stpId = 0; /* restore */

        /*
            1.4. Call function with stpId [0] and stpEntrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgStpEntryWrite(dev, stpId, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, stpId);
    }

    stpId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpEntryWrite(dev, stpId, stpEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpEntryWrite(dev, stpId, stpEntryArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpInit
(
    IN GT_U8    devNum,
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with corect devNum.
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with corect devNum.
            Expected: GT_OK.
        */
        st = cpssDxChBrgStpInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN CPSS_STP_STATE_ENT   state
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpPortSpanningTreeStateSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with state [CPSS_STP_DISABLED_E /
                            CPSS_STP_BLCK_LSTN_E /
                            CPSS_STP_LRN_E /
                            CPSS_STP_FRWRD_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateGet with non NULL statePtr.
    Expected: GT_OK and the same state.
    1.1.3. Call with state [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_STP_STATE_ENT  state    = CPSS_STP_DISABLED_E;
    CPSS_STP_STATE_ENT  stateGet = CPSS_STP_DISABLED_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with state [CPSS_STP_DISABLED_E /
                                        CPSS_STP_BLCK_LSTN_E /
                                        CPSS_STP_LRN_E /
                                        CPSS_STP_FRWRD_E].
                Expected: GT_OK.
            */
            /* call with state = CPSS_STP_DISABLED_E */
            state = CPSS_STP_DISABLED_E;

            st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* call with state = CPSS_STP_BLCK_LSTN_E */
            state = CPSS_STP_BLCK_LSTN_E;

            st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* call with state = CPSS_STP_LRN_E */
            state = CPSS_STP_LRN_E;

            st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* call with state = CPSS_STP_FRWRD_E */
            state = CPSS_STP_FRWRD_E;

            st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with state [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgStpPortSpanningTreeStateSet
                                (dev, port, state),
                                state);
        }

        state = CPSS_STP_DISABLED_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    state = CPSS_STP_DISABLED_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpPortSpanningTreeStateSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT CPSS_STP_STATE_ENT      *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpPortSpanningTreeStateGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_STP_STATE_ENT  state = CPSS_STP_DISABLED_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL statePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpPortSpanningTreeStateGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateModeSet
(
    IN GT_U8                        devNum,
    IN GT_PORT_NUM                  portNum,
    IN CPSS_BRG_STP_STATE_MODE_ENT  stateMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpPortSpanningTreeStateModeSet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with stateMode [CPSS_BRG_STP_E_VLAN_MODE_E /
                                CPSS_BRG_STP_E_PORT_MODE_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateModeGet with non NULL stateModePtr.
    Expected: GT_OK and the same stateMode.
    1.1.3. Call with stateMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_BRG_STP_STATE_MODE_ENT stateMode    = CPSS_BRG_STP_E_VLAN_MODE_E;
    CPSS_BRG_STP_STATE_MODE_ENT stateModeGet = CPSS_BRG_STP_E_VLAN_MODE_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with stateMode [CPSS_BRG_STP_E_VLAN_MODE_E /
                                            CPSS_BRG_STP_E_PORT_MODE_E].
                Expected: GT_OK.
            */
            /* call with stateMode = CPSS_BRG_STP_E_VLAN_MODE_E */
            stateMode = CPSS_BRG_STP_E_VLAN_MODE_E;

            st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateMode);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateModeGet with non NULL stateModePtr.
                Expected: GT_OK and the same stateMode.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(stateMode, stateModeGet,
                       "get another stateMode than was set: %d, %d", dev, port);

            /* call with stateMode = CPSS_BRG_STP_E_PORT_MODE_E */
            stateMode = CPSS_BRG_STP_E_PORT_MODE_E;

            st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, stateMode);

            /*
                1.1.2. Call cpssDxChBrgStpPortSpanningTreeStateModeGet with non NULL stateModePtr.
                Expected: GT_OK and the same stateMode.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgStpPortSpanningTreeStateModeGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(stateMode, stateModeGet,
                       "get another stateMode than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with stateMode [wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgStpPortSpanningTreeStateModeSet
                                (dev, port, stateMode),
                                stateMode);
        }

        stateMode = CPSS_BRG_STP_E_VLAN_MODE_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    stateMode = CPSS_BRG_STP_E_VLAN_MODE_E;
    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpPortSpanningTreeStateModeSet(dev, port, stateMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgStpPortSpanningTreeStateModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_NUM                     portNum,
    OUT CPSS_BRG_STP_STATE_MODE_ENT     *stateModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpPortSpanningTreeStateModeGet)
{
/*
    ITERATE_DEVICES_VIRT_PORTS (SIP5)
    1.1.1. Call with not NULL stateModePtr.
    Expected: GT_OK.
    1.1.2. Call with stateModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev  = 0;
    GT_PORT_NUM port = 0;

    CPSS_BRG_STP_STATE_MODE_ENT  stateMode = CPSS_BRG_STP_E_VLAN_MODE_E;
    GT_U32      notAppFamilyBmp;
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
                1.1.1. Call with not NULL stateModePtr.
                Expected: GT_OK.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with stateModePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, stateModePtr = NULL", dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgStpPortSpanningTreeStateModeGet(dev, port, &stateMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill raw STP table.
*/
UTF_TEST_CASE_MAC(cpssDxChBrgStpFillStpTable)
{
/*
    ITERATE_DEVICE (DxChx)
    1.1. Get table Size
        Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_STG_E]
                                                and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in raw STP table.
         Call cpssDxChBrgStpEntryWrite with stpId [0..numEntries-1]
                                            and stpEntrPtr [{12,1213}].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxChBrgStpEntryWrite with stpId [numEntries]
                                            and stpEntrPtr [{12,1213}].
    Expected: NOT GT_OK.
    1.4. Read all entries in raw STP table and compare with original.
         Call function cpssDxChBrgStpEntryGet with stpId [0]
                                                   and non-NULL stpEntrPtr.
    Expected: GT_OK and stpEntry the same as just written.
    1.5. Try to read entry with index out of range.
         Call function cpssDxChBrgStpEntryGet with stpId [numEntries]
                                                   and non-NULL stpEntrPtr.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_BOOL     isEqual = GT_FALSE;

    GT_U32      numEntries  = 0;
    GT_U16      iTemp       = 0;
    GT_U16               step = 1;
    GT_U32   gmFillMaxIterations = 128;
    GT_U32      validPorts[3+7];
    GT_U32      ii,wordNum,stpValue;

    GT_U32      stpEntryArr [CPSS_DXCH_STG_ENTRY_SIZE_CNS];
    GT_U32      retStpEntryArr [CPSS_DXCH_STG_ENTRY_SIZE_CNS];

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero ((GT_VOID*)stpEntryArr, sizeof(stpEntryArr));
    cpssOsBzero ((GT_VOID*)retStpEntryArr, sizeof(retStpEntryArr));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Fill the entry for raw STP table */
    stpEntryArr [0] = 12;/*ports : 0,2 */
    stpEntryArr [1] = 1213;/*ports : 16,18,20,22 */

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get ports with MAC */
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
            0  /*start port */, &validPorts[0] , 3/* 3 ports  0-2  */);
        prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
            16 /*start port */, &validPorts[3] , 7/* 7 ports 16-22 */);

        cpssOsMemSet(stpEntryArr,0,sizeof(stpEntryArr));
        for(ii = 0 ; ii < (3+7) ; ii++)
        {
            if(validPorts[ii] == PRV_TGF_INVALID_VALUE_CNS)
            {
                continue;
            }

            wordNum = validPorts[ii] / 16; /* 16 ports in word */

            stpValue = 1 + (ii % 3);

            stpEntryArr[wordNum] |=  stpValue  << (2*(validPorts[ii] % 16));
        }

        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_STG_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChCfgTableNumEntriesGet: %d", dev);

        if (IS_NEEDED_TO_REDUCE_ITERATIONS_DUE_TO_POOR_PERFORMANCE_MAC || GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            /* get new step that will match 'max iterations' for GM*/
            step = (GT_U16)prvUtfIterationReduce(dev,numEntries,gmFillMaxIterations,step);
        }

        /* 1.2. Fill all entries in raw STP table */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgStpEntryWrite(dev, iTemp, stpEntryArr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgStpEntryWrite: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxChBrgStpEntryWrite(dev, (GT_U16)numEntries, stpEntryArr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgStpEntryWrite: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in raw STP table and compare with original */
        for(iTemp = 0; iTemp < numEntries; iTemp+=step)
        {
            st = cpssDxChBrgStpEntryGet(dev, iTemp, retStpEntryArr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgStpEntryGet: %d, %d", dev, iTemp);
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)stpEntryArr,
                                         (GT_VOID*)retStpEntryArr,
                                          sizeof(stpEntryArr))) ? GT_FALSE : GT_TRUE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, isEqual,
                "get another stpEntry than was set: %d, %d", dev, iTemp);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxChBrgStpEntryGet(dev, (GT_U16)numEntries, retStpEntryArr);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgStpEntryGet: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgStp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgStp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpEntryWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpPortSpanningTreeStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpPortSpanningTreeStateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpPortSpanningTreeStateModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpPortSpanningTreeStateModeGet)

    /* Test filling Table*/
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgStpFillStpTable)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgStp)

