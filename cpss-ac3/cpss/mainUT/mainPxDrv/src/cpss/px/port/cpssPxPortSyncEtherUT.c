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
* @file cpssPxPortSyncEtherUT.c
*
* @brief Unit tests for cpssPxPortSyncEther, that provides
* CPSS implementation for Sync-E (Synchronous Ethernet)
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
#include <cpss/px/port/private/prvCpssPxPort.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/**
* @internal prvUtGetPortLaneNum function
* @endinternal
*
* @brief   Gets the number of lanes for port (based on port interface mode).
*
* @note   APPLICABLE DEVICES:      Pipe
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum or no lanes on port
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvUtGetPortLaneNum
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32  *maxLaneNumPtr
)
{
    GT_STATUS                       rc;     /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* port interface mode */
    GT_U32 startSerdes;
    GT_U32 maxLane;

    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }
    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes, &maxLane);

    *maxLaneNumPtr = maxLane - 1;

    return rc;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_U8   portNum,
    IN  GT_U32  laneNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSyncEtherRecoveryClkConfigSet)
{
/*
    ITERATE_DEVICES (Pipe):
    1.1.1. Call with recoveryClkType = [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E /
                                        CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E],
                          and enable = [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSyncEtherRecoveryClkConfigGet.
    Expected: GT_OK and the same value.
    1.1.3. Call wrong enum values recoveryClkType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS  st        = GT_OK;
    GT_U8      dev;

    GT_PHYSICAL_PORT_NUM port           = 0;
    GT_PHYSICAL_PORT_NUM      portGet   = 1;
    GT_U32     lane      = 0;
    GT_U32     laneGet   = 0;
    GT_BOOL    enable    = GT_TRUE;
    GT_BOOL    enableGet = GT_FALSE;
    GT_U32     i;
     
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType =
                                                        CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with recoveryClkType =
                                    [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E /
                                     CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E],
                       and enable = [GT_TRUE / GT_FALSE].
                Expected: GT_OK.
            */

            for (recoveryClkType = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E; 
                  recoveryClkType <= CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E; recoveryClkType++)
            {
                enable = GT_TRUE;

                for (i = 0; i < 2; i++)
                {
                    st = cpssPxPortSyncEtherRecoveryClkConfigSet(dev, 
                                                                 recoveryClkType, 
                                                                 enable, port, lane);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    if(GT_OK == st)
                    {
                        /*
                            1.1.2. Call cpssPxPortSyncEtherRecoveryClkConfigGet.
                            Expected: GT_OK and the same value.
                        */

                            st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                                     recoveryClkType, &enableGet, &portGet, &laneGet);

                            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port,
                                                                   enable, recoveryClkType);
                            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                                "got another enable than was set: %d", dev);
                            if (enable == GT_TRUE)
                            {
                                UTF_VERIFY_EQUAL1_STRING_MAC(port, portGet, 
                                                      "got another port than was set: %d", dev);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL1_STRING_MAC(PRV_CPSS_PX_PORTS_NUM_CNS, portGet, 
                                                      "got another port than was set: %d", dev);
                            }
                    }
                    enable = !enable;
                }
            }

            /*
                1.1.3. Call wrong enum values recoveryClkType.
                Expected: GT_BAD_PARAM.
            */
            enable = GT_TRUE;

            UTF_ENUMS_CHECK_MAC(cpssPxPortSyncEtherRecoveryClkConfigSet
                                (dev, recoveryClkType, enable, port, 0),
                                recoveryClkType);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSyncEtherRecoveryClkConfigSet(dev,
                                         recoveryClkType, enable, port, lane);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSyncEtherRecoveryClkConfigSet(dev,
                                     recoveryClkType, enable, port, lane);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_U8    devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_U8    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSyncEtherRecoveryClkConfigGet)
{
/*
    1.1.1. Call function for with non null enable and portNum pointer.
    Expected: GT_OK.
    1.1.2. Call function for with wrong enum values recoveryClkType.
    Expected: GT_BAD_PARAM.
    1.1.3. Call function for with null enable pointer [NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call function for with null portNum pointer [NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call function for with null laneNum pointer [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                       st       = GT_OK;
    GT_U8                                           dev;
    GT_PHYSICAL_PORT_NUM                            port;
    GT_U32                                          lane;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT recoveryClkType;
    GT_BOOL                                         enable;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call function for with non null enable and portNum pointer.
            Expected: GT_OK.
        */

        /*Call with
          recoveryClkType [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E]*/
        recoveryClkType = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, recoveryClkType);

        /*Call with
          recoveryClkType [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E]*/
        recoveryClkType = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E;

        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, recoveryClkType);

        /*
            1.1.2. Call function for with wrong enum values recoveryClkType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortSyncEtherRecoveryClkConfigGet
                            (dev, recoveryClkType, &enable, &port, &lane),
                            recoveryClkType);

        /*
            1.1.3. Call function for with null enable pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, NULL, &port, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);

        /*
            1.1.4. Call function for with null port pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, NULL, &lane);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);

        /*
            1.1.5. Call function for with null lane pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, recoveryClkType);
    }

    recoveryClkType = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E;

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                                 recoveryClkType, &enable, &port, &lane);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSyncEtherRecoveryClkConfigGet(dev,
                             recoveryClkType, &enable, &port, &lane);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSyncEtherRecoveryClkDividerValueSet)
{
    /*
    ITERATE_DEVICES (Pipe)
    1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and value [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E \
                           CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E]
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSyncEtherRecoveryClkDividerValueGet
                `       with non-NULL valueGet and other parameters from 1.1.1
    Expected: GT_OK and same valueGet as written by value
    1.1.3. Call with lane [maxLaneOnPort+1]
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values value.
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                                               dev;
    GT_PHYSICAL_PORT_NUM                                port = 0;
    GT_U32                                              lane = 0;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value =
                                                            CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  valueGet;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect =
                                                            CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;
    CPSS_PORT_INTERFACE_MODE_ENT                        ifMode;
    CPSS_PP_FAMILY_TYPE_ENT                             devFamily;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if( (CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                (CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E == ifMode) )
            {
                continue;
            }

            /*
               1.1.1. Call with lane [0 \ maxLaneOnPort ]
                and value [CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E \
                           CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E]
               Expected: GT_OK.
            */

            lane = 0;
            value = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;

            st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /*
                1.1.2. Call cpssPxPortSyncEtherRecoveryClkDividerValueGet
                `       with non-NULL valueGet and other parameters from 1.1.1
                Expected: GT_OK and same valueGet as written by value
            */

            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &valueGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssPxPortSyncEtherRecoveryClkDividerValueGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(value, valueGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);

            value = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E;

            st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect, value);

            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &valueGet);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssPxPortSyncEtherRecoveryClkDividerValueGet: %d %d %d %d",
                                         dev, port, lane, clockSelect);
            UTF_VERIFY_EQUAL4_STRING_MAC(value, valueGet, "got another value than was set: %d %d %d %d",
                                         dev, port, lane, clockSelect);


            /*
               1.1.3. with lane [maxLaneOnPort+1]
               Expected: GT_BAD_PARAM.
            */

            st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, (GT_U8)(lane+1), clockSelect, value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane+1, value);

            /*
               1.1.4. Call with wrong enum values value.
               Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortSyncEtherRecoveryClkDividerValueSet
                                (dev, port, lane, clockSelect, value),
                                value);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, value);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect, value);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = 0;
    lane = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSyncEtherRecoveryClkDividerValueSet(dev, port, lane, clockSelect, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSyncEtherRecoveryClkDividerValueGet)
{
    /*
    ITERATE_DEVICES (Lion and above)
    1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL valuePtr.
    Expected: GT_OK.
    1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL valuePtr.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with NULL valuePtr.
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8                                               dev;
    GT_PHYSICAL_PORT_NUM                                port = 0;
    GT_U32                                              lane = 0;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    value;
    CPSS_PORT_INTERFACE_MODE_ENT                        ifMode;
    CPSS_PP_FAMILY_TYPE_ENT                             devFamily;
    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E;

    /* there is no Sync Ethernet in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with lane [0 \ maxLaneOnPort ] and non-NULL valuePtr.
                Expected: GT_OK.
            */

            lane = 0;

            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = prvUtGetPortLaneNum(dev, port, &lane);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtGetPortLaneNum: %d %d", dev, port);

            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, clockSelect);

            /*
                1.1.2. Call with lane [maxLaneOnPort+1] and non-NULL valuePtr.
                Expected: GT_BAD_PARAM.
            */

            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, (GT_U8)(lane+1), clockSelect, &value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);

            /*
                1.1.3. Call with NULL valuePtr.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, NULL);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, dev, port, lane);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane, clockSelect);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = 0;
    lane = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSyncEtherRecoveryClkDividerValueGet(dev, port, lane, clockSelect, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortSyncEther suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortSyncEther)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSyncEtherRecoveryClkConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSyncEtherRecoveryClkConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSyncEtherRecoveryClkDividerValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSyncEtherRecoveryClkDividerValueGet)
UTF_SUIT_END_TESTS_MAC(cpssPxPortSyncEther)


