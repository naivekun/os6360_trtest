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
* @file cpssDxChNstUT.c
*
* @brief Unit tests for cpssDxChNst, that provides
* Network Shield Technology facility Cheetah CPSS declarations
*
* @version   26
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
#define NST_VALID_PHY_PORT_CNS  0

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    IN  CPSS_PACKET_CMD_ENT         command
);
*/
UTF_TEST_CASE_MAC(cpssDxChNstBridgeAccessMatrixCmdSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with saAccessLevel[0], daAccessLevel[7] and
command [CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_HARD_E /
CPSS_PACKET_CMD_DROP_SOFT_E]. Expected: GT_OK.
1.2.  Call cpssDxChNstBridgeAccessMatrixCmdGet. Expected: GT_OK
and the same command.
1.3. Call function with out of range saAccessLevel[8],  daAccessLevel[0]
 and and command [CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK.
1.4. Call function with saAccessLevel[0], out of range daAccessLevel[8]
 and and command [CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK.
1.5. Call function with saAccessLevel[0], daAccessLevel[0] and
command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E /
 CPSS_PACKET_CMD_ROUTE_E/  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
 CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /  CPSS_PACKET_CMD_BRIDGE_E /
 CPSS_PACKET_CMD_NONE_E /  CPSS_PACKET_CMD_INVALID_E]. Expected: NON GT_OK.
1.6. Call function with saAccessLevel[0], daAccessLevel[0]
and wrong enum values command.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8                   dev;
    GT_U32                  saAccessLevel;
    GT_U32                  daAccessLevel;
    CPSS_PACKET_CMD_ENT     command;
    CPSS_PACKET_CMD_ENT     retCommand;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with saAccessLevel[0], daAccessLevel[7] and
        command [CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_HARD_E /
        CPSS_PACKET_CMD_DROP_SOFT_E]. Expected: GT_OK. */
        /* 1.2.  Call cpssDxChNstBridgeAccessMatrixCmdGet. Expected: GT_OK
        and the same command.*/

        saAccessLevel = 0;
        daAccessLevel = 7;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, saAccessLevel, daAccessLevel, command);

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
        "cpssDxChNstBridgeAccessMatrixCmdGet: %d, %d, %d", dev,saAccessLevel, daAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
        "get another command than was set: %d, %d, %d", dev, saAccessLevel, daAccessLevel);

        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, saAccessLevel, daAccessLevel, command);

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
        "cpssDxChNstBridgeAccessMatrixCmdGet: %d, %d, %d", dev,saAccessLevel, daAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
        "get another command than was set: %d, %d, %d", dev, saAccessLevel, daAccessLevel);

        command = CPSS_PACKET_CMD_DROP_SOFT_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, saAccessLevel, daAccessLevel, command);

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
        "cpssDxChNstBridgeAccessMatrixCmdGet: %d, %d, %d", dev,saAccessLevel, daAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
        "get another command than was set: %d, %d, %d", dev, saAccessLevel, daAccessLevel);

        /* 1.3. Call function with out of range saAccessLevel[8],  daAccessLevel[0]
        and command [CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK. */
        saAccessLevel = 8;
        daAccessLevel = 0;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", dev, saAccessLevel);

        /* 1.4. Call function with saAccessLevel[0], out of range daAccessLevel[8]
        and command [CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK. */

        saAccessLevel = 0;
        daAccessLevel = 8;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, daAccessLevel=%d", dev, daAccessLevel);

        /* 1.5. Call function with saAccessLevel[0], daAccessLevel[0] and
        command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E /
        CPSS_PACKET_CMD_ROUTE_E/  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /  CPSS_PACKET_CMD_BRIDGE_E /
        CPSS_PACKET_CMD_NONE_E / CPSS_PACKET_CMD_INVALID_E] Expected: NON GT_OK*/

        saAccessLevel = 0;
        daAccessLevel = 0;
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_ROUTE_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_BRIDGE_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_NONE_E;
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, command=%d", dev, command);

        /*
            1.6. Call function with saAccessLevel[0], daAccessLevel[0] and
            wrong enum values command.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNstBridgeAccessMatrixCmdSet
                            (dev, saAccessLevel, daAccessLevel, command),
                            command);
    }

    saAccessLevel = 0;
    daAccessLevel = 7;
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstBridgeAccessMatrixCmdSet(dev, saAccessLevel, daAccessLevel, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstBridgeAccessMatrixCmdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      saAccessLevel,
    IN  GT_U32                      daAccessLevel,
    OUT CPSS_PACKET_CMD_ENT         *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstBridgeAccessMatrixCmdGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with saAccessLevel[7], daAccessLevel[0] and non-null
 commandPtr. Expected: GT_OK.
1.2. Call function with out of range saAccessLevel[8], daAccessLevel[0]
 and non-null commandPtr. Expected: NON GT_OK.
1.3. Call function with saAccessLevel[0], out of range daAccessLevel[8]
 and non-null commandPtr. Expected: NON GT_OK.
1.4. Call function with saAccessLevel[0], daAccessLevel[0] and
commandPtr [NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_U32                  saAccessLevel;
    GT_U32                  daAccessLevel;
    CPSS_PACKET_CMD_ENT     retCommand;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with saAccessLevel[7], daAccessLevel[0] and non-null
        commandPtr. Expected: GT_OK. */
        saAccessLevel = 7;
        daAccessLevel = 0;

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, saAccessLevel, daAccessLevel);

        /* 1.2. Call function with out of range saAccessLevel[8], daAccessLevel[0]
        and non-null commandPtr. Expected: NON GT_OK. */
        saAccessLevel = 8;
        daAccessLevel = 0;

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, %d", dev, saAccessLevel);

        /*1.3. Call function with saAccessLevel[0], out of range daAccessLevel[8]
        and non-null commandPtr. Expected: NON GT_OK. */
        saAccessLevel = 0;
        daAccessLevel = 8;

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, daAccessLevel=%d", dev, daAccessLevel);

        /*1.4. Call function with saAccessLevel[0], daAccessLevel[0] and
        commandPtr [NULL]. Expected: GT_BAD_PTR. */
        saAccessLevel = 0;
        daAccessLevel = 0;

        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, NULL);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL", dev, saAccessLevel, daAccessLevel);
    }

    saAccessLevel = 0;
    daAccessLevel = 7;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstBridgeAccessMatrixCmdGet(dev, saAccessLevel, daAccessLevel, &retCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstDefaultAccessLevelsSet
(
    IN GT_U8                    devNum,
    IN CPSS_NST_AM_PARAM_ENT    paramType,
    IN GT_U32                   accessLevel
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstDefaultAccessLevelsSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E / CPSS_NST_AM_DA_AUTO_LEARNED_E / CPSS_NST_AM_SA_UNKNOWN_E / CPSS_NST_AM_DA_UNKNOWN_E] and accessLevel[0/ 1/ 2/ 7]. Expected: GT_OK.
1.2. Call cpssDxChNstDefaultAccessLevelsGet. Expected: GT_OK and the same accessLevel.
1.3. Call function with wrong enum values paramType and accessLevel[0]. Expected: GT_BAD_PARAM.
1.4. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E] and accessLevel[8] (out of range). Expected: non GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    CPSS_NST_AM_PARAM_ENT   paramType;
    GT_U32                  accessLevel;
    GT_U32                  retAccessLevel;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E
        / CPSS_NST_AM_DA_AUTO_LEARNED_E / CPSS_NST_AM_SA_UNKNOWN_E /
        CPSS_NST_AM_DA_UNKNOWN_E] and accessLevel[0/ 1/ 2/ 7]. Expected: GT_OK.
        1.2. Call cpssDxChNstDefaultAccessLevelsGet. Expected: GT_OK and the same
        accessLevel. */

        paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;
        accessLevel = 0;
        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, paramType, accessLevel);

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
           "cpssDxChNstDefaultAccessLevelsGet: %d, %d", dev, paramType);
        UTF_VERIFY_EQUAL2_STRING_MAC(accessLevel, retAccessLevel,
            "get another accessLevel than was set: %d, %d", dev, paramType);

        paramType = CPSS_NST_AM_DA_AUTO_LEARNED_E;
        accessLevel = 1;
        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, paramType, accessLevel);

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
           "cpssDxChNstDefaultAccessLevelsGet: %d, %d", dev, paramType);
        UTF_VERIFY_EQUAL2_STRING_MAC(accessLevel, retAccessLevel,
            "get another accessLevel than was set: %d, %d", dev, paramType);

        paramType = CPSS_NST_AM_SA_UNKNOWN_E;
        accessLevel = 2;
        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, paramType, accessLevel);

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
           "cpssDxChNstDefaultAccessLevelsGet: %d, %d", dev, paramType);
        UTF_VERIFY_EQUAL2_STRING_MAC(accessLevel, retAccessLevel,
            "get another accessLevel than was set: %d, %d", dev, paramType);

        paramType = CPSS_NST_AM_DA_UNKNOWN_E;
        accessLevel = 7;
        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, paramType, accessLevel);

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
           "cpssDxChNstDefaultAccessLevelsGet: %d, %d", dev, paramType);
        UTF_VERIFY_EQUAL2_STRING_MAC(accessLevel, retAccessLevel,
            "get another accessLevel than was set: %d, %d", dev, paramType);

        /*
            1.3. Call function with wrong enum values paramType and accessLevel[0].
            Expected: GT_BAD_PARAM.
        */
        accessLevel = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChNstDefaultAccessLevelsSet
                            (dev, paramType, accessLevel),
                            paramType);

        /*1.4. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E] and
        accessLevel[8] (out of range). Expected: non GT_OK */
        paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;
        accessLevel = 8;

        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, paramType, accessLevel);
    }

    paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;
    accessLevel = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstDefaultAccessLevelsSet(dev, paramType, accessLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstDefaultAccessLevelsGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_NST_AM_PARAM_ENT    paramType,
    OUT GT_U32                   *accessLevelPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstDefaultAccessLevelsGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E /
 CPSS_NST_AM_DA_AUTO_LEARNED_E / CPSS_NST_AM_SA_UNKNOWN_E /
 CPSS_NST_AM_DA_UNKNOWN_E] and non-null accessLevelPtr. Expected: GT_OK.
1.2. Call function with wrong enum values paramType and non-null
accessLevelPtr. Expected: GT_BAD_PARAM.
1.3. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E] and
accessLevelPtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                    dev;
    CPSS_NST_AM_PARAM_ENT   paramType;
    GT_U32                  retAccessLevel;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E /
         CPSS_NST_AM_DA_AUTO_LEARNED_E / CPSS_NST_AM_SA_UNKNOWN_E /
        CPSS_NST_AM_DA_UNKNOWN_E] and non-null accessLevelPtr. Expected: GT_OK.*/
        paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramType);

        paramType = CPSS_NST_AM_DA_AUTO_LEARNED_E;

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramType);

        paramType = CPSS_NST_AM_SA_UNKNOWN_E;

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramType);

        paramType = CPSS_NST_AM_DA_UNKNOWN_E;

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, paramType);

        /*
            1.2. Call function with wrong enum values paramType
                 and non-null accessLevelPtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNstDefaultAccessLevelsGet
                            (dev, paramType, &retAccessLevel),
                            paramType);

        /* 1.3. Call function with paramType[CPSS_NST_AM_SA_AUTO_LEARNED_E] and
        accessLevelPtr[NULL]. Expected: GT_BAD_PTR. */
        paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;

        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, paramType);
    }

    paramType = CPSS_NST_AM_SA_AUTO_LEARNED_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstDefaultAccessLevelsGet(dev, paramType, &retAccessLevel);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstProtSanityCheckSet
(
    IN GT_U8                 devNum,
    IN CPSS_NST_CHECK_ENT    checkType,
    IN GT_BOOL               enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstProtSanityCheckSet)
{
/*    ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with checkType[CPSS_NST_CHECK_TCP_SYN_DATA_E/
 CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E /  CPSS_NST_CHECK_FRAG_IPV4_ICMP_E /
 CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E / CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E /
 CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E / CPSS_NST_CHECK_SIP_IS_DIP_E /
 CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E ] and enable[GT_TRUE / GT_FALSE].
 Expected: GT_OK.
1.2. Call cpssDxChNstProtSanityCheckGet. Expected: GT_OK and the same enable.
1.3. Call function with wrong enum values checkType and enable[GT_TRUE]
Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8               dev;
    CPSS_NST_CHECK_ENT  checkType;
    GT_BOOL             enable;
    GT_BOOL             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with checkType[CPSS_NST_CHECK_TCP_SYN_DATA_E/
        CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E /  CPSS_NST_CHECK_FRAG_IPV4_ICMP_E /
        CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E / CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E /
        CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E / CPSS_NST_CHECK_SIP_IS_DIP_E /
        CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E ] and enable[GT_TRUE / GT_FALSE].
        Expected: GT_OK.
        1.2. Call cpssDxChNstProtSanityCheckGet. Expected: GT_OK and the same enable. */
        checkType = CPSS_NST_CHECK_TCP_SYN_DATA_E;
        enable = GT_TRUE;

        st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

        st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, checkType);

        checkType = CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E;
        enable = GT_FALSE;

        st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

        st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, checkType);

        checkType = CPSS_NST_CHECK_FRAG_IPV4_ICMP_E;
        enable = GT_TRUE;

        st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

        st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, checkType);

        checkType = CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E;
        enable = GT_FALSE;

        st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

        st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
        UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                "get another enable than was set: %d, %d", dev, checkType);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            checkType = CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E;
            enable = GT_TRUE;

            st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

            st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, checkType);

            checkType = CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E;
            enable = GT_FALSE;

            st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

            st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, checkType);

            checkType = CPSS_NST_CHECK_SIP_IS_DIP_E;
            enable = GT_TRUE;

            st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

            st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, checkType);

            checkType = CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E;
            enable = GT_FALSE;

            st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, checkType, enable);

            st = cpssDxChNstProtSanityCheckGet(dev, checkType, &retEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChNstProtSanityCheckGet: %d, %d", dev, checkType);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, checkType);
        }

        /*
            1.3. Call function with wrong enum values checkType
                 and enable[GT_TRUE]
            Expected: GT_BAD_PARAM.
        */
        enable = GT_TRUE;

        UTF_ENUMS_CHECK_MAC(cpssDxChNstProtSanityCheckSet
                            (dev, checkType, enable),
                            checkType);
    }

    checkType = CPSS_NST_CHECK_TCP_SYN_DATA_E;
    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstProtSanityCheckSet(dev, checkType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstProtSanityCheckGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_NST_CHECK_ENT    checkType,
    OUT GT_BOOL               *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstProtSanityCheckGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with checkType[CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E /
 CPSS_NST_CHECK_TCP_FLAG_ZERO /  CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E /
 CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E /  CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E /
 CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E / CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E /
 CPSS_NST_CHECK_SIP_IS_DIP_E / CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E]
 and non-null enablePtr. Expected: GT_OK.
1.2. Call function with wrong enum values checkType and non-null
enablePtr. Expected: GT_BAD_PARAM.
1.3. Call function with checkType[CPSS_NST_CHECK_TCP_SYN_DATA_E] and
enablePtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS               st = GT_OK;
    GT_U8               dev;
    CPSS_NST_CHECK_ENT  checkType;
    GT_BOOL             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with checkType[CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E /
        CPSS_NST_CHECK_TCP_FLAG_ZERO /  CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E /
        CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E /  CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E /
        CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E / CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E /
        CPSS_NST_CHECK_SIP_IS_DIP_E / CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E]
        and non-null enablePtr. Expected: GT_OK. */
        checkType = CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E;

        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

        checkType = CPSS_NST_CHECK_TCP_FLAG_ZERO_E;

        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

        checkType = CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E;
        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

        checkType = CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E;

        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

        checkType = CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E;

        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            checkType = CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E;

            st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

            checkType = CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E;

            st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

            checkType = CPSS_NST_CHECK_SIP_IS_DIP_E;

            st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);

            checkType = CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E;

            st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, checkType);
        }

        /*
            1.2. Call function with wrong enum values checkType and non-null enablePtr.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChNstProtSanityCheckGet
                            (dev,checkType, &retEnable),
                            checkType);

        /*1.3. Call function with checkType[CPSS_NST_CHECK_TCP_SYN_DATA_E] and
        enablePtr[NULL]. Expected: GT_BAD_PTR. */
        checkType = CPSS_NST_CHECK_TCP_SYN_DATA_E;

        st = cpssDxChNstProtSanityCheckGet(dev,checkType, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, checkType);
    }

    checkType = CPSS_NST_CHECK_TCP_SYN_DATA_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstProtSanityCheckGet(dev,checkType, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstPortIngressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    IN GT_BOOL                          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIngressFrwFilterSet)
{
/*
ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
1.1. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E /
  CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E /
  CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E] and enable [GT_TRUE / GT_FALSE].
 Expected: GT_OK.
1.2. Call cpssDxChNstPortIngressFrwFilterGet. Expected: GT_OK and the same
enable.
1.3. Call function with wrong enum values filterType and enable[GT_TRUE]
.Expected: GT_BAD_PARAM.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                               dev;
    GT_PORT_NUM                         port;
    CPSS_NST_INGRESS_FRW_FILTER_ENT     filterType;
    GT_BOOL                             enable;

    GT_BOOL                             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*1.1. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E /
            CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E /
            CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E] and enable [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
            1.2. Call cpssDxChNstPortIngressFrwFilterGet. Expected: GT_OK and the same
            enable. */
            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;
            enable = GT_TRUE;

            st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortIngressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E;
            enable = GT_FALSE;

            st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortIngressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E;
            enable = GT_TRUE;

            st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortIngressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            /*
                1.3. Call function with wrong enum values filterType and enable[GT_TRUE].
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIngressFrwFilterSet
                                (dev, port, filterType, enable),
                                filterType);
        }

        filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;
        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;
    enable = GT_TRUE;
    port = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIngressFrwFilterSet(dev, port, filterType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstPortIngressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_INGRESS_FRW_FILTER_ENT  filterType,
    OUT GT_BOOL                          *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortIngressFrwFilterGet)
{
/*
ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
1.1. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E
/  CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E /
CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E] and non-null enablePtr.
Expected: GT_OK.
1.2. Call function with wrong enum values filterType and non-null
enablePtr. Expected: GT_BAD_PARAM.
1.3. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E]
 and enablePtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS                      st = GT_OK;
    GT_U8                               dev;
    GT_PORT_NUM                         port;
    CPSS_NST_INGRESS_FRW_FILTER_ENT     filterType;
    GT_BOOL                             retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E
            /  CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E /
            CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E] and non-null enablePtr.
            Expected: GT_OK. */
            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E;

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_ANALYZER_E;

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            /*
                1.2. Call function with wrong enum values filterType and non-null enablePtr.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortIngressFrwFilterGet
                                (dev, port, filterType, &retEnable),
                                filterType);

            /*1.3. Call function with filterType[CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E]
            and enablePtr[NULL]. Expected: GT_BAD_PTR.  */
            filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;

            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, NULL);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL", dev, port, filterType);
        }

        filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    filterType = CPSS_NST_INGRESS_FRW_FILTER_TO_NETWORK_E;
    port = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortIngressFrwFilterGet(dev, port, filterType, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstPortEgressFrwFilterSet
(
    IN GT_U8                            devNum,
    IN GT_PORT_NUM                      portNum,
    IN CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    IN GT_BOOL                          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortEgressFrwFilterSet)
{
/*
ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
1.1. Call function with filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E
/  CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E /  CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E]
 and enable[GT_TRUE / GT_FALSE]. Expected: GT_OK.
1.2. Call cpssDxChNstPortEgressFrwFilterGet. Expected: GT_OK and the same enable.
1.3. Call function with wrong enum values filterType and enable[GT_TRUE]
Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_PORT_NUM                     port;
    CPSS_NST_EGRESS_FRW_FILTER_ENT  filterType;
    GT_BOOL                         enable;
    GT_BOOL                         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /*1.1. Call function with filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E
            /  CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E /  CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E]
            and enable[GT_TRUE / GT_FALSE]. Expected: GT_OK.
            1.2. Call cpssDxChNstPortEgressFrwFilterGet. Expected: GT_OK and the same enable.*/
            filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;
            enable = GT_TRUE;

            st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortEgressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            filterType = CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E;
            enable = GT_FALSE;

            st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortEgressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            filterType = CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E;
            enable = GT_TRUE;

            st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, filterType, enable);

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                    "cpssDxChNstPortEgressFrwFilterGet: %d, %d, %d", dev, port, filterType);
            UTF_VERIFY_EQUAL3_STRING_MAC(enable, retEnable,
                    "get another enable than was set: %d, %d", dev, port, filterType);

            /*
                1.3. Call function with wrong enum values filterType and enable[GT_TRUE]
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortEgressFrwFilterSet
                                (dev, port, filterType, enable),
                                filterType);
        }

        filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;
        enable = GT_TRUE;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;
    enable = GT_TRUE;
    port = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortEgressFrwFilterSet(dev, port, filterType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstPortEgressFrwFilterGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_NUM                      portNum,
    IN  CPSS_NST_EGRESS_FRW_FILTER_ENT   filterType,
    OUT GT_BOOL                          *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortEgressFrwFilterGet)
{
/*
ITERATE_DEVICES_PHY_PORT (DxCh2 and above)
1.1. Call function with filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E
/  CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E /  CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E]
 and non-null enablePtr. Expected: GT_OK.
1.2. Call function with wrong enum values filterType and non-null
enablePtr. Expected: GT_BAD_PARAM.
1.3. Call function with  filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E]
and enablePtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_PORT_NUM                     port;
    CPSS_NST_EGRESS_FRW_FILTER_ENT  filterType;
    GT_BOOL                         retEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            /* 1.1. Call function with filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E
            /  CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E /  CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E]
            and non-null enablePtr. Expected: GT_OK. */
            filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            filterType = CPSS_NST_EGRESS_FRW_FILTER_BRIDGED_E;

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            filterType = CPSS_NST_EGRESS_FRW_FILTER_ROUTED_E;

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, filterType);

            /*
                1.2. Call function with wrong enum values filterType and non-null enablePtr.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChNstPortEgressFrwFilterGet
                                (dev, port, filterType, &retEnable),
                                filterType);

            /* 1.3. Call function with  filterType[CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E]
            and enablePtr[NULL]. Expected: GT_BAD_PTR. */
            filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;

            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, NULL);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL", dev, port, filterType);
        }

        filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PORT_NUM_CNS(dev);

        st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_ENABLED_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    filterType = CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E;
    port = NST_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortEgressFrwFilterGet(dev, port, filterType, &retEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstIngressFrwFilterDropCntrSet
(
    IN GT_U8       devNum,
    IN GT_U32      ingressCnt
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstIngressFrwFilterDropCntrSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with ingressCnt[0 / 100 /0xFFFFFFFE] (
no constraints by contract). Expected: GT_OK.
1.2. Call cpssDxChNstIngressFrwFilterDropCntrGet. Expected: GT_OK
and the same ingressCnt.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          ingressCnt;
    GT_U32          retIngressCnt;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call function with ingressCnt[0 / 100 /0xFFFFFFFE] (
        no constraints by contract). Expected: GT_OK.
        1.2. Call cpssDxChNstIngressFrwFilterDropCntrGet. Expected: GT_OK
        and the same ingressCnt.*/
        ingressCnt = 0;

        st = cpssDxChNstIngressFrwFilterDropCntrSet(dev, ingressCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNstIngressFrwFilterDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                                     "got another ingressCnt than was set: %d", dev);

        ingressCnt = 100;

        st = cpssDxChNstIngressFrwFilterDropCntrSet(dev, ingressCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNstIngressFrwFilterDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                                     "got another ingressCnt than was set: %d", dev);

        ingressCnt = 0xFFFFFFFE;

        st = cpssDxChNstIngressFrwFilterDropCntrSet(dev, ingressCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChNstIngressFrwFilterDropCntrGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                                     "got another ingressCnt than was set: %d", dev);
    }

    ingressCnt = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstIngressFrwFilterDropCntrSet(dev, ingressCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstIngressFrwFilterDropCntrSet(dev, ingressCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstIngressFrwFilterDropCntrGet
(
    IN GT_U8       devNum,
    OUT GT_U32     *ingressCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstIngressFrwFilterDropCntrGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with non-null ingressCntPtr. Expected: GT_OK.
1.2. Call function with ingressCntPtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_U32          retIngressCnt;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with non-null ingressCntPtr. Expected: GT_OK. */
        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*1.2. Call function with ingressCntPtr[NULL]. Expected: GT_BAD_PTR. */
        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstIngressFrwFilterDropCntrGet(dev, &retIngressCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstRouterAccessMatrixCmdSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    IN  CPSS_PACKET_CMD_ENT        command
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstRouterAccessMatrixCmdSet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with sipAccessLevel[0], dipAccessLevel[7] and
command[CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_HARD_E /
CPSS_PACKET_CMD_DROP_SOFT_E]. Expected: GT_OK.
1.2. Call cpssDxChNstRouterAccessMatrixCmdGet. Expected: GT_OK and
the same command.
1.3. Call function with out of range sipAccessLevel[8], dipAccessLevel[0]
 and command[CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK.
1.4. Call function with sipAccessLevel[0], out of range dipAccessLevel[8]
 and command[CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK.
1.5. Call function with sipAccessLevel [0], dipAccessLevel[0] and
command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E
/ CPSS_PACKET_CMD_ROUTE_E/  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /  CPSS_PACKET_CMD_BRIDGE_E /
CPSS_PACKET_CMD_NONE_E /  CPSS_PACKET_CMD_INVALID_E]. Expected: NON GT_OK.
1.6. Call function with sipAccessLevel [0], dipAccessLevel[0]
and wrong enum values command.
Expected: GT_BAD_PARAM.
*/
    GT_STATUS                       st = GT_OK;
    GT_U8                           dev;
    GT_U32                          sipAccessLevel;
    GT_U32                          dipAccessLevel;
    CPSS_PACKET_CMD_ENT             command;

    CPSS_PACKET_CMD_ENT             retCommand;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with sipAccessLevel[0], dipAccessLevel[7] and
        command[CPSS_PACKET_CMD_FORWARD_E / CPSS_PACKET_CMD_DROP_HARD_E /
        CPSS_PACKET_CMD_DROP_SOFT_E]. Expected: GT_OK.
        1.2. Call cpssDxChNstRouterAccessMatrixCmdGet. Expected: GT_OK and
        the same command. */
        sipAccessLevel = 0;
        dipAccessLevel = 7;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, sipAccessLevel, dipAccessLevel, command);

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChNstRouterAccessMatrixCmdGet: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
                   "got another command than was set: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);

        command = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, sipAccessLevel, dipAccessLevel, command);

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChNstRouterAccessMatrixCmdGet: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
                   "got another command than was set: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);

        command = CPSS_PACKET_CMD_DROP_SOFT_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, sipAccessLevel, dipAccessLevel, command);

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxChNstRouterAccessMatrixCmdGet: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);
        UTF_VERIFY_EQUAL3_STRING_MAC(command, retCommand,
                   "got another command than was set: %d, %d, %d", dev, sipAccessLevel, dipAccessLevel);

        /* 1.3. Call function with out of range sipAccessLevel[8], dipAccessLevel[0]
        and command[CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK. */
        sipAccessLevel = 8;
        dipAccessLevel = 0;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", dev, sipAccessLevel);

        /* 1.4. Call function with sipAccessLevel[0], out of range dipAccessLevel[8]
        and command[CPSS_PACKET_CMD_FORWARD_E]. Expected: NON GT_OK. */
        sipAccessLevel = 0;
        dipAccessLevel = 8;
        command = CPSS_PACKET_CMD_FORWARD_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, dipAccessLevel=%d", dev, dipAccessLevel);

        /* 1.5. Call function with sipAccessLevel [0], dipAccessLevel[0] and
        command[CPSS_PACKET_CMD_MIRROR_TO_CPU_E / CPSS_PACKET_CMD_TRAP_TO_CPU_E
        / CPSS_PACKET_CMD_ROUTE_E/  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E /
        CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E /  CPSS_PACKET_CMD_BRIDGE_E /
        CPSS_PACKET_CMD_NONE_E /  CPSS_PACKET_CMD_INVALID_E]. Expected: NON GT_OK. */
        sipAccessLevel = 0;
        dipAccessLevel = 0;
        command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_TRAP_TO_CPU_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_ROUTE_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_BRIDGE_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        command = CPSS_PACKET_CMD_NONE_E;

        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, command=%d", dev, command);

        /*
            1.6. Call function with sipAccessLevel [0], dipAccessLevel[0]
            and wrong enum values command.
            Expected: GT_BAD_PARAM.
        */
        sipAccessLevel = 0;
        dipAccessLevel = 0;

        UTF_ENUMS_CHECK_MAC(cpssDxChNstRouterAccessMatrixCmdSet
                            (dev, sipAccessLevel, dipAccessLevel, command),
                            command);
    }

    sipAccessLevel = 0;
    dipAccessLevel = 0;
    command = CPSS_PACKET_CMD_FORWARD_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstRouterAccessMatrixCmdSet(dev, sipAccessLevel, dipAccessLevel, command);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
cpssDxChNstRouterAccessMatrixCmdGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     sipAccessLevel,
    IN  GT_U32                     dipAccessLevel,
    IN  CPSS_PACKET_CMD_ENT        *commandPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstRouterAccessMatrixCmdGet)
{
/*
ITERATE_DEVICES (DxCh2 and above)
1.1. Call function with sipAccessLevel[0], dipAccessLevel[7] and non-null
commandPtr. Expected: GT_OK.
1.2. Call function with out of range sipAccessLevel[8], dipAccessLevel[0]
and non-null commandPtr. Expected: NON GT_OK.
1.3. Call function with sipAccessLevel[0], out of range dipAccessLevel[8]
and non-null commandPtr. Expected: NON GT_OK.
1.4. Call function with sipAccessLevel [0], dipAccessLevel[0] and
commandPtr[NULL]. Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st = GT_OK;
    GT_U8                       dev;
    GT_U32                      sipAccessLevel;
    GT_U32                      dipAccessLevel;
    CPSS_PACKET_CMD_ENT         retCommand;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call function with sipAccessLevel[0], dipAccessLevel[7] and non-null
        commandPtr. Expected: GT_OK. */
        sipAccessLevel = 0;
        dipAccessLevel = 7;

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, sipAccessLevel, dipAccessLevel);

        /* 1.2. Call function with out of range sipAccessLevel[8], dipAccessLevel[0]
        and non-null commandPtr. Expected: NON GT_OK. */
        sipAccessLevel = 8;
        dipAccessLevel = 0;

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, %d", dev, sipAccessLevel);

        /* 1.3. Call function with sipAccessLevel[0], out of range dipAccessLevel[8]
        and non-null commandPtr. Expected: NON GT_OK. */
        sipAccessLevel = 0;
        dipAccessLevel = 8;

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,"%d, dipAccessLevel = %d",
                                         dev, dipAccessLevel);

        /* 1.4. Call function with sipAccessLevel [0], dipAccessLevel[0] and
        commandPtr[NULL]. Expected: GT_BAD_PTR. */
        sipAccessLevel = 0;
        dipAccessLevel = 0;

        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, NULL);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "%d, %d, %d, NULL",
                                      dev, sipAccessLevel, dipAccessLevel);
    }

    sipAccessLevel = 0;
    dipAccessLevel = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_XCAT2_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstRouterAccessMatrixCmdGet(dev, sipAccessLevel, dipAccessLevel, &retCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChNstPortGroupIngressFrwFilterDropCntrGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *ingressCntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortGroupIngressFrwFilterDropCntrGet)
{
    GT_U32 regAddr;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(utfFirstDevNumGet())->bridgeRegs.nstRegs.ingressFrwDropCounter;
    if( PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr )
    {
        regAddr = 0x0B02000C;
    }

    prvUtfPerPortGroupCounterGet(regAddr, 0, 32,
              cpssDxChNstIngressFrwFilterDropCntrGet,
              cpssDxChNstIngressFrwFilterDropCntrSet,
              cpssDxChNstPortGroupIngressFrwFilterDropCntrGet,
              cpssDxChNstPortGroupIngressFrwFilterDropCntrSet);
}

/*----------------------------------------------------------------------------*/
/*
cpssDxChNstPortGroupIngressFrwFilterDropCntrSet
(
    IN GT_U8                 devNum,
    IN GT_PORT_GROUPS_BMP    portGroupsBmp,
    IN GT_U32                ingressCnt
)
*/
UTF_TEST_CASE_MAC(cpssDxChNstPortGroupIngressFrwFilterDropCntrSet)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxCh2 and above)
    1.1.1. Call function with ingressCnt[0 / 100 / 0xFFFFFFFE] (no constraints).
    Expected: GT_OK.
    1.1.2. Call cpssDxChNstPortGroupIngressFrwFilterDropCntrGet.
    Expected: GT_OK and the same ingressCnt.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;

    GT_PORT_GROUPS_BMP portGroupsBmp = 1;
    GT_U32             portGroupId;

    GT_U32          ingressCnt = 0;
    GT_U32          retIngressCnt;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
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
                1.1.1. Call function with ingressCnt[0 / 100 / 0xFFFFFFFE]
                (no constraints).
                Expected: GT_OK.
            */
            /*call with ingressCnt = 0;*/
            ingressCnt = 0;

            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                                portGroupsBmp, ingressCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

            /*
                1.1.2. Call cpssDxChNstPortGroupIngressFrwFilterDropCntrGet.
                Expected: GT_OK and the same ingressCnt.
            */
            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrGet(dev,
                                                portGroupsBmp, &retIngressCnt);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChNstPortGroupIngressFrwFilterDropCntrGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                                    "got another ingressCnt than was set: %d", dev);

            /*call with ingressCnt = 100;*/
            ingressCnt = 100;

            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                                        portGroupsBmp, ingressCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

            /*
                1.1.2. Call cpssDxChNstPortGroupIngressFrwFilterDropCntrGet.
                Expected: GT_OK and the same ingressCnt.
            */
            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrGet(dev,
                                                    portGroupsBmp, &retIngressCnt);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChNstPortGroupIngressFrwFilterDropCntrGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                               "got another ingressCnt than was set: %d", dev);

            /*call with ingressCnt = 0xFFFFFFFE;*/
            ingressCnt = 0xFFFFFFFE;

            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                                    portGroupsBmp, ingressCnt);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, ingressCnt);

            /*
                1.1.2. Call cpssDxChNstPortGroupIngressFrwFilterDropCntrGet.
                Expected: GT_OK and the same ingressCnt.
            */
            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrGet(dev,
                                                portGroupsBmp, &retIngressCnt);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssDxChNstPortGroupIngressFrwFilterDropCntrGet: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(ingressCnt, retIngressCnt,
                                "got another ingressCnt than was set: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                                    portGroupsBmp, ingressCnt);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.3. For unaware port groups check that function returns GT_OK. */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                                    portGroupsBmp, ingressCnt);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portGroupsBmp);
    }

    portGroupsBmp = 1;
    ingressCnt = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
         UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                            portGroupsBmp, ingressCnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChNstPortGroupIngressFrwFilterDropCntrSet(dev,
                                            portGroupsBmp, ingressCnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChNst suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChNst)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstBridgeAccessMatrixCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstBridgeAccessMatrixCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstDefaultAccessLevelsSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstDefaultAccessLevelsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstProtSanityCheckSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstProtSanityCheckGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIngressFrwFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortIngressFrwFilterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortEgressFrwFilterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortEgressFrwFilterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstIngressFrwFilterDropCntrSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstIngressFrwFilterDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstRouterAccessMatrixCmdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstRouterAccessMatrixCmdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortGroupIngressFrwFilterDropCntrGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChNstPortGroupIngressFrwFilterDropCntrSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChNst)


