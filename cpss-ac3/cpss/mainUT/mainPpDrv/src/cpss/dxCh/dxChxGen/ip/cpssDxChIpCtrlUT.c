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
* @file cpssDxChIpCtrlUT.c
*
* @brief Unit Tests for the CPSS DXCH Ip HW control registers APIs
*
* @version   79
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Invalid enumeration value used for testing */
#define IP_CTRL_INVALID_ENUM_CNS       0x5AAAAAA5

/* Default valid value for port id */
#define IP_CTRL_VALID_PHY_PORT_CNS  0


/* macro to check that device supports 'range' of physical ports rather then only 'existing physical ports' */
#define IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(_dev,_port)   \
    (UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) && ((_port) <= (UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(_dev)-1)))

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpSpecialRouterTriggerEnable
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    IN GT_BOOL                         enableRouterTrigger
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpSpecialRouterTriggerEnable)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with bridgeExceptionCmd [CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E]
               and enableRouterTrigger [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpSpecialRouterTriggerEnableGet.
    Expected: GT_OK and same enableRouterTrigger.
    1.3. Call with wrong enum values bridgeExceptionCmd and enableRouterTrigger [GT_TRUE].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd;
    GT_BOOL                         enableRouterTrigger;
    GT_BOOL                         enableRouterTriggerGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with bridgeExceptionCmd
           [CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E /
           CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E /
           CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E /
           CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E /
           CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E] and enableRouterTrigger
           [GT_FALSE and GT_TRUE]. Expected: GT_OK.    */

        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E;
        enableRouterTrigger = GT_FALSE;

        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd, enableRouterTrigger);

        /* 1.2. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTriggerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableRouterTrigger, enableRouterTriggerGet,
            "cpssDxChIpSpecialRouterTriggerEnableGet: get another enableRouterTrigger than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E;
        enableRouterTrigger = GT_FALSE;

        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd, enableRouterTrigger);

        /* 1.2. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTriggerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableRouterTrigger, enableRouterTriggerGet,
            "cpssDxChIpSpecialRouterTriggerEnableGet: get another enableRouterTrigger than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E;
        enableRouterTrigger = GT_TRUE;

        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd, enableRouterTrigger);

        /* 1.2. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTriggerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableRouterTrigger, enableRouterTriggerGet,
            "cpssDxChIpSpecialRouterTriggerEnableGet: get another enableRouterTrigger than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E;
        enableRouterTrigger = GT_TRUE;

        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd, enableRouterTrigger);

        /* 1.2. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTriggerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableRouterTrigger, enableRouterTriggerGet,
            "cpssDxChIpSpecialRouterTriggerEnableGet: get another enableRouterTrigger than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E;
        enableRouterTrigger = GT_TRUE;

        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd, enableRouterTrigger);

        /* 1.2. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTriggerGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableRouterTrigger, enableRouterTriggerGet,
            "cpssDxChIpSpecialRouterTriggerEnableGet: get another enableRouterTrigger than was set: dev = %d",
                                     dev);

        /*
            1.3. Call with wrong enum values bridgeExceptionCmd and enableRouterTrigger [GT_TRUE].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSpecialRouterTriggerEnable
                            (dev, bridgeExceptionCmd, enableRouterTrigger),
                            bridgeExceptionCmd);
    }

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E;
    enableRouterTrigger = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpSpecialRouterTriggerEnable(dev, bridgeExceptionCmd, enableRouterTrigger);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpSpecialRouterTriggerEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd,
    OUT GT_BOOL                         *enableRouterTriggerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpSpecialRouterTriggerEnableGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with bridgeExceptionCmd [CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E /
                                       CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E]
               and not NULL enableRouterTrigger.
    Expected: GT_OK.
    1.2. Call with wrong enum values bridgeExceptionCmd.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL enableRouterTriggerPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT   bridgeExceptionCmd;
    GT_BOOL                         enableRouterTrigger;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E;
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E;
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E;
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E;
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E;
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, bridgeExceptionCmd);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSpecialRouterTriggerEnableGet
                            (dev, bridgeExceptionCmd, &enableRouterTrigger),
                            bridgeExceptionCmd);
        /* 1.1. */
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, bridgeExceptionCmd);

    }

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E;
    enableRouterTrigger = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpSpecialRouterTriggerEnableGet(dev, bridgeExceptionCmd, &enableRouterTrigger);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpExceptionCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN CPSS_PACKET_CMD_ENT              command
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpExceptionCommandSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                  CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                  CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                   protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                  CPSS_IP_PROTOCOL_IPV6_E /
                                  CPSS_IP_PROTOCOL_IPV4V6_E]
                    and command [CPSS_PACKET_CMD_DROP_HARD_E /
                                 CPSS_PACKET_CMD_DROP_SOFT_E /
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E].
       Expected: GT_OK.
    1.2. Call with exceptionType[CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                 CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E /
                                 CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E],
                  protocolStack [CPSS_IP_PROTOCOL_IPV4_E]
         and command [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] (not possible to set).
    Expected: non GT_OK.
    1.3. Call with wrong enum values exceptionType and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values protocolStack and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum values command and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                        st = GT_OK;
    GT_U8                            dev;
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType;
    CPSS_IP_PROTOCOL_STACK_ENT       protocolStack;
    CPSS_PACKET_CMD_ENT              command;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                          CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                          CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                           protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                          CPSS_IP_PROTOCOL_IPV6_E /
                                          CPSS_IP_PROTOCOL_IPV4V6_E]
                            and command [CPSS_PACKET_CMD_DROP_HARD_E /
                                         CPSS_PACKET_CMD_DROP_SOFT_E /
                                         CPSS_PACKET_CMD_TRAP_TO_CPU_E].
           Expected: GT_OK.
        */
        /* call with first group of params values */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* call with second group of params values */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* call with third group of params values */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);


        /*
            1.2. Call with exceptionType[CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                         CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E /
                                         CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E],
                          protocolStack [CPSS_IP_PROTOCOL_IPV4_E]
                 and command [CPSS_PACKET_CMD_MIRROR_TO_CPU_E] (not possible to set).
            Expected: non GT_OK.
        */

        /*call with exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E; */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);

        /*call with exceptionType = CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E; */
        exceptionType = CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);

        /*call with exceptionType = CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E; */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E;

        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack, command);

        /*
            1.3. Call with wrong enum values exceptionType and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpExceptionCommandSet
                            (dev, exceptionType, protocolStack, command),
                            exceptionType);

        /*
            1.4. Call with wrong enum values protocolStack and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpExceptionCommandSet
                            (dev, exceptionType, protocolStack, command),
                            protocolStack);

        /*
            1.5. Call with wrong enum values command and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpExceptionCommandSet
                           (dev, exceptionType, protocolStack, command),
                           command);
    }

    exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    command = CPSS_PACKET_CMD_DROP_HARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpExceptionCmdGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_EXMXPM_IP_EXCEPTION_TYPE_ENT   exceptionType,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT CPSS_PACKET_CMD_ENT                 *exceptionCmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpExceptionCmdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                  CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                  CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                   protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                  CPSS_IP_PROTOCOL_IPV6_E /
                                  CPSS_IP_PROTOCOL_IPV4V6_E]
                   and non-NULL exceptionCmdPtr.
    Expected: GT_OK.
    1.2. Call with exceptionType[CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E /
                                 CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E /
                   protocolStack [CPSS_IP_PROTOCOL_IPV4_E] (this exceptionType are for IPv6 only)
                   and non-NULL exceptionCmdPtr.
    Expected: non GT_OK.
    1.3. Call with out of range exceptionType [0x5AAAAAA5]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E] (not supported)
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range protocolStack [0x5AAAAAA5]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with exceptionCmdPtr [NULL]
                   and other parameters from 1.1.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT         exceptionType;
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack =CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_PACKET_CMD_ENT                     exceptionCmd  = CPSS_PACKET_CMD_NONE_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
             1.1. Call with exceptionType  [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                            CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                            CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                            protocolStack  [CPSS_IP_PROTOCOL_IPV4_E /
                                            CPSS_IP_PROTOCOL_IPV6_E /
                                            CPSS_IP_PROTOCOL_IPV4V6_E]
                            and non-NULL exceptionCmdPtr.
            Expected: GT_OK.
        */

        if (PRV_CPSS_PP_MAC(dev)->devFamily==CPSS_PP_FAMILY_DXCH_XCAT2_E ||
            PRV_CPSS_PP_MAC(dev)->devFamily==CPSS_PP_FAMILY_CHEETAH2_E   ||
            PRV_CPSS_PP_MAC(dev)->devFamily==CPSS_PP_FAMILY_CHEETAH_E)
        {
            exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
            exceptionCmd=CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            st = cpssDxChIpExceptionCommandSet(dev, exceptionType, protocolStack,exceptionCmd);
        }

        else
        {
            exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E ;
        }

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack,&exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack);

        /* Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E ] */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E ;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack,&exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack);

        /* Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E] */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack,&exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack);
        /*
        1.2. Call with  exceptionType [CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E /
                        protocolStack [CPSS_IP_PROTOCOL_IPV4_E] (this exceptionType are for IPv6 only)
                        and non-NULL exceptionCmdPtr.
             Expected: NOT GT_OK.
        */

        exceptionType = CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, exceptionType, protocolStack);

        /*
            1.3. Call with out of range exceptionType [0x5AAAAAA5]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */

        exceptionType = IP_CTRL_INVALID_ENUM_CNS;


       st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, exceptionType);



        /*
            1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E] (not supported)
                           and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E ;
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, protocolStack = %d", dev, protocolStack);

        /*
            1.5. Call with out of range protocolStack [0x5AAAAAA5]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */

        protocolStack = IP_CTRL_INVALID_ENUM_CNS;

        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, protocolStack = %d", dev, protocolStack);

        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;


        /*
            1.6. Call with exceptionCmdPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, exceptionCmdPtr = NULL", dev);
    }

    exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E ;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.
                         */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpExceptionCommandGet(dev, exceptionType, protocolStack, &exceptionCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRouteAgingModeSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  refreshEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRouteAgingModeSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with refreshEnable [GT_TRUE and GT_FALSE].
Expected: GT_OK.
1.2. Call cpssDxChIpUcRouteAgingModeGet.
Expected: GT_OK and same refreshEnable.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                 refreshEnable;
    GT_BOOL                 refreshEnableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with refreshEnable [GT_TRUE and GT_FALSE].
           Expected: GT_OK. */

        refreshEnable = GT_TRUE;

        st = cpssDxChIpUcRouteAgingModeSet(dev, refreshEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, refreshEnable);

        /* 1.2.  */
        st = cpssDxChIpUcRouteAgingModeGet(dev, &refreshEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(refreshEnable, refreshEnableGet,
            "cpssDxChIpUcRouteAgingModeGet: get another refreshEnable than was set: dev = %d",
                                     dev);

        refreshEnable = GT_FALSE;

        st = cpssDxChIpUcRouteAgingModeSet(dev, refreshEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, refreshEnable);

        /* 1.2.  */
        st = cpssDxChIpUcRouteAgingModeGet(dev, &refreshEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(refreshEnable, refreshEnableGet,
            "cpssDxChIpUcRouteAgingModeGet: get another refreshEnable than was set: dev = %d",
                                     dev);
    }

    refreshEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRouteAgingModeSet(dev, refreshEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRouteAgingModeSet(dev, refreshEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRouteAgingModeGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *refreshEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRouteAgingModeGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with not NULL refreshEnable.
Expected: GT_OK.
1.2. Call with NULL refreshEnable.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                 refreshEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.  */
        st = cpssDxChIpUcRouteAgingModeGet(dev, &refreshEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1.  */
        st = cpssDxChIpUcRouteAgingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRouteAgingModeGet(dev, &refreshEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRouteAgingModeGet(dev, &refreshEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterSourceIdSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E]
        sourceIdMask [0 / 0xFFF] and sourceId [0 / 31].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterSourceIdGet.
    Expected: GT_OK and same sourceId and mask.
    1.3. Call with sourceId[32] and other valid parameters from 1.1.
    Expected: non GT_OK.
    1.4. Call with wrong enum values ucMcSet and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with ucMcSet [CPSS_IP_MULTICAST_E] and wrong value sourceIdMask [0xFFFF].
    Expected: non GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceId;
    GT_U32                          sourceIdGet;
    GT_U32                          sourceIdMask = 0xFFF;
    GT_U32                          sourceIdMaskGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E]
           and sourceId [0 / 31]. Expected: GT_OK.   */

        ucMcSet = CPSS_IP_UNICAST_E;
        sourceId = 0;
        sourceIdMask = 0;

        st = cpssDxChIpRouterSourceIdSet(dev, ucMcSet, sourceId, sourceIdMask);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucMcSet, sourceId);

        /* 1.2. */
        st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceIdGet, &sourceIdMaskGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ucMcSet);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(sourceId, sourceIdGet,
            "cpssDxChIpRouterSourceIdGet: get another sourceId than was set: dev = %d",
                                     dev);

        ucMcSet = CPSS_IP_MULTICAST_E;
        sourceId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(dev);
        sourceIdMask = 0xFFF;

        st = cpssDxChIpRouterSourceIdSet(dev, ucMcSet, sourceId, sourceIdMask);

        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucMcSet, sourceId);
            /* 1.2. */
            st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceIdGet, &sourceIdMaskGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ucMcSet);

            /* Verifying values */
            if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(sourceIdMask, sourceIdMaskGet,
                    "cpssDxChIpRouterSourceIdGet: get another sourceIdMask than was set: dev = %d", dev);
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(sourceId, sourceIdGet,
                "cpssDxChIpRouterSourceIdGet: get another sourceId than was set: dev = %d", dev);
        }
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3. Call with sourceId[32] and other valid parameters
           from 1.1. Expected: non GT_OK.   */

        ucMcSet = CPSS_IP_UNICAST_E;
        sourceId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(dev) + 1;

        st = cpssDxChIpRouterSourceIdSet(dev, ucMcSet, sourceId, sourceIdMask);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucMcSet, sourceId);

        sourceId = 0;

        /*
            1.4. Call with wrong enum values ucMcSet  and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpRouterSourceIdSet
                            (dev, ucMcSet, sourceId, sourceIdMask),
                            ucMcSet);
        }

        /*
            1.5. Call with ucMcSet [CPSS_IP_MULTICAST_E] and wrong value sourceIdMask [0xFFFF].
            Expected: non GT_OK.
        */
        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            st = cpssDxChIpRouterSourceIdSet(dev, CPSS_IP_MULTICAST_E, 0, 0xFFFF);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucMcSet, sourceId);
        }
    }

    ucMcSet = CPSS_IP_UNICAST_E;
    sourceId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterSourceIdSet(dev, ucMcSet, sourceId, sourceIdMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterSourceIdSet(dev, ucMcSet, sourceId, sourceIdMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterSourceIdGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E].
    Expected: GT_OK.
    1.2. Call with wrong enum values ucMcSet and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL sourceIdPtr.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL sourceIdMaskPtr (eArch).
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet;
    GT_U32                          sourceId = 0;
    GT_U32                          sourceIdMask = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */

        ucMcSet = CPSS_IP_UNICAST_E;

        st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceId, &sourceIdMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ucMcSet);

        /* 1.1. */
        ucMcSet = CPSS_IP_MULTICAST_E;
        sourceId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(dev);

        st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceId, &sourceIdMask);

        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ucMcSet);
        else
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*  1.2.  */
        if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChIpRouterSourceIdGet
                            (dev, ucMcSet, &sourceId, &sourceIdMask),
                            ucMcSet);
        }

        /* 1.3. */
        st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, NULL, &sourceIdMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, ucMcSet);

        /* 1.4. */
        if (GT_TRUE == (PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.eArch.supported))
        {
            st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceId, NULL);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucMcSet, sourceId);
        }
    }

    ucMcSet = CPSS_IP_UNICAST_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceId, &sourceIdMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterSourceIdGet(dev, ucMcSet, &sourceId, &sourceIdMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupRouterSourceIdSet
(
    IN  GT_U8                           devNum,
    IN GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupRouterSourceIdSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChx)
    1.1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E],
                     sourceId [0 / 31].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortGroupRouterSourceIdGet.
    Expected: GT_OK and same sourceId.
    1.1.3. Call with sourceId[32] and other valid parameters from 1.1.1.
    Expected: non GT_OK.
    1.1.4. Call with wrong enum values ucMcSet and other valid parameters from 1.1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32             portGroupId   = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;

    CPSS_IP_UNICAST_MULTICAST_ENT ucMcSet  = CPSS_IP_UNICAST_E;
    GT_U32                        sourceId = 0;
    GT_U32                        sourceIdGet = 0;
    GT_U32                        sourceIdMask = 0xFFF;
    GT_U32                        sourceIdMaskGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E],
                                 sourceId [0 / 31].
                Expected: GT_OK.
            */
            ucMcSet  = CPSS_IP_UNICAST_E;
            sourceId = 0;

            st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp,
                                                      ucMcSet, sourceId, sourceIdMask);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet, sourceId);

            /* 1.1.2. */
            st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                      ucMcSet, &sourceIdGet, &sourceIdMaskGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(sourceId, sourceIdGet,
                "cpssDxChIpPortGroupRouterSourceIdGet: get another sourceId than was set: dev = %d",
                                         dev);

            ucMcSet = CPSS_IP_MULTICAST_E;
            sourceId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(dev);

            st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);

            if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet, sourceId);

                /* 1.1.2. */
                st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                          ucMcSet, &sourceIdGet, &sourceIdMaskGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(sourceId, sourceIdGet,
                    "cpssDxChIpPortGroupRouterSourceIdGet: get another sourceId than was set: dev = %d",
                                             dev);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
            }

            /*
                1.1.3. Call with sourceId[32] and other valid parameters from 1.1.1.
                Expected: non GT_OK.
            */
            ucMcSet = CPSS_IP_UNICAST_E;
            sourceId = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(dev) + 1;

            st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);
            UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet, sourceId);

            sourceId = 0;

            /*
                1.1.4. Call with wrong enum values ucMcSet  and other valid parameters from 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChIpPortGroupRouterSourceIdSet
                                    (dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask),
                                    ucMcSet);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    ucMcSet  = CPSS_IP_UNICAST_E;
    sourceId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupRouterSourceIdSet(dev, portGroupsBmp, ucMcSet, sourceId, sourceIdMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupRouterSourceIdGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupRouterSourceIdGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxChx)
    1.1.1. Call with ucMcSet [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E].
    Expected: GT_OK.
    1.1.2. Call with wrong enum values ucMcSet and other valid parameters from 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with NULL sourceIdPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st  = GT_OK;
    GT_U8     dev = 0;
    GT_U32             portGroupId   = 0;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;

    CPSS_IP_UNICAST_MULTICAST_ENT ucMcSet  = CPSS_IP_UNICAST_E;
    GT_U32                        sourceId = 0;
    GT_U32                        sourceIdMask = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*  1.1.1.  */
            ucMcSet  = CPSS_IP_UNICAST_E;

            st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                      ucMcSet, &sourceId, &sourceIdMask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet);

            /*  1.1.1.  */
            ucMcSet = CPSS_IP_MULTICAST_E;

            st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp, ucMcSet, &sourceId, &sourceIdMask);
            if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portGroupsBmp, ucMcSet, sourceId);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
            }

            /*  1.1.2.  */
            if ((PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E) && (PRV_CPSS_PP_MAC(dev)->devFamily != CPSS_PP_FAMILY_CHEETAH_E))
            {
                UTF_ENUMS_CHECK_MAC(cpssDxChIpPortGroupRouterSourceIdGet
                                    (dev, portGroupsBmp, ucMcSet, &sourceId, &sourceIdMask),
                                    ucMcSet);
            }

            /*  1.1.3.  */
            st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp, ucMcSet, NULL, &sourceIdMask);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp, ucMcSet);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                      ucMcSet, &sourceId, &sourceIdMask);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        ucMcSet  = CPSS_IP_UNICAST_E;

        st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                  ucMcSet, &sourceId, &sourceIdMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    ucMcSet  = CPSS_IP_UNICAST_E;
    sourceId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                                  ucMcSet, &sourceId, &sourceIdMask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupRouterSourceIdGet(dev, portGroupsBmp,
                                              ucMcSet, &sourceId, &sourceIdMask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ctrlMultiTargetTCQueue,
    IN  GT_U32  failRpfMultiTargetTCQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with ctrlMultiTargetTCQueue [0 / 3] and failRpfMultiTargetTCQueue [3 / 0].
Expected: GT_OK.
1.2. Call cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet.
Expected: GT_OK and the same ctrlMultiTargetTCQueue, failRpfMultiTargetTCQueue.
1.3. Call with ctrlMultiTargetTCQueue [4] and failRpfMultiTargetTCQueue[0]
Expected: NOT GT_OK.
1.4. Call with ctrlMultiTargetTCQueue[0] and failRpfMultiTargetTCQueue[4]
Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  ctrlMultiTargetTCQueue;
    GT_U32                  failRpfMultiTargetTCQueue;
    GT_U32                  ctrlMultiTargetTCQueueGet;
    GT_U32                  failRpfMultiTargetTCQueueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with ctrlMultiTargetTCQueue [0 / 3] and
           failRpfMultiTargetTCQueue [3 / 0]. Expected: GT_OK.  */

        ctrlMultiTargetTCQueue = 0;
        failRpfMultiTargetTCQueue = 3;

        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                                failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ctrlMultiTargetTCQueue,
                                                    failRpfMultiTargetTCQueue);

        /* 1.2. */
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueueGet,
                                                                &failRpfMultiTargetTCQueueGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ctrlMultiTargetTCQueueGet,
                                                    failRpfMultiTargetTCQueueGet);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlMultiTargetTCQueue, ctrlMultiTargetTCQueueGet,
            "cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet: get another ctrlMultiTargetTCQueue than was set: dev = %d",
                                     dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(failRpfMultiTargetTCQueue, failRpfMultiTargetTCQueueGet,
            "cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet: get another failRpfMultiTargetTCQueueGet than was set: dev = %d",
                                     dev);

        /*  1.1  */
        ctrlMultiTargetTCQueue = 3;
        failRpfMultiTargetTCQueue = 0;

        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                                failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ctrlMultiTargetTCQueue,
                                                    failRpfMultiTargetTCQueue);

        /* 1.2. */
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueueGet,
                                                                &failRpfMultiTargetTCQueueGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ctrlMultiTargetTCQueueGet,
                                                    failRpfMultiTargetTCQueueGet);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ctrlMultiTargetTCQueue, ctrlMultiTargetTCQueueGet,
            "cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet: get another ctrlMultiTargetTCQueue than was set: dev = %d",
                                     dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(failRpfMultiTargetTCQueue, failRpfMultiTargetTCQueueGet,
            "cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet: get another failRpfMultiTargetTCQueueGet than was set: dev = %d",
                                     dev);

        /* 1.3. Call with ctrlMultiTargetTCQueue [4]
           and failRpfMultiTargetTCQueue[0]. Expected: NOT GT_OK.   */

        ctrlMultiTargetTCQueue = 4;
        failRpfMultiTargetTCQueue = 0;

        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                                failRpfMultiTargetTCQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ctrlMultiTargetTCQueue);

        /* 1.4. Call with ctrlMultiTargetTCQueue[0] and
           failRpfMultiTargetTCQueue[4]. Expected: NOT GT_OK.   */

        ctrlMultiTargetTCQueue = 0;
        failRpfMultiTargetTCQueue = 4;

        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                                failRpfMultiTargetTCQueue);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, failRpfMultiTargetTCQueue = %d",
                                         dev, failRpfMultiTargetTCQueue);
    }

    ctrlMultiTargetTCQueue = 0;
    failRpfMultiTargetTCQueue = 3;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                                failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet(dev, ctrlMultiTargetTCQueue,
                                                            failRpfMultiTargetTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet
(
    IN   GT_U8   devNum,
    OUT  GT_U32  *ctrlMultiTargetTCQueuePtr,
    OUT  GT_U32  *failRpfMultiTargetTCQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with not NULL ctrlMultiTargetTCQueuePtr, failRpfMultiTargetTCQueuePtr.
Expected: GT_OK.
1.2. Call with NULL ctrlMultiTargetTCQueuePtr.
Expected: GT_BAD_PTR.
1.3. Call with NULL failRpfMultiTargetTCQueuePtr.
Expected: GT_BAD_PTR.

*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  ctrlMultiTargetTCQueue;
    GT_U32                  failRpfMultiTargetTCQueue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueue,
                                                                &failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, NULL,
                                                                &failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueue,
                                                                NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueue,
                                                                &failRpfMultiTargetTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet(dev, &ctrlMultiTargetTCQueue,
                                                            &failRpfMultiTargetTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    IN  GT_U32  multiTargeTCQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpQosProfileToMultiTargetTCQueueMapSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with qosProfile [1 / 100 / 127] and multiTargeTCQueue [1 / 2 / 3].
Expected: GT_OK.
1.2. Call cpssDxChIpQosProfileToMultiTargetTCQueueMapGet.
Expected: GT_OK and same multiTargeTCQueue.
1.3. Call with qosProfile [1] and multiTargeTCQueue [1] (already set).
Expected: GT_OK.
1.4. Call with qosProfile [128] (out of range) and multiTargeTCQueue [0] (out of range)
Expected: NOT GT_OK
1.5. Call with qosProfile [0] and multiTargeTCQueue [4] (out of range)
Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  qosProfile;
    GT_U32                  multiTargeTCQueue;
    GT_U32                  multiTargeTCQueueGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with qosProfile [1 / 100 / 127] and multiTargeTCQueue
           [1 / 2 / 3]. Expected: GT_OK.    */
        qosProfile = 1;
        multiTargeTCQueue = 1;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    multiTargeTCQueue);

        /*  1.2.  */
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargeTCQueue, multiTargeTCQueueGet,
            "cpssDxChIpQosProfileToMultiTargetTCQueueMapGet: get another multiTargeTCQueueGet than was set: dev = %d",
                                     dev);

        qosProfile = 100;
        multiTargeTCQueue = 2;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    multiTargeTCQueue);

        /*  1.2.  */
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargeTCQueue, multiTargeTCQueueGet,
            "cpssDxChIpQosProfileToMultiTargetTCQueueMapGet: get another multiTargeTCQueueGet than was set: dev = %d",
                                     dev);

        qosProfile = 127;
        multiTargeTCQueue = 3;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    multiTargeTCQueue);

        /*  1.2.  */
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueueGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargeTCQueue, multiTargeTCQueueGet,
            "cpssDxChIpQosProfileToMultiTargetTCQueueMapGet: get another multiTargeTCQueueGet than was set: dev = %d",
                                     dev);

        /* 1.3. Call with qosProfile [1] and multiTargeTCQueue [1] (already set).
            Expected: GT_OK.    */

        qosProfile = 1;
        multiTargeTCQueue = 1;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    multiTargeTCQueue);

        /* 1.4. Call with qosProfile [128] (out of range) and multiTargeTCQueue [0]
           (out of range) Expected: NOT GT_OK   */
        qosProfile = PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(dev);
        multiTargeTCQueue = 0;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, qosProfile = %d",
                                         dev, qosProfile);

        /* 1.5. Call with qosProfile [0] and multiTargeTCQueue [4]
           (out of range) Expected: NOT GT_OK   */

        qosProfile = 0;
        multiTargeTCQueue = 4;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, multiTargeTCQueue = %d",
                                         dev, multiTargeTCQueue);
    }

    qosProfile = 1;
    multiTargeTCQueue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                                multiTargeTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpQosProfileToMultiTargetTCQueueMapSet(dev, qosProfile,
                                                            multiTargeTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpQosProfileToMultiTargetTCQueueMapGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  qosProfile,
    OUT GT_U32  *multiTargetTCQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpQosProfileToMultiTargetTCQueueMapGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with qosProfile [1 / 100 / 127] and not NULL multiTargeTCQueue.
Expected: GT_OK.
1.3. Call with qosProfile [128] (out of range).
Expected: NOT GT_OK
1.4. Call with NULL multiTargetTCQueuePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  qosProfile;
    GT_U32                  multiTargeTCQueue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.  */
        qosProfile = 1;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.1.  */
        qosProfile = 100;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.1.  */
        qosProfile = 127;

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.2.  */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            qosProfile = 1024;
        }
        else
        {
            qosProfile = 128;
        }

        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);
        qosProfile = 0;

        /* 1.3.  */
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, qosProfile);
    }

    qosProfile = 1;
    multiTargeTCQueue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                                &multiTargeTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpQosProfileToMultiTargetTCQueueMapGet(dev, qosProfile,
                                                            &multiTargeTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntGet
(
    IN   GT_U8    devNum,
    OUT  GT_U32   *dropPktsPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetQueueFullDropCntGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with non-null dropPktsPtr. Expected: GT_OK.
1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropPktsPtr. Expected: GT_OK.    */

        st = cpssDxChIpMultiTargetQueueFullDropCntGet(dev, &dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR. */

        st = cpssDxChIpMultiTargetQueueFullDropCntGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropPktsPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetQueueFullDropCntGet(dev, &dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetQueueFullDropCntGet(dev, &dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetQueueFullDropCntSet
(
    IN GT_U8     devNum,
    IN GT_U32    dropPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetQueueFullDropCntSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with dropPkts [10]. Expected: GT_OK.
1.2. Call cpssDxChIpMultiTargetQueueFullDropCntGet. Expected: GT_OK and the same dropPkts.
1.3. Call with dropPkts [0xFFFFFFFF] (no any onstraints). Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropPkts;
    GT_U32                  dropPktsRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with dropPkts [10]. Expected: GT_OK.   */

        dropPkts = 10;

        st = cpssDxChIpMultiTargetQueueFullDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);

        /* 1.2. Call cpssDxChIpMultiTargetQueueFullDropCntGet.
           Expected: GT_OK and the same dropPkts.   */

        st = cpssDxChIpMultiTargetQueueFullDropCntGet(dev, &dropPktsRet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(dropPkts, dropPktsRet,
            "cpssDxChIpMultiTargetQueueFullDropCntGet: get another value than was set: dev = %d, dropPkts = %d",
                                     dev, dropPktsRet);

        /* 1.3. Call with dropPkts [0xFFFFFFFF] (no any constraints).
           Expected: GT_OK. */

        dropPkts = 0xFFFFFFFF;

        st = cpssDxChIpMultiTargetQueueFullDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);
    }

    dropPkts = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetQueueFullDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetQueueFullDropCntSet(dev, dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    IN  CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingMode,
    IN  GT_U32                                  queueWeight
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetTCQueueSchedModeSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with multiTargeTCQueue [0 / 3], schedulingMode[CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E / CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E], queueWeight[10 / 255].
Expected: GT_OK.
1.2. Call cpssDxChIpMultiTargetTCQueueSchedModeGet.
Expected: GT_OK and the same schedulingMode.
1.3. Call with multiTargeTCQueue [0], schedulingMode[CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E], queueWeight[10/255].
Expected: GT_OK.
1.4. Check out-of-range for queueWeight. Call with multiTargeTCQueue [0], schedulingMode[CPSS_DXCH_IP_MT_TC_QUEUE_ SDWRR _SCHED_MODE_E], queueWeight[256].
Expected: non GT_OK.
1.5. Call with out-of-range multiTargeTCQueue [4] and other parameters from 1.1.
Expected: NOT GT_OK.
1.6. Call with wrong enum values schedulingMode  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                                  multiTargetTcQueue;
    CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingMode;
    GT_U32                                  queueWeight;
    CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingModeGet;
    GT_U32                                  queueWeightGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        multiTargetTcQueue = 0;
        schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E;
        queueWeight = 10;

        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue,
                                    schedulingMode, queueWeight);

        /*  1.2. */
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingModeGet, &queueWeightGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(schedulingMode, schedulingModeGet,
            "cpssDxChIpMultiTargetTCQueueSchedModeGet: get another schedulingMode than was set: dev = %d",
                                     dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(queueWeight, queueWeightGet,
            "cpssDxChIpMultiTargetTCQueueSchedModeGet: get another queueWeight than was set: dev = %d",
                                     dev);

        /*  1.1.  */
        queueWeight = 255;
        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue,
                                    schedulingMode, queueWeight);

        /*  1.2. */
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingModeGet, &queueWeightGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(schedulingMode, schedulingModeGet,
            "cpssDxChIpMultiTargetTCQueueSchedModeGet: get another schedulingMode than was set: dev = %d",
                                     dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(queueWeight, queueWeightGet,
            "cpssDxChIpMultiTargetTCQueueSchedModeGet: get another queueWeight than was set: dev = %d",
                                     dev);

        /*  1.1.  */
        multiTargetTcQueue = 3;
        schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E;

        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue,
                                    schedulingMode, queueWeight);

        /*  1.2. */
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingModeGet, &queueWeightGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(schedulingMode, schedulingModeGet,
            "cpssDxChIpMultiTargetTCQueueSchedModeGet: get another schedulingMode than was set: dev = %d",
                                     dev);

        /* 1.3.  */
        multiTargetTcQueue = 0;
        schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E;
        queueWeight = 10;

        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue,
                                    schedulingMode, queueWeight);

        /* 1.4.   */
        multiTargetTcQueue = 0;
        schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E;
        queueWeight = 256;

        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, queueWeight = %d", dev, queueWeight);

        /* 1.5.  */
        multiTargetTcQueue = 4;
        schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E;
        queueWeight = 10;

        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, multiTargetTcQueue = %d",
                                         dev,  multiTargetTcQueue);

        multiTargetTcQueue = 0; /* restore */

        /* 1.6.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpMultiTargetTCQueueSchedModeSet
                            (dev, multiTargetTcQueue, schedulingMode, queueWeight),
                            schedulingMode);
    }

    multiTargetTcQueue = 0;
    schedulingMode = CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E;
    queueWeight = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                      schedulingMode, queueWeight);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetTCQueueSchedModeSet(dev, multiTargetTcQueue,
                                                  schedulingMode, queueWeight);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetTCQueueSchedModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  multiTargetTcQueue,
    OUT CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT *schedulingModePtr,
    OUT GT_U32                                  *queueWeightPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetTCQueueSchedModeGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with multiTargeTCQueue [0 / 3] and not NULL pointers.
Expected: GT_OK.
1.2. Call with out of range multiTargeTCQueue [4] and not NULL pointers.
Expected: not GT_OK.
1.3. Call with NULL schedulingModePtr.
Expected: GT_BAD_PTR.
1.4. Call with NULL queueWeightPtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                                  multiTargetTcQueue;
    CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT schedulingMode;
    GT_U32                                  queueWeight = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        multiTargetTcQueue = 0;

        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingMode, &queueWeight);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);

        /*  1.1.  */
        multiTargetTcQueue = 3;

        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingMode, &queueWeight);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);

        /*  1.2.  */
        multiTargetTcQueue = 4;

        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingMode, &queueWeight);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev,  multiTargetTcQueue);
        multiTargetTcQueue = 0;

        /*  1.3.  */
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      NULL, &queueWeight);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,  multiTargetTcQueue);

        /*  1.4.  */
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingMode, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev,  multiTargetTcQueue);
    }

    multiTargetTcQueue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                      &schedulingMode, &queueWeight);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetTCQueueSchedModeGet(dev, multiTargetTcQueue,
                                                  &schedulingMode, &queueWeight);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpBridgeServiceEnable
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    IN  GT_BOOL                                             enableService
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpBridgeServiceEnable)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with bridgeService [CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E / CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E / CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E], enableDisableMode [CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E / CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E / CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E] and enableService [GT_FALSE and GT_ TRUE].
Expected: GT_OK.
1.2. Call cpssDxChIpBridgeServiceEnable.
Expected: GT_OK and the same enableService.
1.3. Call with CPSS_DXCH_IP_BRG_SERVICE_LAST_E and wrong enum values bridgeService and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with wrong enum values enableDisableMode  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode;
    GT_BOOL                                             enableService;
    GT_BOOL                                             enableServiceGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
        enableService = GT_FALSE;

        st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                           enableService);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode,
                                           enableService);

        /* 1.2. */
        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableServiceGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableService, enableServiceGet,
            "cpssDxChIpBridgeServiceEnableGet: get another enableService than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeService = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;
        enableService = GT_TRUE;

        st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                           enableService);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode,
                                           enableService);

        /* 1.2. */
        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableServiceGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableService, enableServiceGet,
            "cpssDxChIpBridgeServiceEnableGet: get another enableService than was set: dev = %d",
                                     dev);

        /* 1.1. */
        bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;
        enableService = GT_TRUE;

        st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                           enableService);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode,
                                           enableService);

        /* 1.2. */
        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableServiceGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enableService, enableServiceGet,
            "cpssDxChIpBridgeServiceEnableGet: get another enableService than was set: dev = %d",
                                     dev);

        /* 1.3. */
        bridgeService = CPSS_DXCH_IP_BRG_SERVICE_LAST_E;

        st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                           enableService);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, bridgeService = %d", dev, bridgeService);

        bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;    /* restore */

        UTF_ENUMS_CHECK_MAC(cpssDxChIpBridgeServiceEnable
                            (dev, bridgeService, enableDisableMode, enableService),
                            bridgeService);

        /* 1.4.*/
        UTF_ENUMS_CHECK_MAC(cpssDxChIpBridgeServiceEnable
                            (dev, bridgeService, enableDisableMode, enableService),
                            enableDisableMode);
    }

    bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
    enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
    enableService = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                           enableService);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpBridgeServiceEnable(dev, bridgeService, enableDisableMode,
                                       enableService);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpBridgeServiceEnableGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService,
    IN  CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode,
    OUT GT_BOOL                                             *enableServicePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpBridgeServiceEnableGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with bridgeService [CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E / CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E / CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E], enableDisableMode [CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E / CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E / CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E] and enableService [GT_FALSE and GT_ TRUE].
Expected: GT_OK.
1.2. Call with wrong enum values bridgeService and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.3. Call with wrong enum values enableDisableMode  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with NULL enableServicePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_IP_BRG_SERVICE_ENT                        bridgeService;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT    enableDisableMode;
    GT_BOOL                                             enableService;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;

        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableService);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        bridgeService = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;

        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableService);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;

        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableService);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, bridgeService, enableDisableMode);

        /* 1.2. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpBridgeServiceEnableGet
                            (dev, bridgeService, enableDisableMode, &enableService),
                            bridgeService);

        /* 1.3. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpBridgeServiceEnableGet
                            (dev, bridgeService, enableDisableMode, &enableService),
                            enableDisableMode);

        /* 1.4. */
        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, bridgeService, enableDisableMode);
    }

    bridgeService = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
    enableDisableMode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                           &enableService);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpBridgeServiceEnableGet(dev, bridgeService, enableDisableMode,
                                       &enableService);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllBridgeEnable)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with mllBridgeEnable [GT_TRUE and GT_FALSE].
Expected: GT_OK.
1.2. Call cpssDxChIpMllBridgeEnableGet.
Expected: GT_OK and the same mllBridgeEnable.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                 mllBridgeEnable;
    GT_BOOL                 mllBridgeEnableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1  */
        mllBridgeEnable = GT_TRUE;

        st = cpssDxChIpMllBridgeEnable(dev, mllBridgeEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllBridgeEnable);

        /*  1.2.  */
        st = cpssDxChIpMllBridgeEnableGet(dev, &mllBridgeEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mllBridgeEnable, mllBridgeEnableGet,
                        "get another mllBridgeEnable than was set: dev - %d", dev);

        /*  1.1  */
        mllBridgeEnable = GT_FALSE;

        st = cpssDxChIpMllBridgeEnable(dev, mllBridgeEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllBridgeEnable);

        /*  1.2.  */
        st = cpssDxChIpMllBridgeEnableGet(dev, &mllBridgeEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mllBridgeEnable, mllBridgeEnableGet,
                        "get another mllBridgeEnable than was set: dev - %d", dev);
    }

    mllBridgeEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllBridgeEnable(dev, mllBridgeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllBridgeEnable(dev, mllBridgeEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllBridgeEnableGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *mllBridgeEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllBridgeEnableGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with not NULL mllBridgeEnablePtr.
Expected: GT_OK.
1.2. Call with NULL mllBridgeEnablePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                 mllBridgeEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllBridgeEnableGet(dev, &mllBridgeEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllBridgeEnable);

        /*  1.2.  */
        st = cpssDxChIpMllBridgeEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, mllBridgeEnable);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllBridgeEnableGet(dev, &mllBridgeEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllBridgeEnableGet(dev, &mllBridgeEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetRateShaperSet
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    multiTargetRateShaperEnable,
    IN   GT_U32     windowSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetRateShaperSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with multiTargetRateShaperEnable [GT_TRUE and GT_FALSE] and windowSize [10 / 0xFFFF].
Expected: GT_OK.
1.2. Call cpssDxChIpMultiTargetRateShaperGet.
Expected: GT_OK and the same multiTargetRateShaperEnable, windowSize.
1.3. Call with multiTargetRateShaperEnable [GT_FALSE] (in this case windowSize is not relevant) and windowSize [0xFFFFFFFF].
Expected: GT_OK.
1.4. Call with multiTargetRateShaperEnable [GT_TRUE] (in this case windowSize is relevant) and out-of-range windowSize [0x10000].
Expected: non GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL    multiTargetRateShaperEnable;
    GT_U32     windowSize;
    GT_BOOL    multiTargetRateShaperEnableGet;
    GT_U32     windowSizeGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        multiTargetRateShaperEnable = GT_TRUE;
        windowSize = 10;

        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnableGet,
                                                &windowSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                        "get another multiTargetRateShaperEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                        "get another windowSize than was set: ", dev);

        /*  1.1.  */
        windowSize = 0xFFFF;

        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnableGet,
                                                &windowSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                        "get another multiTargetRateShaperEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                        "get another windowSize than was set: ", dev);

        /*  1.1.  */
        multiTargetRateShaperEnable = GT_FALSE;

        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnableGet,
                                                &windowSizeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                        "get another multiTargetRateShaperEnable than was set: ", dev);

        /*  1.3. */
        windowSize = 0xFFFFFFFF;

        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);

        /*  1.4.  */
        multiTargetRateShaperEnable = GT_TRUE;
        windowSize = 0x10000;

        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);
    }

    multiTargetRateShaperEnable = GT_TRUE;
    windowSize = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                                windowSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetRateShaperSet(dev, multiTargetRateShaperEnable,
                                            windowSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetRateShaperGet
(
    IN    GT_U8      devNum,
    OUT   GT_BOOL    *multiTargetRateShaperEnablePtr,
    OUT   GT_U32     *windowSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetRateShaperGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with not NULL pointers.
Expected: GT_OK.
1.2. Call with NULL multiTargetRateShaperEnablePtr.
Expected: GT_BAD_PTR.
1.3. Call with NULL windowSizePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL    multiTargetRateShaperEnable = GT_FALSE;
    GT_U32     windowSize = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnable,
                                                &windowSize);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                                windowSize);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, NULL,
                                                &windowSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnable,
                                                NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnable,
                                                &windowSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetRateShaperGet(dev, &multiTargetRateShaperEnable,
                                            &windowSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetUcSchedModeSet
(
    IN   GT_U8                              devNum,
    IN   GT_BOOL                            ucSPEnable,
    IN   GT_U32                             ucWeight,
    IN   GT_U32                             mcWeight,
    IN   CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtu
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetUcSchedModeSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with ucSPEnable [GT_TRUE/GT_FALSE], ucWeight[10 / 255], mcWeight[10 / 255] and schedMtu [CPSS_DXCH_IP_MT_UC_SCHED_MTU_2K_E / CPSS_DXCH_IP_MT_UC_SCHED_MTU_8K_E].
Expected: GT_OK.

1.2. Call cpssDxChIpMultiTargetUcSchedModeGet.
Expected: GT_OK and same ucSPEnable, schedMtu.

1.3. Call with ucSPEnable [GT_TRUE] (in this case ucWeight is not relevant), ucWeight [0xFFFFFFFF] and other parameters from 1.1.
Expected: GT_OK.

1.4. Call with ucSPEnable [GT_FALSE] (in this case ucWeight is relevant), ucWeight [256] (out of range) and other parameters from 1.1.
Expected: non GT_OK.
1.5. Call with ucSPEnable [GT_FALSE], mcWeight [256] (out of range) and other parameters from 1.1.
Expected: non GT_OK.
1.6. Call with wrong enum values schedMtu  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                            ucSPEnable;
    GT_U32                             ucWeight;
    GT_U32                             mcWeight;
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtu;
    GT_BOOL                            ucSPEnableGet = 0;
    GT_U32                             ucWeightGet = 0;
    GT_U32                             mcWeightGet = 0;
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtuGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        ucSPEnable = GT_FALSE;
        ucWeight = 255;
        mcWeight = 255;
        schedMtu = CPSS_DXCH_IP_MT_UC_SCHED_MTU_2K_E;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnableGet, &ucWeightGet,
                                                 &mcWeightGet, &schedMtuGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ucSPEnable, ucSPEnableGet,
                        "get another ucSPEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(schedMtu, schedMtuGet,
                        "get another schedMtu than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ucWeight, ucWeightGet,
                        "get another ucWeight than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mcWeight, mcWeightGet,
                        "get another mcWeight than was set: ", dev);

        /*  1.1.  */
        ucWeight = 10;
        mcWeight = 10;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnableGet, &ucWeightGet,
                                                 &mcWeightGet, &schedMtuGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ucSPEnable, ucSPEnableGet,
                        "get another ucSPEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(schedMtu, schedMtuGet,
                        "get another schedMtu than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ucWeight, ucWeightGet,
                        "get another ucWeight than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mcWeight, mcWeightGet,
                        "get another mcWeight than was set: ", dev);

        /*  1.1.  */
        schedMtu = CPSS_DXCH_IP_MT_UC_SCHED_MTU_8K_E;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnableGet, &ucWeightGet,
                                                 &mcWeightGet, &schedMtuGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ucSPEnable, ucSPEnableGet,
                        "get another ucSPEnable than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(schedMtu, schedMtuGet,
                        "get another schedMtu than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ucWeight, ucWeightGet,
                        "get another ucWeight than was set: ", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mcWeight, mcWeightGet,
                        "get another mcWeight than was set: ", dev);

        /* 1.3. */
        ucSPEnable = GT_TRUE;
        ucWeight = 0xFFFFFFFF;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucSPEnable, ucWeight);

        /* 1.4. */
        ucSPEnable = GT_FALSE;
        ucWeight = 256;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, ucSPEnable, ucWeight);

        ucWeight = 10; /* restore */

        /* 1.5.  */
        mcWeight = 256;

        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, mcWeight = %d",
                                     dev, ucSPEnable, mcWeight);

        mcWeight = 10;      /* restore */

        /* 1.6.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpMultiTargetUcSchedModeSet
                            (dev, ucSPEnable, ucWeight, mcWeight, schedMtu),
                            schedMtu);
    }

    ucSPEnable = GT_TRUE;
    ucWeight = 255;
    mcWeight = 255;
    schedMtu = CPSS_DXCH_IP_MT_UC_SCHED_MTU_2K_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                                 mcWeight, schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetUcSchedModeSet(dev, ucSPEnable, ucWeight,
                                             mcWeight, schedMtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMultiTargetUcSchedModeGet
(
    IN   GT_U8                              devNum,
    OUT  GT_BOOL                            *ucSPEnablePtr,
    OUT  GT_U32                             *ucWeightPtr,
    OUT  GT_U32                             *mcWeightPtr,
    OUT  CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   *schedMtuPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMultiTargetUcSchedModeGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with not NULL ucSPEnablePtr, ucWeightPtr, mcWeightPtr, schedMtuPtr.
Expected: GT_OK.
1.2. Call with NULL ucSPEnablePtr.
Expected: GT_BAD_PTR.
1.3. Call with NULL ucWeightPtr.
Expected: GT_BAD_PTR.
1.4. Call with NULL mcWeightPtr.
Expected: GT_BAD_PTR.
1.5. Call with NULL schedMtuPtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_BOOL                            ucSPEnable;
    GT_U32                             ucWeight;
    GT_U32                             mcWeight;
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT   schedMtu;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, &ucWeight,
                                                 &mcWeight, &schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, NULL, &ucWeight,
                                                 &mcWeight, &schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, NULL,
                                                 &mcWeight, &schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.4.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, &ucWeight,
                                                 NULL, &schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.5.  */
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, &ucWeight,
                                                 &mcWeight, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, &ucWeight,
                                                 &mcWeight, &schedMtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMultiTargetUcSchedModeGet(dev, &ucSPEnable, &ucWeight,
                                             &mcWeight, &schedMtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpArpBcModeSet
(
    IN GT_U8               devNum,
    IN CPSS_PACKET_CMD_ENT arpBcMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpArpBcModeSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with arpBcMode [CPSS_PACKET_CMD_NONE_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E / CPSS_PACKET_CMD_MIRROR_TO_CPU_E].
Expected: GT_OK.
1.2. Call cpssDxChIpArpBcModeGet.
Expected: GT_OK and same arpBcMode.
1.3. Call with arpBcMode [CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_HARD_E / CPSS_PACKET_CMD_DROP_SOFT_E] (these commands are not possible).
Expected: NOT GT_OK.
1.4. Call with wrong enum values arpBcMode.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PACKET_CMD_ENT     arpBcMode;
    CPSS_PACKET_CMD_ENT     arpBcModeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*   1.1.   */
        arpBcMode = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        /*   1.2.   */
        st = cpssDxChIpArpBcModeGet(dev, &arpBcModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(arpBcMode, arpBcModeGet,
                        "get another arpBcMode than was set: ", dev);

        /*   1.1   */
        arpBcMode = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        /*   1.2.   */
        st = cpssDxChIpArpBcModeGet(dev, &arpBcModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(arpBcMode, arpBcModeGet,
                        "get another arpBcMode than was set: ", dev);

        arpBcMode = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        /*   1.2.   */
        st = cpssDxChIpArpBcModeGet(dev, &arpBcModeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verify values */
        UTF_VERIFY_EQUAL1_STRING_MAC(arpBcMode, arpBcModeGet,
                        "get another arpBcMode than was set: ", dev);

        /*    1.3.    */
        arpBcMode = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        arpBcMode = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        arpBcMode = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, arpBcMode);

        /*   1.4.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpArpBcModeSet
                            (dev, arpBcMode),
                            arpBcMode);
    }

    arpBcMode = CPSS_PACKET_CMD_NONE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpArpBcModeSet(dev, arpBcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpArpBcModeGet
(
    IN  GT_U8               devNum,
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpArpBcModeGet)
{
    /*
        ITERATE_DEVICES (DxCh2 and above)
        1.1. Call with not NULL arpBcModePtr
        Expected: GT_OK.
        1.2. Call with NULL arpBcModePtr
        Expected: GT_BAD_PTR.
    */
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_PACKET_CMD_ENT     arpBcMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*   1.1.   */
        st = cpssDxChIpArpBcModeGet(dev, &arpBcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*   1.2.   */
        st = cpssDxChIpArpBcModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpArpBcModeGet(dev, &arpBcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpArpBcModeGet(dev, &arpBcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortRoutingEnable
(
    IN GT_U8                            devNum,
    IN GT_U8                            portNum,
    IN CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN GT_BOOL                          enableRouting
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortRoutingEnable)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
    1.1. Call with ucMcEnable [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E],
    protocolStack[CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
                  CPSS_IP_PROTOCOL_IPV4V6_E]
    and enableRouting [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpPortRoutingEnableGet with the same parameters.
    Expected: GT_OK and the same value.
    1.3. Call with wrong enum values ucMcEnable  and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values protocolStack
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8  dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_IP_UNICAST_MULTICAST_ENT ucMcEnable;
    CPSS_IP_PROTOCOL_STACK_ENT    protocolStack;
    GT_BOOL                       enableRouting;
    GT_BOOL                       enableRoutingGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(   (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)) ?
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E :
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*
                1.1. Call with ucMcEnable [CPSS_IP_UNICAST_E / CPSS_IP_MULTICAST_E],
                protocolStack[CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
                              CPSS_IP_PROTOCOL_IPV4V6_E]
                and enableRouting [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call with ucMcEnable [CPSS_IP_UNICAST_E],
             * protocolStack[CPSS_IP_PROTOCOL_IPV4_E]
             * and enableRouting [GT_FALSE].  */

            ucMcEnable = CPSS_IP_UNICAST_E;
            protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
            enableRouting = GT_FALSE;

            st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                             protocolStack, enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /*
                1.2. Call cpssDxChIpPortRoutingEnableGet with the same parameters.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                                protocolStack, &enableRoutingGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpPortRoutingEnableGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enableRouting, enableRoutingGet,
                                 "get another enableRouting than was set: %d", dev);


            /* Call with enableRouting [GT_TRUE]  */
            enableRouting = GT_TRUE;

            st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                             protocolStack, enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /*
                1.2. Call cpssDxChIpPortRoutingEnableGet with the same parameters.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                                protocolStack, &enableRoutingGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpPortRoutingEnableGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enableRouting, enableRoutingGet,
                                 "get another enableRouting than was set: %d", dev);

            /* Call with ucMcEnable [CPSS_IP_MULTICAST_E],
             * protocolStack[CPSS_IP_PROTOCOL_IPV6_E] and enableRouting [GT_TRUE].  */

            ucMcEnable = CPSS_IP_MULTICAST_E;
            protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
            enableRouting = GT_TRUE;

            st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                             protocolStack, enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /*
                1.2. Call cpssDxChIpPortRoutingEnableGet with the same parameters.
                Expected: GT_OK and the same value.
            */
            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                                protocolStack, &enableRoutingGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpPortRoutingEnableGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enableRouting, enableRoutingGet,
                                 "get another enableRouting than was set: %d", dev);

            /* Call with protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E] */
            protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;

            st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                             protocolStack, enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /* 1.3. Call with wrong enum values ucMcEnable  and
               other valid parameters from 1.1. Expected: GT_BAD_PARAM. */

            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortRoutingEnable
                                (dev, port, ucMcEnable, protocolStack, enableRouting),
                                ucMcEnable);

            /* 1.4. Call with wrong enum values protocolStack  and
               other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortRoutingEnable
                                (dev, port, ucMcEnable, protocolStack, enableRouting),
                                protocolStack);
        }

        ucMcEnable = CPSS_IP_UNICAST_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        enableRouting = GT_FALSE;

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                             protocolStack, enableRouting);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                         protocolStack, enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                         protocolStack, enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;
    ucMcEnable = CPSS_IP_UNICAST_E;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    enableRouting = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                         protocolStack, enableRouting);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortRoutingEnable(dev, port, ucMcEnable,
                                     protocolStack, enableRouting);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortRoutingEnableGet
(
    IN  GT_U8                            devNum,
    IN  GT_U8                            portNum,
    IN  CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable,
    IN  CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT GT_BOOL                          *enableRoutingPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortRoutingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
    1.1. Call with ucMcEnable [CPSS_IP_UNICAST_E],
    protocolStack[CPSS_IP_PROTOCOL_IPV4_E].
    Expected: GT_OK.
    1.2. Call with ucMcEnable [CPSS_IP_MULTICAST_E],
    protocolStack[CPSS_IP_PROTOCOL_IPV6_E / CPSS_IP_PROTOCOL_IPV4V6_E].
    Expected: GT_OK.
    1.3. Call with ucMcEnable [CPSS_IP_MULTICAST_E],
    protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values ucMcEnable
    and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum values protocolStack
    and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with bad enableRoutingPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_IP_UNICAST_MULTICAST_ENT    ucMcEnable;
    CPSS_IP_PROTOCOL_STACK_ENT       protocolStack;
    GT_BOOL                          enableRouting;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(   (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)) ?
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E :
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);
        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call with ucMcEnable [CPSS_IP_UNICAST_E], protocolStack
              [CPSS_IP_PROTOCOL_IPV4_E]
              Expected: GT_OK.    */

            ucMcEnable = CPSS_IP_UNICAST_E;
            protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                             protocolStack, &enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /* 1.2. Call with ucMcEnable [CPSS_IP_MULTICAST_E],
               protocolStack[CPSS_IP_PROTOCOL_IPV6_E]
               Expected: GT_OK.    */

            ucMcEnable = CPSS_IP_MULTICAST_E;
            protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                             protocolStack, &enableRouting);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            /* 1.3. Call with ucMcEnable [CPSS_IP_MULTICAST_E],
               protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
               Expected: GT_BAD_PARAM.    */

            protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;

            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                             protocolStack, &enableRouting);

            /* can not get both values for ipv4 and ipv6 in the same get*/
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, ucMcEnable,
                                             protocolStack, enableRouting);

            protocolStack = CPSS_IP_PROTOCOL_IPV6_E; /* restore */

            /* 1.4. Call with wrong enum values ucMcEnable  and
               other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortRoutingEnableGet
                                (dev, port, ucMcEnable, protocolStack, &enableRouting),
                                ucMcEnable);

            /* 1.5. Call with wrong enum values protocolStack  and
               other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortRoutingEnableGet
                                (dev, port, ucMcEnable, protocolStack, &enableRouting),
                                protocolStack);

            /* 1.6. Call with bad enableRoutingPtr [NULL].
            Expected: GT_BAD_PTR. */

            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                                protocolStack, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, protocolStack = NULL", dev);
        }

        ucMcEnable = CPSS_IP_UNICAST_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                             protocolStack, &enableRouting);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                         protocolStack, &enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                         protocolStack, &enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;
    ucMcEnable = CPSS_IP_UNICAST_E;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    enableRouting = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                            protocolStack, &enableRouting);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortRoutingEnableGet(dev, port, ucMcEnable,
                                        protocolStack, &enableRouting);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    IN  GT_U32               routeEntryOffset
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpQosProfileToRouteEntryMapSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with qosProfile [1 / 100 / 127] and routeEntryOffset [1 / 2 / 3].
Expected: GT_OK.
1.2. Call cpssDxChIpQosProfileToRouteEntryMapGet.
Expected: GT_OK and same routeEntryOffset.
1.3. Call with qosProfile [1] and routeEntryOffset [0xFFFFFFFF] (no any constraints).
Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  qosProfile;
    GT_U32                  routeEntryOffset;
    GT_U32                  routeEntryOffsetGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with qosProfile [1 / 100 / 127] and
           routeEntryOffset [1 / 2 / 3]. Expected: GT_OK.   */

        qosProfile = 1;
        routeEntryOffset = 1;

        st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                    routeEntryOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    routeEntryOffset);

        /* 1.2.  */
        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    &routeEntryOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routeEntryOffset, routeEntryOffsetGet,
                "get another routeEntryOffset than was set: %d", dev);

        qosProfile = 100;
        routeEntryOffset = 2;

        st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                    routeEntryOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    routeEntryOffset);

        /* 1.2.  */
        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    &routeEntryOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routeEntryOffset, routeEntryOffsetGet,
                "get another routeEntryOffset than was set: %d", dev);

        qosProfile = 127;
        routeEntryOffset = 3;

        st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                    routeEntryOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    routeEntryOffset);

        /* 1.2.  */
        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    &routeEntryOffsetGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routeEntryOffset, routeEntryOffsetGet,
                "get another routeEntryOffset than was set: %d", dev);

        /* 1.3. Call with qosProfile [1] and routeEntryOffset
           [0xFFFFFFFF] (no any constraints). Expected: GT_OK.  */

        qosProfile = 1;
        routeEntryOffset = 0xFFFFFFFF;

        st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                    routeEntryOffset);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfile,
                                                    routeEntryOffset);
    }

    qosProfile = 1;
    routeEntryOffset = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                    routeEntryOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpQosProfileToRouteEntryMapSet(dev, qosProfile,
                                                routeEntryOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpQosProfileToRouteEntryMapGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               qosProfile,
    OUT GT_U32               *routeEntryOffsetPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpQosProfileToRouteEntryMapGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with qosProfile [1 / 100 / 127] and not NULL routeEntryOffset.
Expected: GT_OK.
1.2. Call with qosProfile [128] out of range.
Expected: not GT_OK.
1.3. Call with NULL routeEntryOffsetPtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  qosProfile;
    GT_U32                  routeEntryOffset;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.   */
        qosProfile = 1;

        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    &routeEntryOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.1.   */
        qosProfile = 100;

        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    &routeEntryOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.1.   */
        qosProfile = 127;

        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                     &routeEntryOffset);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);

        /* 1.2.  */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            qosProfile = 1024;
        }
        else
        {
            qosProfile = 128;
        }

        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                     &routeEntryOffset);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfile);
        qosProfile = 1;

        /* 1.3.   */
        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                    NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, qosProfile);
    }

    qosProfile = 1;
    routeEntryOffset = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                     &routeEntryOffset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpQosProfileToRouteEntryMapGet(dev, qosProfile,
                                                 &routeEntryOffset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRoutingEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enableRouting
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRoutingEnable)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with enableRouting [GT_TRUE and GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRoutingEnableGet with the same params.
    Expected: GT_OK and the same value.
*/
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_BOOL    enableRouting;
    GT_BOOL    enableRoutingGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enableRouting [GT_TRUE and GT_FALSE].
            Expected: GT_OK.
        */
        enableRouting = GT_TRUE;

        st = cpssDxChIpRoutingEnable(dev, enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableRouting);

        /*
            1.2. Call cpssDxChIpRoutingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpRoutingEnableGet(dev, &enableRoutingGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableRouting);


        enableRouting = GT_FALSE;

        st = cpssDxChIpRoutingEnable(dev, enableRouting);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableRouting);

        /*
            1.2. Call cpssDxChIpRoutingEnableGet with the same params.
            Expected: GT_OK and the same value.
        */
        st = cpssDxChIpRoutingEnableGet(dev, &enableRoutingGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableRouting);
    }

    enableRouting = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRoutingEnable(dev, enableRouting);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpRoutingEnable(dev, enableRouting);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRoutingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRoutingEnableGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpRoutingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpRoutingEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRoutingEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRoutingEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpCntGet
(
    IN  GT_U8                        devNum,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *counters
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpCntGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E / CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E] and non-null counters.
Expected: GT_OK.
1.2. Call with wrong enum values cntSet  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.3. Call with cntSet [CPSS_IP_CNT_SET0_E], counters [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_CNT_SET_ENT          cntSet;
    CPSS_DXCH_IP_COUNTER_SET_STC counters;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E /
           CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E] and non-null counters.
           Expected: GT_OK.     */

        cntSet = CPSS_IP_CNT_SET0_E;

        st = cpssDxChIpCntGet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        cntSet = CPSS_IP_CNT_SET1_E;

        st = cpssDxChIpCntGet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        cntSet = CPSS_IP_CNT_SET2_E;

        st = cpssDxChIpCntGet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        cntSet = CPSS_IP_CNT_SET3_E;

        st = cpssDxChIpCntGet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /* 1.2. Call with wrong enum values cntSet  and other
           valid parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntGet
                            (dev, cntSet, &counters),
                            cntSet);

        /* 1.3. Call with cntSet [CPSS_IP_CNT_SET0_E], counters [NULL].
           Expected: GT_BAD_PTR.  */

        cntSet = CPSS_IP_CNT_SET0_E;

        st = cpssDxChIpCntGet(dev, cntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, counters = NULL", dev);

    }

    cntSet = CPSS_IP_CNT_SET0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpCntGet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpCntGet(dev, cntSet, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpCntSetModeSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    IN  CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode,
    IN  CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpCntSetModeSet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E /
                           CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E],
         cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E /
                     CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E]
         and interfaceModeCfgPtr {portTrunkCntMode
         [CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E /
          CPSS_DXCH_IP_PORT_CNT_MODE_E / CPSS_DXCH_IP_TRUNK_CNT_MODE_E],
         ipMode [CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
         CPSS_IP_PROTOCOL_IPV4V6_E],
         vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E /
                   CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E], devNum [dev],
         portTrunk {port[0], trunk[0]}, vlanId [BRG_VLAN_TESTED_VLAN_ID_CNS = 100]}.
    Expected: GT_OK.
    1.2. Call cpssDxChIpCntSetModeGet
    Expected: GT_OK, same cntSetMode and interfaceModeCfgPtr.
    1.3. Call with wrong enum values cntSet
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values cntSetMode
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.5. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         wrong enum values interfaceModeCfgPtr->portTrunkCntMode
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         wrong enum values interfaceModeCfgPtr->ipMode
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.7. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         wrong enum values interfaceModeCfgPtr->vlanMode
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.8. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_TRUNK_CNT_MODE_E]
         (in this case port is not relevant), out of range portTrunk.port
         [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] and other valid parameters from 1.1.
    Expected: GT_OK.
    1.9. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
         (in this case trunk is not relevant), out of range portTrunk.trunk
         [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128] and other valid parameters from 1.1.
    Expected:  GT_BAD_PARAM.
    1.10 Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
         (in this case trunk is not relevant), portTrunk.port = 0  and other valid parameters from 1.1.
    Expected: GT_OK.
    1.11. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
         interfaceModeCfgPtr {vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E]
         (in this case vlanId is not relevant), out of range vlanId
         [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
    Expected: GT_OK.
    1.12. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
          interfaceModeCfgPtr [NULL] and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_CNT_SET_ENT                        cntSet;
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode;
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetModeGet = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
        cpssOsBzero((GT_VOID*) &interfaceModeCfgGet, sizeof(interfaceModeCfgGet));
        cntSet = CPSS_IP_CNT_SET0_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cntSet, cntSetMode);
        if(GT_OK == st)
        {
            /*  1.2.  */
            st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetModeGet, &interfaceModeCfgGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);
            if(GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(cntSetMode, cntSetModeGet,
                        "get another cntSetMode than was set: %d", dev);

                UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunkCntMode, interfaceModeCfgGet.portTrunkCntMode,
                        "get another interfaceModeCfg.portTrunkCntMode than was set: %d", dev);
            }
        }

        /*  1.1.  */
        cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
        cpssOsBzero((GT_VOID*) &interfaceModeCfgGet, sizeof(interfaceModeCfgGet));
        cntSet = CPSS_IP_CNT_SET1_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceModeCfg.portTrunk.trunk = 127;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cntSet, cntSetMode);
        if(GT_OK == st)
        {
            /*  1.2.  */
            st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetModeGet, &interfaceModeCfgGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);
            if(GT_OK == st)
            {

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(cntSetMode, cntSetModeGet,
                        "get another cntSetMode than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunkCntMode, interfaceModeCfgGet.portTrunkCntMode,
                        "get another interfaceModeCfg.portTrunkCntMode than was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunk.trunk, interfaceModeCfgGet.portTrunk.trunk,
                        "get another interfaceModeCfgGet.portTrunk.trunk than was set: %d", dev);

            }
        }

        /*  1.1.  */
        cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
        cpssOsBzero((GT_VOID*) &interfaceModeCfgGet, sizeof(interfaceModeCfgGet));
        cntSet = CPSS_IP_CNT_SET1_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;

        interfaceModeCfg.portTrunk.port = 55;
        interfaceModeCfg.hwDevNum = dev;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        /* due to Lion fam errata:
           FEr#3116: Wrong counting of IP counters per port - IP counters cannot be set by {device+port} */
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_LION_WRONG_COUNTING_OF_IP_COUNTERS_PER_PORT_WA_E))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cntSet, cntSetMode);
            if(GT_OK == st)
            {
                /*  1.2.  */
                st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetModeGet, &interfaceModeCfgGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);
                if(GT_OK == st)
                {
                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(cntSetMode, cntSetModeGet,
                            "get another cntSetMode than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunkCntMode, interfaceModeCfgGet.portTrunkCntMode,
                            "get another interfaceModeCfg.portTrunkCntMode than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunk.trunk, interfaceModeCfgGet.portTrunk.trunk,
                            "get another interfaceModeCfgGet.portTrunk.trunk than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.hwDevNum, interfaceModeCfgGet.hwDevNum,
                            "get another interfaceModeCfg.hwDevNum than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunk.port, interfaceModeCfgGet.portTrunk.port,
                            "get another interfaceModeCfg.portTrunk.port than was set: %d", dev);
                }
            }
        }



        /*  1.1.  */
        cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
        cpssOsBzero((GT_VOID*) &interfaceModeCfgGet, sizeof(interfaceModeCfgGet));
        cntSet = CPSS_IP_CNT_SET1_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;

        interfaceModeCfg.portTrunk.port = 0;
        interfaceModeCfg.hwDevNum = dev;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        /* due to Lion fam errata:
           FEr#3116: Wrong counting of IP counters per port - IP counters cannot be set by {device+port} */
        if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(dev,PRV_CPSS_DXCH_LION_WRONG_COUNTING_OF_IP_COUNTERS_PER_PORT_WA_E))
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, cntSet, cntSetMode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cntSet, cntSetMode);
            if(GT_OK == st)
            {
                /*  1.2.  */
                st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetModeGet, &interfaceModeCfgGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);
                if(GT_OK == st)
                {
                    /* Verifying values */
                    UTF_VERIFY_EQUAL1_STRING_MAC(cntSetMode, cntSetModeGet,
                            "get another cntSetMode than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunkCntMode, interfaceModeCfgGet.portTrunkCntMode,
                            "get another interfaceModeCfg.portTrunkCntMode than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunk.trunk, interfaceModeCfgGet.portTrunk.trunk,
                            "get another interfaceModeCfgGet.portTrunk.trunk than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.hwDevNum, interfaceModeCfgGet.hwDevNum,
                            "get another interfaceModeCfg.hwDevNum than was set: %d", dev);
                    UTF_VERIFY_EQUAL1_STRING_MAC(interfaceModeCfg.portTrunk.port, interfaceModeCfgGet.portTrunk.port,
                            "get another interfaceModeCfg.portTrunk.port than was set: %d", dev);
                }
            }
        }

        /*  1.1.  */
        cntSet = CPSS_IP_CNT_SET3_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, cntSet, cntSetMode);
        if(GT_OK == st)
        {
            /*  1.2.  */
            st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetModeGet, &interfaceModeCfgGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);
            if(GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(cntSetMode, cntSetModeGet,
                        "get another cntSetMode than was set: %d", dev);
            }
        }

        /* restore: */

        cntSet = CPSS_IP_CNT_SET0_E;
        cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceModeCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
        interfaceModeCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;

        /* 1.3. Call with wrong enum values cntSet  and other valid
           parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeSet
                            (dev, cntSet, cntSetMode, &interfaceModeCfg),
                            cntSet);

        /* 1.4. Call with wrong enum values cntSetMode  and other
           valid parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeSet
                            (dev, cntSet, cntSetMode, &interfaceModeCfg),
                            cntSetMode);

        /* 1.5. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           wrong enum values interfaceModeCfgPtr->portTrunkCntMode  and
           other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeSet
                            (dev, cntSet, cntSetMode, &interfaceModeCfg),
                            interfaceModeCfg.portTrunkCntMode);

        /* 1.6. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           wrong enum values interfaceModeCfgPtr->ipMode  and other valid
           parameters from 1.1. Expected: GT_BAD_PARAM. */

        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeSet
                            (dev, cntSet, cntSetMode, &interfaceModeCfg),
                            interfaceModeCfg.ipMode);

        /* 1.7. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           wrong enum values interfaceModeCfgPtr->vlanMode  and other
           valid parameters from 1.1.   Expected: GT_BAD_PARAM. */

        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeSet
                            (dev, cntSet, cntSetMode, &interfaceModeCfg),
                            interfaceModeCfg.vlanMode);

        /* 1.8. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_TRUNK_CNT_MODE_E]
           (change only trunk as port is inside union), and other valid
           parameters from 1.1. Expected: GT_OK. */

        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceModeCfg.portTrunk.trunk = 0;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
         "%d, interfaceModeCfg.portTrunkCntMode = %d, interfaceModeCfg.portTrunk.port = %d",
         dev, interfaceModeCfg.portTrunkCntMode, interfaceModeCfg.portTrunk.port);

        /* restore */
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceModeCfg.portTrunk.port = 0;


        /* 1.9. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), out of range portTrunk.trunk
           [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128] and other valid parameters from 1.1.
           Expected: GT_BAD_PARAM. */

        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceModeCfg.portTrunk.trunk  = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
         "%d, interfaceModeCfg.portTrunkCntMode = %d, interfaceModeCfg.portTrunk.trunk = %d",
         dev, interfaceModeCfg.portTrunkCntMode, interfaceModeCfg.portTrunk.trunk);


        /* 1.10. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           interfaceModeCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), portTrunk.port = 0
           and other valid parameters from 1.1.
           Expected: GT_BAD_PARAM. */

        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;
        interfaceModeCfg.portTrunk.port = 0;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E ||
            PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st, "%d, interfaceModeCfg.portTrunkCntMode = %d, interfaceModeCfg.portTrunk.port = %d",
                                         dev, interfaceModeCfg.portTrunkCntMode, interfaceModeCfg.portTrunk.port);
        }
        else
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, interfaceModeCfg.portTrunkCntMode = %d, interfaceModeCfg.portTrunk.port = %d",
                                         dev, interfaceModeCfg.portTrunkCntMode, interfaceModeCfg.portTrunk.port);
        }

        /* restore */
        interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceModeCfg.portTrunk.trunk = 0;

        /* 1.11. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           interfaceModeCfgPtr {vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E]
           (in this case vlanId is not relevant), out of range vlanId
           [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
           Expected: GT_OK. */

        interfaceModeCfg.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, interfaceModeCfg.vlanMode = %d, interfaceModeCfg.vlanId = %d",
                         dev, interfaceModeCfg.vlanMode, interfaceModeCfg.vlanId);
        /* restore */
        interfaceModeCfg.vlanId = 100;


        /* 1.12. Call with cntSetMode [CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E],
           interfaceModeCfgPtr [NULL] and other valid parameters from 1.1.
           Expected: GT_BAD_PTR.    */

        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, interfaceModeCfgPtr = NULL", dev);
    }

    cntSet = CPSS_IP_CNT_SET0_E;
    cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
    interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
    interfaceModeCfg.hwDevNum = dev;
    interfaceModeCfg.portTrunk.port = 0;
    interfaceModeCfg.portTrunk.trunk = 0;
    interfaceModeCfg.vlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpCntSetModeSet(dev, cntSet, cntSetMode, &interfaceModeCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpCntSetModeGet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_IP_CNT_SET_ENT                        cntSet,
    OUT CPSS_DXCH_IP_CNT_SET_MODE_ENT              *cntSetModePtr,
    OUT CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceModeCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpCntSetModeGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E /
                           CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E]
         and not NULL cntSetModePtr and interfaceModeCfgPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values cntSet
         and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL cntSetModePtr.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL interfaceModeCfgPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_CNT_SET_ENT                        cntSet;
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));
        cntSet = CPSS_IP_CNT_SET0_E;

        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /*  1.1.  */
        cntSet = CPSS_IP_CNT_SET1_E;

        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /*  1.1.  */
        cntSet = CPSS_IP_CNT_SET2_E;

        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /*  1.1.  */
        cntSet = CPSS_IP_CNT_SET3_E;

        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /*  1.1.  */
        cntSet = CPSS_IP_CNT_SET3_E;

        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSetModeGet
                            (dev, cntSet, &cntSetMode, &interfaceModeCfg),
                            cntSet);

        /*  1.3.  */
        st = cpssDxChIpCntSetModeGet(dev, cntSet, NULL, &interfaceModeCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, cntSet);

        /*  1.4.  */
        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, cntSet);
    }

    cntSet = CPSS_IP_CNT_SET0_E;
    cpssOsBzero((GT_VOID*) &interfaceModeCfg, sizeof(interfaceModeCfg));

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpCntSetModeGet(dev, cntSet, &cntSetMode, &interfaceModeCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpCntSet
(
    IN GT_U8                         devNum,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *counters
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpCntSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E / CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E] and counters{inUcPkts [1], inMcPkts [1], inUcNonRoutedExcpPkts [0], inUcNonRoutedNonExcpPkts [0], inMcNonRoutedExcpPkts [0], inMcNonRoutedNonExcpPkts [0], inUcTrappedMirrorPkts [1], inMcTrappedMirrorPkts [1], mcRfpFailPkts [0], outUcRoutedPkts [1]}.
Expected: GT_OK.
1.2. Call cpssDxChIpCntGet with the same cntSet.
Expected: GT_OK and the same counters.
1.3. Call with wrong enum values cntSet  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with counters [NULL] and other valid parameters from 1.1.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IP_CNT_SET_ENT           cntSet;
    CPSS_DXCH_IP_COUNTER_SET_STC  counters;
    CPSS_DXCH_IP_COUNTER_SET_STC  countersRet;
    GT_BOOL     isEqual;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with cntSet [CPSS_IP_CNT_SET0_E / CPSS_IP_CNT_SET1_E /
           CPSS_IP_CNT_SET2_E / CPSS_IP_CNT_SET3_E] and counters{inUcPkts [1],
           inMcPkts [1], inUcNonRoutedExcpPkts [0], inUcNonRoutedNonExcpPkts [0],
           inMcNonRoutedExcpPkts [0], inMcNonRoutedNonExcpPkts [0],
           inUcTrappedMirrorPkts [1], inMcTrappedMirrorPkts [1],
           mcRfpFailPkts [0], outUcRoutedPkts [1]}. Expected: GT_OK.    */

        cntSet = CPSS_IP_CNT_SET3_E;
        cpssOsBzero((GT_VOID*) &counters, sizeof(counters));
        counters.inUcPkts  = 1;
        counters.inMcPkts  = 1;
        counters.inUcNonRoutedExcpPkts = 0;
        counters.inUcNonRoutedNonExcpPkts = 0;
        counters.inMcNonRoutedExcpPkts = 0;
        counters.inMcNonRoutedNonExcpPkts = 0;
        counters.inUcTrappedMirrorPkts = 1;
        counters.inMcTrappedMirrorPkts = 1;
        counters.mcRfpFailPkts  = 0;
        counters.outUcRoutedPkts = 1;

        st = cpssDxChIpCntSet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        cntSet = CPSS_IP_CNT_SET0_E;

        st = cpssDxChIpCntSet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

        /* 1.2. Call cpssDxChIpCntGet with the same cntSet.
           Expected: GT_OK and the same counters.   */

        cpssOsBzero((GT_VOID*) &countersRet, sizeof(countersRet));

        st = cpssDxChIpCntGet(dev, cntSet, &countersRet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpCntGet: %d, %d", dev, cntSet);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&counters, (GT_VOID*)&countersRet, sizeof (counters)))
                  ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                                     "get another counters than was set: %d, %d", dev, cntSet);

        /* 1.3. Call with wrong enum values cntSet  and other
           valid parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpCntSet
                            (dev, cntSet, &counters),
                            cntSet);

        /* 1.4. Call with counters [NULL] and other valid parameters
           from 1.1. Expected: GT_BAD_PTR.  */

        st = cpssDxChIpCntSet(dev, cntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, counters = NULL", dev);
    }

    cntSet = CPSS_IP_CNT_SET3_E;
    cpssOsBzero((GT_VOID*) &counters, sizeof(counters));
    counters.inUcPkts  = 1;
    counters.inMcPkts  = 1;
    counters.inUcNonRoutedExcpPkts = 0;
    counters.inUcNonRoutedNonExcpPkts = 0;
    counters.inMcNonRoutedExcpPkts = 0;
    counters.inMcNonRoutedNonExcpPkts = 0;
    counters.inUcTrappedMirrorPkts = 1;
    counters.inMcTrappedMirrorPkts = 1;
    counters.mcRfpFailPkts  = 0;
    counters.outUcRoutedPkts = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpCntSet(dev, cntSet, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpCntSet(dev, cntSet, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllSkippedEntriesCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *skipCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllSkippedEntriesCountersGet)
{
/*
ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
1.1. Call with non-null dropPktsPtr. Expected: GT_OK.
1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  skipCounter;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropPktsPtr. Expected: GT_OK.    */

        st = cpssDxChIpMllSkippedEntriesCountersGet(dev, &skipCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChIpMllSkippedEntriesCountersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, skipCounter = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllSkippedEntriesCountersGet(dev, &skipCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllSkippedEntriesCountersGet(dev, &skipCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpSetMllCntInterface
(
    IN GT_U8                                      devNum,
    IN GT_U32                                     mllCntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpSetMllCntInterface)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with mllCntSet [0] and interfaceCfgPtr
     {portTrunkCntMode[CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E /
     CPSS_DXCH_IP_PORT_CNT_MODE_E / CPSS_DXCH_IP_TRUNK_CNT_MODE_E],
     ipMode [CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
     CPSS_IP_PROTOCOL_IPV4V6_E], vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E /
     CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E], devNum [dev], portTrunk {port[0], trunk[0]},
     vlanId [BRG_VLAN_TESTED_VLAN_ID_CNS = 100]}.
Expected: GT_OK.
1.2. Call with out of range mllCntSet [5] and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.3. Call with wrong enum values interfaceCfgPtr->portTrunkCntMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with wrong enum values interfaceCfgPtr->ipMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.5. Call with wrong enum values interfaceCfgPtr->vlanMode
     and other valid parameters from 1.1.
Expected: GT_BAD_PARAM.
1.6. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E],
     out of range portTrunk.port [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
     and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.7. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_TRUNK_CNT_MODE_E],
     out of range portTrunk.trunk [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128]
     and other valid parameters from 1.1.
Expected: NOT GT_OK.
1.8. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_TRUNK_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.port
     [UTF_CPSS_PP_MAX_PORTS_NUM_CNS] and other valid parameters from 1.1.
Expected: GT_BAD_PARAM
1.9. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.trunk
    [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128]  and other valid parameters from 1.1.
Expected: GT_BAD_PARAM
1.10. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
     (in this case trunk is not relevant), out of range portTrunk.port = 0
     and other valid parameters from 1.1.
Expected: GT_OK.
1.11. Call with interfaceCfgPtr {vlanMode [CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E],
      out of range vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
      and other valid parameters from 1.1.
Expected: non GT_OK.
1.12. Call with interfaceCfgPtr {vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E]
      (in this case vlanId is not relevant), out of range vlanId
      [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
Expected: GT_OK.
1.12. Call with interfaceCfgPtr [NULL] and other valid parameters from 1.1.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                                     mllCntSet;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceCfg;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with mllCntSet [0] and interfaceCfgPtr
          {portTrunkCntMode[CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E /
          CPSS_DXCH_IP_PORT_CNT_MODE_E / CPSS_DXCH_IP_TRUNK_CNT_MODE_E],
          ipMode [CPSS_IP_PROTOCOL_IPV4_E / CPSS_IP_PROTOCOL_IPV6_E /
          CPSS_IP_PROTOCOL_IPV4V6_E], vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E
          / CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E], devNum [dev], portTrunk {port[0],
          trunk[0]}, vlanId [BRG_VLAN_TESTED_VLAN_ID_CNS = 100]}.Expected: GT_OK.   */

        mllCntSet = 0;
        cpssOsBzero((GT_VOID*) &interfaceCfg, sizeof(interfaceCfg));
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
        interfaceCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
        interfaceCfg.hwDevNum = dev;
        interfaceCfg.portTrunk.port = 0;
        interfaceCfg.portTrunk.trunk = 0;
        interfaceCfg.vlanId = 100;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4V6_E;
        interfaceCfg.vlanMode = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        /* restore: */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
        interfaceCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;

        /* 1.2. Call with out of range mllCntSet [5] and other valid
           parameters from 1.1. Expected: NOT GT_OK.    */

        mllCntSet = 5;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        mllCntSet = 0;

        /* 1.3. Call with wrong enum values interfaceCfgPtr->portTrunkCntMode
            and other valid parameters from 1.1. Expected: GT_BAD_PARAM.*/
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSetMllCntInterface
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.portTrunkCntMode);

        /* 1.4. Call with wrong enum values interfaceCfgPtr->ipMode  and
           other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSetMllCntInterface
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.ipMode);

        /* 1.5. Call with wrong enum values interfaceCfgPtr->vlanMode
           and other valid parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSetMllCntInterface
                            (dev, mllCntSet, &interfaceCfg),
                            interfaceCfg.vlanMode);

        /* 1.8. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_TRUNK_CNT_MODE_E]
           (in this case port is not relevant but part of union), so set trunk
           and other valid parameters from 1.1. Expected: GT_OK. */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = 0;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
         "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.port = %d",
                dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.port);
        /* restore */
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.port = 0;


        /* 1.9. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), out of range portTrunk.trunk
           [UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev) = 128] and other valid parameters from 1.1.
           Expected: GT_BAD_PARAM. */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = UTF_CPSS_PP_MAX_TRUNK_ID_MAC(dev);

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st,
            "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.trunk = %d",
                    dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.trunk);


        /* 1.10. Call with interfaceCfgPtr {portTrunkCntMode[CPSS_DXCH_IP_PORT_CNT_MODE_E]
           (in this case trunk is not relevant), oportTrunk.port =0
           and other valid parameters from 1.1.
           Expected: GT_OK */

        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;
        interfaceCfg.portTrunk.port = 0;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "%d, interfaceCfg.portTrunkCntMode = %d, interfaceCfg.portTrunk.port = %d",
                                     dev, interfaceCfg.portTrunkCntMode, interfaceCfg.portTrunk.port);

        /* restore */
        interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
        interfaceCfg.portTrunk.trunk = 0;

        /* 1.11. Call with
           interfaceCfgPtr {vlanMode [CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E]
           (in this case vlanId is not relevant), out of range vlanId
           [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096] and other valid parameters from 1.1.
           Expected: GT_OK. */

        interfaceCfg.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                "%d, interfaceCfg.vlanMode = %d, interfaceCfg.vlanId = %d",
                          dev, interfaceCfg.vlanMode, interfaceCfg.vlanId);
        /* restore */
        interfaceCfg.vlanId = 100;


        /* 1.12. Call with
           interfaceCfgPtr [NULL] and other valid parameters from 1.1.
           Expected: GT_BAD_PTR.    */

        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, interfaceCfgPtr = NULL", dev);
    }

    mllCntSet = 0;
    cpssOsBzero((GT_VOID*) &interfaceCfg, sizeof(interfaceCfg));
    interfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
    interfaceCfg.hwDevNum = dev;
    interfaceCfg.portTrunk.port = 0;
    interfaceCfg.portTrunk.trunk = 0;
    interfaceCfg.vlanId = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpSetMllCntInterface(dev, mllCntSet, &interfaceCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllCntGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with mllCntSet [0] and non-null mllOutMCPktsPtr.
Expected: GT_OK.
1.2. Call with mllCntSet [10] and non-null mllOutMCPktsPtr.
Expected: GT_BAD_PARAM.
1.3. Call with mllCntSet [0] and mllOutMCPkts [NULL].
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32  mllCntSet;
    GT_U32  mllOutMCPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with mllCntSet [0] and non-null mllOutMCPktsPtr.
           Expected: GT_OK. */

        mllCntSet = 0;

        st = cpssDxChIpMllCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

        /* 1.2. Call with mllCntSet [10] and non-null mllOutMCPktsPtr.
                Expected: GT_BAD_PARAM. */

        mllCntSet = 10;

        st = cpssDxChIpMllCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mllCntSet);

        /* 1.3. Call with mllCntSet [0] and mllOutMCPkts [NULL].
                Expected: GT_BAD_PTR.   */

        mllCntSet = 0;

        st = cpssDxChIpMllCntGet(dev, mllCntSet, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllOutMCPktsPtr = NULL", dev);
    }

    mllCntSet = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllCntGet(dev, mllCntSet, &mllOutMCPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllCntGet(dev, mllCntSet, &mllOutMCPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllCntSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with mllCntSet [1]and mllOutMCPkts [10]. Expected: GT_OK.
1.2. Call cpssDxChIpMllCntGet with the same mllCntSet. Expected: GT_OK and the same mllOutMCPkts.
1.3. Check out of range mllCntSet. Call with mllCntSet [10] and mllOutMCPkts [10] (no any constraints). Expected: GT_BAD_PARAM.
1.4. Call with mllCntSet [1] and mllOutMCPkts [0xFFFFFFFF] (no any constraints). Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32   mllCntSet;
    GT_U32   mllOutMCPkts;
    GT_U32   mllOutMCPktsRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with mllCntSet [1]and mllOutMCPkts [10]. Expected: GT_OK.*/

        mllCntSet = 1;
        mllOutMCPkts = 10;

        st = cpssDxChIpMllCntSet(dev, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllCntSet, mllOutMCPkts);

        /* 1.2. Call cpssDxChIpMllCntGet with the same mllCntSet.
                Expected: GT_OK and the same mllOutMCPkts.  */

        st = cpssDxChIpMllCntGet(dev, mllCntSet, &mllOutMCPktsRet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpMllCntGet: %d, %d",
                                     dev, mllCntSet);

        UTF_VERIFY_EQUAL2_STRING_MAC(mllOutMCPkts, mllOutMCPktsRet,
            "get another mllOutMCPkts than was set: %d, %d", dev, mllCntSet);

        /* 1.3. Check out of range mllCntSet. Call with mllCntSet [10] and
           mllOutMCPkts [10] (no any constraints). Expected: GT_BAD_PARAM.  */

        mllCntSet = 10;
        mllOutMCPkts = 10;

        st = cpssDxChIpMllCntSet(dev, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, mllCntSet);

        /* 1.4. Call with mllCntSet [1] and mllOutMCPkts [0xFFFFFFFF]
           (no any constraints). Expected: GT_OK.   */

        mllCntSet = 1;
        mllOutMCPkts = 0xFFFFFFFF;

        st = cpssDxChIpMllCntSet(dev, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllCntSet, mllOutMCPkts);
    }

    mllCntSet = 1;
    mllOutMCPkts = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllCntSet(dev, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllCntSet(dev, mllCntSet, mllOutMCPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllSilentDropCntGet)
{
/*
ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
1.1. Call with non-null dropPktsPtr. Expected: GT_OK.
1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  silentDropPkts;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropPktsPtr. Expected: GT_OK.    */

        st = cpssDxChIpMllSilentDropCntGet(dev, &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChIpMllSilentDropCntGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, silentDropPkts = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                           UTF_LION_E | UTF_LION2_E );
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllSilentDropCntGet(dev, &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllSilentDropCntGet(dev, &silentDropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpDropCntSet
(
    IN GT_U8 devNum,
    IN GT_U32  dropPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpDropCntSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with dropPkts [10]. Expected: GT_OK.
1.2. Call cpssDxChIpDropCntGet.
Expected: GT_OK and the same dropPkts.
1.3. Call with dropPkts [0xFFFFFFFF] (no any constraints). Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropPkts;
    GT_U32                  dropPktsRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with dropPkts [10]. Expected: GT_OK.   */

        dropPkts = 10;

        st = cpssDxChIpDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);

        /* 1.2. Call cpssDxChIpDropCntGet. Expected: GT_OK and the same dropPkts.*/

        st = cpssDxChIpDropCntGet(dev, &dropPktsRet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpDropCntGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(dropPkts, dropPktsRet,
            "get another dropPkts than was set: %d", dev);

        /* 1.3. Call with dropPkts [0xFFFFFFFF] (no any constraints).
           Expected: GT_OK. */

        dropPkts = 0xFFFFFFFF;

        st = cpssDxChIpDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);
    }

    dropPkts = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpDropCntSet(dev, dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpDropCntSet(dev, dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpSetDropCntMode
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_IP_DROP_CNT_MODE_ENT dropCntMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpSetDropCntMode)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with dropCntMode [CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E / CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E / CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E].
Expected: GT_OK.
1.2. Call with wrong enum values dropCntMode .
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_DXCH_IP_DROP_CNT_MODE_ENT  dropCntMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with dropCntMode [CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E
           / CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E /
             CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E]. Expected: GT_OK.  */

        dropCntMode = CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E;

        st = cpssDxChIpSetDropCntMode(dev, dropCntMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCntMode);

        dropCntMode = CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E;

        st = cpssDxChIpSetDropCntMode(dev, dropCntMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCntMode);

        dropCntMode = CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E;

        st = cpssDxChIpSetDropCntMode(dev, dropCntMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropCntMode);

        /* 1.2. Call with wrong enum values dropCntMode .
                Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpSetDropCntMode
                            (dev, dropCntMode),
                            dropCntMode);
    }

    dropCntMode = CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpSetDropCntMode(dev, dropCntMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpSetDropCntMode(dev, dropCntMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *dropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpDropCntGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with non-null dropPktsPtr. Expected: GT_OK.
1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  dropPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null dropPktsPtr. Expected: GT_OK.    */

        st = cpssDxChIpDropCntGet(dev, &dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with dropPktsPtr [NULL]. Expected: GT_BAD_PTR.*/

        st = cpssDxChIpDropCntGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropPktsPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpDropCntGet(dev, &dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpDropCntGet(dev, &dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMtuProfileSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with mtuProfileIndex [0/3/7] and mtu [0/0x1FFF/0x3FFF].
Expected: GT_OK.
1.2. Call cpssDxChIpMtuProfileGet.
Expected: GT_OK and same mtu.
1.3. Call with out of range mtu[0x4000] and mtuProfileIndex[0].
Expected: Not GT_OK.
1.4. Call with out of range mtuProfileIndex [8] and mtu [100].
Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  mtuProfileIndex;
    GT_U32                  mtu;
    GT_U32                  mtuGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. */
        mtuProfileIndex = 0;
        mtu = 0;

        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex, mtu);

        /* 1.2. */
        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtuGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpMtuProfileGet: %d, %d", dev, mtuProfileIndex);

         /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /* 1.1. */
        mtuProfileIndex = 3;
        mtu = 0x1FFF;

        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex, mtu);

        /* 1.2. */
        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtuGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpMtuProfileGet: %d, %d", dev, mtuProfileIndex);

         /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /* 1.1. */
        mtuProfileIndex = 7;
        mtu = 0x3FFF;

        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex, mtu);

        /* 1.2. */
        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtuGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpMtuProfileGet: %d, %d", dev, mtuProfileIndex);

         /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /* 1.3.  */
        mtuProfileIndex = 0;
        mtu = 0x4000;

        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex, mtu);

        /* 1.4.  */

        mtuProfileIndex = 8;
        mtu = 0;

        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex);
    }

    mtuProfileIndex = 0;
    mtu = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMtuProfileSet(dev, mtuProfileIndex, mtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMtuProfileGet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    OUT GT_U32 *mtuPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMtuProfileGet)
{
/*
    ITERATE_DEVICES (DxCh2 and above)
    1.1. Call with mtuProfileIndex [0 / 7]
                   and non-NULL mtuPtr.
    Expected: GT_OK.
    1.2. Call with out of range mtuProfileIndex [8]
                   and other parameters from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with mtuPtr [NULL]
                   and other parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      mtuProfileIndex = 0;
    GT_U32      mtu             = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mtuProfileIndex [0 / 7]
                           and non-NULL mtuPtr.
            Expected: GT_OK.
        */

        /* Call with mtuProfileIndex [0] */
        mtuProfileIndex = 0;

        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtu);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex);

        /* Call with mtuProfileIndex [7] */
        mtuProfileIndex = 7;

        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtu);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex);

        /*
            1.2. Call with out of range mtuProfileIndex [8]
                           and other parameters from 1.1.
            Expected: NOT GT_OK.
        */
        mtuProfileIndex = 8;

        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtu);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mtuProfileIndex);

        mtuProfileIndex = 0;

        /*
            1.3. Call with mtuPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mtu = NULL", dev);
    }

    mtuProfileIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMtuProfileGet(dev, mtuProfileIndex, &mtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                     prefix,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6AddrPrefixScopeSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with prefix [200, \85, 200, 10, 1],prefixLen [1 / 4 / 8 / 16], addressScope [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E] and prefixScopeIndex [0 / 1 / 2 / 3].
Expected: GT_OK.
1.2. Call with out of range prefixLen [17] and other parameters from 1.1.
Expected: NOT GT_OK.
1.3. Call with wrong enum values addressScope  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with out of range prefixScopeIndex [5] and other parameters from 1.1.
Expected: NOT GT_OK.
1.5. Call function with prefix.arIP[0] [1/64/255], prefix.arIP[1] [1/64/255],
     prefixLen[3/7/15], addressScope[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E/
                                     CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E/
                                     CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
     and prefixScopeIndex [0 / 1 / 3].
Expected: GT_OK.
1.6. Call cpssDxChIpv6AddrPrefixScopeGet with prefixScopeIndex [0 / 1 / 3].
Expected: GT_OK and same prefix.arIP[0], prefix.arIP[1], prefixLen, addressScope.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_IPV6ADDR                     prefix;
    GT_U32                          prefixLen;
    CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope;
    GT_IPV6ADDR                     prefixGet;
    GT_U32                          prefixLenGet;
    CPSS_IPV6_PREFIX_SCOPE_ENT      addressScopeGet;
    GT_U32                          prefixScopeIndex;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with prefix [200, \85, 200, 10, 1],prefixLen [1 / 4 / 8
           / 16], addressScope [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E] and prefixScopeIndex
           [0 / 1 / 2 / 3]. Expected: GT_OK.    */

        cpssOsMemSet((GT_VOID*) &(prefix), 200, sizeof(prefix));
        prefix.arIP[14] = 10;
        prefix.arIP[15] = 1;
        prefixLen = 1;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        prefixScopeIndex = 0;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                   "%d, prefixLen=%d, addressScope=%d, prefixScopeIndex=%d",
                   dev, prefixLen, addressScope, prefixScopeIndex);

        prefixLen = 4;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        prefixScopeIndex = 1;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                   "%d, prefixLen=%d, addressScope=%d, prefixScopeIndex=%d",
                   dev, prefixLen, addressScope, prefixScopeIndex);

        prefixLen = 16;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        prefixScopeIndex = 3;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
                   "%d, prefixLen=%d, addressScope=%d, prefixScopeIndex=%d",
                   dev, prefixLen, addressScope, prefixScopeIndex);

        /* 1.2. Call with out of range prefixLen [17] and other parameters
                from 1.1. Expected: NOT GT_OK.    */

        prefixLen = 17;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        prefixScopeIndex = 0;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixLen = %d",
                                         dev, prefixLen);
        prefixLen = 1;  /* restore */

        /* 1.3. Call with wrong enum values addressScope  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6AddrPrefixScopeSet
                            (dev, &prefix, prefixLen, addressScope, prefixScopeIndex),
                            addressScope);

        /* 1.4. Call with out of range prefixScopeIndex [5] and other
           parameters from 1.1. Expected: NOT GT_OK.    */

        prefixScopeIndex = 5;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        prefixScopeIndex = 0;  /* restore */

        /*  1.5.  */
        cpssOsMemSet((GT_VOID*) &(prefix), 0, sizeof(prefix));
        cpssOsMemSet((GT_VOID*) &(prefixGet), 0, sizeof(prefixGet));
        prefix.arIP[0] = 1;
        prefix.arIP[1] = 64;
        prefixLen = 3;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        prefixScopeIndex = 0;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /*  1.6.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefixGet, &prefixLenGet,
                                            &addressScopeGet, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[0], prefixGet.arIP[0],
                   "get another prefix.arIP[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[1], prefixGet.arIP[1],
                   "get another prefix.arIP[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                   "get another prefixLen than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addressScope, addressScopeGet,
                   "get another addressScope than was set: %d", dev);

        /*  1.5.  */
        cpssOsMemSet((GT_VOID*) &(prefix), 0, sizeof(prefix));
        cpssOsMemSet((GT_VOID*) &(prefixGet), 0, sizeof(prefixGet));
        prefix.arIP[0] = 64;
        prefix.arIP[1] = 255;
        prefixLen = 7;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        prefixScopeIndex = 1;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /*  1.6.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefixGet, &prefixLenGet,
                                            &addressScopeGet, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[0], prefixGet.arIP[0],
                   "get another prefix.arIP[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[1], prefixGet.arIP[1],
                   "get another prefix.arIP[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                   "get another prefixLen than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addressScope, addressScopeGet,
                   "get another addressScope than was set: %d", dev);

        /*  1.5.  */
        cpssOsMemSet((GT_VOID*) &(prefix), 0, sizeof(prefix));
        cpssOsMemSet((GT_VOID*) &(prefixGet), 0, sizeof(prefixGet));
        prefix.arIP[0] = 255;
        prefix.arIP[1] = 1;
        prefixLen = 15;
        addressScope = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        prefixScopeIndex = 3;

        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /*  1.6.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefixGet, &prefixLenGet,
                                            &addressScopeGet, prefixScopeIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, prefixScopeIndex = %d",
                                     dev, prefixScopeIndex);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[0], prefixGet.arIP[0],
                   "get another prefix.arIP[0] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefix.arIP[1], prefixGet.arIP[1],
                   "get another prefix.arIP[1] than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(prefixLen, prefixLenGet,
                   "get another prefixLen than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(addressScope, addressScopeGet,
                   "get another addressScope than was set: %d", dev);
    }

    cpssOsMemSet((GT_VOID*) &(prefix), 200, sizeof(prefix));
    prefix.arIP[14] = 10;
    prefix.arIP[15] = 1;
    prefixLen = 1;
    addressScope = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    prefixScopeIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                            addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6AddrPrefixScopeSet(dev, &prefix, prefixLen,
                                        addressScope, prefixScopeIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6AddrPrefixScopeGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with prefixScopeIndex[0/1/3] and not NULL prefixPtr, prefixLenPtr, addressScopePtr.
Expected: GT_OK.
1.2. Call with out of range prefixScopeIndex[4].
Expected: not GT_OK.
1.3. Call with NULL prefixPtr.
Expected: GT_BAD_PTR.
1.4. Call with NULL prefixLenPtr.
Expected: GT_BAD_PTR.
1.5. Call with NULL addressScopePtr.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_IPV6ADDR                     prefix = {{0}};
    GT_U32                          prefixLen = 0;
    CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope = 0;
    GT_U32                          prefixScopeIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        prefixScopeIndex = 0;

        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        prefixScopeIndex = 1;

        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        prefixScopeIndex = 3;

        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        prefixScopeIndex = 4;

        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        prefixScopeIndex = 4;

        /*  1.3.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, NULL, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.4.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, NULL,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.5.  */
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            NULL, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    prefixScopeIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                            &addressScope, prefixScopeIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6AddrPrefixScopeGet(dev, &prefix, &prefixLen,
                                        &addressScope, prefixScopeIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6UcScopeCommandSet
(
    IN GT_U8                      devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN GT_BOOL                    borderCrossed,
    IN CPSS_PACKET_CMD_ENT        scopeCommand
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6UcScopeCommandSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], addressScopeDest [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], borderCrossed [GT_TRUE and GT_FALSE] and scopeCommand [CPSS_PACKET_CMD_ROUTE_E / CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E / CPSS_PACKET_CMD_DROP_SOFT_E / CPSS_PACKET_CMD_DROP_HARD_E].
Expected: GT_OK.
1.2. Call  cpssDxChIpv6UcScopeCommandGet.
Expected: GT_OK and same scopeCommand.
1.3. Call with scopeCommand [CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E / CPSS_PACKET_CMD_BRIDGE_E / CPSS_PACKET_CMD_NONE_E / CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_MIRROR_TO_CPU_E] (these commands are not possible) and other parameters from 1.1.
Expected: NOT GT_OK.
1.4. Call with wrong enum values addressScopeSrc  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.5. Call with wrong enum values addressScopeDest  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.6. Call with wrong enum values scopeCommand  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest;
    GT_BOOL                    borderCrossed;
    CPSS_PACKET_CMD_ENT        scopeCommand;
    CPSS_PACKET_CMD_ENT        scopeCommandGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], addressScopeDest
           [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], borderCrossed
           [GT_TRUE and GT_FALSE] and scopeCommand [CPSS_PACKET_CMD_ROUTE_E
           / CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E
           / CPSS_PACKET_CMD_DROP_SOFT_E / CPSS_PACKET_CMD_DROP_HARD_E].
            Expected: GT_OK.    */

        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed, scopeCommand);

        /*  1.2.  */
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);


        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed, scopeCommand);

        /*  1.2.  */
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);

        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        borderCrossed = GT_FALSE;
        scopeCommand = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed, scopeCommand);

        /*  1.2.  */
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);

        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        borderCrossed = GT_FALSE;
        scopeCommand = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed, scopeCommand);

        /*  1.2.  */
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);

        /* 1.3. Call with scopeCommand [CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E
           / CPSS_PACKET_CMD_BRIDGE_E / CPSS_PACKET_CMD_NONE_E /
             CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_MIRROR_TO_CPU_E]
           (these commands are not possible) and other parameters from 1.1.
            Expected: NOT GT_OK.    */

        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_ROUTE_E; /* restore */

        /* 1.4. Call with wrong enum values addressScopeSrc  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */

        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6UcScopeCommandSet
                            (dev, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommand),
                            addressScopeSrc);

        /* 1.5. Call with wrong enum values addressScopeDest  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6UcScopeCommandSet
                            (dev, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommand),
                            addressScopeDest);

        /* 1.6. Call with wrong enum values scopeCommand  and other
           parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6UcScopeCommandSet
                            (dev, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommand),
                            scopeCommand);
    }

    addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    borderCrossed = GT_TRUE;
    scopeCommand = CPSS_PACKET_CMD_ROUTE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6UcScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                       borderCrossed, scopeCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6UcScopeCommandGet
(
    IN  GT_U8                      devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest,
    IN  GT_BOOL                    borderCrossed,
    OUT CPSS_PACKET_CMD_ENT        *scopeCommandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6UcScopeCommandGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
               addressScopeDest[CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
               borderCrossed [GT_TRUE and GT_FALSE]
      and not NULL scopeCommand.
Expected: GT_OK.
1.2. Call with wrong enum values addressScopeSrc  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.3. Call with wrong enum values addressScopeDest  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with NULL scopeCommand  and other parameters from 1.1.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT addressScopeDest;
    GT_BOOL                    borderCrossed;
    CPSS_PACKET_CMD_ENT        scopeCommand = CPSS_PACKET_CMD_FORWARD_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;

        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        borderCrossed = GT_TRUE;

        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        borderCrossed = GT_FALSE;

        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        borderCrossed = GT_FALSE;

        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6UcScopeCommandGet
                            (dev, addressScopeSrc, addressScopeDest, borderCrossed, &scopeCommand),
                            addressScopeSrc);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6UcScopeCommandGet
                            (dev, addressScopeSrc, addressScopeDest, borderCrossed, &scopeCommand),
                            addressScopeDest);

        /*  1.4.  */
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, NULL);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, addressScopeSrc,
                              addressScopeDest, borderCrossed);
    }

    addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    borderCrossed = GT_TRUE;
    scopeCommand = CPSS_PACKET_CMD_ROUTE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6UcScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                       borderCrossed, &scopeCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6McScopeCommandSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], addressScopeDest [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], borderCrossed [GT_TRUE and GT_FALSE], scopeCommand [CPSS_PACKET_CMD_ROUTE_E / CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E / CPSS_PACKET_CMD_DROP_SOFT_E / CPSS_PACKET_CMD_DROP_HARD_E / CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E / CPSS_PACKET_CMD_BRIDGE_E] and mllSelectionRule [CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E / CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E].
Expected: GT_OK.
1.2. Call cpssDxChIpv6McScopeCommandGet
Expected: GT_OK and same scopeCommand, mllSelectionRule.
1.3. Call with scopeCommand [CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_NONE_E] (these commands are not possible) and other parameters from 1.1.
Expected: NOT GT_OK.
1.4. Call with wrong enum values addressScopeSrc  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.5. Call with wrong enum values addressScopeDest  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.6. Call with wrong enum values scopeCommand  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.7. Call with wrong enum values mllSelectionRule  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest;
    GT_BOOL                          borderCrossed;
    CPSS_PACKET_CMD_ENT              scopeCommand;
    CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule;
    CPSS_PACKET_CMD_ENT              scopeCommandGet;
    CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRuleGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E
           / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
           CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E], addressScopeDest
           [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E
           / CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E / CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
           borderCrossed [GT_TRUE and GT_FALSE], scopeCommand [CPSS_PACKET_CMD_ROUTE_E
           / CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E /
           CPSS_PACKET_CMD_DROP_SOFT_E / CPSS_PACKET_CMD_DROP_HARD_E /
           CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E / CPSS_PACKET_CMD_BRIDGE_E] and
           mllSelectionRule [CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E /
           CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E]. Expected: GT_OK. */

        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_ROUTE_E;
        mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed, scopeCommand, mllSelectionRule);

        /*  1.2. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet, &mllSelectionRuleGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllSelectionRule, mllSelectionRuleGet,
                   "get another mllSelectionRule than was set: %d", dev);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed, scopeCommand, mllSelectionRule);

        /*  1.2. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet, &mllSelectionRuleGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllSelectionRule, mllSelectionRuleGet,
                   "get another mllSelectionRule than was set: %d", dev);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        borderCrossed = GT_FALSE;
        scopeCommand = CPSS_PACKET_CMD_BRIDGE_E;
        mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed, scopeCommand, mllSelectionRule);

        /*  1.2. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet, &mllSelectionRuleGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllSelectionRule, mllSelectionRuleGet,
                   "get another mllSelectionRule than was set: %d", dev);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        borderCrossed = GT_FALSE;
        scopeCommand = CPSS_PACKET_CMD_DROP_HARD_E;
        mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_GLOBAL_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed, scopeCommand, mllSelectionRule);

        /*  1.2. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommandGet, &mllSelectionRuleGet);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scopeCommand, scopeCommandGet,
                   "get another scopeCommand than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(mllSelectionRule, mllSelectionRuleGet,
                   "get another mllSelectionRule than was set: %d", dev);

        /* 1.3. Call with scopeCommand [CPSS_PACKET_CMD_FORWARD_E /
           CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_NONE_E]
           (these commands are not possible) and other parameters from 1.1.
            Expected: NOT GT_OK.    */


        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;
        scopeCommand = CPSS_PACKET_CMD_FORWARD_E;
        mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, scopeCommand = %d",
                                         dev, scopeCommand);

        scopeCommand = CPSS_PACKET_CMD_ROUTE_E; /* restore */

        /* 1.4. Call with wrong enum values addressScopeSrc  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule),
                    addressScopeSrc);

        /* 1.5. Call with wrong enum values addressScopeDest  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */

        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule),
                    addressScopeDest);

        /* 1.6. Call with wrong enum values scopeCommand  and other
           parameters from 1.1. Expected: GT_BAD_PARAM. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule),
                    scopeCommand);

        /* 1.7. Call with wrong enum values mllSelectionRule  and
           other parameters from 1.1. Expected: GT_BAD_PARAM.   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule),
                    mllSelectionRule);
    }

    addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    borderCrossed = GT_TRUE;
    scopeCommand = CPSS_PACKET_CMD_ROUTE_E;
    mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, scopeCommand, mllSelectionRule);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6McScopeCommandSet(dev, addressScopeSrc, addressScopeDest,
                                       borderCrossed, scopeCommand, mllSelectionRule);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpv6McScopeCommandGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call with addressScopeSrc [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
                                CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
               addressScopeDest [CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E /
                                 CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E /
                                 CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E /
                                 CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E],
               borderCrossed [GT_TRUE and GT_FALSE],
      and not NULL scopeCommandPtr and  mllSelectionRulePtr.
Expected: GT_OK.
1.2. Call with wrong enum values addressScopeSrc  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.3. Call with wrong enum values addressScopeDest  and other parameters from 1.1.
Expected: GT_BAD_PARAM.
1.4. Call with NULL scopeCommandPtr  and other parameters from 1.1.
Expected: GT_BAD_PTR.
1.5. Call with NULL mllSelectionRulePtr  and other parameters from 1.1.
Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc;
    CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest;
    GT_BOOL                          borderCrossed;
    CPSS_PACKET_CMD_ENT              scopeCommand = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule = CPSS_IPV6_MLL_SELECTION_RULE_LOCAL_E;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        borderCrossed = GT_TRUE;

        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand, &mllSelectionRule);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E;
        borderCrossed = GT_TRUE;

        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand, &mllSelectionRule);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        borderCrossed = GT_FALSE;

        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand, &mllSelectionRule);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /*  1.1. */
        addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
        addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E;
        borderCrossed = GT_FALSE;

        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand, &mllSelectionRule);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /*  1.2. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, &scopeCommand, &mllSelectionRule),
                    addressScopeSrc);

        /*  1.3. */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc,
                    addressScopeDest, borderCrossed, &scopeCommand, &mllSelectionRule),
                    addressScopeDest);

        /*  1.4. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, NULL, &mllSelectionRule);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);

        /*  1.5. */
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand,NULL);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, addressScopeSrc, addressScopeDest,
                                    borderCrossed);
    }

    addressScopeSrc = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    addressScopeDest = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    borderCrossed = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                           borderCrossed, &scopeCommand, &mllSelectionRule);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpv6McScopeCommandGet(dev, addressScopeSrc, addressScopeDest,
                                       borderCrossed, &scopeCommand, &mllSelectionRule);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterMacSaBaseSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with macPtr [{0,1, 2, 3, 4, 5}].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpRouterMacSaBaseGet  with non-NULL macPtr
    Expected: GT_OK and mac tha same as just written.
    1.1.3. Call with macPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_ETHERADDR    mac = {{1, 2, 3, 4, 5}};
    GT_ETHERADDR    retMac;
    GT_BOOL         isEqual = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with macPtr [{0,1, 2, 3, 4, 5}].
        Expected: GT_OK. */
        st = cpssDxChIpRouterMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1.2. Call cpssDxChIpRouterMacSaBaseGet  with non-NULL macPtr
        Expected: GT_OK and mac tha same as just written. */
        st = cpssDxChIpRouterMacSaBaseGet(dev, &retMac);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterMacSaBaseGet: %d", dev);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&mac, (GT_VOID*)&retMac, sizeof (mac.arEther[0])*5))
                   ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mac than was set: %d", dev);

        /* 1.1.3. Call with macPtr [NULL].
        Expected: GT_BAD_PTR. */
        st = cpssDxChIpRouterMacSaBaseSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterMacSaBaseSet(dev, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterMacSaBaseGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with non-NULL macPtr
    Expected: GT_OK.
    1.1.2. Call with macPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_ETHERADDR    retMac;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.1. Call with non-NULL macPtr
        Expected: GT_OK. */
        st = cpssDxChIpRouterMacSaBaseGet(dev, &retMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1.2. Call with macPtr [NULL].
        Expected: GT_BAD_PTR. */
        st = cpssDxChIpRouterMacSaBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterMacSaBaseGet(dev, &retMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterMacSaBaseGet(dev, &retMac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortRouterMacSaLsbModeSet)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORT (DxCh3 and above)
    1.1.1. Call with saLsbMode [CPSS_SA_LSB_PER_PORT_E /
                                CPSS_SA_LSB_PER_PKT_VID_E /
                                CPSS_SA_LSB_PER_VLAN_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortRouterMacSaLsbModeGet with non-NULL saLsbModePtr.
    Expected: GT_OK and the same saLsbMode.
    1.1.3. Call with wrong enum values saLsbMode .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode    = CPSS_SA_LSB_PER_PORT_E;
    CPSS_MAC_SA_LSB_MODE_ENT    saLsbModeGet = CPSS_SA_LSB_PER_PORT_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with saLsbMode [CPSS_SA_LSB_PER_PORT_E /
                                            CPSS_SA_LSB_PER_PKT_VID_E /
                                            CPSS_SA_LSB_PER_VLAN_E].
                Expected: GT_OK.
            */

            /* Call with saLsbMode [CPSS_SA_LSB_PER_PORT_E] */
            saLsbMode = CPSS_SA_LSB_PER_PORT_E;

            st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, saLsbMode);
                /*
                    1.1.2. Call cpssDxChIpPortRouterMacSaLsbModeGet with non-NULL saLsbModePtr.
                    Expected: GT_OK and the same saLsbMode.
                */
                st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbModeGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssDxChIpPortRouterMacSaLsbModeGet: %d, %d", dev, port);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(saLsbMode, saLsbModeGet,
                           "get another saLsbMode than was set: %d, %d", dev, port);
            }

            /* Call with saLsbMode [CPSS_SA_LSB_PER_PKT_VID_E] */
            saLsbMode = CPSS_SA_LSB_PER_PKT_VID_E;

            st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {                   
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, saLsbMode);
            }

            /*
                1.1.2. Call cpssDxChIpPortRouterMacSaLsbModeGet with non-NULL saLsbModePtr.
                Expected: GT_OK and the same saLsbMode.
            */
            st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpPortRouterMacSaLsbModeGet: %d, %d", dev, port);

            /* Verifying values */            
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_SA_LSB_PER_VLAN_E, saLsbModeGet,
                       "get another saLsbMode than was set: %d, %d", dev, port);
            }
            else
            {                   
                UTF_VERIFY_EQUAL2_STRING_MAC(saLsbMode, saLsbModeGet,
                       "get another saLsbMode than was set: %d, %d", dev, port);
            }

            /* Call with saLsbMode [CPSS_SA_LSB_PER_VLAN_E] */
            saLsbMode = CPSS_SA_LSB_PER_VLAN_E;

            st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, saLsbMode);

            /*
                1.1.2. Call cpssDxChIpPortRouterMacSaLsbModeGet with non-NULL saLsbModePtr.
                Expected: GT_OK and the same saLsbMode.
            */
            st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbModeGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpPortRouterMacSaLsbModeGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(saLsbMode, saLsbModeGet,
                       "get another saLsbMode than was set: %d, %d", dev, port);

            /*
                1.1.3. Call with wrong enum values saLsbMode.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortRouterMacSaLsbModeSet
                                (dev, port, saLsbMode),
                                saLsbMode);
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /*since the device not supports 'per port'*/
            saLsbMode = CPSS_SA_LSB_PER_PKT_VID_E;
        }
        else
        {
            saLsbMode = CPSS_SA_LSB_PER_PORT_E;
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            /*since the device not supports 'per port'*/
            saLsbMode = CPSS_SA_LSB_PER_PKT_VID_E;
        }

        st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev) && !(PRV_CPSS_SIP_6_CHECK_MAC(dev)))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    saLsbMode = CPSS_SA_LSB_PER_PORT_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortRouterMacSaLsbModeSet(dev, port, saLsbMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortRouterMacSaLsbModeGet)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORT (DxCh3 and above)
    1.1.1. Call with non-NULL saLsbModePtr.
    Expected: GT_OK.
    1.1.2. Call with saLsbModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode = CPSS_SA_LSB_PER_PORT_E;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL saLsbModePtr.
                Expected: GT_OK.
            */
            st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with saLsbModePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, saLsbModePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortRouterMacSaLsbModeGet(dev, port, &saLsbMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterPortMacSaLsbSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    saMac
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterPortMacSaLsbSet)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORT (DxCh3 and above)
    1.1.1. Call with saMac [0 / 0xFF].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpRouterPortMacSaLsbGet with non-NULL saMacPtr.
    Expected: GT_OK and the same saMac.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_U8       saMac    = 0;
    GT_U8       saMacGet = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with saMac [0 / 0xFF].
                Expected: GT_OK.
            */

            /* Call with saMac [0] */
            saMac = 0;

            st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, saMac);

            /*
                1.1.2. Call cpssDxChIpRouterPortMacSaLsbGet with non-NULL saMacPtr.
                Expected: GT_OK and the same saMac.
            */
            st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMacGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpRouterPortMacSaLsbGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(saMac, saMacGet,
                       "get another saMac than was set: %d, %d", dev, port);

            /* Call with saMac [0xFF] */
            saMac = 0xFF;

            st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, saMac);

            /*
                1.1.2. Call cpssDxChIpRouterPortMacSaLsbGet with non-NULL saMacPtr.
                Expected: GT_OK and the same saMac.
            */
            st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMacGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpRouterPortMacSaLsbGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(saMac, saMacGet,
                       "get another saMac than was set: %d, %d", dev, port);
        }

        saMac = 0;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    saMac = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterPortMacSaLsbSet(dev, port, saMac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterPortMacSaLsbGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *saMacPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterPortMacSaLsbGet)
{
/*
    ITERATE_DEVICES_VIRT_CPU_PORT (DxCh3 and above)
    1.1.1. Call with non-NULL saLsbModePtr.
    Expected: GT_OK.
    1.1.2. Call with saLsbModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_U8   saMac = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL saLsbModePtr.
                Expected: GT_OK.
            */

            st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with saLsbModePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, saLsbModePtr = NULL", dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available virtual ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterPortMacSaLsbGet(dev, port, &saMac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterVlanMacSaLsbSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlan,
    IN GT_U32   saMac
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterVlanMacSaLsbSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with vlan [100 / 4095]
                   and saMac [0 / 0xFF].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterVlanMacSaLsbGet with the same vlan and non-NULL saMacPtr.
    Expected: GT_OK and the same saMac.
    1.3. Call with out of range vlan [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U16      vlan     = 0;
    GT_U32      saMac    = 0;
    GT_U32      saMacGet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlan [100 / 4095]
                           and saMac [0 / 0xFF].
            Expected: GT_OK.
        */

        /* Call with vlan [100] */
        vlan  = 100;
        saMac = 0;

        st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlan, saMac);

        /*
            1.2. Call cpssDxChIpRouterVlanMacSaLsbGet with the same vlan and non-NULL saMacPtr.
            Expected: GT_OK and the same saMac.
        */
        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMacGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpRouterVlanMacSaLsbGet: %d, %d", dev, vlan);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(saMac, saMacGet,
                   "get another saMac than was set: %d", dev);

        /* Call with vlan [4095] */
        vlan  = 4095;
        saMac = 0xFF;

        st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlan, saMac);

        /*
            1.2. Call cpssDxChIpRouterVlanMacSaLsbGet with the same vlan and non-NULL saMacPtr.
            Expected: GT_OK and the same saMac.
        */
        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMacGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                   "cpssDxChIpRouterVlanMacSaLsbGet: %d, %d", dev, vlan);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(saMac, saMacGet,
                   "get another saMac than was set: %d", dev);

        /*
            1.3. Call with out of range vlan [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlan = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
        if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* support eVlans range */
            if(vlan > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlan);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlan);
            }

            /* support eVlans range */
            vlan = (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
            /* supports 12 bits*/
            saMac = BIT_12-1;/*0xfff*/
            st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlan, saMac);

            st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMacGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpRouterVlanMacSaLsbGet: %d, %d", dev, vlan);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(saMac, saMacGet,
                       "get another saMac than was set: %d", dev);
            /*saMac fail*/
            vlan = (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
            saMac = BIT_12;/*0x1000*/
            st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlan, saMac);

            /*vlan fail*/
            vlan = 1 + (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
            saMac = BIT_12 - 1;/*0xfff*/
            st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlan, saMac);

        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlan);
        }
    }

    vlan  = 100;
    saMac = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterVlanMacSaLsbSet(dev, vlan, saMac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterVlanMacSaLsbGet
(
    IN  GT_U8   devNum,
    IN  GT_U16  vlan,
    OUT GT_U32  *saMacPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterVlanMacSaLsbGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with vlan [100 / 4095]
                   and non-NULL saMacPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlan [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
                   and other parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with saMacPtr [NULL]
                   and other parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U16      vlan  = 0;
    GT_U32      saMac = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlan [100 / 4095]
                           and non-NULL saMacPtr.
            Expected: GT_OK.
        */

        /* Call with vlan [100] */
        vlan = 100;

        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlan);

        /* Call with vlan [4095] */
        vlan = 4095;

        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlan);

        /*
            1.2. Call with out of range vlan [PRV_CPSS_MAX_NUM_VLANS_CNS = 4096]
                           and other parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        vlan = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
        if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* support eVlans range */
            if(vlan > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlan);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlan);
            }

            /* support eVlans range */
            vlan = (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
            st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpRouterVlanMacSaLsbGet: %d, %d", dev, vlan);

            /*vlan fail*/
            vlan = 1 + (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
            st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssDxChIpRouterVlanMacSaLsbGet: %d, %d", dev, vlan);

        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vlan);
        }

        vlan = 0;

        /*
            1.3. Call with saMacPtr [NULL]
                           and other parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, saMacPtr = NULL", dev);
    }

    vlan = 100;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterVlanMacSaLsbGet(dev, vlan, &saMac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_PORT_NUM                portNum,
    IN  GT_BOOL                    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterMacSaModifyEnable)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpRouterMacSaModifyEnableGet with  non-NULL enable.
    Expected: GT_OK and enable values the same as have been just set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     retEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call with portNum [0/ 31] and enable [GT_TRUE/ GT_FALSE].
            Expected: GT_OK. */
            /* 1.2. Call cpssDxChIpRouterMacSaModifyEnableGet with portNum [0/ 31]
            and non-NULL enable.
            Expected: GT_OK and enable values the same as have been just set. */

            /* 1.1. for portNum = 0 and enable =GT_TRUE*/
            enable = GT_TRUE;

            st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2. for portNum = 0 and enable =GT_TRUE*/
            st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpRouterMacSaModifyEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable value than was set: %d, %d", dev, port);

            /* 1.1. enable =GT_FALSE*/
            enable = GT_FALSE;

            st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2. for enable =GT_FALSE*/
            st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpRouterMacSaModifyEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable value than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port   = IP_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterMacSaModifyEnable(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterMacSaModifyEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_NUM                 portNum,
    OUT GT_BOOL                     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterMacSaModifyEnableGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with non-NULL enable. Expected: GT_OK.
    1.1.2. Call with enable [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     retEnable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call with non-NULL enable. Expected: GT_OK. */
            st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.2. Call with enable [NULL]. Expected: GT_BAD_PTR. */
            st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL",
                                         dev, port);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
            if(IS_E_ARCH_AND_PHYSICAL_PORT_IN_RANGE_MAC(dev,port))
            {
                /* the device supports 256 port regardless to 'physical port' existence */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterMacSaModifyEnableGet(dev, port, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpUcRpfCheckEnableSet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with enable [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpEcmpUcRpfCheckEnableGet with non-NULL enablePtr.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */

        /* Call with enable [GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChIpEcmpUcRpfCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChIpEcmpUcRpfCheckEnableGet with non-NULL enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpEcmpUcRpfCheckEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /* Call with enable [GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChIpEcmpUcRpfCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
            1.2. Call cpssDxChIpEcmpUcRpfCheckEnableGet with non-NULL enablePtr.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                   "cpssDxChIpEcmpUcRpfCheckEnableGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    enable = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpUcRpfCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpUcRpfCheckEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpEcmpUcRpfCheckEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpEcmpUcRpfCheckEnableGet)
{
/*
    ITERATE_DEVICES (DxCh3 and above)
    1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpEcmpUcRpfCheckEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupCntGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN  CPSS_IP_CNT_SET_ENT          cntSet,
    OUT CPSS_DXCH_IP_COUNTER_SET_STC *countersPtr
)
*/

typedef enum
{
    PRV_UTF_COUNTER_SET   = 0,
    PRV_UTF_COUNTER_PORT_GROUP_SET = 1,
    PRV_UTF_COUNTER_GET   = 2,
    PRV_UTF_COUNTER_PORT_GROUP_GET = 3
}PRV_UTF_COUNTER_SET_GET_ENT;


static GT_STATUS prvUtfPrivateCpssDxChIpPortGroupCntGetSet
(
    IN   GT_U8                          devNum,
    IN   GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN   GT_U32                         regAddr,
    IN   PRV_UTF_COUNTER_SET_GET_ENT    setGetOption,
    IN   GT_U32                         counterValue,
    OUT  GT_U32                         *counterValuePtr
)
{
    CPSS_DXCH_IP_COUNTER_SET_STC       ipCounter;   /* ip counter */
    GT_STATUS                          st;          /* return status */
    GT_U32                             i;           /* iterator */
    GT_U32                         regAddrBase;

    /* reset ipCounter */
    cpssOsMemSet(&ipCounter, 0, sizeof(CPSS_DXCH_IP_COUNTER_SET_STC));

    regAddrBase = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerMngInUcPktCntSet[0];
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddrBase )
    {
        regAddrBase = 0x02800900;
    }

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        convertRegAddrToNewUnitsBaseAddr(devNum,&regAddrBase);
    }

    for(i = 0; i < 4; i++)
   {
       if(regAddr == (regAddrBase + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inUcPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inUcPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+4) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inMcPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inMcPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }

       }
       if(regAddr == ((regAddrBase+8) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inUcNonRoutedNonExcpPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inUcNonRoutedNonExcpPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcNonRoutedNonExcpPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcNonRoutedNonExcpPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0xc) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inUcNonRoutedExcpPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inUcNonRoutedExcpPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcNonRoutedExcpPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcNonRoutedExcpPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x10) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inMcNonRoutedNonExcpPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inMcNonRoutedNonExcpPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcNonRoutedNonExcpPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcNonRoutedNonExcpPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x14) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inMcNonRoutedExcpPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inMcNonRoutedExcpPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcNonRoutedExcpPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcNonRoutedExcpPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x18) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inUcTrappedMirrorPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inUcTrappedMirrorPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcTrappedMirrorPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inUcTrappedMirrorPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x1C) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.inMcTrappedMirrorPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.inMcTrappedMirrorPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcTrappedMirrorPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.inMcTrappedMirrorPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x20) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.mcRfpFailPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.mcRfpFailPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.mcRfpFailPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.mcRfpFailPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
       if(regAddr == ((regAddrBase+0x24) + i * 0x100))
       {
           switch(setGetOption)
           {
              case PRV_UTF_COUNTER_SET:
                  ipCounter.outUcRoutedPkts = counterValue;
                  return cpssDxChIpCntSet(devNum, i, &ipCounter);

              case PRV_UTF_COUNTER_PORT_GROUP_SET:
                  ipCounter.outUcRoutedPkts = counterValue;
                  return cpssDxChIpPortGroupCntSet(devNum, portGroupsBmp,
                                                   i, &ipCounter);
              case PRV_UTF_COUNTER_GET:
                  st = cpssDxChIpCntGet(devNum, i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.outUcRoutedPkts;
                  return GT_OK;

              case PRV_UTF_COUNTER_PORT_GROUP_GET:
                  st = cpssDxChIpPortGroupCntGet(devNum, portGroupsBmp,
                                                 i, &ipCounter);
                  if(st != GT_OK)
                      return st;

                  *counterValuePtr = ipCounter.outUcRoutedPkts;
                  return GT_OK;

              default:
                  return GT_BAD_PARAM;
           }
       }
   }
    return GT_OK;
}

static GT_STATUS prvUtfPrivateCpssDxChIpCntSet
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         regAddr,
    IN   GT_U32                         counterValue
)
{
    return prvUtfPrivateCpssDxChIpPortGroupCntGetSet(devNum, 0, regAddr,
                                                     PRV_UTF_COUNTER_SET,
                                                     counterValue,
                                                     NULL);
}

static GT_STATUS prvUtfPrivateCpssDxChIpPortGroupCntSet
(
    IN   GT_U8                          devNum,
    IN   GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN   GT_U32                         regAddr,
    IN   GT_U32                         counterValue
)
{
    return prvUtfPrivateCpssDxChIpPortGroupCntGetSet(devNum, portGroupsBmp,
                                               regAddr,
                                               PRV_UTF_COUNTER_PORT_GROUP_SET,
                                               counterValue,
                                               NULL);
}

static GT_STATUS prvUtfPrivateCpssDxChIpCntGet
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         regAddr,
    OUT  GT_U32                         *counterValuePtr
)
{
    return prvUtfPrivateCpssDxChIpPortGroupCntGetSet(devNum, 0,
                                                     regAddr,
                                                     PRV_UTF_COUNTER_GET,
                                                     0,
                                                     counterValuePtr);
}

static GT_STATUS prvUtfPrivateCpssDxChIpPortGroupCntGet
(
    IN   GT_U8                          devNum,
    IN   GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN   GT_U32                         regAddr,
    OUT  GT_U32                         *counterValuePtr
)
{
    return prvUtfPrivateCpssDxChIpPortGroupCntGetSet(devNum, portGroupsBmp,
                                               regAddr,
                                               PRV_UTF_COUNTER_PORT_GROUP_GET,
                                               0,
                                               counterValuePtr);
}

UTF_TEST_CASE_MAC(cpssDxChIpPortGroupCntGet)
{
    GT_U32 regAddr = 0xFFFFFFFF,regAddr1; /* register address not initialized */
    GT_U32 i;                    /* iterator */
    GT_U32 cntSet;               /* cntSet iterator */
    GT_U8     dev;               /* device number */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->ipRegs.routerMngInUcPktCntSet[0];
        if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
        {
            regAddr = 0x02800900;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            convertRegAddrToNewUnitsBaseAddr(dev,&regAddr);
        }

        break;
    }

    if(regAddr != 0xFFFFFFFF)
    {

        for(i = 0; i < 10; i++)
            for(cntSet = 0; cntSet < 4; cntSet++)
            {
                regAddr1 = regAddr + i*4 + cntSet * 0x100;

                prvUtfPerPortGroupCounterPerRegGet(regAddr1, 0, 32,
                                                   prvUtfPrivateCpssDxChIpCntGet,
                                                   prvUtfPrivateCpssDxChIpCntSet,
                                                   prvUtfPrivateCpssDxChIpPortGroupCntGet,
                                                   prvUtfPrivateCpssDxChIpPortGroupCntSet);
            }
    }

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *dropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupDropCntSet)
{
    GT_U32 regAddr = 0xFFFFFFFF; /* register address not initialized */
    GT_U8     dev;               /* device number */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->ipRegs.routerDropCnt;
        if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
        {
            regAddr = 0x02800950;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            convertRegAddrToNewUnitsBaseAddr(dev,&regAddr);
        }
        break;
    }

    if(regAddr != 0xFFFFFFFF)
    {
        prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
              cpssDxChIpDropCntGet,
              cpssDxChIpDropCntSet,
              cpssDxChIpPortGroupDropCntGet,
              cpssDxChIpPortGroupDropCntSet);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet
(
    IN   GT_U8                  devNum,
    IN   GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT  GT_U32                 *dropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet)
{
    GT_U32 regAddr = 0xFFFFFFFF; /* register address not initialized */
    GT_U8     dev;               /* device number */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            regAddr = 0x0d800984;
        }
        else if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regAddr = 0x19000984;
        }
        else
        {
            regAddr = 0x0C800984;
        }
        break;
    }

    if(regAddr != 0xFFFFFFFF)
        prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                                     cpssDxChIpMultiTargetQueueFullDropCntGet,
                                     cpssDxChIpMultiTargetQueueFullDropCntSet,
                                     cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet,
                                     cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMllCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
)
*/
static GT_STATUS prvUtfCpssDxChIpPortGroupMllCntSet0Get
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    return cpssDxChIpPortGroupMllCntGet(devNum,portGroupsBmp, 0, mllOutMCPktsPtr);

}
static GT_STATUS prvUtfCpssDxChIpPortGroupMllCntSet1Get
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    return cpssDxChIpPortGroupMllCntGet(devNum,portGroupsBmp, 1 ,mllOutMCPktsPtr);

}
static GT_STATUS prvUtfCpssDxChIpMllCntSet0Get
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    return cpssDxChIpMllCntGet(devNum, 0, mllOutMCPktsPtr);

}
static GT_STATUS prvUtfCpssDxChIpMllCntSet1Get
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    return cpssDxChIpMllCntGet(devNum, 1 ,mllOutMCPktsPtr);

}

static GT_STATUS prvUtfCpssDxChIpPortGroupMllCntSet0Set
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllOutMCPkts
)
{
    return cpssDxChIpPortGroupMllCntSet(devNum,portGroupsBmp, 0, mllOutMCPkts);

}
static GT_STATUS prvUtfCpssDxChIpPortGroupMllCntSet1Set
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllOutMCPkts
)
{
    return cpssDxChIpPortGroupMllCntSet(devNum,portGroupsBmp, 1 ,mllOutMCPkts);

}
static GT_STATUS prvUtfCpssDxChIpMllCntSet0Set
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  mllOutMCPkts
)
{
    return cpssDxChIpMllCntSet(devNum, 0, mllOutMCPkts);

}
static GT_STATUS prvUtfCpssDxChIpMllCntSet1Set
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  mllOutMCPkts
)
{
    return cpssDxChIpMllCntSet(devNum, 1 ,mllOutMCPkts);
}


UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMllCntGet)
{
    GT_U32 regAddr = 0xFFFFFFFF; /* register address not initialized */
    GT_U8     dev;               /* device number */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
         if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            regAddr = 0x0d800900;
        }
        else if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            regAddr = 0x19000900;
        }
        else
        {
            regAddr = 0x0C800900;
        }
        break;
    }

    if(regAddr != 0xFFFFFFFF)
    {
        prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                                     prvUtfCpssDxChIpMllCntSet0Get,
                                     prvUtfCpssDxChIpMllCntSet0Set,
                                     prvUtfCpssDxChIpPortGroupMllCntSet0Get,
                                     prvUtfCpssDxChIpPortGroupMllCntSet0Set);

        regAddr += 0x100;
        prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
                                     prvUtfCpssDxChIpMllCntSet1Get,
                                     prvUtfCpssDxChIpMllCntSet1Set,
                                     prvUtfCpssDxChIpPortGroupMllCntSet1Get,
                                     prvUtfCpssDxChIpPortGroupMllCntSet1Set);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupCntSet
(
    IN GT_U8                         devNum,
    IN GT_PORT_GROUPS_BMP            portGroupsBmp,
    IN CPSS_IP_CNT_SET_ENT           cntSet,
    IN CPSS_DXCH_IP_COUNTER_SET_STC *counters
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupCntSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with cntSet [CPSS_IP_CNT_SET0_E].
         and counters{inUcPkts [1],
            inMcPkts [1],
            inUcNonRoutedExcpPkts [0],
            inUcNonRoutedNonExcpPkts [0],
            inMcNonRoutedExcpPkts [0],
            inMcNonRoutedNonExcpPkts [0],
            inUcTrappedMirrorPkts [1],
            inMcTrappedMirrorPkts [1],
            mcRfpFailPkts [0],
            outUcRoutedPkts [1]}.
    Expected: GT_OK.
    1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
    Expected: GT_OK and the same counters.
    1.1.2. Call with cntSet [CPSS_IP_CNT_SET1_E].
        and counters{inUcPkts [10],
            inMcPkts [10],
            inUcNonRoutedExcpPkts [100],
            inUcNonRoutedNonExcpPkts [100],
            inMcNonRoutedExcpPkts [1000],
            inMcNonRoutedNonExcpPkts [1000],
            inUcTrappedMirrorPkts [1100],
            inMcTrappedMirrorPkts [1100],
            mcRfpFailPkts [1000],
            outUcRoutedPkts [1001]}.
    Expected: GT_OK.
    1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
    Expected: GT_OK and the same counters.
    1.1.3. Call with cntSet [CPSS_IP_CNT_SET2_E].
        and counters{inUcPkts [30],
            inMcPkts [30],
            inUcNonRoutedExcpPkts [300],
            inUcNonRoutedNonExcpPkts [300],
            inMcNonRoutedExcpPkts [3000],
            inMcNonRoutedNonExcpPkts [3000],
            inUcTrappedMirrorPkts [3300],
            inMcTrappedMirrorPkts [3300],
            mcRfpFailPkts [3000],
            outUcRoutedPkts [3003]}.
    Expected: GT_OK.
    1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
    Expected: GT_OK and the same counters.
    1.1.4. Call with cntSet [CPSS_IP_CNT_SET3_E].
        and counters{inUcPkts [990],
            inMcPkts [990],
            inUcNonRoutedExcpPkts [9900],
            inUcNonRoutedNonExcpPkts [9900],
            inMcNonRoutedExcpPkts [99000],
            inMcNonRoutedNonExcpPkts [99000],
            inUcTrappedMirrorPkts [999900],
            inMcTrappedMirrorPkts [999900],
            mcRfpFailPkts [99000],
            outUcRoutedPkts [990099]}.
    Expected: GT_OK.
    1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
    Expected: GT_OK and the same counters.
    1.1.6. Call with wrong enum values cntSet  and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.1.7. Call with counters [NULL] and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;

    CPSS_IP_CNT_SET_ENT           cntSet = CPSS_IP_CNT_SET0_E;
    CPSS_DXCH_IP_COUNTER_SET_STC  counters = {0};
    CPSS_DXCH_IP_COUNTER_SET_STC  countersRet = {0};

    GT_BOOL     isEqual;
    GT_U32      portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with cntSet [CPSS_IP_CNT_SET0_E].
                    and counters{inUcPkts [1],
                        inMcPkts [1],
                        inUcNonRoutedExcpPkts [0],
                        inUcNonRoutedNonExcpPkts [0],
                        inMcNonRoutedExcpPkts [0],
                        inMcNonRoutedNonExcpPkts [0],
                        inUcTrappedMirrorPkts [1],
                        inMcTrappedMirrorPkts [1],
                        mcRfpFailPkts [0],
                        outUcRoutedPkts [1]}.
                Expected: GT_OK.
            */
            cntSet = CPSS_IP_CNT_SET0_E;
            counters.inUcPkts  = 1;
            counters.inMcPkts  = 1;
            counters.inUcNonRoutedExcpPkts = 0;
            counters.inUcNonRoutedNonExcpPkts = 0;
            counters.inMcNonRoutedExcpPkts = 0;
            counters.inMcNonRoutedNonExcpPkts = 0;
            counters.inUcTrappedMirrorPkts = 1;
            counters.inMcTrappedMirrorPkts = 1;
            counters.mcRfpFailPkts  = 0;
            counters.outUcRoutedPkts = 1;

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

            /*
               1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
               Expected: GT_OK and the same counters.
            */
            cpssOsBzero((GT_VOID*) &countersRet, sizeof(countersRet));

            st = cpssDxChIpPortGroupCntGet(dev, portGroupsBmp, cntSet, &countersRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChIpPortGroupCntGet: %d, %d", dev, cntSet);

            isEqual =
                (0 == cpssOsMemCmp((GT_VOID*)&counters, (GT_VOID*)&countersRet, sizeof (counters)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                      "get another counters than was set: %d, %d", dev, cntSet);

            /*
                1.1.2. Call with cntSet [CPSS_IP_CNT_SET1_E].
                    and counters{inUcPkts [10],
                        inMcPkts [10],
                        inUcNonRoutedExcpPkts [100],
                        inUcNonRoutedNonExcpPkts [100],
                        inMcNonRoutedExcpPkts [1000],
                        inMcNonRoutedNonExcpPkts [1000],
                        inUcTrappedMirrorPkts [1100],
                        inMcTrappedMirrorPkts [1100],
                        mcRfpFailPkts [1000],
                        outUcRoutedPkts [1001]}.
                Expected: GT_OK.
            */
            cntSet = CPSS_IP_CNT_SET1_E;

            counters.inUcPkts  = 10;
            counters.inMcPkts  = 10;
            counters.inUcNonRoutedExcpPkts = 100;
            counters.inUcNonRoutedNonExcpPkts = 100;
            counters.inMcNonRoutedExcpPkts = 1000;
            counters.inMcNonRoutedNonExcpPkts = 1000;
            counters.inUcTrappedMirrorPkts = 1100;
            counters.inMcTrappedMirrorPkts = 1100;
            counters.mcRfpFailPkts  = 1000;
            counters.outUcRoutedPkts = 1001;

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

            /*
               1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
               Expected: GT_OK and the same counters.
            */
            cpssOsBzero((GT_VOID*) &countersRet, sizeof(countersRet));

            st = cpssDxChIpPortGroupCntGet(dev, portGroupsBmp, cntSet, &countersRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChIpPortGroupCntGet: %d, %d", dev, cntSet);

            isEqual =
                (0 == cpssOsMemCmp((GT_VOID*)&counters, (GT_VOID*)&countersRet, sizeof (counters)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                      "get another counters than was set: %d, %d", dev, cntSet);

            /*
                1.1.3. Call with cntSet [CPSS_IP_CNT_SET2_E].
                    and counters{inUcPkts [30],
                        inMcPkts [30],
                        inUcNonRoutedExcpPkts [300],
                        inUcNonRoutedNonExcpPkts [300],
                        inMcNonRoutedExcpPkts [3000],
                        inMcNonRoutedNonExcpPkts [3000],
                        inUcTrappedMirrorPkts [3300],
                        inMcTrappedMirrorPkts [3300],
                        mcRfpFailPkts [3000],
                        outUcRoutedPkts [3003]}.
                Expected: GT_OK.
            */
            cntSet = CPSS_IP_CNT_SET2_E;
            counters.inUcPkts  = 30;
            counters.inMcPkts  = 30;
            counters.inUcNonRoutedExcpPkts = 300;
            counters.inUcNonRoutedNonExcpPkts = 300;
            counters.inMcNonRoutedExcpPkts = 3000;
            counters.inMcNonRoutedNonExcpPkts = 3000;
            counters.inUcTrappedMirrorPkts = 3300;
            counters.inMcTrappedMirrorPkts = 3300;
            counters.mcRfpFailPkts  = 3000;
            counters.outUcRoutedPkts = 3003;

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

            /*
               1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
               Expected: GT_OK and the same counters.
            */
            cpssOsBzero((GT_VOID*) &countersRet, sizeof(countersRet));

            st = cpssDxChIpPortGroupCntGet(dev, portGroupsBmp, cntSet, &countersRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChIpPortGroupCntGet: %d, %d", dev, cntSet);

            isEqual =
                (0 == cpssOsMemCmp((GT_VOID*)&counters, (GT_VOID*)&countersRet, sizeof (counters)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                      "get another counters than was set: %d, %d", dev, cntSet);

            /*
                1.1.4. Call with cntSet [CPSS_IP_CNT_SET3_E].
                    and counters{inUcPkts [990],
                        inMcPkts [990],
                        inUcNonRoutedExcpPkts [9900],
                        inUcNonRoutedNonExcpPkts [9900],
                        inMcNonRoutedExcpPkts [99000],
                        inMcNonRoutedNonExcpPkts [99000],
                        inUcTrappedMirrorPkts [999900],
                        inMcTrappedMirrorPkts [999900],
                        mcRfpFailPkts [99000],
                        outUcRoutedPkts [990099]}.
                Expected: GT_OK.
            */
            cntSet = CPSS_IP_CNT_SET3_E;
            counters.inUcPkts  = 990;
            counters.inMcPkts  = 990;
            counters.inUcNonRoutedExcpPkts = 9900;
            counters.inUcNonRoutedNonExcpPkts = 9900;
            counters.inMcNonRoutedExcpPkts = 99000;
            counters.inMcNonRoutedNonExcpPkts = 99000;
            counters.inUcTrappedMirrorPkts = 999900;
            counters.inMcTrappedMirrorPkts = 999900;
            counters.mcRfpFailPkts  = 99000;
            counters.outUcRoutedPkts = 990099;

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, cntSet);

            /*
               1.1.5. Call cpssDxChIpPortGroupCntGet with the same cntSet.
               Expected: GT_OK and the same counters.
            */
            cpssOsBzero((GT_VOID*) &countersRet, sizeof(countersRet));

            st = cpssDxChIpPortGroupCntGet(dev, portGroupsBmp, cntSet, &countersRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssDxChIpPortGroupCntGet: %d, %d", dev, cntSet);

            isEqual =
                (0 == cpssOsMemCmp((GT_VOID*)&counters, (GT_VOID*)&countersRet, sizeof (counters)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isEqual,
                      "get another counters than was set: %d, %d", dev, cntSet);

            /*
                1.1.6. Call with wrong enum values cntSet  and other valid parameters from 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChIpPortGroupCntSet
                                (dev, portGroupsBmp, cntSet, &counters),
                                cntSet);

            /*
                1.1.7. Call with counters [NULL] and other valid parameters from 1.1.
                Expected: GT_BAD_PTR.
            */

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, counters = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    cntSet = CPSS_IP_CNT_SET3_E;
    counters.inUcPkts  = 1;
    counters.inMcPkts  = 1;
    counters.inUcNonRoutedExcpPkts = 0;
    counters.inUcNonRoutedNonExcpPkts = 0;
    counters.inMcNonRoutedExcpPkts = 0;
    counters.inMcNonRoutedNonExcpPkts = 0;
    counters.inUcTrappedMirrorPkts = 1;
    counters.inMcTrappedMirrorPkts = 1;
    counters.mcRfpFailPkts  = 0;
    counters.outUcRoutedPkts = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupCntSet(dev, portGroupsBmp, cntSet, &counters);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupDropCntGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_GROUPS_BMP   portGroupsBmp,
    OUT GT_U32              *dropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupDropCntGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with non-null dropPktsPtr.
    Expected: GT_OK.
    1.1.2. Call with dropPktsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;

    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  dropPkts;
    GT_U32                  portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with non-null dropPktsPtr.
                Expected: GT_OK.
            */
            st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, &dropPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call with dropPktsPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropPktsPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, &dropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, &dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, &dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupDropCntGet(dev, portGroupsBmp, &dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMllCntSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_GROUPS_BMP       portGroupsBmp,
    IN GT_U32                   mllCntSet,
    IN GT_U32                   mllOutMCPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMllCntSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with mllCntSet [0 / 1]and mllOutMCPkts [0 / 10].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortGroupMllCntGet with the same mllCntSet.
    Expected: GT_OK and the same mllOutMCPkts.
    1.1.3. Check out of range mllCntSet.  Call with mllCntSet [10].
    Expected: NOT GT_OK.
    1.1.4. Call with mllCntSet [1] and mllOutMCPkts [0xFFFFFFFF] (no any constraints).
    Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;

    GT_U32   mllCntSet    = 0;
    GT_U32   mllOutMCPkts = 0;
    GT_U32   mllOutMCPktsRet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with mllCntSet [0 / 1]and mllOutMCPkts [0 / 10].
                Expected: GT_OK.
            */
            /*call with mllCntSet = 0 and  mllOutMCPkts = 0; */
            mllCntSet = 0;
            mllOutMCPkts = 0;

            st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllCntSet, mllOutMCPkts);

            /*
                1.1.2. Call cpssDxChIpPortGroupMllCntGet with the same mllCntSet.
                Expected: GT_OK and the same mllOutMCPkts.
            */
            st = cpssDxChIpPortGroupMllCntGet(dev, portGroupsBmp, mllCntSet, &mllOutMCPktsRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpPortGroupMllCntGet: %d, %d",
                                         dev, mllCntSet);

            UTF_VERIFY_EQUAL2_STRING_MAC(mllOutMCPkts, mllOutMCPktsRet,
                "get another mllOutMCPkts than was set: %d, %d", dev, mllCntSet);

            /*call with mllCntSet = 1 and  mllOutMCPkts = 10; */
            mllCntSet = 1;
            mllOutMCPkts = 10;

            st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllCntSet, mllOutMCPkts);

            /*
                1.1.2. Call cpssDxChIpPortGroupMllCntGet with the same mllCntSet.
                Expected: GT_OK and the same mllOutMCPkts.
            */
            st = cpssDxChIpPortGroupMllCntGet(dev, portGroupsBmp, mllCntSet, &mllOutMCPktsRet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChIpPortGroupMllCntGet: %d, %d",
                                         dev, mllCntSet);

            UTF_VERIFY_EQUAL2_STRING_MAC(mllOutMCPkts, mllOutMCPktsRet,
                "get another mllOutMCPkts than was set: %d, %d", dev, mllCntSet);

            /*
                1.1.3. Check out of range mllCntSet.  Call with mllCntSet [10].
                Expected: NOT GT_OK.
            */
            mllCntSet = 10;
            mllOutMCPkts = 10;

            st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, mllCntSet);

            /*
                1.1.4. Call with mllCntSet [1] and mllOutMCPkts [0xFFFFFFFF] (no any constraints).
                Expected: GT_OK.
            */
            mllCntSet = 1;
            mllOutMCPkts = 0xFFFFFFFF;

            st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, mllCntSet, mllOutMCPkts);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;
    mllCntSet = 1;
    mllOutMCPkts = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupMllCntSet(dev, portGroupsBmp, mllCntSet, mllOutMCPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet
(
    IN GT_U8               devNum,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32              dropPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with dropPkts [10 / 1000 /1000000].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet.
    Expected: GT_OK and the same dropPkts.
    1.1.3. Call with dropPkts [0xFFFFFFFF] (no any onstraints).
    Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  dropPkts = 0;
    GT_U32                  dropPktsRet;
    GT_U32                  portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
               1.1.1. Call with dropPkts [10 / 1000 /1000000].
               Expected: GT_OK.
            */
            /*call with dropPkts = 10*/
            dropPkts = 10;
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                    portGroupsBmp, dropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);

            /*
               1.1.2. Call cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet.
               Expected: GT_OK and the same dropPkts.
            */
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet(dev,
                                                    portGroupsBmp, &dropPktsRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(dropPkts, dropPktsRet,
                "cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet: get another value than was set: dev = %d, dropPkts = %d",
                                         dev, dropPktsRet);

            /*call with dropPkts = 1000*/
            dropPkts = 1000;
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                    portGroupsBmp, dropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);

            /*
               1.1.2. Call cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet.
               Expected: GT_OK and the same dropPkts.
            */
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet(dev,
                                                    portGroupsBmp, &dropPktsRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(dropPkts, dropPktsRet,
                "cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet: get another value than was set: dev = %d, dropPkts = %d",
                                         dev, dropPktsRet);

            /*call with dropPkts = 1000000*/
            dropPkts = 1000000;
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                    portGroupsBmp, dropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);

            /*
               1.1.2. Call cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet.
               Expected: GT_OK and the same dropPkts.
            */
            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet(dev,
                                                    portGroupsBmp, &dropPktsRet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(dropPkts, dropPktsRet,
                "cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet: get another value than was set: dev = %d, dropPkts = %d",
                                         dev, dropPktsRet);

            /*
               1.1.3. Call with dropPkts [0xFFFFFFFF] (no any onstraints).
               Expected: GT_OK.
            */
            dropPkts = 0xFFFFFFFF;

            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                portGroupsBmp, dropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dropPkts);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                portGroupsBmp, dropPkts);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev, portGroupsBmp, dropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;
    dropPkts = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                    portGroupsBmp, dropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet(dev,
                                                    portGroupsBmp, dropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRpfModeSet
(
    IN  GT_U8                        devNum,
    IN  GT_U16                       vid,
    IN  CPSS_DXCH_IP_URPF_MODE_ENT   uRpfMode
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRpfModeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with vid [0 / 100 / 0xFFF],
                   uRpfMode [CPSS_DXCH_IP_URPF_DISABLE_MODE_E   /
                             CPSS_DXCH_IP_URPF_VLAN_MODE_E      /
                             CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E/
                             CPSS_DXCH_IP_URPF_LOOSE_MODE_E     ].
    Expected: GT_OK.
    1.2. Call cpssDxChIpUcRpfModeGet with the same vid.
    Expected: GT_OK and the same uRpfMode.
    1.3. Call with out of range vid[PRV_CPSS_MAX_NUM_VLANS_CNS]
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum values uRpfMode and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16                       vid;
    CPSS_DXCH_IP_URPF_MODE_ENT   uRpfMode;
    CPSS_DXCH_IP_URPF_MODE_ENT   uRpfModeGet;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vid [0 / 100 / 0xFFF],
                           uRpfMode [CPSS_DXCH_IP_URPF_DISABLE_MODE_E   /
                             CPSS_DXCH_IP_URPF_VLAN_MODE_E      /
                             CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E/
                             CPSS_DXCH_IP_URPF_LOOSE_MODE_E     ].
            Expected: GT_OK.
        */
        vid = 0;
        uRpfMode = CPSS_DXCH_IP_URPF_DISABLE_MODE_E;

        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vid, uRpfMode);

        /*
            1.2. Call cpssDxChIpUcRpfModeGet with the same vid.
            Expected: GT_OK and the same uRpfMode.
        */
        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(uRpfMode, uRpfModeGet,
                                     "cpssDxChIpUcRpfModeGet: get another value than was set: %d, %d",
                                     dev, uRpfModeGet);

        /*
            1.1. Call with vid [0 / 100 / 0xFFF],
                           uRpfMode [CPSS_DXCH_IP_URPF_DISABLE_MODE_E   /
                             CPSS_DXCH_IP_URPF_VLAN_MODE_E      /
                             CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E/
                             CPSS_DXCH_IP_URPF_LOOSE_MODE_E     ].
            Expected: GT_OK.
        */

        vid = 100;
        uRpfMode = CPSS_DXCH_IP_URPF_LOOSE_MODE_E;
        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vid, uRpfMode);

        /*
            1.2. Call cpssDxChIpUcRpfModeGet with the same vid.
            Expected: GT_OK and the same uRpfMode.
        */
        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(uRpfMode, uRpfModeGet,
                                     "cpssDxChIpUcRpfModeGet: get another value than was set: %d, %d",
                                     dev, uRpfModeGet);

        /*
            1.1. Call with vid [0 / 100 / 0xFFF],
                           uRpfMode [CPSS_DXCH_IP_URPF_DISABLE_MODE_E   /
                             CPSS_DXCH_IP_URPF_VLAN_MODE_E      /
                             CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E/
                             CPSS_DXCH_IP_URPF_LOOSE_MODE_E     ].
            Expected: GT_OK.
        */
        uRpfMode = CPSS_DXCH_IP_URPF_VLAN_MODE_E;

        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* CPSS_DXCH_IP_URPF_VLAN_MODE_E is not supported for Lion */
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, vid, uRpfMode);
        }
        else
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vid, uRpfMode);
        }

        /*
            1.2. Call cpssDxChIpUcRpfModeGet with the same vid.
            Expected: GT_OK and the same uRpfMode.
        */

        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Verifying values */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
        {
            /* CPSS_DXCH_IP_URPF_VLAN_MODE_E is not supported for Lion,
               so we are expecting to get the previous configured mode */
            uRpfMode = CPSS_DXCH_IP_URPF_LOOSE_MODE_E;
        }
        UTF_VERIFY_EQUAL2_STRING_MAC(uRpfMode, uRpfModeGet,
                                     "cpssDxChIpUcRpfModeGet: get another value than was set: %d, %d",
                                     dev, uRpfModeGet);

        /*
            1.1. Call with vid [0 / 100 / 0xFFF],
                           uRpfMode [CPSS_DXCH_IP_URPF_DISABLE_MODE_E   /
                             CPSS_DXCH_IP_URPF_VLAN_MODE_E      /
                             CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E/
                             CPSS_DXCH_IP_URPF_LOOSE_MODE_E     ].
            Expected: GT_OK.
        */
        vid = 0xFFF;
        uRpfMode = CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E;

        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vid, uRpfMode);

        /*
            1.2. Call cpssDxChIpUcRpfModeGet with the same vid.
            Expected: GT_OK and the same uRpfMode.
        */
        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(uRpfMode, uRpfModeGet,
                                     "cpssDxChIpUcRpfModeGet: get another value than was set: %d, %d",
                                     dev, uRpfModeGet);

        /*
            1.3. Call with out of range vid[PRV_CPSS_MAX_NUM_VLANS_CNS]
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
           vid  = BIT_13;
        }
        else
        {
            vid = PRV_CPSS_MAX_NUM_VLANS_CNS;
        }

        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vid);

        vid = 100;

        /*
            1.4. Call with wrong enum values uRpfMode and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpUcRpfModeSet
                            (dev, vid, uRpfMode),
                            uRpfMode);
    }

    vid = 0;
    uRpfMode = CPSS_DXCH_IP_URPF_DISABLE_MODE_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRpfModeSet(dev, vid, uRpfMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUcRpfModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_U16                              vid,
    IN  CPSS_DXCH_IP_URPF_MODE_ENT          *uRpfModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRpfModeGet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with vid [0 / 100 / 0xFFF],
                   non NULL uRpfModePtr.
    Expected: GT_OK.
    1.2. Call with out of range vid[PRV_CPSS_MAX_NUM_VLANS_CNS]
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL uRpfModePtr and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U16                       vid;
    CPSS_DXCH_IP_URPF_MODE_ENT   uRpfMode;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vid [0 / 100 / 0xFFF],
                           non NULL uRpfModePtr.
            Expected: GT_OK.
        */

        /* call with vid = 0 */
        vid = 0;

        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* call with vid = 100 */
        vid = 100;

        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /* call with vid = 0xFFF */
        vid = 0xFFF;

        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vid);

        /*
            1.2. Call with out of range vid[PRV_CPSS_MAX_NUM_VLANS_CNS]
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
           vid  = BIT_13;
        }
        else
        {
            vid = PRV_CPSS_MAX_NUM_VLANS_CNS;
        }

        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, vid);

        vid = 100;

        /*
            1.3. Call with NULL uRpfModePtr and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpUcRpfModeGet(dev, vid, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, uRpfModePtr = NULL", dev);
    }

    vid = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRpfModeGet(dev, vid, &uRpfMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUrpfLooseModeTypeSet
(
    IN  GT_U8                              devNum,
    IN  CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT looseModeType
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUrpfLooseModeTypeSet)
{
/*
    ITERATE_DEVICES (Lion and above)
    1.1. Call with looseModeType CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E.
    Expected: GT_OK.
    1.2. Call cpssDxChIpUrpfLooseModeTypeGet.
    Expected: GT_OK and the same looseModeType.
    1.3. Call with looseModeType CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E.
    Expected: GT_OK.
    1.4. Call cpssDxChIpUrpfLooseModeTypeGet.
    Expected: GT_OK and the same looseModeType.
    1.5. Call with wrong enum value looseModeType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT looseModeType;
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT looseModeTypeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with looseModeType CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E.
            Expected: GT_OK.
        */
        looseModeType = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E;

        st = cpssDxChIpUrpfLooseModeTypeSet(dev, looseModeType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, looseModeType);

        /*
            1.2. Call cpssDxChIpUrpfLooseModeTypeGet.
            Expected: GT_OK and the same looseModeType.
        */
        st = cpssDxChIpUrpfLooseModeTypeGet(dev, &looseModeTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(looseModeType, looseModeTypeGet,
                                     "cpssDxChIpUrpfLooseModeTypeGet: get another value than was set: %d, %d",
                                     dev, looseModeTypeGet);

        /*
            1.3. Call with looseModeType CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E.
            Expected: GT_OK.
        */
        looseModeType = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E;

        st = cpssDxChIpUrpfLooseModeTypeSet(dev, looseModeType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, looseModeType);

        /*
            1.4. Call cpssDxChIpUrpfLooseModeTypeGet.
            Expected: GT_OK and the same looseModeType.
        */
        st = cpssDxChIpUrpfLooseModeTypeGet(dev, &looseModeTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(looseModeType, looseModeTypeGet,
                                     "cpssDxChIpUrpfLooseModeTypeGet: get another value than was set: %d, %d",
                                     dev, looseModeTypeGet);

        /*
            1.5. Call with wrong enum value looseModeType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpUrpfLooseModeTypeSet
                            (dev, looseModeType),
                            looseModeType);
    }

    looseModeType = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUrpfLooseModeTypeSet(dev, looseModeType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUrpfLooseModeTypeSet(dev, looseModeType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpUrpfLooseModeTypeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT* looseModeTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpUrpfLooseModeTypeGet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT looseModeType;

    /* 1. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E |
                                    UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUrpfLooseModeTypeGet(dev, &looseModeType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 2. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUrpfLooseModeTypeGet(dev, &looseModeType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_U8                        portNum,
    IN  GT_BOOL                      enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortSipSaEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
    1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortSipSaEnableGet with  non-NULL enablePtr.
    Expected: GT_OK  and the same enable.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(   (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)) ?
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E :
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_TRUE/ GT_FALSE].
                Expected: GT_OK.
            */

            /* call with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChIpPortSipSaEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChIpPortSipSaEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpPortSipSaEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);

            /* call with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssDxChIpPortSipSaEnableGet with  non-NULL enablePtr.
                Expected: GT_OK  and the same enable.
            */
            st = cpssDxChIpPortSipSaEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChIpPortSipSaEnableGet: %d, %d", dev, port);

            /* Verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                "get another enable value than was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    enable = GT_TRUE;
    port   = IP_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortSipSaEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_U8                        portNum,
    IN  GT_BOOL                      *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortSipSaEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Lion and above)
    1.1.1. Call with non-NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        prvUtfNextGenericPortItaratorTypeSet(   (0 == UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev)) ?
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_PHYSICAL_E :
                                                UTF_GENERIC_PORT_ITERATOR_TYPE_VIRTUAL_E);

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL enablePtr.
                Expected: GT_OK.
            */
            st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with enablePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpPortSipSaEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextGenericPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextGenericPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK        */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    port = IP_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortSipSaEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperSet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_BOOL            multiTargetRateShaperEnable,
    IN  GT_U32             windowSize
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMultiTargetRateShaperSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with multiTargetRateShaperEnable [GT_TRUE and GT_FALSE]
                     and windowSize [10 / 0xFFFF].
    Expected: GT_OK.

    1.1.2. Call cpssDxChIpPortGroupMultiTargetRateShaperGet.
    Expected: GT_OK and same multiTargetRateShaperEnable, windowSize.

    1.1.3. Call with multiTargetRateShaperEnable [GT_FALSE]
                     and windowSize [0xFFFF + 1] (not relevant).
    Expected: GT_OK.
    1.1.4. Call with multiTargetRateShaperEnable [GT_TRUE]
                     and out-of-range windowSize [0xFFFF + 1].
    Expected: NOT GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;

    GT_BOOL                 multiTargetRateShaperEnable = GT_FALSE;
    GT_U32                  windowSize = 0;
    GT_BOOL                 multiTargetRateShaperEnableGet = GT_FALSE;
    GT_U32                  windowSizeGet = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*  1.1.1. */
            multiTargetRateShaperEnable = GT_TRUE;

            /* call with windowSize = 10 */
            windowSize = 10;

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                        windowSize);

            /*  1.1.2. */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnableGet, &windowSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                       "get another multiTargetRateShaperEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                       "get another windowSize than was set: %d", dev);

            /*  1.1.1. */
            windowSize = 0xFFFF;

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                        windowSize);

            /*  1.1.2. */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnableGet, &windowSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                       "get another multiTargetRateShaperEnable than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(windowSize, windowSizeGet,
                       "get another windowSize than was set: %d", dev);

            /*  1.1.1.  */
            multiTargetRateShaperEnable = GT_FALSE;

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                        windowSize);

            /*  1.1.2. */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnableGet, &windowSizeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(multiTargetRateShaperEnable, multiTargetRateShaperEnableGet,
                       "get another multiTargetRateShaperEnable than was set: %d", dev);

            /*  1.1.3.  */
            multiTargetRateShaperEnable = GT_FALSE;
            windowSize = 0xFFFF + 1;

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                        windowSize);

            windowSize = 10;

            /*  1.1.4.  */
            multiTargetRateShaperEnable = GT_TRUE;
            windowSize = 0xFFFF + 1;

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev,  multiTargetRateShaperEnable,
                                            windowSize);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        /* set valid input parameters */
        multiTargetRateShaperEnable = GT_TRUE;
        windowSize = 10;

        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                    multiTargetRateShaperEnable, windowSize);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                multiTargetRateShaperEnable, windowSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    multiTargetRateShaperEnable = GT_TRUE;
    windowSize = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                                multiTargetRateShaperEnable, windowSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupMultiTargetRateShaperSet(dev, portGroupsBmp,
                            multiTargetRateShaperEnable, windowSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupMultiTargetRateShaperGet
(
    IN  GT_U8              devNum,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    OUT GT_BOOL            *multiTargetRateShaperEnablePtr,
    OUT GT_U32             *windowSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupMultiTargetRateShaperGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call with not NULL multiTargetRateShaperEnable and windowSize.
    Expected: GT_OK.
    1.1.2. Call with NULL multiTargetRateShaperEnable.
    Expected: GT_BAD_PTR.
    1.1.3. Call with NULL windowSizePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;

    GT_BOOL                 multiTargetRateShaperEnable = GT_FALSE;
    GT_U32                  windowSize = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*  1.1.1.  */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnable, &windowSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*  1.1.2.  */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    NULL, &windowSize);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            /*  1.1.3.  */
            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnable, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        /* set valid input parameters */
        multiTargetRateShaperEnable = GT_TRUE;
        windowSize = 10;

        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                    &multiTargetRateShaperEnable, &windowSize);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                &multiTargetRateShaperEnable, &windowSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    multiTargetRateShaperEnable = GT_TRUE;
    windowSize = 10;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                                &multiTargetRateShaperEnable, &windowSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupMultiTargetRateShaperGet(dev, portGroupsBmp,
                            &multiTargetRateShaperEnable, &windowSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
(
    IN  GT_U8               dev,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_U32              *routerBridgedExceptionPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with not nullportGroupsBmp.
    Expected: GT_OK.
    1.1.2. Call api with wrong routerBridgedExceptionPktsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8               dev;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              routerBridgedExceptionPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with not nullportGroupsBmp.
                Expected: GT_OK.
            */
            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call api with wrong routerBridgedExceptionPktsPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, routerBridgedExceptionPktsPtr = NULL", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet
(
    IN  GT_U8              dev,
    IN  GT_PORT_GROUPS_BMP portGroupsBmp,
    IN  GT_U32             routerBridgedExceptionPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS(DxCh2 and above)
    1.1.1. Call with routerBridgedExceptionPkts[0 / 100 / 555],
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;
    GT_U32    portGroupId;

    GT_U8              dev;
    GT_PORT_GROUPS_BMP portGroupsBmp = 1;
    GT_U32             routerBridgedExceptionPkts = 0;
    GT_U32             routerBridgedExceptionPktsGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with routerBridgedExceptionPkts[0 / 100 / 555],
                Expected: GT_OK.
            */

            /* call with routerBridgedExceptionPkts[0] */
            routerBridgedExceptionPkts = 0;

            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1.2. Call cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPktsGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                         routerBridgedExceptionPktsGet,
                           "got another routerBridgedExceptionPkts then was set: %d", dev);

            /* call with routerBridgedExceptionPkts[100] */
            routerBridgedExceptionPkts = 100;

            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPktsGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                         routerBridgedExceptionPktsGet,
                           "got another routerBridgedExceptionPkts then was set: %d", dev);

            /* call with routerBridgedExceptionPkts[555] */
            routerBridgedExceptionPkts = 555;

            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.1.2. */
            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet(dev,
                                     portGroupsBmp, &routerBridgedExceptionPktsGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                         routerBridgedExceptionPktsGet,
                           "got another routerBridgedExceptionPkts then was set: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* restore correct values */
    routerBridgedExceptionPkts = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet(dev,
                                     portGroupsBmp, routerBridgedExceptionPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntGet
(
    IN  GT_U8   dev,
    OUT GT_U32  *routerBridgedExceptionPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterBridgedPacketsExceptionCntGet)
{
/*
    ITERATE_DEVICES(DxCh2 and above)
    1.1. Call with not null routerBridgedExceptionPktsPtr.
    Expected: GT_OK.
    1.2. Call api with wrong routerBridgedExceptionPktsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8   dev;
    GT_U32  routerBridgedExceptionPkts;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null routerBridgedExceptionPktsPtr.
            Expected: GT_OK.
        */
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev, &routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong routerBridgedExceptionPktsPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, routerBridgedExceptionPktsPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev, &routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev, &routerBridgedExceptionPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterBridgedPacketsExceptionCntSet
(
    IN  GT_U8  dev,
    IN  GT_U32 routerBridgedExceptionPkts
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterBridgedPacketsExceptionCntSet)
{
/*
    ITERATE_DEVICES(DxCh2 and above)
    1.1. Call with routerBridgedExceptionPkts[0 / 100 / 555],
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterBridgedPacketsExceptionCntGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_U32 routerBridgedExceptionPkts = 0;
    GT_U32 routerBridgedExceptionPktsGet = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with routerBridgedExceptionPkts[0 / 100 / 555],
            Expected: GT_OK.
        */

        /* call with routerBridgedExceptionPkts[0] */
        routerBridgedExceptionPkts = 0;

        st = cpssDxChIpRouterBridgedPacketsExceptionCntSet(dev, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChIpRouterBridgedPacketsExceptionCntGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev,
                                &routerBridgedExceptionPktsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpRouterBridgedPacketsExceptionCntGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                     routerBridgedExceptionPktsGet,
                       "got another routerBridgedExceptionPkts then was set: %d", dev);

        /* call with routerBridgedExceptionPkts[100] */
        routerBridgedExceptionPkts = 100;

        st = cpssDxChIpRouterBridgedPacketsExceptionCntSet(dev, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev,
                                &routerBridgedExceptionPktsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpRouterBridgedPacketsExceptionCntGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                     routerBridgedExceptionPktsGet,
                       "got another routerBridgedExceptionPkts then was set: %d", dev);

        /* call with routerBridgedExceptionPkts[555] */
        routerBridgedExceptionPkts = 555;

        st = cpssDxChIpRouterBridgedPacketsExceptionCntSet(dev, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChIpRouterBridgedPacketsExceptionCntGet(dev,
                                &routerBridgedExceptionPktsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChIpRouterBridgedPacketsExceptionCntGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(routerBridgedExceptionPkts,
                                  routerBridgedExceptionPktsGet,
                       "got another routerBridgedExceptionPkts then was set: %d", dev);
    }

    /* restore correct values */
    routerBridgedExceptionPkts = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpRouterBridgedPacketsExceptionCntSet(dev, routerBridgedExceptionPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterBridgedPacketsExceptionCntSet(dev, routerBridgedExceptionPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpHeaderErrorMaskSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpHeaderErrorMaskSet with relevant value
         ipHeaderErrorType[CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT]
         protocolStack[CPSS_IP_PROTOCOL_IPV4_E/
                       CPSS_IP_PROTOCOL_IPV6_E],
         prefixType[CPSS_UNICAST_E/
                    CPSS_MULTICAST_E],
         mask[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpHeaderErrorMaskGet
    Expected: GT_OK and same mask.
    1.3. Call with wrong enum value ipHeaderErrorType.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum value protocolStack.
    Expected: GT_BAD_PARAM.
    1.5. Call with wrong enum value prefixType.
    Expected: GT_BAD_PARAM.
    1.6. Call with not applicable protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
    Expected: NOT GT_OK.
    1.7. Call with not applicable prefixType[CPSS_UNICAST_MULTICAST_E]
    Expected: NOT GT_OK.
    1.8. Call with protocolStack[CPSS_IP_PROTOCOL_IPV6_E] and
              ipHeaderErrorType[CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_IP_HEADER_ERROR_ENT        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
    CPSS_IP_PROTOCOL_STACK_ENT           protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT           prefixType = CPSS_UNICAST_E;
    GT_BOOL                              mask = GT_FALSE;
    GT_BOOL                              maskGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;
        mask = GT_FALSE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpHeaderErrorMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskGet,
                   "get another mask than was set: %d", dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;
        mask = GT_TRUE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpHeaderErrorMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskGet,
                   "get another mask than was set: %d", dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;
        mask = GT_TRUE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpHeaderErrorMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskGet,
                   "get another mask than was set: %d", dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;
        mask = GT_FALSE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &maskGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpHeaderErrorMaskGet: %d", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mask, maskGet,
                   "get another mask than was set: %d", dev);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask),
                            ipHeaderErrorType);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask),
                            protocolStack);

        /*  1.5.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask),
                            prefixType);

        /*  1.6.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        prefixType = CPSS_UNICAST_E;
        mask = GT_FALSE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        /*  1.7.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_MULTICAST_E;
        mask = GT_FALSE;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        prefixType = CPSS_UNICAST_E;

        /*  1.7.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  restore  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;
        mask = GT_FALSE;
    }

    ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    prefixType = CPSS_UNICAST_E;
    mask = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpHeaderErrorMaskSet(dev, ipHeaderErrorType, protocolStack, prefixType, mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpHeaderErrorMaskGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpHeaderErrorMaskSet with relevant value
         ipHeaderErrorType[CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT/
                           CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT]
         protocolStack[CPSS_IP_PROTOCOL_IPV4_E/
                       CPSS_IP_PROTOCOL_IPV6_E],
         prefixType[CPSS_UNICAST_E/
                    CPSS_MULTICAST_E],
         and not NULL maskPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum value ipHeaderErrorType.
    Expected: GT_BAD_PARAM.
    1.3. Call with wrong enum value protocolStack.
    Expected: GT_BAD_PARAM.
    1.4. Call with wrong enum value prefixType.
    Expected: GT_BAD_PARAM.
    1.5. Call with NULL maskPtr
    Expected: GT_BAD_PTR.
    1.6. Call with not applicable protocolStack[CPSS_IP_PROTOCOL_IPV4V6_E]
    Expected: NOT GT_OK.
    1.7. Call with not applicable prefixType[CPSS_UNICAST_MULTICAST_E]
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_IP_HEADER_ERROR_ENT        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
    CPSS_IP_PROTOCOL_STACK_ENT           protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT           prefixType = CPSS_UNICAST_E;
    GT_BOOL                              mask = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_MULTICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.1.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask),
                            ipHeaderErrorType);

        /*  1.3.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask),
                            protocolStack);

        /*  1.4.  */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask),
                            prefixType);

        /*  1.5.  */
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.6.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        prefixType = CPSS_UNICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        /*  1.7.  */
        ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        prefixType = CPSS_UNICAST_MULTICAST_E;

        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        prefixType = CPSS_UNICAST_E;
    }

    ipHeaderErrorType = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    prefixType = CPSS_UNICAST_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpHeaderErrorMaskGet(dev, ipHeaderErrorType, protocolStack, prefixType, &mask);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpExceptionCommandGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT CPSS_PACKET_CMD_ENT             *exceptionCmdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpExceptionCommandGet)
{
/*
    ITERATE_DEVICES (DxCh)
    1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                  CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                  CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                   protocolStack [CPSS_IP_PROTOCOL_IPV4_E /
                                  CPSS_IP_PROTOCOL_IPV6_E /
                                  CPSS_IP_PROTOCOL_IPV4V6_E]
                   and non-NULL exceptionCmdPtr.
    Expected: GT_OK.
    1.2. Call with invalid exceptionType
                                [CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E /
                                 CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E],
                           protocolStack [CPSS_IP_PROTOCOL_IPV4_E]
         (this exceptionType are for IPv6 only) and non-NULL exceptionCmdPtr.
    Expected: non GT_OK.
    1.3. Call with out of range exceptionType and other parameters same as 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E] (not supported)
         and other parameters same as 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range protocolStack and other parameters same as 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with exceptionCmdPtr [NULL] and other parameters same as 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;

    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT  exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
    CPSS_IP_PROTOCOL_STACK_ENT       protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_PACKET_CMD_ENT              exceptionCmd  = CPSS_PACKET_CMD_NONE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with exceptionType
                                 [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E /
                                  CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E /
                                  CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                            protocolStack
                                [CPSS_IP_PROTOCOL_IPV4_E /
                                 CPSS_IP_PROTOCOL_IPV6_E /
                                 CPSS_IP_PROTOCOL_IPV4V6_E]
                            and non-NULL exceptionCmdPtr.
            Expected: GT_OK.
        */

        if (PRV_CPSS_PP_MAC(devNum)->devFamily==CPSS_PP_FAMILY_DXCH_XCAT2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily==CPSS_PP_FAMILY_CHEETAH2_E ||
            PRV_CPSS_PP_MAC(devNum)->devFamily==CPSS_PP_FAMILY_CHEETAH_E)
        {
            exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
            exceptionCmd  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            st = cpssDxChIpExceptionCommandSet(devNum, exceptionType,
                                               protocolStack,exceptionCmd);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType,
                                        protocolStack);
        }
        else
        {
            exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
        }

        /*
           1.1. Call with exceptionType[CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E] for
                     xCat2/Cheetah2 and
                     exceptionType[CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E]
                     for others,
                     protocolStack [CPSS_IP_PROTOCOL_IPV4_E]
           Expected: GT_OK
        */
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType,
                                    protocolStack);

        /*
            1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E],
                      protocolStack [CPSS_IP_PROTOCOL_IPV6_E]
            Expected: GT_OK
        */

        exceptionType = CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E ;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;

        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType,
                                    protocolStack);

        /*
            1.1. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E],
                      protocolStack [CPSS_IP_PROTOCOL_IPV6_E]
            Expected: GT_OK
        */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType,
                                    protocolStack);
        /*
             1.2. Call with exceptionType [CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E],
                            protocolStack [CPSS_IP_PROTOCOL_IPV4_E]
                  (this exceptionType is valid for IPv6 only) and non-NULL
                  exceptionCmdPtr.
             Expected: NOT GT_OK.
        */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, devNum, exceptionType,
                                        protocolStack);

        /*
            1.3. Call with out of range exceptionType and other parameters
                 same as 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpExceptionCommandGet(devNum, exceptionType,
                                                          protocolStack,
                                                          &exceptionCmd),
                                                          exceptionType);

        /*
            1.4. Call with protocolStack [CPSS_IP_PROTOCOL_IPV4V6_E]
                 (not supported) and other parameters same as 1.1.
            Expected: NOT GT_OK.
        */
        exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E ;
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, protocolStack = %d",
                                         devNum, protocolStack);

        /*
            1.5. Call with out of range protocolStack and other parameters
                 same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpExceptionCommandGet(devNum, exceptionType,
                                                          protocolStack,
                                                          &exceptionCmd),
                                                          protocolStack);
        protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

        /*
            1.6. Call with exceptionCmdPtr [NULL] and other parameters
                 same as 1.1
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, exceptionCmdPtr = NULL", devNum);
    }

    /* restore valid parameters */
    exceptionType = CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
    protocolStack = CPSS_IP_PROTOCOL_IPV4_E;

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                           &exceptionCmd);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpExceptionCommandGet(devNum, exceptionType, protocolStack,
                                       &exceptionCmd);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterGlobalMacSaGet
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       routerMacSaIndex,
    OUT GT_ETHERADDR                 *macSaAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterGlobalMacSaGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1  Call with routerMacSaIndex [0 / 127 / 255] and valid
         macSaAddr[non-NULL]
    Expected: GT_OK.
    1.2  Call with out of range routerMacSaIndex [256] and other parameters
        same as 1.1
    Expected: GT_BAD_PARAM.
    1.3  Call a function with out of range macSaAddrPtr[NULL] and other
        parameters same 1.1
    Expected: GT_BAD_PTR
*/
    GT_STATUS       st               = GT_OK;
    GT_U8           devNum           = 0;
    GT_U32          routerMacSaIndex = 0;
    GT_ETHERADDR    macSaAddr        = {{0,0,0,0,0,0}};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with routerMacSaIndex [0] and valid
                 macSaAddr[non-NULL]
            Expected: GT_OK.
        */
        routerMacSaIndex = 0;
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1. Call with routerMacSaIndex [127] and valid
                 macSaAddr[non-NULL]
            Expected: GT_OK.
        */
        routerMacSaIndex = 127;
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1. Call with routerMacSaIndex [255] and valid
                 macSaAddr[non-NULL]
            Expected: GT_OK.
        */
        routerMacSaIndex = 255;
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with out of range routerMacSaIndex[256] and
            macSaAddr the same as 1.1
            Expected: GT_BAD_PARAM.
        */
        routerMacSaIndex = 256;
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, Out of range",
                                     routerMacSaIndex);

        /*
           1.3 Call a function with routerMacSaIndex [0] and invalid
               macSaAddr [NULL]
           Expected: GT_BAD_PTR
        */
        routerMacSaIndex = 0;
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }

    /*
       2. For not-active devices and devices from non-applicable family
          check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* restore valid parameters */
    routerMacSaIndex = 127;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id. */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex, &macSaAddr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterGlobalMacSaSet
(
    IN GT_U8        devNum,
    IN GT_U32       routerMacSaIndex,
    IN GT_ETHERADDR *macSaAddrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterGlobalMacSaSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1. Call with macSaAddr[{0x00, 0x00, 0x00, 0x00, 0x00, 0x00} /
                             {0x00, 0xAB, 0xCD, 0xEF, 0x00, 0x01} /
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}],
                   routerMacSaIndex [0 / 127 / 255].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterGlobalMacSaGet
    Expected: GT_OK and the same macSaAddr
    1.3. Call function with out of range routerMacSaIndex [256] and
         not NULL macSaAddrPtr
    Expected: GT_OK.
    1.4. Call with out of range macSaAddrPtr [NULL], routerMacSaIndex [127]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st               = GT_OK;
    GT_U8           devNum           = 0;
    GT_U32          routerMacSaIndex = 0;
    GT_BOOL         isEqual          = GT_FALSE;

    GT_ETHERADDR    macSaAddr        = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    GT_ETHERADDR    macSaAddrMid     = {{0x00, 0xAB, 0xCD, 0xEF, 0x00, 0x01}};
    GT_ETHERADDR    macSaAddrFF      = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    GT_ETHERADDR    retMacSaAddr     = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call with routerMacSaIndex [0] and
                 macSaAddr [{0,0,0,0,0,0}]
            Expected: GT_OK.
        */
        routerMacSaIndex = 0;
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssDxChIpRouterGlobalMacSaGet
            Expected: GT_OK and the same macSaAddr
        */
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &retMacSaAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterGlobalMacSaGet: %d",
                                     devNum);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&macSaAddr,
                                     (GT_VOID*)&retMacSaAddr,
                                      sizeof(macSaAddr))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mac than was set: %d",
                                     devNum);

        /*
            1.1. Call function with routerMacSaIndex [127] and
                macSaAddr[{0x00, 0xAB, 0xCD, 0xEF, 0x00, 0x01}]
            Expected: GT_OK.
        */
        routerMacSaIndex = 127;
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                            &macSaAddrMid);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssDxChIpRouterGlobalMacSaGet
            Expected: GT_OK and the same macSaAddr
        */
        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &retMacSaAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterGlobalMacSaGet: %d",
                                     devNum);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&macSaAddrMid,
                                     (GT_VOID*)&retMacSaAddr,
                                      sizeof (macSaAddrMid)))
                                      ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                    "get another mac than was set: %d", devNum);

        /*
            1.1. Call function with routerMacSaIndex [255] and
            macSaAddr[{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}]
            Expected: GT_OK.
        */
        routerMacSaIndex = 255;
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                            &macSaAddrFF);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call cpssDxChIpRouterGlobalMacSaGet
            Expected: GT_OK and the same macSaAddr
        */

        st = cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex,
                                            &retMacSaAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterGlobalMacSaGet: %d",
                                     devNum);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*)&macSaAddrFF,
                                     (GT_VOID*)&retMacSaAddr,
                                      sizeof (macSaAddrFF)))
                                      ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                     "get another mac than was set: %d",
                                     devNum);

        /*
            1.3. Call function with out of range routerMacSaIndex [256]
                and the same macSaAddr as in 1.1
            Expected: GT_BAD_PARAM
        */
        routerMacSaIndex = 256;
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.4. Call with macSaAddrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        routerMacSaIndex = 127;
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, NULL", devNum);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                            &macSaAddr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpRouterGlobalMacSaSet(devNum, routerMacSaIndex,
                                        &macSaAddrMid);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterPortGlobalMacSaIndexGet
(
    IN  GT_U8        devNum,
    IN  GT_PORT_NUM  portNum,
    OUT GT_U32       *routerMacSaIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterPortGlobalMacSaIndexGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid routerMacSaIndexPtr [non NULL]
    Expected: GT_OK.
    1.1.2. Call with invalid routerMacSaIndexPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS    st               = GT_OK;
    GT_U8        devNum           = 0;
    GT_PORT_NUM  portNum          = 0;
    GT_U32       routerMacSaIndex = 0;
    GT_U32       notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with valid routerMacSaIndexPtr[non NULL]
                Expected: GT_OK.
            */
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                         &routerMacSaIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.2. Call with invalid routerMacSaIndexPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                        "%d, routerMacSaIndexPtr = NULL",
                                        devNum);
        }

        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                         &routerMacSaIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                     &routerMacSaIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_OK
            for CPU port number.
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                     &routerMacSaIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* restore correct values */
    portNum = 0;

    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                     &routerMacSaIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                 &routerMacSaIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterPortGlobalMacSaIndexSet
(
    IN  GT_U8       devNum,
    IN  GT_PORT_NUM portNum,
    IN  GT_U32      routerMacSaIndex
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterPortGlobalMacSaIndexSet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with routerMacSaIndex [0 / 127 / 255],
    Expected: GT_OK.
    1.1.2. Call cpssDxChIpRouterPortGlobalMacSaIndexGet.
    Expected: GT_OK and the same values that was set.
    1.1.3. Call with out of range routerMacSaIndex [256].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8       devNum              = 0;
    GT_PORT_NUM portNum             = 0;
    GT_U32      routerMacSaIndex    = 0;
    GT_U32      routerMacSaIndexGet = 0;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with routerMacSaIndex [0 / 127 / 255],
                Expected: GT_OK.
            */

            /* 1.1.1. Call with routerMacSaIndex[0] */
            routerMacSaIndex = 0;
            st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                         routerMacSaIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.2. Call cpssDxChIpRouterPortGlobalMacSaIndexGet.
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                         &routerMacSaIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpRouterPortGlobalMacSaIndexGet: %d ",
                                         devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(routerMacSaIndex, routerMacSaIndexGet,
                                         "got another routerMacSaIndex than was set: %d",
                                         devNum);

            /*
                1.1.1. Call with routerMacSaIndex[127]
                Expected: GT_OK.
            */
            routerMacSaIndex = 127;
            st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                         routerMacSaIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.2. Call cpssDxChIpRouterPortGlobalMacSaIndexGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                         &routerMacSaIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpRouterPortGlobalMacSaIndexGet: %d ",
                                         devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(routerMacSaIndex, routerMacSaIndexGet,
                                         "got another routerMacSaIndex than was set: %d",
                                         devNum);

            /*
                1.1.1 Call with routerMacSaIndex[255]
                Expected: GT_OK.
            */
            routerMacSaIndex = 255;
            st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                         routerMacSaIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.2. Call cpssDxChIpRouterPortGlobalMacSaIndexGet
                Expected: GT_OK and the same values that was set.
            */
            st = cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum,
                                                         &routerMacSaIndexGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChIpRouterPortGlobalMacSaIndexGet: %d ",
                                        devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(routerMacSaIndex, routerMacSaIndexGet,
                                         "got another routerMacSaIndex than was set: %d",
                                         devNum);

            /*
                1.1.3. Call with out of range routerMacSaIndex [256].
                Expected: GT_BAD_PARAM.
            */
            routerMacSaIndex = 256;
            st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                         routerMacSaIndex);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
        }

        routerMacSaIndex = 0;
        st = prvUtfNextVirtPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                         routerMacSaIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /*
            1.3. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
        st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                     routerMacSaIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /*
            1.4. For active device check that function returns GT_OK
            for CPU port number
        */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                     routerMacSaIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
    }

    /* restore correct values */
    portNum          = 0;
    routerMacSaIndex = 0;

    /*
       2. For not-active devices and devices from non-applicable family
       check that function returns GT_BAD_PARAM.
    */

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                     routerMacSaIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, portNum,
                                                 routerMacSaIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterSourceIdOverrideEnableGet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with valid enablePtr [non-NULL]
    Expected: GT_OK.
    1.2. Call with invalid enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       devNum = 0;
    GT_BOOL     enable = GT_TRUE;;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(GT_FALSE == PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(devNum))
            SKIP_TEST_MAC

        /*
            1.1. Call with valid enablePtr [non-NULL]
            Expected: GT_OK.
        */
        st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2. Call with invalid enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "%d, enablePtr = NULL", devNum);
    }

    /*
        2. For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpRouterSourceIdOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpRouterSourceIdOverrideEnableSet)
{
/*
    ITERATE_DEVICES (Lion2)
    1.1. Call with valid enable [GT_TRUE/GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpRouterSourceIdOverrideEnableGet with valid
         enablePtr [non-NULL]
    Expected: GT_OK and the same value of enable as was set.
*/
    GT_STATUS   st        = GT_OK;
    GT_U8       devNum    = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(GT_FALSE == PRV_CPSS_DXCH_LION2_B1_AND_ABOVE_CHECK_MAC(devNum))
            SKIP_TEST_MAC

        /*
            1.1. Call with enable [GT_TRUE].
            Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChIpRouterSourceIdOverrideEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChIpRouterSourceIdOverrideEnableGet with valid
                 enablePtr [non-NULL]
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterSourceIdOverrideEnableGet: %d",
                                     devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d",
                                     devNum);

        /*
             1.1 Call with enable [GT_FALSE]
             Expected: GT_OK.
         */
        enable    = GT_FALSE;
        enableGet = GT_TRUE;

        st = cpssDxChIpRouterSourceIdOverrideEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, enable);

        /*
            1.2. Call cpssDxChIpRouterSourceIdOverrideEnableGet with valid
                 enablePtr [non-NULL]
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChIpRouterSourceIdOverrideEnableGet(devNum, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChIpRouterSourceIdOverrideEnableGet: %d",
                                     devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                     "get another enable than was set: %d",
                                     devNum);
    }

    /*
       2. For not-active devices and devices from non-applicable family
       check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChIpRouterSourceIdOverrideEnableSet(devNum, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpRouterSourceIdOverrideEnableSet(devNum, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpTcDpToMultiTargetTcQueueMapSet
(
    IN  GT_U8              devNum,
    IN  GT_U32             tc,
    IN  CPSS_DP_LEVEL_ENT  dp,
    IN  GT_U32             multiTargetTCQueue
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapSet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  multiTargeTCQueue;
    GT_U32                  multiTargeTCQueueGet;
    CPSS_DP_LEVEL_ENT       dp;
    GT_U32                  tc;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with all legal parameters. Expected: GT_OK.*/
        multiTargeTCQueue = 0;
        tc = 4;
        dp = CPSS_DP_GREEN_E;

        st = cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, tc,dp, multiTargeTCQueue);

        /*  1.2.  Get for same input parameters. Expected: GT_OK. */
        /*  and verify multiTargeTCQueue                          */
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueueGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tc, dp);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargeTCQueue, multiTargeTCQueueGet,
            "cpssDxChIpTcDpToMultiTargetTcQueueMapGet: get another multiTargeTCQueueGet than was set: dev = %d",
                                     dev);

        /* 1.3. Call with all legal parameters. Expected: GT_OK.*/
        multiTargeTCQueue = 3;
        tc = 7;
        dp = CPSS_DP_YELLOW_E;

        st = cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, tc,dp, multiTargeTCQueue);

        /*  1.4.  Get for same input parameters. Expected: GT_OK. */
        /*  and verify multiTargeTCQueue                          */
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueueGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tc, dp);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(multiTargeTCQueue, multiTargeTCQueueGet,
            "cpssDxChIpTcDpToMultiTargetTcQueueMapGet: get another multiTargeTCQueueGet than was set: dev = %d",
                                     dev);

        /* 1.5. Call with tc out of range and same input parameters. Expected: GT_BAD_PARAM   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue),tc);

        /* 1.6. Call with dp out of range and same input parameters. Expected: GT_BAD_PARAM   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue),dp);

        /* 1.7. Call with multiTargeTCQueue out of range and same input parameters. Expected: GT_BAD_PARAM   */
        UTF_ENUMS_CHECK_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue),multiTargeTCQueue);
    }

    tc = 0;
    dp = CPSS_DP_GREEN_E;
    multiTargeTCQueue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpTcDpToMultiTargetTcQueueMapSet(dev, tc, dp, multiTargeTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpTcDpToMultiTargetTcQueueMapGet
(
    IN  GT_U8              devNum,
    IN  GT_U32             tc,
    IN  CPSS_DP_LEVEL_ENT  dp,
    OUT  GT_U32            *multiTargetTCQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapGet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  multiTargeTCQueue;
    CPSS_DP_LEVEL_ENT       dp;
    GT_U32                  tc;
    GT_U32      notAppFamilyBmp;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1.  Call with all legal parameters. Expected: GT_OK. */
        tc = 4;
        dp = CPSS_DP_GREEN_E;

        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tc, dp);

        /* 1.2. Call with all legal parameters. Expected: GT_OK.*/
        tc = 7;
        dp = CPSS_DP_YELLOW_E;
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, tc, dp);

        /*
            1.5. Call with null multiTargeTCQueue [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, tc, dp);
    }

    tc = 0;
    dp = CPSS_DP_GREEN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st =  cpssDxChIpTcDpToMultiTargetTcQueueMapGet(dev, tc, dp, &multiTargeTCQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRoutingVid1AssignEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpUcRoutingVid1AssignEnableSet with relevant value
              enable[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpUcRoutingVid1AssignEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpUcRoutingVid1AssignEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpUcRoutingVid1AssignEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRoutingVid1AssignEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpUcRoutingVid1AssignEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpUcRoutingVid1AssignEnableGet with not NULL enablePtr.
    Expected: GT_OK.
    1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpUcRoutingVid1AssignEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpGetDropCntMode
(
    IN  GT_U8                          devNum,
    OUT CPSS_DXCH_IP_DROP_CNT_MODE_ENT *dropCntModePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpGetDropCntMode)
{
    /*
        ITERATE_DEVICES (DxCh2, DxCh3, xCat, Lion, Lion2, Bobcat2, Caelum, Bobcat3)
        1.1. Call cpssDxChIpGetDropCntMode with valid dropCntModePtr [NON-NULL].
        Expected: GT_OK.
        1.2. Call with invalid dropCntModePtr[NULL].
        Expected: GT_BAD_PTR.
    */

    GT_STATUS                       st = GT_OK;
    GT_U8                           dev = 0;
    CPSS_DXCH_IP_DROP_CNT_MODE_ENT  dropCntMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E |
                                           UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChIpGetDropCntMode with
                valid dropCntModePtr [NON-NULL].
            Expected: GT_OK.
        */
        st = cpssDxChIpGetDropCntMode(dev, &dropCntMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with invalid dropCntModePtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChIpGetDropCntMode(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E |
                                           UTF_CH1_DIAMOND_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpGetDropCntMode(dev, &dropCntMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpGetDropCntMode(dev, &dropCntMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllPortGroupSilentDropCntGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid silentDropPktsPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid silentDropPktsPtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                  = GT_OK;
    GT_STATUS                   res                 = GT_OK;
    GT_U8                       dev                 = 0;
    GT_U32                      notAppFamilyBmp     = 0;
    GT_U32                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp       = 1;
    GT_U32                      silentDropPkts      = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with valid silentDropPktsPtr[NON-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                        &silentDropPkts);

            res = GT_OK;
            UTF_VERIFY_EQUAL1_PARAM_MAC(res, st, dev);

            /*
                1.1.2. Call with invalid silentDropPktsPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                        NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                        &silentDropPkts);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                    &silentDropPkts);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                    &silentDropPkts);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpMllPortGroupSilentDropCntGet(dev, portGroupsBmp,
                                                &silentDropPkts);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpMllPortGroupSkippedEntriesCountersGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_GROUPS_BMP       portGroupsBmp,
    OUT GT_U32                   *skipCounterPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChIpMllPortGroupSkippedEntriesCountersGet)
{
/*
    ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with valid skipCounterPtr[NON-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid skipCounterPtr[NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                   st                  = GT_OK;
    GT_STATUS                   res                 = GT_OK;
    GT_U8                       dev                 = 0;
    GT_U32                      notAppFamilyBmp     = 0;
    GT_U32                      portGroupId         = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp       = 1;
    GT_U32                      skipCounter         = 0;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* ITERATE_DEVICES (Bobcat2, Caelum, Bobcat3) */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with valid silentDropPktsPtr[NON-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                              portGroupsBmp, &skipCounter);
            res = GT_OK;
            UTF_VERIFY_EQUAL1_PARAM_MAC(res, st, dev);

            /*
                1.1.2. Call with invalid silentDropPktsPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                              portGroupsBmp, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                              portGroupsBmp, &skipCounter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                                             portGroupsBmp,
                                                             &skipCounter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                                             portGroupsBmp,
                                                             &skipCounter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChIpMllPortGroupSkippedEntriesCountersGet(dev,
                                                         portGroupsBmp,
                                                         &skipCounter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperBaselineSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperBaselineSet with relevant value
              baseline [0/1000/0xFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperBaselineGet.
    Expected: GT_OK and the same baseline.
    1.3. Call cpssDxChIpMllMultiTargetShaperBaselineSet with out of range value
              baseline [0xFFFFFF+1].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     baseline = 0;
    GT_U32     baselineGet = 0;
    GT_U32     defaultBaselineGet = 0;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with baseline [0 / 1000 / 0xFFFFFF]
            Expected: GT_OK.
        */

        /* keep hw baseline for restore */
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &defaultBaselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperBaselineGet: %d", dev);

        baseline = 1000;

        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*   1.2. Call cpssDxChIpMllMultiTargetShaperBaselineGet.
             Expected: GT_OK and the same baseline.  */

        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperBaselineGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "get another baseline than was set: %d", dev);

        /*  1.1.  */
        baseline = 0;

        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperBaselineGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "get another baseline than was set: %d", dev);

        /*  1.1.  */
        baseline = 0xFFFFFF;

        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperBaselineGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(baseline, baselineGet,
                   "get another baseline than was set: %d", dev);

        /* 1.3. Call cpssDxChIpMllMultiTargetShaperBaselineSet with out of range value
              baseline [0xFFFFFF+1].
           Expected: GT_OUT_OF_RANGE. */

        baseline = 0xFFFFFF+1;

        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* restore default baseline */
        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, defaultBaselineGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperBaselineSet: %d", dev);


    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    baseline = 1000;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperBaselineSet(dev, baseline);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperBaselineGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperBaselineGet with not NULL baselinePtr.
    Expected: GT_OK.
    1.3. Call with NULL baselinePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     baseline = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baseline);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperBaselineGet(dev, &baseline);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperMtuSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperMtuSet with relevant value
              mtu [0/0x800/0xFFFFFF].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperMtuGet.
    Expected: GT_OK and the same mtu.
    1.3. Call cpssDxChIpMllMultiTargetShaperMtuSet with out of range value
              mtu [0xFFFFFF+1].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     mtu = 0;
    GT_U32     mtuGet = 0;
    GT_U32     defaultMtu = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* keep hw mtu for restore */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &defaultMtu);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperMtuGet: %d", dev);

        /*  1.1.  */
        mtu = 0x800;

        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtuGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperMtuGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /*  1.1.  */
        mtu = 0;

        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtuGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperMtuGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /*  1.1.  */
        mtu = 0xFFFFFF;

        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtuGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperMtuGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(mtu, mtuGet,
                   "get another mtu than was set: %d", dev);

        /*  1.3.  */
        mtu = 0xFFFFFF+1;

        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);


        /* restore default mtu */
        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, defaultMtu);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperMtuSet: %d", dev);


    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    mtu = 0x800;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperMtuSet(dev, mtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperMtuGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperMtuGet with not NULL mtuPtr.
    Expected: GT_OK.
    1.3. Call with NULL mtuPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     mtu = 0;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtu);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperMtuGet(dev, &mtu);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperTokenBucketModeSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperTokenBucketModeSet with relevant value
              tokenBucketMode [CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E,
                               CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperTokenBucketModeGet.
    Expected: GT_OK and the same tokenBucketMode.
    1.3. Call cpssDxChIpMllMultiTargetShaperTokenBucketModeGet with out of range value
              tokenBucketMode.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E;
    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketModeGet = CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E;
    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            defaultTokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* keep hw defaultTokenBucketMode for restore */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &defaultTokenBucketMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperTokenBucketModeSet: %d", dev);

        /*  1.1.  */
        tokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E;

        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &tokenBucketModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperTokenBucketModeGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(tokenBucketMode, tokenBucketModeGet,
                   "get another tokenBucketMode than was set: %d", dev);

        /*  1.1.  */
        tokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E;

        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &tokenBucketModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperTokenBucketModeGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(tokenBucketMode, tokenBucketModeGet,
                   "get another mtu than was set: %d", dev);

        /*  1.3.  */
        tokenBucketMode = 2;

        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);


        /* restore default mtu */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, defaultTokenBucketMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperTokenBucketModeSet: %d", dev);


    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    tokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_BYTE_MODE_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(dev, tokenBucketMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperTokenBucketModeGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperTokenBucketModeGet with not NULL tokenBucketModePtr.
    Expected: GT_OK.
    1.3. Call with NULL tokenBucketModePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PORT_TX_DROP_SHAPER_MODE_ENT   tokenBucketMode = CPSS_PORT_TX_DROP_SHAPER_PACKET_MODE_E;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &tokenBucketMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(dev, &tokenBucketMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperEnableSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperEnableSet with relevant value
              enable[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        enable = GT_FALSE;

        st = cpssDxChIpMllMultiTargetShaperEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        /*  1.1.  */
        enable = GT_TRUE;

        st = cpssDxChIpMllMultiTargetShaperEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpMllMultiTargetShaperEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperEnableGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpUcRoutingVid1AssignEnableGet with not NULL enablePtr.
    Expected: GT_OK.
    1.3. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperConfigurationSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with relevant value
              maxBucketSize[1 / 0xFFF]
              non-null maxRatePtr.
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperConfigurationGet.
    Expected: GT_OK and the same maxBucketSize
    1.3. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with out of range value
              maxBucketSize[0x2000]
    Expected: GT_OUT_OF_RANGE.
    1.4. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with
              maxRatePtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                                       maxBucketSize=1;
    GT_U32                                       maxBucketSizeGet=2;
    GT_U32                                       maxRate      = 0xFFFF;
    GT_U32                                       maxRateGet   = 0xFFF1;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with relevant value
              maxBucketSize[1 / 0xFFF]
              non-null maxRatePtr.
        Expected: GT_OK. */

        maxBucketSize= 1;
        maxRate = 65;

        st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, &maxRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2. Call cpssDxChIpMllMultiTargetShaperConfigurationGet.
        Expected: GT_OK and the same maxBucketSize */

        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, &maxRateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(maxBucketSize, maxBucketSizeGet,
                             "get another maxBucketSize than was set:maxBucketSize[%d],maxBucketSizeGet[%d]", maxBucketSize, maxBucketSizeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(maxRate, maxRateGet,
                             "get another maxRate than was set:maxRate[%d],maxRateGet[%d]", maxRate, maxRateGet);

        /*1.1. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with relevant value
              maxBucketSize[0 / 0xFFF]
              non-null maxRatePtr.
        Expected: GT_OK. */

        maxBucketSize= 0xFFF;
        maxRate = 32;

        st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, &maxRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2. Call cpssDxChIpMllMultiTargetShaperConfigurationGet.
        Expected: GT_OK and the same maxBucketSize */

        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, &maxRateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(maxBucketSize, maxBucketSizeGet,
                             "get another maxBucketSize than was set:maxBucketSize[%d],maxBucketSizeGet[%d]", maxBucketSize, maxBucketSizeGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(maxRate, maxRateGet,
                             "get another maxRate than was set:maxRate[%d],maxRateGet[%d]", maxRate, maxRateGet);


        /*1.3. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with out of range value
              maxBucketSize[0x2000]
        Expected: GT_OUT_OF_RANGE.*/

        maxBucketSize= 0x2000;
        st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, &maxRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /*1.4. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with
              maxRatePtr [NULL]
        Expected: GT_BAD_PTR.*/

        st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    maxBucketSize= 0xFFF;
    maxRate = 32;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, &maxRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperConfigurationSet(dev, maxBucketSize, &maxRate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperConfigurationGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperConfigurationSet with not NULL
         maxBucketSizeGet, maxRateGet.
    Expected: GT_OK.
    1.2. Call with NULL maxBucketSizeGet.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL maxRateGet.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                                       maxBucketSizeGet=1;
    GT_U32                                       maxRateGet   = 0xFFFF;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, &maxRateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, NULL, &maxRateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, &maxRateGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperConfigurationGet(dev, &maxBucketSizeGet, &maxRateGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet with relevant value
              tokenBucketIntervalSlowUpdateRatio[1 / 16]
              tokenBucketUpdateInterval[1/15]
              tokenBucketIntervalUpdateRatio[64/1024]
    Expected: GT_OK.
    1.2. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet.
    Expected: GT_OK and the same
              tokenBucketIntervalSlowUpdateRatio[1 / 16]
              tokenBucketUpdateInterval[1/15]
              tokenBucketIntervalUpdateRatio[64/1024]
    1.3. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet with out of range value
              tokenBucketIntervalSlowUpdateRatio[17]
              tokenBucketUpdateInterval[16]
              tokenBucketIntervalUpdateRatio[0xff]
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                                          tokenBucketIntervalSlowUpdateRatio=1;
    GT_U32                                          tokenBucketUpdateInterval=1;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatio=CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
    GT_U32                                          tokenBucketIntervalSlowUpdateRatioGet=2;
    GT_U32                                          tokenBucketUpdateIntervalGet=2;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatioGet=CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
    GT_U32                                          defaultTokenBucketIntervalSlowUpdateRatio;
    GT_U32                                          defaultTokenBucketUpdateInterval;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   defaultTokenBucketIntervalUpdateRatio;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

         st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &defaultTokenBucketIntervalSlowUpdateRatio,
                                                                    &defaultTokenBucketUpdateInterval,
                                                                    &defaultTokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet with relevant value
              tokenBucketIntervalSlowUpdateRatio[1 / 16]
              tokenBucketUpdateInterval[1/15]
              tokenBucketIntervalUpdateRatio[64/1024]
          Expected: GT_OK. */


        tokenBucketIntervalSlowUpdateRatio= 1;
        tokenBucketUpdateInterval = 1;
        tokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet.
            Expected: GT_OK and the same
              tokenBucketIntervalSlowUpdateRatio[1 / 16]
              tokenBucketUpdateInterval[1/15]
              tokenBucketIntervalUpdateRatio[64/1024]
        */

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatioGet,
                                                                    &tokenBucketUpdateIntervalGet,
                                                                    &tokenBucketIntervalUpdateRatioGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketIntervalSlowUpdateRatio, tokenBucketIntervalSlowUpdateRatioGet,
                                     "get another maxBucketSize than was set:tokenBucketIntervalSlowUpdateRatio[%d],tokenBucketIntervalSlowUpdateRatioGet[%d]",
                                      tokenBucketIntervalSlowUpdateRatio, tokenBucketIntervalSlowUpdateRatioGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketUpdateInterval, tokenBucketUpdateIntervalGet,
                                     "get another maxRate than was set:tokenBucketUpdateInterval[%d],tokenBucketUpdateIntervalGet[%d]",
                                     tokenBucketUpdateInterval, tokenBucketUpdateIntervalGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketIntervalUpdateRatio, tokenBucketIntervalUpdateRatioGet,
                                     "get another maxRate than was set:tokenBucketIntervalUpdateRatio[%d],tokenBucketIntervalUpdateRatioGet[%d]",
                                     tokenBucketIntervalUpdateRatio, tokenBucketIntervalUpdateRatioGet);

        /* 1.1. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet with relevant value
          tokenBucketIntervalSlowUpdateRatio[1 / 16]
          tokenBucketUpdateInterval[1/15]
          tokenBucketIntervalUpdateRatio[64/1024]
          Expected: GT_OK. */


        tokenBucketIntervalSlowUpdateRatio= 16;
        tokenBucketUpdateInterval = 15;
        tokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet.
            Expected: GT_OK and the same
              tokenBucketIntervalSlowUpdateRatio[1 / 16]
              tokenBucketUpdateInterval[1/15]
              tokenBucketIntervalUpdateRatio[64/1024]
        */

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatioGet,
                                                                    &tokenBucketUpdateIntervalGet,
                                                                    &tokenBucketIntervalUpdateRatioGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketIntervalSlowUpdateRatio, tokenBucketIntervalSlowUpdateRatioGet,
                                     "get another maxBucketSize than was set:tokenBucketIntervalSlowUpdateRatio[%d],tokenBucketIntervalSlowUpdateRatioGet[%d]",
                                      tokenBucketIntervalSlowUpdateRatio, tokenBucketIntervalSlowUpdateRatioGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketUpdateInterval, tokenBucketUpdateIntervalGet,
                                     "get another maxRate than was set:tokenBucketUpdateInterval[%d],tokenBucketUpdateIntervalGet[%d]",
                                     tokenBucketUpdateInterval, tokenBucketUpdateIntervalGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(tokenBucketIntervalUpdateRatio, tokenBucketIntervalUpdateRatioGet,
                                     "get another maxRate than was set:tokenBucketIntervalUpdateRatio[%d],tokenBucketIntervalUpdateRatioGet[%d]",
                                     tokenBucketIntervalUpdateRatio, tokenBucketIntervalUpdateRatioGet);


        /* 1.3. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet with out of range value
              tokenBucketIntervalSlowUpdateRatio[17]
              tokenBucketUpdateInterval[16]
              tokenBucketIntervalUpdateRatio[0xff]
            Expected: GT_OUT_OF_RANGE. */

        tokenBucketIntervalSlowUpdateRatio= 17;
        tokenBucketUpdateInterval = 15;
        tokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        tokenBucketIntervalSlowUpdateRatio= 16;
        tokenBucketUpdateInterval = 16;
        tokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        tokenBucketIntervalSlowUpdateRatio= 16;
        tokenBucketUpdateInterval = 15;
        tokenBucketIntervalUpdateRatio = 0xff;

        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);



        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    defaultTokenBucketIntervalSlowUpdateRatio,
                                                                    defaultTokenBucketUpdateInterval,
                                                                    defaultTokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    tokenBucketIntervalSlowUpdateRatio= 16;
    tokenBucketUpdateInterval = 15;
    tokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(dev,
                                                                    tokenBucketIntervalSlowUpdateRatio,
                                                                    tokenBucketUpdateInterval,
                                                                    tokenBucketIntervalUpdateRatio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet)
{
/*
    ITERATE_DEVICES
    1.1. Call cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet with not NULL
         tokenBucketIntervalSlowUpdateRatio
         tokenBucketUpdateInterval
         tokenBucketIntervalUpdateRatio
    Expected: GT_OK.
    1.2. Call with NULL tokenBucketIntervalSlowUpdateRatio.
    Expected: GT_BAD_PTR.
    1.3. Call with NULL tokenBucketUpdateInterval.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL tokenBucketIntervalUpdateRatio.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32                                          tokenBucketIntervalSlowUpdateRatio=1;
    GT_U32                                          tokenBucketUpdateInterval=1;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   tokenBucketIntervalUpdateRatio=CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;



    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*  1.1.  */
        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatio,
                                                                    &tokenBucketUpdateInterval,
                                                                    &tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*  1.2.  */
        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    NULL,
                                                                    &tokenBucketUpdateInterval,
                                                                    &tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.3.  */
        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatio,
                                                                    NULL,
                                                                    &tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*  1.4.  */
        st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatio,
                                                                    &tokenBucketUpdateInterval,
                                                                    NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
         st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                    &tokenBucketIntervalSlowUpdateRatio,
                                                                    &tokenBucketUpdateInterval,
                                                                    &tokenBucketIntervalUpdateRatio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

     st = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(dev,
                                                                &tokenBucketIntervalSlowUpdateRatio,
                                                                &tokenBucketUpdateInterval,
                                                                &tokenBucketIntervalUpdateRatio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet)
{
/*
    ITERATE_DEVICES
    Call cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet with relevant value
              enable[GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    Call cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;


    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        enable = GT_FALSE;

        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);

        enable = GT_TRUE;

        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet: %d", dev);
        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                   "get another enable than was set: %d", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet)
{
/*
    ITERATE_DEVICES
    Call cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet with not NULL enablePtr.
    Expected: GT_OK.
    Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;

    GT_U32      notAppFamilyBmp;
    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpVlanMrstBitmapSet)
{
/*
    ITERATE_DEVICES
    Call cpssDxChIpVlanMrstBitmapSet to write MRST bitmap.
    Expected: GT_OK.
    Call cpssDxChIpVlanMrstBitmapGet to check that read value equal to written value.
    Expected: GT_OK.
    Call cpssDxChIpVlanMrstBitmapSet to test vlanId parameter range.
    Expected: GT_OK for vlanId = 0, 3000, 8191.
              GT_BAD_PARAM for vlanId = 8192, 0xFF00.
    Call cpssDxChIpVlanMrstBitmapSet with dev parameter that out of bound value for device id.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U64     mrstBmp;
    GT_U64     mrstBmpRead;
    GT_U16     vlanId = 800;
    GT_U32     notAppFamilyBmp;
    GT_U16     maxVlanIndex;


    mrstBmp.l[0] = 0x12345678;
    mrstBmp.l[1] = 0x99AABBCC;
    mrstBmpRead.l[0] = 0x0;
    mrstBmpRead.l[1] = 0x0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxVlanIndex = (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
        /* write MRST bitmap */
        st = cpssDxChIpVlanMrstBitmapSet(dev, vlanId, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* check that read value equal to written MRST bitmap */
        st = cpssDxChIpVlanMrstBitmapGet(dev, vlanId, &mrstBmpRead);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(mrstBmp.l[0], mrstBmpRead.l[0], dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(mrstBmp.l[1], mrstBmpRead.l[1], dev);
        /* test vlanId parameter */
        st = cpssDxChIpVlanMrstBitmapSet(dev, 0, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssDxChIpVlanMrstBitmapSet(dev, 3000, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        st = cpssDxChIpVlanMrstBitmapSet(dev, maxVlanIndex, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapSet(dev, maxVlanIndex+1, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        st = cpssDxChIpVlanMrstBitmapSet(dev, 0xFF00, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpVlanMrstBitmapSet(dev, vlanId, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpVlanMrstBitmapSet(dev, vlanId, &mrstBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpVlanMrstBitmapGet)
{
/*
    ITERATE_DEVICES
    Call cpssDxChIpVlanMrstBitmapGet to test vlanId parameter range.
    Expected: GT_OK for vlanId = 0, 3000, 8191.
              GT_BAD_PARAM for vlanId = 8192, 0xFF00.
    Call cpssDxChIpVlanMrstBitmapGet with NULL MRST bitmap.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS  st = GT_OK;
    GT_U8      dev;
    GT_U64     mrstBmp;
    GT_U32     notAppFamilyBmp;
    GT_U16     maxVlanIndex;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        maxVlanIndex = (GT_U16)PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapGet(dev, 0, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapGet(dev, 3000, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapGet(dev, maxVlanIndex, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapGet(dev, maxVlanIndex+1, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        /* pass vlanId that exceeds max range */
        st = cpssDxChIpVlanMrstBitmapGet(dev, 0xFF00, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        /* pass NULL MRST bitmap */
        st = cpssDxChIpVlanMrstBitmapGet(dev, 3000, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpVlanMrstBitmapGet(dev, 3000, &mrstBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpVlanMrstBitmapGet(dev, 3000, &mrstBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpPortFcoeForwardingEnableSet)
{
/*
    1.1. ITERATE_DEVICES_VIRT_PORT
    1.2.1. Call cpssDxChIpPortFcoeForwardingEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpPortFcoeForwardingEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpPortFcoeForwardingEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpPortFcoeForwardingEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpPortFcoeForwardingEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_PORT_NUM port = IP_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2.2 */
            st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, enable, enable, port);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.2.4 */
            st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FALSE, enable, enable, port);

            /* 1.2.5 */
            st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2.6. For all active devices go over all non available ports. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.7. Call function for each non-active port */
            st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

            st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /*
            1.2.8. For active device check that function returns GT_BAD_PARAM
            for out of bound value for port number.
        */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);
        st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /*
            1.2.9. For active device check that function returns GT_OK
            for CPU port number
        */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpPortFcoeForwardingEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpPortFcoeForwardingEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeExceptionPacketCommandSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over commands.
    1.1.1 Call cpssDxChIpFcoeExceptionPacketCommandSet.
    Expected: GT_OK.
    1.1.2 Call cpssDxChIpFcoeExceptionPacketCommandGet.
    Expected: GT_OK.
    1.1.3 Compare command we get with command we set.
    Expected: GT_OK.
    1.2. Call cpssDxChIpFcoeExceptionPacketCommandSet with wrong command.
    Expected: GT_BAD_PARAM.
    1.3 Call cpssDxChIpFcoeExceptionPacketCommandGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  notAppFamilyBmp;
    CPSS_PACKET_CMD_ENT     command=0, temp_command, saved_command;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFcoeExceptionPacketCommandGet(dev, &saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(command = 0; command <= CPSS_PACKET_CMD_DROP_SOFT_E; command++)
        {
            /*  1.1.1  */
            st = cpssDxChIpFcoeExceptionPacketCommandSet(dev, command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, command);

            /*  1.1.2  */
            st = cpssDxChIpFcoeExceptionPacketCommandGet(dev, &temp_command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(command, temp_command, command);
        }
        /* 1.2 */
        st = cpssDxChIpFcoeExceptionPacketCommandSet(dev, command+1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 */
        st = cpssDxChIpFcoeExceptionPacketCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChIpFcoeExceptionPacketCommandSet(dev, saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeExceptionPacketCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeExceptionPacketCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeExceptionPacketCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeExceptionPacketCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeExceptionCpuCodeSet)
{
/*
    ITERATE_DEVICES
    1.1 Call cpssDxChIpFcoeExceptionCpuCodeSet with acceptable code.
    Expected: GT_OK.
    1.2 Call cpssDxChIpFcoeExceptionCpuCodeSet.
    Expected: GT_OK.
    1.3 Compare code we get with code we set.
    Expected: GT_OK.
    1.4 Call cpssDxChIpFcoeExceptionCpuCodeSet with wrong code.
    Expected: GT_BAD_PARAM.
    1.5 Call cpssDxChIpFcoeExceptionCpuCodeSet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      notAppFamilyBmp;
    CPSS_NET_RX_CPU_CODE_ENT    code, temp_code, saved_code;

    /* to be initialized */
    code = CPSS_NET_CONTROL_E;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFcoeExceptionCpuCodeGet(dev, &saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1  */
        st = cpssDxChIpFcoeExceptionCpuCodeSet(dev, CPSS_NET_FCOE_SIP_NOT_FOUND_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, code);

        /* 1.2  */
        st = cpssDxChIpFcoeExceptionCpuCodeGet(dev, &temp_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.3  */
        UTF_VERIFY_EQUAL1_PARAM_MAC(CPSS_NET_FCOE_SIP_NOT_FOUND_E, temp_code, code);

        /* 1.4 */
        st = cpssDxChIpFcoeExceptionCpuCodeSet(dev, CPSS_NET_ALL_CPU_OPCODES_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5 */
        st = cpssDxChIpFcoeExceptionCpuCodeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChIpFcoeExceptionCpuCodeSet(dev, saved_code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeExceptionCpuCodeSet(dev, code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeExceptionCpuCodeGet(dev, &code);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeExceptionCpuCodeSet(dev, code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeExceptionCpuCodeGet(dev, &code);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeSoftDropRouterEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeSoftDropRouterEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeSoftDropRouterEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeSoftDropRouterEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeSoftDropRouterEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeSoftDropRouterEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeSoftDropRouterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeSoftDropRouterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeSoftDropRouterEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeSoftDropRouterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeSoftDropRouterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeSoftDropRouterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeTrapRouterEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeTrapRouterEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeTrapRouterEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeTrapRouterEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeTrapRouterEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeTrapRouterEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeTrapRouterEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeTrapRouterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeTrapRouterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeTrapRouterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeTrapRouterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeTrapRouterEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeTrapRouterEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeTrapRouterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeTrapRouterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeTrapRouterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeTrapRouterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedUrpfCheckEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeBridgedUrpfCheckEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeBridgedUrpfCheckEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeBridgedUrpfCheckEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeBridgedUrpfCheckEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeBridgedUrpfCheckEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedUrpfCheckEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedUrpfCheckEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedUrpfCheckCommandSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over commands.
    1.1.1 Call cpssDxChIpFcoeBridgedUrpfCheckCommandSet.
    Expected: GT_OK.
    1.1.2 Call cpssDxChIpFcoeBridgedUrpfCheckCommandGet.
    Expected: GT_OK.
    1.1.3 Compare command we get with command we set.
    Expected: GT_OK.
    1.2. Call cpssDxChIpFcoeBridgedUrpfCheckCommandSet with wrong command.
    Expected: GT_BAD_PARAM.
    1.3 Call cpssDxChIpFcoeBridgedUrpfCheckCommandGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  notAppFamilyBmp;
    CPSS_PACKET_CMD_ENT     command=0, temp_command, saved_command;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(dev, &saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(command = 0; command <= CPSS_PACKET_CMD_DROP_SOFT_E; command++)
        {
            /*  1.1.1  */
            st = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(dev, command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, command);

            /*  1.1.2  */
            st = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(dev, &temp_command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(command, temp_command, command);
        }
        /* 1.2 */
        st = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(dev, command+1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 */
        st = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(dev, saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedUrpfCheckCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedUrpfCheckCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

UTF_TEST_CASE_MAC(cpssDxChIpFcoeUcRpfAccessLevelSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over levels.
    1.1.1 Call cpssDxChIpFcoeUcRpfAccessLevelSet.
    Expected: GT_OK.
    1.1.2 Call cpssDxChIpFcoeUcRpfAccessLevelGet.
    Expected: GT_OK.
    1.1.3 Compare level we get with level we set.
    Expected: GT_OK.
    1.2. Call cpssDxChIpFcoeUcRpfAccessLevelSet with wrong level.
    Expected: GT_BAD_PARAM.
    1.3 Call cpssDxChIpFcoeUcRpfAccessLevelGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_U32      level=0, temp_level, saved_level;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFcoeUcRpfAccessLevelGet(dev, &saved_level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(level = 0; level <= BIT_5; level++)
        {
            /*  1.1.1  */
            st = cpssDxChIpFcoeUcRpfAccessLevelSet(dev, level);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, level);

            /*  1.1.2  */
            st = cpssDxChIpFcoeUcRpfAccessLevelGet(dev, &temp_level);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(level, temp_level, level);
        }
        /* 1.2 */
        st = cpssDxChIpFcoeUcRpfAccessLevelSet(dev, BIT_6);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 */
        st = cpssDxChIpFcoeUcRpfAccessLevelGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChIpFcoeUcRpfAccessLevelSet(dev, saved_level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeUcRpfAccessLevelSet(dev, level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeUcRpfAccessLevelGet(dev, &level);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeUcRpfAccessLevelSet(dev, level);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeUcRpfAccessLevelGet(dev, &level);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedSidSaMismatchCommandSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over commands.
    1.1.1 Call cpssDxChIpFcoeBridgedSidSaMismatchCommandSet.
    Expected: GT_OK.
    1.1.2 Call cpssDxChIpFcoeBridgedSidSaMismatchCommandGet.
    Expected: GT_OK.
    1.1.3 Compare command we get with command we set.
    Expected: GT_OK.
    1.2. Call cpssDxChIpFcoeBridgedSidSaMismatchCommandSet with wrong command.
    Expected: GT_BAD_PARAM.
    1.3 Call cpssDxChIpFcoeBridgedSidSaMismatchCommandGet with NULL counterValue pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  notAppFamilyBmp;
    CPSS_PACKET_CMD_ENT     command=0, temp_command, saved_command;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(dev, &saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(command = 0; command <= CPSS_PACKET_CMD_DROP_SOFT_E; command++)
        {
            /*  1.1.1  */
            st = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(dev, command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, command);

            /*  1.1.2  */
            st = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(dev, &temp_command);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(command, temp_command, command);
        }
        /* 1.2 */
        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(dev, command+1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3 */
        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* restore original state */
        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(dev, saved_command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(dev, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(dev, &command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedSidSaMismatchCommandSet(dev, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedSidSaMismatchCommandGet(dev, &command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedSidFilterEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeBridgedSidFilterEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeBridgedSidFilterEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeBridgedSidFilterEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeBridgedSidFilterEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeBridgedSidFilterEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeBridgedSidFilterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeBridgedSidFilterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeBridgedSidFilterEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedSidFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedSidFilterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedSidFilterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp;
    GT_BOOL     enable=GT_FALSE, saved_enable;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enable, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, &enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enable, enable);

            /* 1.2.5 */
            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFdbRoutePrefixLenSet)
{
/*
    ITERATE_DEVICES
    1.1 Iterate over IP stack protocols.
    1.1.1 Call cpssDxChIpFdbRoutePrefixLenSet.
    Expected: GT_OK.
    1.1.2 Compare prefix length we get with value we set.
    Expected: GT_OK.
    1.2. Call cpssDxChIpFdbRoutePrefixLenSet with wrong IP stack protocol.
    Expected: GT_BAD_PARAM.
    1.3. Call cpssDxChIpFdbRoutePrefixLenSet with wrong prefix length.
    1.3.1. Call cpssDxChIpFdbRoutePrefixLenSet with wrong IPv4 prefix length.
    Expected: GT_BAD_PARAM.
    1.3.2. Call cpssDxChIpFdbRoutePrefixLenSet with wrong IPv4 prefix length.
    Expected: GT_BAD_PARAM.
    1.4 Call cpssDxChIpFdbRoutePrefixLenGet with NULL prefixLen pointer.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  notAppFamilyBmp;
    CPSS_IP_PROTOCOL_STACK_ENT   protocolStack;
    GT_U32                  prefixLen, prefixLenGet;
    GT_U32                  ipv4PrefixLenSaved, ipv6PrefixLenSaved;

	ipv4PrefixLenSaved = 0;
	ipv6PrefixLenSaved = 0;

    /* this feature is on eArch devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save original state */
        st = cpssDxChIpFdbRoutePrefixLenGet(dev, CPSS_IP_PROTOCOL_IPV4_E, &ipv4PrefixLenSaved);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpFdbRoutePrefixLenGet(dev, CPSS_IP_PROTOCOL_IPV6_E, &ipv6PrefixLenSaved);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1 */
        for(protocolStack = CPSS_IP_PROTOCOL_IPV4_E; protocolStack <= CPSS_IP_PROTOCOL_IPV6_E; protocolStack++)
        {
            if (CPSS_IP_PROTOCOL_IPV4_E == protocolStack)
            {
                prefixLen = 24;
            }
            else
            {
                prefixLen = 96;
            }
            /*  1.1.1  */
            st = cpssDxChIpFdbRoutePrefixLenSet(dev, protocolStack, prefixLen);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, prefixLen);

            /*  1.1.2  */
            st = cpssDxChIpFdbRoutePrefixLenGet(dev, protocolStack, &prefixLenGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(prefixLen, prefixLenGet, prefixLen);
        }
        /* 1.2 */
        prefixLen = 24;
        st = cpssDxChIpFdbRoutePrefixLenSet(dev, protocolStack+1, prefixLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.3.1 */
        st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV4_E, 33);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* 1.3.2 */
        st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV4_E, 129);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* Restore original state */
        st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV4_E, ipv4PrefixLenSaved);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV6_E, ipv6PrefixLenSaved);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV4_E, ipv4PrefixLenSaved);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFdbRoutePrefixLenGet(dev, CPSS_IP_PROTOCOL_IPV4_E, &prefixLenGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFdbRoutePrefixLenSet(dev, CPSS_IP_PROTOCOL_IPV4_E, ipv4PrefixLenSaved);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFdbRoutePrefixLenGet(dev, CPSS_IP_PROTOCOL_IPV4_E, &prefixLenGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChIpFdbUnicastRouteForPbrEnableSet)
{
/*
    ITERATE_DEVICES
    1.2.1. Call cpssDxChIpFdbUnicastRouteForPbrEnableSet with enable = GT_TRUE
    Expected: GT_OK
    1.2.2. Call cpssDxChIpFdbUnicastRouteForPbrEnableGet.
    Expected: GT_OK and enable == GT_TRUE
    1.2.3. Call cpssDxChIpFdbUnicastRouteForPbrEnableSet with enable = GT_FALSE
    Expected: GT_OK
    1.2.4. Call cpssDxChIpFdbUnicastRouteForPbrEnableGet.
    Expected: GT_OK and enable == GT_FALSE
    1.2.5. Call cpssDxChIpFdbUnicastRouteForPbrEnableGet with NULL ptr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_BOOL     enable=GT_FALSE, enableGet, saved_enable;

    /* Test applicable for SIP6 devices */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp | UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_ALDRIN2_E | UTF_AC3X_E);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, &saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2.1 */
            enable = GT_TRUE;
            st = cpssDxChIpFdbUnicastRouteForPbrEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.2 */
            st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, enableGet, enable);

            /* 1.2.3 */
            enable = GT_FALSE;
            st = cpssDxChIpFdbUnicastRouteForPbrEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /* 1.2.4 */
            st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, &enableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, enableGet, enable);

            /* 1.2.5 */
            st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIpFdbUnicastRouteForPbrEnableSet(dev, saved_enable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                     */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIpFdbUnicastRouteForPbrEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);

        st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpFdbUnicastRouteForPbrEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    st = cpssDxChIpFdbUnicastRouteForPbrEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChIpCtrl suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIpCtrl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpSpecialRouterTriggerEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpSpecialRouterTriggerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpExceptionCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpExceptionCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRouteAgingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRouteAgingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterSourceIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterSourceIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupRouterSourceIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupRouterSourceIdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpQosProfileToMultiTargetTCQueueMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpQosProfileToMultiTargetTCQueueMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetQueueFullDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetQueueFullDropCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetTCQueueSchedModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetTCQueueSchedModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpBridgeServiceEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpBridgeServiceEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllBridgeEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllBridgeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetRateShaperSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetRateShaperGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetUcSchedModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMultiTargetUcSchedModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpArpBcModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpArpBcModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortRoutingEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortRoutingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpQosProfileToRouteEntryMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpQosProfileToRouteEntryMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRoutingEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRoutingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpCntSetModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpCntSetModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllSkippedEntriesCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpSetMllCntInterface)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllSilentDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpDropCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpSetDropCntMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMtuProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMtuProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6AddrPrefixScopeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6AddrPrefixScopeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6UcScopeCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6UcScopeCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6McScopeCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpv6McScopeCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterMacSaBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterMacSaBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortRouterMacSaLsbModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortRouterMacSaLsbModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterPortMacSaLsbSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterPortMacSaLsbGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterVlanMacSaLsbSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterVlanMacSaLsbGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterMacSaModifyEnable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterMacSaModifyEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpUcRpfCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpEcmpUcRpfCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupDropCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMllCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMllCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRpfModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRpfModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUrpfLooseModeTypeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUrpfLooseModeTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortSipSaEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortSipSaEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMultiTargetRateShaperSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupMultiTargetRateShaperGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterBridgedPacketsExceptionCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterBridgedPacketsExceptionCntSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpHeaderErrorMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpHeaderErrorMaskGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpExceptionCommandGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterGlobalMacSaGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterGlobalMacSaSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterPortGlobalMacSaIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterPortGlobalMacSaIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterSourceIdOverrideEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpRouterSourceIdOverrideEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpTcDpToMultiTargetTcQueueMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRoutingVid1AssignEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpUcRoutingVid1AssignEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpGetDropCntMode)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllPortGroupSilentDropCntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllPortGroupSkippedEntriesCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperBaselineSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperBaselineGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperMtuSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperMtuGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperTokenBucketModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperTokenBucketModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpVlanMrstBitmapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpVlanMrstBitmapGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpPortFcoeForwardingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeExceptionPacketCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeExceptionCpuCodeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeSoftDropRouterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeTrapRouterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedUrpfCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedUrpfCheckCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeUcRpfAccessLevelSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedSidSaMismatchCommandSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedSidFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFdbRoutePrefixLenSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpFdbUnicastRouteForPbrEnableSet)

UTF_SUIT_END_TESTS_MAC(cpssDxChIpCtrl)

