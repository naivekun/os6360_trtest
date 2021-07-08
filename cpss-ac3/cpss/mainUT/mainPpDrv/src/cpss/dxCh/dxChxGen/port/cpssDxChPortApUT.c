/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELLSEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
*******************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPortApUT.c
*
* @brief Unit tests for cpssDxChPortAp, that provides CPSS implementation
* for 802.3ap standard (defines the auto negotiation for backplane
* Ethernet) configuration and control facility.
*
* @version   6
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
 * must come from C files that
    already fixed the types of ports from GT_U8 !

    NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* defines */

/* Macro to define all port groups in bitmap */
#define PORT_AP_ALL_PORTS_BMP_CNS    0x7FFFFFF

/* Default valid value for port id */
#define PORT_AP_VALID_PHY_PORT_CNS  0

#ifndef ASIC_SIMULATION

static GT_STATUS apBasicInit
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
    )
{
    GT_STATUS   rc;
    GT_BOOL     apEnabled;

    rc = cpssDxChPortApEnableGet(devNum, portGroupId, &apEnabled);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (!apEnabled)
    {
        rc = cpssDxChPortApEnableSet(devNum, (1 << portGroupId), GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }

        cpssOsTimerWkAfter(2000);
    }
#if 0
    {
        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_PORT_802_3_AP_E
                                    +portGroupId, CPSS_EVENT_UNMASK_E);
    }
#endif
    return rc;
}
#endif

/*----------------------------------------------------------------------------*/
/*
 GT_STATUS cpssDxChPortApEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_BOOL *enabledPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApEnableGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Lion2)
    1.1.1. Call with valid enabledPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid enabledPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8            devNum             = 0;
#ifndef ASIC_SIMULATION
    GT_STATUS        st                 = GT_OK;
    GT_U32           portGroupNum       = 0;
    GT_BOOL          enabled            = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            /*
                1.1.1. Call with valid enabledPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enabled);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                portGroupNum, enabled);

            /*
                1.1.2. Call with invalid enabledPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "%d, enabled = NULL", devNum);
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)

        /* 1.2. For not-active port groups check that function
           returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enabled);
            if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st,
                                                          devNum, portGroupNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);
            }
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT2_E | UTF_LION_E | UTF_FALCON_E);

    /* Go over all non-applicable devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enabled);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enabled);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_BOOL             enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApEnableSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Lion2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE]
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortApEnableGet().
    Expected: GT_OK and the same parameter value as was set in 1.1.1.
*/
    GT_U8                   devNum          = 0;
#ifndef ASIC_SIMULATION
    GT_STATUS               st              = GT_OK;
    GT_PORT_GROUPS_BMP      portGroupsBmp   = 0;
    GT_BOOL                 enable          = GT_FALSE;
    GT_BOOL                 apEnable;
    GT_U32                  portGroupNum    = 0;
    GT_BOOL                 enableGet       = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT2_E | UTF_LION_E |
                                        UTF_BOBCAT3_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6128);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            /*
                1.1.2. Call cpssDxChPortApEnableGet().
                Expected: GT_OK and the same parameter value as was set
                    in 1.1.1.
            */
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, &apEnable);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChPortApEnableGet: %d", devNum);

            /* set next active port */
            portGroupsBmp = (1 << portGroupNum);

            /*
                1.1.1. Call with enable [GT_FALSE]
                Expected: GT_OK.
            */
            enable = GT_FALSE;

            if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily){
                st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                    portGroupsBmp, enable);
            }
            else
            {
              st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
              UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
            }

            /*
                1.1.2. Call cpssDxChPortApEnableGet().
                Expected: GT_OK and the same parameter value as was set
                    in 1.1.1.
            */
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChPortApEnableGet: %d", devNum);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                "got another enable then was set: %d", devNum);

            /*
                1.1.1. Call with enable [GT_TRUE]
                Expected: GT_OK.
            */
            enable = GT_TRUE;

            st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
            if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily){
                st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                    portGroupsBmp, enable);
            }
            else
            {
              st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
              UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
            }

            cpssOsTimerWkAfter(1000);
            /*
                1.1.2. Call cpssDxChPortApEnableGet().
                Expected: GT_OK and the same parameter value as was set
                    in 1.1.1.
            */
            st = cpssDxChPortApEnableGet(devNum, portGroupNum, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChPortApEnableGet: %d", devNum);

            /* verifying values */
            if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily){
                UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                    "got another enable then was set: %d", devNum);
            }

            /* restore original state */
            if (apEnable != enable)
            {
                st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, apEnable);
                if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily){
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                                                         portGroupsBmp, apEnable);
                }
            }
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)

        /* 1.2. For not-active port groups check that function
                returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupNum);

            st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
            if(PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st,
                                                       devNum, portGroupsBmp);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
            }
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
        if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily){
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupsBmp);
        }
        else
        {
            st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
        }
    }

    /* restore valid values */
    portGroupsBmp = 1;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT2_E | UTF_LION_E |
                                        UTF_BOBCAT3_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApEnableSet(devNum, portGroupsBmp, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApPortConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *apEnablePtr,
    OUT CPSS_DXCH_PORT_AP_PARAMS_STC    *apParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApPortConfigGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with valid apEnablePtr [non-NULL] and
        valid apParamsPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid apEnablePtr [NULL] and other valid parameters
        same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with invalid apParamsPtr [NULL] and other valid parameters
        same as 1.1.1.
    Expected: GT_BAD_PTR.
*/
    GT_U8                           devNum   =  0;
#ifndef ASIC_SIMULATION
    GT_STATUS                       st       =  GT_OK;
    GT_PHYSICAL_PORT_NUM            portNum  =  PORT_AP_VALID_PHY_PORT_CNS;
    GT_BOOL                         apEnable =  GT_FALSE;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams =  { 0 };
    GT_U32                          portGroupNum;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
#endif

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        if (devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            st = cpssDxChPortApEnableGet(devNum, PORT_AP_ALL_PORTS_BMP_CNS, &apEnable);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            if (!apEnable)
            {
                SKIP_TEST_MAC
            }
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily)
            {
                if (portNum % 16 == 0)
                { /* on first port of new port group enable AP */
                    portGroupNum = portNum/16;
                    st = apBasicInit(devNum, portGroupNum);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);
                }
            }

            /*
                1.1.1. Call with valid apEnablePtr [non-NULL] and
                    valid apParamsPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnable, &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid apEnablePtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum, NULL, &apParams);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, NULL", devNum, portNum);

            /*
                1.1.3. Call with invalid apParamsPtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                "%d, %d, NULL", devNum, portNum);
        }

        /*
           1.2. For all active devices go over all non available
               physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnable, &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_AP_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);
    /* Go over all non applicable devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

#ifndef ASIC_SIMULATION

static GT_VOID stopApOnPort
(
    GT_U8                           devNum,
    GT_PHYSICAL_PORT_NUM            portNum
    )
{
    GT_STATUS st;
    GT_U32    addr;
    GT_U32    data;
    GT_U32    i;

    /* show that the function alive */
    utfPrintKeepAlive();

    /* stop AP on port before move to next case */
    st = cpssDxChPortApPortConfigSet(devNum, portNum, GT_FALSE, NULL);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

    cpssOsTimerWkAfter(500);

    st = cpssDxChPortApUnLock(devNum, portNum);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

    /* ensure AP state machine stopped */
    addr = 0x370F0 + (portNum % 16) * 0x4;
    for (i = 0; (i < 10) && (data != 0xdead); i++)
    {
        st = cpssDrvPpHwRegBitMaskRead(devNum, (portNum / 16), addr, 0xffffffff, &data);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        cpssOsTimerWkAfter(1000);
    }

    return;
}
#endif

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApPortConfigSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_BOOL                           apEnable,
    IN  CPSS_DXCH_PORT_AP_PARAMS_STC      *apParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApPortConfigSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with
           apEnable [GT_TRUE / GT_FALSE],
           apParams {
                     fcPause [GT_FALSE / GT_TRUE],
                     fcAsmDir [CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E /
                               CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E],
                     fecSupported [GT_FALSE / GT_TRUE],
                     fecRequired [GT_FALSE / GT_TRUE],
                     noneceDisable [GT_FALSE / GT_TRUE],
                     laneNum [0 / 32 / 63],
                     modesAdvertiseArr[CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS]
                             {
                               [
                                {ifMode [CPSS_PORT_INTERFACE_MODE_1000BASE_X_E /
                                         CPSS_PORT_INTERFACE_MODE_XGMII_E /
                                         CPSS_PORT_INTERFACE_MODE_KR_E],
                                 speed  [CPSS_PORT_SPEED_1000_E /
                                         CPSS_PORT_SPEED_10000_E /
                                         CPSS_PORT_SPEED_40000_E]}
                               ]
                             }
                    }
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortApPortConfigGet().
    Expected: GT_OK and parameters values are the same as were set
        in 1.1.1.
    1.1.3. Call with invalid fcAsmDir [wrong enum values] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range laneNum [64] and other
        valid parameters same as 1.1.1.
    Expected: GT_OUT_OF_RANGE.
    1.1.5. Call with invalid modesAdvertiseArr[].ifMode [wrong enum values] and
        other valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with invalid modesAdvertiseArr[].speed [wrong enum values] and
        other valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with apParamsPtr[NULL] for CPSS defaults.
    Expected: GT_OK.
    1.1.8. Call cpssDxChPortApPortConfigGet().
    Expected: GT_OK and other parameters value the same as in CPSS default
        parameters prvCpssDxChPortApDefaultParams structure.
*/
    GT_U8                           devNum      =   0;
#ifndef ASIC_SIMULATION
    GT_STATUS                       st          =   GT_OK;
    GT_PHYSICAL_PORT_NUM            portNum     =   PORT_AP_VALID_PHY_PORT_CNS;
    GT_BOOL                         apEnable    =   GT_FALSE;
    GT_BOOL                         apEnableGet =   GT_FALSE;

    CPSS_DXCH_PORT_AP_PARAMS_STC    apParamsGet =   { 0 };
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams    =   { 0 };

    GT_U32          i         = 0; /* iterator */
    GT_BOOL         msMatched = GT_FALSE; /* matcher */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT            speed;
    CPSS_PORTS_BMP_STC             portsBmp;
    GT_BOOL                        forceLinkPass;
    GT_U32                                                    portGroupNum;
    CPSS_PP_FAMILY_TYPE_ENT        devFamily;
#endif

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
            UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        if (devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            st = cpssDxChPortApEnableGet(devNum, PORT_AP_ALL_PORTS_BMP_CNS, &apEnable);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            if (!apEnable)
            {
                SKIP_TEST_MAC
            }
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with
                       apEnable [GT_TRUE],
                       apParams {
                                 fcPause [GT_FALSE],
                                 fcAsmDir [CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E],
                                 fecSupported [GT_FALSE],
                                 fecRequired [GT_FALSE],
                                 noneceDisable [GT_FALSE ],
                                 laneNum [0],
                                 modesAdvertiseArr[CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS]
                                         {
                                           [
                                            {ifMode [CPSS_PORT_INTERFACE_MODE_1000BASE_X_E /
                                                     CPSS_PORT_INTERFACE_MODE_XGMII_E /
                                                     CPSS_PORT_INTERFACE_MODE_KR_E],
                                             speed  [CPSS_PORT_SPEED_1000_E /
                                                     CPSS_PORT_SPEED_10000_E /
                                                     CPSS_PORT_SPEED_40000_E]}
                                           ]
                                         }
                                }
                Expected: GT_OK.
            */

            if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily)
            {
                if (portNum % 16 == 0)
                { /* on first port of new port group enable AP */
                    portGroupNum = portNum/16;
                    st = apBasicInit(devNum, portGroupNum);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);
                }
            }

            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            st = cpssDxChPortSpeedGet(devNum, portNum, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            if((ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) &&
                (speed != CPSS_PORT_SPEED_NA_E))
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
                st = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode, speed);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);
                st = cpssDxChPortForceLinkPassEnableGet(devNum, portNum, &forceLinkPass);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
                if (GT_TRUE == forceLinkPass)
                {
                    st = cpssDxChPortForceLinkPassEnableSet(devNum, portNum, GT_FALSE);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
                    st = cpssDxChPortForceLinkPassEnableGet(devNum, portNum, &forceLinkPass);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
                }
            }

            apEnable = GT_TRUE;
            apParams.fcPause       = GT_FALSE;
            apParams.fcAsmDir      = CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;
            apParams.fecSupported  = GT_FALSE;
            apParams.fecRequired   = GT_FALSE;
            apParams.noneceDisable = GT_TRUE;
            apParams.laneNum       = 0;

            apParams.modesAdvertiseArr[0].ifMode =
                CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            apParams.modesAdvertiseArr[0].speed  = CPSS_PORT_SPEED_1000_E;
            apParams.modesAdvertiseArr[1].ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            apParams.modesAdvertiseArr[1].speed  = CPSS_PORT_SPEED_NA_E;

            st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
                &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortApPortConfigGet().
                Expected: GT_OK and the same parameters value as were set
                    in 1.1.1.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnableGet, &apParamsGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(apEnable, apEnableGet, devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcPause, apParamsGet.fcPause,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcAsmDir, apParamsGet.fcAsmDir,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecSupported,
                apParamsGet.fecSupported,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecRequired,
                apParamsGet.fecRequired,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.noneceDisable,
                apParamsGet.noneceDisable,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.laneNum, apParamsGet.laneNum,
                devNum, portNum);

            /* search for set parameters in returned array */
            for (i = 0; i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
            {
                if ((apParams.modesAdvertiseArr[0].ifMode ==
                            apParamsGet.modesAdvertiseArr[i].ifMode) &&
                    (apParams.modesAdvertiseArr[0].speed ==
                            apParamsGet.modesAdvertiseArr[i].speed))
                {
                    msMatched = GT_TRUE;
                    break;
                }
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, msMatched, devNum, portNum);
            /* restore default value */
            msMatched = GT_FALSE;

            /* stop AP on port before move to next case */
            stopApOnPort(devNum, portNum);

            /*
                1.1.1. Call with
                       apEnable [GT_TRUE],
                       apParams {
                                 fcPause [GT_TRUE],
                                 fcAsmDir [CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E],
                                 fecSupported [GT_TRUE],
                                 fecRequired [GT_TRUE],
                                 noneceDisable [GT_TRUE ],
                                 laneNum [32],
                                 modesAdvertiseArr[CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS]
                                         {
                                           [
                                            {ifMode [CPSS_PORT_INTERFACE_MODE_1000BASE_X_E /
                                                     CPSS_PORT_INTERFACE_MODE_XGMII_E /
                                                     CPSS_PORT_INTERFACE_MODE_KR_E],
                                             speed  [CPSS_PORT_SPEED_1000_E /
                                                     CPSS_PORT_SPEED_10000_E /
                                                     CPSS_PORT_SPEED_40000_E]}
                                           ]
                                         }
                                }
                Expected: GT_OK.
            */

            apEnable = GT_TRUE;
            apParams.fcPause       = GT_TRUE;
            apParams.fcAsmDir      = CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E;
            apParams.noneceDisable = GT_TRUE;
            if ((portNum % 4 != 0) && (portNum % 16 != 9))
            {
                apParams.laneNum       = 0;
                apParams.modesAdvertiseArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_KR_E;
                apParams.fecSupported  = GT_TRUE;
                apParams.fecRequired   = GT_TRUE;
            }
            else
            {
                apParams.laneNum       = 1;
                apParams.modesAdvertiseArr[0].ifMode =
                    CPSS_PORT_INTERFACE_MODE_XGMII_E;
                apParams.fecSupported  = GT_FALSE;
                apParams.fecRequired   = GT_FALSE;
            }
            apParams.modesAdvertiseArr[0].speed  = CPSS_PORT_SPEED_10000_E;
            apParams.modesAdvertiseArr[1].ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            apParams.modesAdvertiseArr[1].speed  = CPSS_PORT_SPEED_NA_E;

            st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
                &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortApPortConfigGet().
                Expected: GT_OK and the same parameters value as were set
                    in 1.1.1.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnableGet, &apParamsGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(apEnable, apEnableGet, devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcPause, apParamsGet.fcPause,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcAsmDir, apParamsGet.fcAsmDir,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecSupported,
                apParamsGet.fecSupported,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecRequired,
                apParamsGet.fecRequired,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.noneceDisable,
                apParamsGet.noneceDisable,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.laneNum, apParamsGet.laneNum,
                devNum, portNum);
            /* search for set parameters in returned array */
            for (i = 0; i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
            {
                if ((apParams.modesAdvertiseArr[0].ifMode ==
                            apParamsGet.modesAdvertiseArr[i].ifMode) &&
                    (apParams.modesAdvertiseArr[0].speed ==
                            apParamsGet.modesAdvertiseArr[i].speed))
                {
                    msMatched = GT_TRUE;
                    break;
                }

            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, msMatched, devNum, portNum);
            /* restore default value */
            msMatched = GT_FALSE;

            /* stop AP on port before move to next case */
            stopApOnPort(devNum, portNum);

            /*
                1.1.1. Call with
                       apEnable [GT_TRUE],
                       apParams {
                                 fcPause [GT_FALSE],
                                 fcAsmDir [CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E],
                                 fecSupported [GT_FALSE],
                                 fecRequired [GT_TRUE],
                                 noneceDisable [GT_TRUE ],
                                 laneNum [63],
                                 modesAdvertiseArr[CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS]
                                       {
                                          [
                                           {ifMode [CPSS_PORT_INTERFACE_MODE_1000BASE_X_E /
                                                    CPSS_PORT_INTERFACE_MODE_XGMII_E /
                                                    CPSS_PORT_INTERFACE_MODE_KR_E],
                                            speed  [CPSS_PORT_SPEED_1000_E /
                                                    CPSS_PORT_SPEED_10000_E /
                                                    CPSS_PORT_SPEED_40000_E]}
                                          ]
                                        }
                                }
                Expected: GT_OK.
            */

            apEnable = GT_TRUE;
            apParams.fcPause       = GT_FALSE;
            apParams.fcAsmDir      = CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;
            apParams.fecSupported  = GT_FALSE;
            apParams.fecRequired   = GT_TRUE;
            apParams.noneceDisable = GT_TRUE;
            if ((portNum % 4 != 0) && (portNum % 16 != 9))
            {
                apParams.laneNum       = 0;
                apParams.modesAdvertiseArr[0].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                apParams.modesAdvertiseArr[0].speed  = CPSS_PORT_SPEED_10000_E;
            }
            else
            {
                apParams.laneNum       = 3;
                apParams.modesAdvertiseArr[0].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
                apParams.modesAdvertiseArr[0].speed  = CPSS_PORT_SPEED_40000_E;
            }
            apParams.modesAdvertiseArr[1].ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            apParams.modesAdvertiseArr[1].speed  = CPSS_PORT_SPEED_NA_E;

            st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
                &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortApPortConfigGet().
                Expected: GT_OK and the same parameters value as were set
                    in 1.1.1.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnableGet, &apParamsGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(apEnable, apEnableGet, devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcPause, apParamsGet.fcPause,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fcAsmDir, apParamsGet.fcAsmDir,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecSupported,
                apParamsGet.fecSupported,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.fecRequired,
                apParamsGet.fecRequired,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.noneceDisable,
                apParamsGet.noneceDisable,
                devNum, portNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(apParams.laneNum, apParamsGet.laneNum,
                devNum, portNum);

            /* search for set parameters in returned array */
            for (i = 0; i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
            {
                if ((apParams.modesAdvertiseArr[0].ifMode ==
                            apParamsGet.modesAdvertiseArr[i].ifMode) &&
                    (apParams.modesAdvertiseArr[0].speed ==
                            apParamsGet.modesAdvertiseArr[i].speed))
                {
                    msMatched = GT_TRUE;
                    break;
                }

            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, msMatched, devNum, portNum);
            /* restore default value */
            msMatched = GT_FALSE;

            /*
                1.1.1. Call with
                       apEnable [GT_FALSE],
                       apParams {
                                 fcPause [GT_TRUE],
                                 fcAsmDir [CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E],
                                 fecSupported [GT_TRUE],
                                 fecRequired [GT_FALSE],
                                 noneceDisable [GT_FALSE ],
                                 laneNum [32],
                                 modesAdvertiseArr[CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS]
                                       {
                                          [
                                           {ifMode [CPSS_PORT_INTERFACE_MODE_1000BASE_X_E /
                                                    CPSS_PORT_INTERFACE_MODE_XGMII_E /
                                                    CPSS_PORT_INTERFACE_MODE_KR_E],
                                            speed  [CPSS_PORT_SPEED_1000_E /
                                                    CPSS_PORT_SPEED_10000_E /
                                                    CPSS_PORT_SPEED_40000_E]}
                                          ]
                                        }
                                }
                Expected: GT_OK.
            */
            apEnable = GT_FALSE;

            st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
                &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortApPortConfigGet().
                Expected: GT_OK and the same parameters value as were set
                    in 1.1.1.
            */
            st = cpssDxChPortApPortConfigGet(devNum, portNum,
                &apEnableGet, &apParamsGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(apEnable, apEnableGet, devNum, portNum);

            /* restore valid values */
            apEnable = GT_TRUE;

            /*
                1.1.3. Call with invalid fcAsmDir [wrong enum values] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortApPortConfigSet
                (devNum, portNum, apEnable, &apParams),
                apParams.fcAsmDir);

            /* restore valid values */
            apParams.fcAsmDir = CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;

            /*
                1.1.4. Call with out of range laneNum [64] and other
                    valid parameters same as 1.1.1.
                Expected: GT_OUT_OF_RANGE.
            */
            apParams.laneNum = 4;

            st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
                &apParams);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /* restore valid values */
            apParams.laneNum = 0;

            /*
                1.1.5. Call with invalid modesAdvertiseArr[].ifMode
                        [wrong enum values] and other
                        valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            for (i = 0; i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChPortApPortConfigSet
                    (devNum, portNum, apEnable, &apParams),
                    apParams.modesAdvertiseArr[i].ifMode);

                /* restore valid values */
                apParams.modesAdvertiseArr[i].ifMode =
                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            }

            /*
                1.1.6. Call with invalid modesAdvertiseArr[].speed
                        [wrong enum values] and other
                        valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            for (i = 0; i < CPSS_DXCH_PORT_AP_IF_ARRAY_SIZE_CNS; i++)
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChPortApPortConfigSet
                    (devNum, portNum, apEnable, &apParams),
                    apParams.modesAdvertiseArr[i].speed);

                /* restore valid values */
                apParams.modesAdvertiseArr[i].speed = CPSS_PORT_SPEED_10000_E;
            }

            /*
                1.1.7. Call with apParamsPtr[NULL] for CPSS defaults.
                Expected: GT_OK.
            */
            apEnable = GT_TRUE;
            if ((portNum % 4 == 0) || (portNum % 16 == 9))
            {
                /* Ensure AP stopped on port new enable */
                stopApOnPort(devNum, portNum);

                st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*
                    1.1.8. Call cpssDxChPortApPortConfigGet().
                    Expected: GT_OK and other parameters value the same as in CPSS
                        default parameters prvCpssDxChPortApDefaultParams structure.
                */

                st = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnableGet,
                    &apParamsGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL2_PARAM_MAC(apEnable, apEnableGet, devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, apParamsGet.fcPause,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E,
                    apParamsGet.fcAsmDir, devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, apParamsGet.fecSupported,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FALSE, apParamsGet.fecRequired,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, apParamsGet.noneceDisable,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(0, apParamsGet.laneNum,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_INTERFACE_MODE_XGMII_E,
                    apParamsGet.modesAdvertiseArr[0].ifMode,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_SPEED_10000_E,
                    apParamsGet.modesAdvertiseArr[0].speed,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
                    apParamsGet.modesAdvertiseArr[1].ifMode,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_SPEED_1000_E,
                    apParamsGet.modesAdvertiseArr[1].speed,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_INTERFACE_MODE_KR_E,
                    apParamsGet.modesAdvertiseArr[2].ifMode,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_SPEED_10000_E,
                    apParamsGet.modesAdvertiseArr[2].speed,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_INTERFACE_MODE_KR_E,
                    apParamsGet.modesAdvertiseArr[3].ifMode,
                    devNum, portNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(CPSS_PORT_SPEED_40000_E,
                    apParamsGet.modesAdvertiseArr[3].speed,
                    devNum, portNum);
            }
            else
            {
                st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            }

            /* stop AP on port before move to next case */
            stopApOnPort(devNum, portNum);

            if((ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) &&
                (speed != CPSS_PORT_SPEED_NA_E))
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
                st = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);
                st = cpssDxChPortForceLinkPassEnableSet(devNum, portNum, forceLinkPass);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
            }
        }
    }

    /* restore valid values */
    portNum   = PORT_AP_VALID_PHY_PORT_CNS;
    apEnable  = GT_TRUE;

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);

    /* Go over all non-applicable devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable,
            &apParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApPortConfigSet(devNum, portNum, apEnable, &apParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApPortStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_DXCH_PORT_AP_STATUS_STC   *apStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApPortStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with valid apStatusPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid apStatusPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_U8                           devNum   =  0;
#ifndef ASIC_SIMULATION
    GT_STATUS                       st       =  GT_OK;
    GT_PHYSICAL_PORT_NUM            portNum  =  PORT_AP_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_AP_STATUS_STC    apStatus =  { 0 };
    GT_U32 portGroupNum;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    GT_BOOL                         apEnable;
#endif

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        if (devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            st = cpssDxChPortApEnableGet(devNum, PORT_AP_ALL_PORTS_BMP_CNS, &apEnable);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            if (!apEnable)
            {
                SKIP_TEST_MAC
            }
        }

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily)
            {
                if (portNum % 16 == 0)
                { /* on first port of new port group enable AP */
                    portGroupNum = portNum / 16;
                    st = apBasicInit(devNum, portGroupNum);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);
                }
            }
            /*
                1.1.1. Call with valid apStatusPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid apStatusPtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssDxChPortApPortStatusGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL",
                devNum, portNum);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_AP_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_FALCON_E);
    /* Go over all non applicable devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApResolvedPortsBmpGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupNum,
    OUT GT_U32  *apResolvedPortsBmpPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortApResolvedPortsBmpGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Lion2)
    1.1.1. Call with valid apResolvedPortsBmpPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid apResolvedPortsBmpPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8        devNum             = 0;
#ifndef ASIC_SIMULATION
    GT_U32       portGroupNum       = 0;
    GT_U32       apResolvedPortsBmp = 0;

    GT_STATUS    st                 = GT_OK;
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |
                                        UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_CAELUM_E
                                        | UTF_ALDRIN_E | UTF_AC3X_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            st = apBasicInit(devNum, portGroupNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);

            /*
                1.1.1. Call with valid apResolvedPortsBmpPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum,
                &apResolvedPortsBmp);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum,
                portGroupNum, apResolvedPortsBmp);
            /*
                1.1.2. Call with invalid apResolvedPortsBmpPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                "%d, apResolvedPortsBmp = NULL", devNum);
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupNum)

        /* 1.2. For not-active port groups check that function
           returns GT_BAD_PARAM. */
        PRV_TGF_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)
        {
            st = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum,
                &apResolvedPortsBmp);
            if (PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st,
                    devNum, portGroupNum);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portGroupNum);
            }
        }
        PRV_TGF_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(devNum, portGroupNum)
    }

    /* 2. For not applicable devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                        UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                        UTF_XCAT3_E | UTF_XCAT2_E | UTF_LION_E |
                                        UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_CAELUM_E
                                        | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all non-applicable devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum,
            &apResolvedPortsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortApResolvedPortsBmpGet(devNum, portGroupNum,
        &apResolvedPortsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
#endif
}


#if 0 CPSS_TBD_BOOKMARK

/*---------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortApPortConfigSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_BOOL                           apEnable,
    IN  CPSS_DXCH_PORT_AP_PARAMS_STC      *apParamsPtr
)
*/
extern GT_STATUS lion2AppDemoPortsConfig
(
    IN  GT_U8                           dev,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          numOfPorts,
    ...
    );

UTF_TEST_CASE_MAC(cpssDxChPortApPortConfigSetUnSet)
{
/*
 * Following customers test caused PEX hang, implement following scenario:
 * in loop - AP enable on port, immidiate disable and configure port to 10G KR
 */
    GT_U8                           devNum      =   0;
#ifndef ASIC_SIMULATION
    GT_STATUS                       st          =   GT_OK;
    GT_PHYSICAL_PORT_NUM            portNum     =   PORT_AP_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_AP_PARAMS_STC    apParams    =   { 0 };

    GT_U32    i; /* iterator */
    GT_U32    data, addr; /* address and data AP state machine register */
    GT_U32    data1, addr1; /* address and data MMPCS register */
    GT_CHAR   errorMsg[256]; /* error message string */
    GT_U32    portGroupId;   /* core number of port */
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT   speed;
#endif

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
            UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
            UTF_XCAT2_E | UTF_LION_E | UTF_BOBCAT2_E |
            UTF_BOBCAT3_E | UTF_FALCON_E | UTF_CAELUM_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC

#ifndef ASIC_SIMULATION

        PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    apParams.fcAsmDir       = CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E;
    apParams.fcPause         = GT_FALSE;
    apParams.fecRequired   = GT_FALSE;
    apParams.fecSupported = GT_FALSE;
    apParams.laneNum        = 0;
    apParams.noneceDisable = GT_TRUE;
    apParams.modesAdvertiseArr[0].ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
    apParams.modesAdvertiseArr[0].speed  = CPSS_PORT_SPEED_10000_E;
    apParams.modesAdvertiseArr[1].ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    apParams.modesAdvertiseArr[1].speed  = CPSS_PORT_SPEED_NA_E;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
        {
            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
            if (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssDxChPortSpeedGet(devNum, portNum, &speed);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

                if (speed != CPSS_PORT_SPEED_NA_E)
                {
                    st = lion2AppDemoPortsConfig(devNum, ifMode, speed,
                        GT_FALSE, 1, portNum);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
                }
            }

            addr  = 0x370F0 + portNum * 0x4;
            addr1 = 0x88C0414 + portNum * 0x1000;
            for (i = 0; i < 10000; i++)
            {
                /* show that the function alive */
                utfPrintKeepAlive();

                st = cpssDxChPortApPortConfigSet(devNum, portNum, GT_TRUE, &apParams);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

                st = cpssDxChPortApPortConfigSet(devNum, portNum, GT_FALSE, &apParams);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

                /* read status of AP state machine on port */
                if (data != 0xdead)
                {
                    cpssOsSprintf(errorMsg, "AP not dead:0x%x=0x%x\n", addr, data);
                    (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
                }

                portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                    portNum);
                st = cpssDrvPpHwRegBitMaskRead(devNum, portGroupId, addr1, 0xffffffff, &data1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
                if (0xffffffff == data1)
                {
                    cpssOsSprintf(errorMsg, "PCS blocked 1:0x%x=0x%x\n", addr1, data1);
                    (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
                    st = cpssDrvPpHwRegBitMaskRead(devNum, portGroupId, 0x4c, 0xffffffff, &data1);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
                    if (0xffffffff == data1)
                    {
                        cpssOsSprintf(errorMsg, "PEX dead 1\n");
                        (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
                    }
                }

                st = lion2AppDemoPortsConfig(devNum, CPSS_PORT_INTERFACE_MODE_KR_E,
                    CPSS_PORT_SPEED_10000_E,
                    GT_TRUE, 1, portNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);

                st = cpssDrvPpHwRegBitMaskRead(devNum, portGroupId, addr1, 0xffffffff, &data1);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
                if (0xffffffff == data1)
                {
                    cpssOsSprintf(errorMsg,
                        "PCS blocked 2:0x%x=0x%x\n", addr1, data1);
                    (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
                    st = cpssDrvPpHwRegBitMaskRead(devNum, portGroupId, 0x4c, 0xffffffff, &data1);
                    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
                    if (0xffffffff == data1)
                    {
                        cpssOsSprintf(errorMsg, "PEX dead 2\n");
                        (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
                    }
                }

            } /* for(i = 0; i < 10000; i++) */

            if ((ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
                && (speed != CPSS_PORT_SPEED_NA_E))
            {
                st = lion2AppDemoPortsConfig(devNum, ifMode, speed,
                    GT_TRUE, 1, portNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
            }
            else
            {
                st = lion2AppDemoPortsConfig(devNum, CPSS_PORT_INTERFACE_MODE_KR_E,
                    CPSS_PORT_SPEED_10000_E,
                    GT_FALSE, 1, portNum);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, portNum);
            }

        } /* while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE)) */

    } /* while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) */
#endif

}

#endif

/*---------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortAp suit
 */

UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortAp)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApPortConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApPortConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApPortStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortApResolvedPortsBmpGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortAp)


