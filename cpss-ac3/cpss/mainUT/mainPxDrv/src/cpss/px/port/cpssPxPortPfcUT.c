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
* @file cpssPxPortPfcUT.c
*
* @brief Unit tests for cpssPxPortPfc, that provides
* CPSS implementation for Priority Flow Control functionality.
*
* @version   25
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

/* Default valid value for port id */
#define PORT_PFC_VALID_PHY_PORT_CNS  0

/* Invalid profileIndex */
#define PORT_PFC_INVALID_PROFILEINDEX_CNS   8

/* Invalid profileIndex */
#define PORT_PFC_INVALID_COUNTERNUM_CNS   128


/* Invalid tcQueue */
#define PORT_PFC_INVALID_TCQUEUE_CNS        8

/* Invalid xoffThreshold */
#define PORT_PFC_INVALID_XOFFTHRESHOLD_CNS  0x7FF+1

/* Invalid xoffThreshold */
#define PORT_PFC_INVALID_XONTHRESHOLD_CNS   0x1

/* Invalid dropThreshold */
#define PORT_PFC_INVALID_DROPTHRESHOLD_CNS  0x7FF+1

#define PORT_PFC_INVALID_DROPTHRESHOLD_CNS  0x7FF+1

#define PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE   0x1FFFFF
#define PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM      21


#define PORT_PFC_INVALID_XOFFTHRESHOLD_MAC  (PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE + 1)

#define PORT_PFC_INVALID_XONTHRESHOLD_MAC (PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE + 1)

#define PORT_PFC_INVALID_DROPTHRESHOLD_MAC (PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE + 1)

#define PORT_PFC_INVALID_XOFFTHRESHOLD_STEP_MAC ((GT_U32)(BIT_20))

#define PORT_PFC_INVALID_XONTHRESHOLD_STEP_MAC ((GT_U32)(BIT_20))

#define PORT_PFC_INVALID_DROPTHRESHOLD__STEP_MAC    (BIT_20)

/* Invalid pfc Timer */
#define PORT_PFC_INVALID_TIMER_CNS        8


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcEnableSet
(
    IN  GT_U8     devNum,
    IN  CPSS_PX_PORT_PFC_ENABLE_ENT pfcEnable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call with state [CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E/
                          CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E].
        Expected: GT_OK.
    1.2. Call cpssPxPortPfcEnableGet.
        Expected: GT_OK and the same enable.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    CPSS_PX_PORT_PFC_ENABLE_ENT state;
    CPSS_PX_PORT_PFC_ENABLE_ENT enable = CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

     /*
       1.1. Call with state.
       Expected: GT_OK.
      */
        st = cpssPxPortPfcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
          1.2. Call cpssPxPortPfcEnableGet.
           Expected: GT_OK and the same enable options.
         */
        st = cpssPxPortPfcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "[cpssPxhPortPfcEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                                     "get another enable value than was set: %d, %d", dev);
        /*
           1.3. Check wrong enum values
        */

        UTF_ENUMS_CHECK_MAC(cpssPxPortPfcEnableSet(dev, enable),enable);

    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcEnableGet
(
    IN   GT_U8     devNum,
    OUT CPSS_PX_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcEnableGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    CPSS_PX_PORT_PFC_ENABLE_ENT state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssPxPortPfcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortPfcEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
     PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcProfileIndexSet
(
    IN GT_U8    devNum,
    IN GT_U8    portNum,
    IN GT_U32   profileIndex
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcProfileIndexSet)
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with profileIndex [0 / 7].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPfcProfileIndexGet with the same params.
    Expected: GT_OK and the same profileIndex.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM

*/
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_PFC_VALID_PHY_PORT_CNS ;
    GT_U32      profileIndex = 0;
    GT_U32      profileIndexGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                Call with profileIndex [0 / 7].
                Expected: GT_OK.*/
            for(profileIndex = 0;profileIndex < PORT_PFC_INVALID_PROFILEINDEX_CNS;profileIndex++)
            {
                st = cpssPxPortPfcProfileIndexSet(dev, port, profileIndex);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssPxPortPfcProfileIndexGet.
                       Expected: GT_OK and the same profileIndex.
                    */
                    st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndexGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                    "[cpssPxPortPfcProfileIndexGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(profileIndex, profileIndexGet,
                                    "get another profileIndex value than was set: %d, %d", dev, port);
                }
            }

         }
        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {

            /* 1.2.1. Call function for each non-active port */
            /* profileIndex = 7    */
            st = cpssPxPortPfcProfileIndexSet(dev, port, profileIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        /* profileIndex = 7    */
        st = cpssPxPortPfcProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_PFC_VALID_PHY_PORT_CNS ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* profileIndex = 7    */
        st = cpssPxPortPfcProfileIndexSet(dev, port, profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcProfileIndexSet(dev, port, profileIndex);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U32  *profileIndexPtr
)
*/


UTF_TEST_CASE_MAC(cpssPxPortPfcProfileIndexGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL profileIndexPtr
    Expected: GT_OK.
    1.1.2. Call function with profileIndexPtr [NULL] .
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_U32     profileIndex;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call function with non-NULL profileIndexPtr
               Expected: GT_OK.
            */
            st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndex);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, profileIndex);

            /*
               1.1.2. Call function with enablePtr [NULL] .
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPfcProfileIndexGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);


        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndex);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_PFC_VALID_PHY_PORT_CNS ;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcProfileIndexGet(dev, port, &profileIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxPortPfcDbaAvailableBuffersSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32         buffsNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcDbaAvailableBuffersSet)
{
/*
    1. Go over all active devices.
{
{
    1.1. Call with number of buffers = 0x10000
        Expected: GT_OK.
    1.2. Call ccpssPxPortPfcDbaAvailableBuffersGet.
        Expected: GT_OK and the same buffsNum.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32         buffsNumGet;
    GT_U32         buffsNum = 0x10000;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

     /*
       1.1. Call with state.
       Expected: GT_OK.
      */
        st = cpssPxPortPfcDbaAvailableBuffersSet(dev, buffsNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, buffsNum);

        /*
          1.2. Call cpssPxPortPfcDbaAvailableBuffersGet.
           Expected: GT_OK and the same buffsNum options.
         */
        st = cpssPxPortPfcDbaAvailableBuffersGet(dev, &buffsNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "[cpssPxhPortPfcBuffersGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(buffsNum, buffsNumGet,
                                     "get another buffers number value than was set: %d, %d", dev);
        /*
           1.3. Check wrong values
        */

        st =cpssPxPortPfcDbaAvailableBuffersSet(dev, BIT_24);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcDbaAvailableBuffersSet(dev, buffsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcDbaAvailableBuffersSet(dev, buffsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcDbaAvailableBuffersGet
(
    IN   GT_U8     devNum,
    IN GT_U32      *pfcDbaBuffsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcDbaAvailableBuffersGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL pfcDbaBuffsPtr
    Expected: GT_OK.
    1.1.2. Call function with pfcDbaBuffsPtr [NULL] .
    Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_U32         buffsNum;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssPxPortPfcDbaAvailableBuffersGet(dev, &buffsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortPfcDbaAvailableBuffersGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
     PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcDbaAvailableBuffersGet(dev, &buffsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcDbaAvailableBuffersGet(dev, &buffsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*GT_STATUS cpssPxPortPfcProfileQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32   profileIndex,
    IN GT_U32    tcQueue,
    IN CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcProfileQueueConfigSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with profileIndex [0 / 7], tcQueue [0 /7],
                        pfcProfileCfg.xonThreshold = 10,
                        pfcProfileCfg.xoffThreshold = 10.
                        pfcProfileCfg.xonAlpha = 1,
                        pfcProfileCfg.xoffAlpha = 1.
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPfcProfileQueueConfigGet with with not-NULL pfcProfileCfgPtr.
    Expected: GT_OK and the same pfcProfileCfg as was set.
    1.3. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
    Expected: Expected: GT_BAD_PARAM
    1.4. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: Expected: GT_BAD_PARAM
    1.5. Call with wrong pfcProfileCfg [NULL].
    Expected: GT_BAD_PTR.
    2. Call the function with not active devices
    Expected: GT_NOT_APPLICABLE_DEVICE

*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_U32   profileIndex;
    GT_U8    tcQueue;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC   pfcProfileCfg;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC   pfcProfileCfgGet;



    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    pfcProfileCfg.xonThreshold = 10;
    pfcProfileCfg.xoffThreshold = 10;
    pfcProfileCfg.xonAlpha = 1;
    pfcProfileCfg.xoffAlpha = 1;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            Call with profileIndex [0 / 7].
            Expected: GT_OK.*/
        for(profileIndex = 0;profileIndex < PORT_PFC_INVALID_PROFILEINDEX_CNS;profileIndex++)
        {
            for(tcQueue = 0;tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS;tcQueue++)
            {
                st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call cpssPxPortPfcProfileQueueConfigGet with not-NULL pfcProfileCfgPtr.
                    Expected: GT_OK and the same pfcProfileCfg as was set.
                */
                st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfgGet);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssPxPortPfcProfileQueueConfigGet: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonThreshold,
                                             pfcProfileCfgGet.xonThreshold,
                       "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffThreshold,
                                             pfcProfileCfgGet.xoffThreshold,
                       "got another pfcProfileCfg.xonThreshold then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xonAlpha,
                                             pfcProfileCfgGet.xonAlpha,
                       "got another pfcProfileCfg.xonAlpha then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(pfcProfileCfg.xoffAlpha,
                                             pfcProfileCfgGet.xoffAlpha,
                       "got another pfcProfileCfg.xoffAlpha then was set: %d", dev);


            }
        }
        /*1.3. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
        Expected: Expected: GT_BAD_PARAM */
        profileIndex = PORT_PFC_INVALID_PROFILEINDEX_CNS;
        tcQueue = 7;
        st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndex = 7;
        /*1.4. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
        Expected: Expected: GT_BAD_PARAM */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 7;

        /*
            1.5. Call with wrong pfcProfileCfg [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* profileIndex = 7   tcQueue = 7 */
        profileIndex = 7;
        tcQueue = 7;
        st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* profileIndex = 7   tcQueue = 7 */
    profileIndex = 7;
    tcQueue = 7;
    st = cpssPxPortPfcProfileQueueConfigSet(dev, profileIndex, tcQueue, &pfcProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}
/*GT_STATUS cpssPxPortPfcProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U32    tcQueue,
    OUT CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);
*/

UTF_TEST_CASE_MAC(cpssPxPortPfcProfileQueueConfigGet)
{
/*
     1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null pfcProfileCfg.
    Expected: GT_OK.
    1.2. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong  pfcProfileCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_U32   profileIndex;
    GT_U8    tcQueue;
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC   pfcProfileCfg;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 7], tcQueue [0 / 7] and not null pfcProfileCfg.
            Expected: GT_OK.
        */
        for(profileIndex = 0;profileIndex < PORT_PFC_INVALID_PROFILEINDEX_CNS;profileIndex++)
        {
            for(tcQueue = 0;tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS;tcQueue++)
            {
                st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }
        /*
            1.2. Call with wrong profileIndex [PORT_PFC_INVALID_PROFILEINDEX_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = 7;
        profileIndex = PORT_PFC_INVALID_PROFILEINDEX_CNS;

        st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        profileIndex = 7;

        /*
            1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 7;

        /*
            1.4. Call with wrong  pfcProfileCfg pointer [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, pfcProfileCfg = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* profileIndex = 7   tcQueue = 7 */
        profileIndex = 7;
        tcQueue = 7;
        st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* profileIndex = 7   tcQueue = 7 */
    profileIndex = 7;
    tcQueue = 7;
    st = cpssPxPortPfcProfileQueueConfigGet(dev, profileIndex, tcQueue, &pfcProfileCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*GT_STATUS cpssPxPortPfcCountingModeSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
);
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcCountingModeSet)
{
/*
    1. Go over all active devices.
    1.1. Call with all correct pfcCountMode [CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E /
                                            CPSS_PX_PORT_PFC_COUNT_PACKETS_E].
    Expected: GT_OK.
    1.2. Call cpssPxPortPfcCountingModeGet with not-NULL pfcCountModePtr.
    Expected: GT_OK and the same pfcCountMode as was set.
    1.3. Call with wrong enum values pfcCountMode.
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode = CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E;
    CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountModeGet;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

     /*
       1.1. Call with state.
       Expected: GT_OK.
      */
        st = cpssPxPortPfcCountingModeSet(dev, pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortPfcCountingModeGet with not-NULL pfcCountModePtr.
            Expected: GT_OK and the same pfcCountMode as was set.
        */
        st = cpssPxPortPfcCountingModeGet(dev, &pfcCountModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortPfcCountingModeGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(pfcCountMode, pfcCountModeGet,
                         "got another pfcCountMode then was set: %d", dev);
        /*
           1.3. Check wrong enum values
        */

        UTF_ENUMS_CHECK_MAC(cpssPxPortPfcCountingModeSet(dev, pfcCountMode),pfcCountMode);

    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcCountingModeSet(dev, pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcCountingModeSet(dev, pfcCountMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*
GT_STATUS cpssPxPortPfcCountingModeGet
(
    IN  GT_U8    devNum,
    OUT CPSS_PX_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcCountingModeGet)
{
/*
    1. Go over all active devices.
    1.1. Call with not null pfcCountModePtr.
    Expected: GT_OK.
    1.2. Call with wrong pfcCountModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null pfcCountModePtr.
            Expected: GT_OK.
        */
        st = cpssPxPortPfcCountingModeGet(dev, &pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong pfcCountModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcCountingModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcCountingModeGet(dev, &pfcCountMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcCountingModeGet(dev, &pfcCountMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*static GT_STATUS internal_cpssPxPortPfcGlobalDropEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcGlobalDropEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.2. Call cpssPxPortPfcGlobalDropEnableGet.
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
           1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_FALSE] */
        enable = GT_FALSE;

        st = cpssPxPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssPxPortPfcGlobalDropEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortPfcGlobalDropEnableGet(dev, &state);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "[cpssPxPortPfcGlobalDropEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                        "get another enable value than was set: %d, %d", dev);

        /*
           1.1. Call with state [GT_FALSE and GT_TRUE].
           Expected: GT_OK.
        */

        /* Call function with enable = GT_TRUE] */
        enable = GT_TRUE;

        st = cpssPxPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        /*
           1.2. Call cpssPxPortPfcGlobalDropEnableGet.
           Expected: GT_OK and the same enable.
        */
        st = cpssPxPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "[cpssPxPortPfcGlobalDropEnableGet]: %d, %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                      "get another enable value than was set: %d, %d", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcGlobalDropEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcGlobalDropEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcGlobalDropEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcGlobalDropEnableGet)
{
/*
    1. Go over all active devices.
    1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null statePtr.
           Expected: GT_OK.
        */
        st = cpssPxPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with statePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortPfcGlobalDropEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcGlobalDropEnableGet(dev, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcGlobalDropEnableGet(dev, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxPortPfcGlobalQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U8    tcQueue
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcGlobalQueueConfigSet)
{
/*
    1. Go over all active devices.
    1.1. Call with tcQueue [0 - 7],
                   xoffThreshold [0 - 0x7FF],
                   dropThreshold [0 - 0x7FF].
    Expected: GT_OK.
    1.2. Call cpssPxPortPfcGlobalQueueConfigGet with not-NULL pointers.
    Expected: GT_OK and the same values as was set.
    1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
    Expected: NOT GT_OK.
    1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   tcQueue = 0;
    GT_U32  xoffThreshold = 0;
    GT_U32  xoffThresholdGet = 1;
    GT_U32  dropThreshold = 0;
    GT_U32  dropThresholdGet = 1;
    GT_U32  xonThreshold = 0;
    GT_U32  xonThresholdGet = 1;
    GT_U32  xoffStep;
    GT_U32  xonStep;
    GT_U32  dropStep;
    GT_U8   queueStep;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tcQueue [0 - 7], xoffThreshold [0 - 0x0x1FFFFF], dropThreshold [0 - 0x0x1FFFFF].
            Expected: GT_OK.
        */

        queueStep = 1;
        xoffStep  = PORT_PFC_INVALID_XOFFTHRESHOLD_STEP_MAC;
        xonStep   = PORT_PFC_INVALID_XONTHRESHOLD_STEP_MAC;
        dropStep  = PORT_PFC_INVALID_DROPTHRESHOLD__STEP_MAC;

        if (GT_TRUE == prvUtfReduceLogSizeFlagGet())
        {
            queueStep *= 4;
            xoffStep  *= 4;
            xonStep   *= 4;
            dropStep  *= 4;
        }

        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue += queueStep)
            for(xoffThreshold = 0;
                xoffThreshold < PORT_PFC_INVALID_XOFFTHRESHOLD_MAC;
                xoffThreshold += xoffStep)
                for(dropThreshold = 0;
                    dropThreshold < PORT_PFC_INVALID_DROPTHRESHOLD_MAC;
                    dropThreshold += dropStep)
                    for(xonThreshold = 0;
                        xonThreshold < PORT_PFC_INVALID_XONTHRESHOLD_MAC;
                        xonThreshold += xonStep)

                    {
                        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue,
                                                                 xoffThreshold, dropThreshold, xonThreshold);
                        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                        if(GT_OK == st)
                        {
                            /*
                                1.2. Call cpssPxPortPfcGlobalQueueConfigGet with not-NULL tcQueuePtr.
                                Expected: GT_OK and the same tcQueue as was set.
                            */
                            st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue,
                                                                     &xoffThresholdGet, &dropThresholdGet, &xonThresholdGet);
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssPxPortPfcGlobalQueueConfigGet: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(xoffThreshold, xoffThresholdGet,
                                                         "got another xoffThreshold then was set: %d", dev);
                            UTF_VERIFY_EQUAL1_STRING_MAC(dropThreshold, dropThresholdGet,
                                                         "got another dropThreshold then was set: %d", dev);

                            UTF_VERIFY_EQUAL1_STRING_MAC(xonThreshold, xonThresholdGet,
                                                             "got another xoffThreshold then was set: %d", dev);
                        }
                    }

        /*
            1.3. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        tcQueue = 0;

        /*
            1.4. Call with wrong xoffThreshold [PORT_PFC_INVALID_XOFFTHRESHOLD_CNS].
            Expected: NOT GT_OK.
        */
        xoffThreshold = PORT_PFC_INVALID_XOFFTHRESHOLD_MAC;

        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        xoffThreshold = 0;

        /*
            1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
            Expected: NOT GT_OK.
        */
        dropThreshold = PORT_PFC_INVALID_DROPTHRESHOLD_MAC;

        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        dropThreshold = 0;

        /*
            1.5. Call with wrong dropThreshold [PORT_PFC_INVALID_DROPTHRESHOLD_CNS].
            Expected: NOT GT_OK.
         */
        xonThreshold = PORT_PFC_INVALID_XONTHRESHOLD_MAC;

        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        xonThreshold = 0;
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcGlobalQueueConfigSet(dev, tcQueue, xoffThreshold, dropThreshold, xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcGlobalQueueConfigGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcGlobalQueueConfigGet)
{
/*
    1. Go over all active devices.
    1.1. Call with tcQueue [0 - 7] and  not-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
    Expected: NOT GT_OK.
    1.3. Call with wrong  xoffThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong  dropThresholdPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8      tcQueue = 0;
    GT_U32  xoffThreshold = 0;
    GT_U32  dropThreshold = 0;
    GT_U32  xonThreshold = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL tcQueue.
            Expected: GT_OK.
        */
        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
        {
            st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tcQueue = NULL", dev);

        tcQueue = 0;

        /*
            1.3. Call with wrong xoffThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, NULL, &dropThreshold, &xonThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xoffThresholdPtr = NULL", dev);

        /*
            1.4. Call with wrong dropThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, NULL, &xonThreshold);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dropThresholdPtr = NULL", dev);

        /*
            1.5. Call with wrong dropThresholdPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, xonThresholdPtr = NULL", dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcGlobalQueueConfigGet(dev, tcQueue, &xoffThreshold, &dropThreshold, &xonThreshold);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcTimerMapEnableSet
(
    IN  GT_SW_DEV_NUM                                  devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT      profileSet,
    IN  GT_BOOL                                        enable

);
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcTimerMapEnableSet)
{
/*
    1. Go over all active devices.
    1.1 call with  all possible profile values                                       .
    1.1.1 Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2 Call cpssPxPortPfcTimerMapEnableGet.
    Expected: GT_OK and the same enable.
    1.2. Call function with wrong enum values profile.
    Expected: GT_BAD_PARAM.
    2. Call the function with not active devices
    Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
                                                                          .
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT      profile;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid profile */
        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E; profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E; profile++)
        {
            /*
               1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssPxPortPfcTimerMapEnableSet(dev, profile, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssPxPortPfcGlobalDropEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortPfcTimerMapEnableGet(dev, profile, &state);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "[cpssPxPortPfcTimerMapEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                            "get another enable value than was set: %d, %d", dev);

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssPxPortPfcTimerMapEnableSet(dev, profile, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssPxPortPfcGlobalDropEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortPfcTimerMapEnableGet(dev, profile, &state);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "[cpssPxPortPfcTimerMapEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, state,
                          "get another enable value than was set: %d, %d", dev);
        }
    }
    /*
       1.2. Check wrong enum values
    */
    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
    UTF_ENUMS_CHECK_MAC(cpssPxPortPfcTimerMapEnableSet(dev, profile, enable), profile);

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcTimerMapEnableSet(dev, profile, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcTimerMapEnableSet(dev, profile, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcTimerMapEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profileSet,
    OUT GT_BOOL                                         *enablePtr
);
*/

UTF_TEST_CASE_MAC(cpssPxPortPfcTimerMapEnableGet)
{
/*
    1. Go over all active devices.
    1.1 call with  all possible profile values                                       .
    1.1.1 Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with enable [NULL].
    Expected: GT_BAD_PTR
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_BOOL     state = GT_FALSE;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT      profile;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid profile */
        for (profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E; profile <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E; profile++)
        {
            /* 1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortPfcTimerMapEnableGet(dev, profile, &state);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* 1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortPfcTimerMapEnableGet(dev, profile, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    profile = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcTimerMapEnableGet(dev, profile, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcTimerMapEnableGet(dev, profile, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcTimerToQueueMapSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

);
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcTimerToQueueMapSet)
{
/*
    1. Go over all active devices.
    1.1. Call with pfcTimer [0 .. 7],
                   tcQueue [0 .. 7].
    Expected: GT_OK.
    1.2. Call cpssPxPortPfcTimerToQueueMapGet with the same pfcTimer.
    Expected: GT_OK and the same tcQueue.
    1.3. Call with out of range pfcTimer [8]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      pfcTimer   = 0;
    GT_U32      tcQueue    = 0;
    GT_U32      tcQueueGet = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid pfcTimer  */
       /*
            1.1. Call with pfcTimer [0 / 5 / 7],
                           tcQueue [0 / 5 / 7].
            Expected: GT_OK.
        */
        for(pfcTimer = 0; pfcTimer < PORT_PFC_INVALID_TIMER_CNS; pfcTimer ++)
        {
            for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue ++)
            {
                st = cpssPxPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, pfcTimer, tcQueue);

                /*
                    1.2. Call cpssPxPortPfcTimerToQueueMapGet with the same pfcTimer.
                    Expected: GT_OK and the same tcQueue.
                */
                st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueueGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortPfcTimerToQueueMapGet: %d", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(tcQueue, tcQueueGet,
                           "get another value than was set: %d, %d", dev, tcQueueGet);
            }

        }
        /*
            1.3. Call with out of range pfcTimer [8]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        pfcTimer = PORT_PFC_INVALID_TIMER_CNS;

        st = cpssPxPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);

        pfcTimer = 0;

        /*
            1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssPxPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);
    }

    pfcTimer = 0;
    tcQueue  = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcTimerToQueueMapSet(dev, pfcTimer, tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcTimerToQueueMapGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcTimerToQueueMapGet)
{
/*
    1. Go over all active devices.
    1.1. Call with pfcTimer [0 .. 7],
                   non NULL tcQueuePtr.
    Expected: GT_OK.
    1.2. Call with out of range pfcTimer [8]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with NULL tcQueuePtr and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with out of range pfcTimer [8]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range tcQueue [CPSS_TC_RANGE_CNS]
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      pfcTimer = 0;
    GT_U32      tcQueue  = 0;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with pfcTimer[0..7],
                           non NULL tcQueuePtr.
            Expected: GT_OK.
        */

        for(pfcTimer = 0; pfcTimer < PORT_PFC_INVALID_TIMER_CNS; pfcTimer ++)
        {
            st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, pfcTimer);
        }

        /*
            1.2. Call with out of range pfcTimer [8]
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        pfcTimer = 8;

        st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, pfcTimer);

        pfcTimer = 0;

        /*
            1.3. Call with NULL tcQueuePtr and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, tcQueuePtr = NULL", dev);
    }

    pfcTimer = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcTimerToQueueMapGet(dev, pfcTimer, &tcQueue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcShaperToPortRateRatioSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                      tcQueue,
    IN  GT_U32                                     shaperToPortRateRatio
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcShaperToPortRateRatioSet)
{
/*
    1. Go over all active devices.
    1.1 call with  all possible profile, tcQueue and
        shaperToPortRateRatio values.
    Expected: GT_OK.
    1.2. Call cpssPxPortPfcShaperToPortRateRatioGet with the same profileSet and tcQueue.
    pected: GT_OK and the same shaperToPortRateRatio.
    1.3. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range shaperToPortRateRatio [101],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_U8                                      tcQueue;
    GT_U32                                     ratio;
    GT_U32                                     ratioGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid profile */
        for (profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E; profileSet <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E; profileSet++)
        {
            for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue ++)
            {
                for (ratio = 0; ratio <= 100; ratio++)
                {
                    st = cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue, ratio);

                    /*
                        1.2. Call cpssPxPortPfcShaperToPortRateRatioGet with the same profileSet.
                        Expected: GT_OK and the same shaperToPortRateRatio.
                    */
                    st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratioGet);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortPfcShaperToPortRateRatioGet: %d", dev);

                    /* Verifying values */
                    UTF_VERIFY_EQUAL2_STRING_MAC(ratio, ratioGet,
                               "get another value than was set: %d, %d", dev, ratioGet);
                }
            }
        }



        profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E;
        tcQueue = 0;
        ratio = 0;
        /*
            1.4. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio),
                            profileSet);

        /*
            1.5. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);

        tcQueue = 0;

        /*
            1.6. Call with out of range shaperToPortRateRatio [101],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        ratio = 101;

        st = cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, ratio);
    }

    profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E;
    tcQueue    = 0;
    ratio      = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcShaperToPortRateRatioSet(dev, profileSet, tcQueue, ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcShaperToPortRateRatioGet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U8                                      tcQueue,
    OUT GT_U32                                     *shaperToPortRateRatioPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcShaperToPortRateRatioGet)
{
/*
    1. Go over all active devices.
    1.1 call with  all possible profile, tcQueue and
        not NULL shaperToPortRateRatioPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values profileSet
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                   and other valid parameters from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with NULL shaperToPortRateRatioPtr,
                   and other valid parameters from 1.1.
    Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet;
    GT_U8                                      tcQueue;
    GT_U32                                     ratio;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* go over all valid profile */
        for (profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E; profileSet <= CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E; profileSet++)
        {
            for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue ++)
            {
                st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, profileSet, tcQueue);
            }
        }
        /*
            1.3. Call with wrong enum values profileSet
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PARAM.
        */

        profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E;
        tcQueue = 0;
        UTF_ENUMS_CHECK_MAC(cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio),
                            profileSet);

        /*
            1.4. Call with out of range tcQueue [CPSS_TC_RANGE_CNS],
                           and other valid parameters from 1.1.
            Expected: NOT GT_OK.
        */
        tcQueue = CPSS_TC_RANGE_CNS;

        st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tcQueue);

        tcQueue = 0;

        /*
            1.5. Call with NULL shaperToPortRateRatioPtr,
                           and other valid parameters from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, shaperToPortRateRatioPtr = NULL", dev);
    }

    profileSet = CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E;
    tcQueue    = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcShaperToPortRateRatioGet(dev, profileSet, tcQueue, &ratio);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcForwardEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcForwardEnableSet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPfcForwardEnableGet.
    Expected: GT_OK and the same enable value as was set.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.

*/
    GT_U8                   dev;
    GT_STATUS               st          = GT_OK;
    GT_PHYSICAL_PORT_NUM    port        = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable      = GT_FALSE;
    GT_BOOL                 enableGet   = GT_TRUE;

    /* there is no PFC/QCN in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            enable = GT_FALSE;
            st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssPxPortForwardxEnableGet.
                    Expected: GT_OK and the same enable value as was set.
                */
                st = cpssPxPortPfcForwardEnableGet(dev, port, &enableGet);

                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortPfcForwardEnableGet: %d", dev);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d, %d", dev, port);
            }
            enable = GT_TRUE;
            st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            if(GT_OK == st)
            {
                /*
                    1.1.2. Call cpssPxPortForwardxEnableGet.
                    Expected: GT_OK and the same enable value as was set.
                */
                st = cpssPxPortPfcForwardEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "cpssPxPortPfcForwardEnableGet: %d", dev);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                        "get another enable than was set: %d, %d", dev, port);
            }

         }
        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {

            /* 1.2.1. Call function for each non-active port */
            /* profileIndex = 7    */
            st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_PFC_VALID_PHY_PORT_CNS ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcForwardEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);


}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcForwardEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcForwardEnableGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
    2. Go over all not-active devices and devices from non-applicable family.
       Expected: GT_NOT_APPLICABLE_DEVICE.
    3. Call function with out of bound value of device id.
       Expected: GT_BAD_PARAM.
*/
    GT_U8                   dev;
    GT_STATUS               st      = GT_OK;
    GT_PHYSICAL_PORT_NUM    port    = PORT_PFC_VALID_PHY_PORT_CNS;
    GT_BOOL                 enable  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
           /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortPfcForwardEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPfcForwardEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }


        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortPfcForwardEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPfcForwardEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_PFC_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcForwardEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcForwardEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*

GT_STATUS cpssPxPortPfcPacketClassificationEnableSet
(

    IN GT_SW_DEV_NUM                devNum,
    IN CPSS_PX_PACKET_TYPE          packetType,
    IN GT_U32 		                udbpIndex,
    IN GT_BOOL                      enable
);
 */
UTF_TEST_CASE_MAC(cpssPxPortPfcPacketClassificationEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call with packetType = 0, udbpIndex= 0-3 and enable [GT_FALSE / GT_TRUE].
        Expected: GT_OK.
    1.2. Call cpssPxPortPfcPacketTypeGet.
        Expected: GT_OK and the same packetType.
    1.3 Call
        Expected: GT_OK and the same enable.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                        dev;
    CPSS_PX_PACKET_TYPE          packetType;
    GT_U32                       packetTypeGet;
    GT_U32                       udbpIndex;
    GT_BOOL                      enable,enableGet;
    GT_BOOL                      isEqual;
    GT_U32                       i, port;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData,keyDataGet;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask,keyMaskGet;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey, portKeyGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    packetType = 0;
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

     /*
       1.1. Call with packetType = 0, udbpIndex= 0-3 and enable [GT_FALSE / GT_TRUE].
       Expected: GT_OK.
      */
        for (udbpIndex = 0; udbpIndex <= 3; udbpIndex++)

        {
            enable = GT_FALSE;
            /* Clear keyData & keyMask needed to configure packet type */
            cpssOsMemSet(&keyData, 0, sizeof(keyData));
            cpssOsMemSet(&keyMask, 0, sizeof(keyMask));
            st = cpssPxPortPfcPacketClassificationEnableSet(dev, packetType, udbpIndex, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, udbpIndex,enable);

            /*
              1.2. Call cpssPxPortPfcEnableGet.
               Expected: GT_OK and the same enable options.
             */
            st = cpssPxPortPfcPacketTypeGet(dev, &packetTypeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "[cpssPxPortPfcPacketTypeGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(0x1f, packetTypeGet,
                                         "get another packetType value than was set: %d, %d", dev);
            st = cpssPxIngressPacketTypeKeyEntryEnableGet(dev, packetType, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "[cpssPxIngressPacketTypeKeyEntryEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                                         "get another enable value than was set: %d, %d", dev);

            enable = GT_TRUE;
            keyData.macDa.arEther[0] = 0x01;
            keyData.macDa.arEther[1] = 0x80;
            keyData.macDa.arEther[2] = 0xC2;
            keyData.macDa.arEther[3] = 0x0;
            keyData.macDa.arEther[4] = 0x0;
            keyData.macDa.arEther[5] = 0x01;
            keyData.etherType = 0x8808;
            keyMask.etherType = 0xFFFF;
            for (i = 0; i < 6; i++)
            {
                keyMask.macDa.arEther[i] = 0xFF;
            }

            keyData.udbPairsArr[udbpIndex].udb[0] = 0x01;
            keyData.udbPairsArr[udbpIndex].udb[1] = 0x01;

            keyMask.udbPairsArr[udbpIndex].udb[0] = 0xFF;
            keyMask.udbPairsArr[udbpIndex].udb[1] = 0xFF;

            portKey.portUdbPairArr[udbpIndex].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[udbpIndex].udbByteOffset = 2;

            st = cpssPxPortPfcPacketClassificationEnableSet(dev, packetType, udbpIndex, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, udbpIndex,enable);

            /*
              1.2. Call cpssPxPortPfcEnableGet.
               Expected: GT_OK and the same enable options.
             */
            st = cpssPxPortPfcPacketTypeGet(dev, &packetTypeGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "[cpssPxhPortPfcEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(packetType, packetTypeGet,
                                         "get another packetType value than was set: %d, %d", dev);

            st = cpssPxIngressPacketTypeKeyEntryGet (dev, packetTypeGet, &keyDataGet, &keyMaskGet);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, packetType);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(keyData.etherType, keyDataGet.etherType,
                       "get another keyDataGet.etherType than was set: %d", keyDataGet.etherType);
            UTF_VERIFY_EQUAL1_STRING_MAC(keyMask.etherType, keyMaskGet.etherType,
                       "get another keyMaskGet.etherType than was set: %d", keyMaskGet.etherType);


            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyData.macDa, (GT_VOID*)&keyDataGet.macDa, sizeof(GT_ETHERADDR)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another keyDataGet.macDa than was set: %d", dev);
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyMask.macDa, (GT_VOID*)&keyMaskGet.macDa, sizeof(GT_ETHERADDR)))
                      ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another keyMaskGet.macDa than was set: %d", dev);



            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyData.udbPairsArr,
                                         (GT_VOID*)&keyDataGet.udbPairsArr,
                                         CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC)))
                                         ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another keyDataGet.udbPairsArr than was set: %d", dev);
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)&keyMask.udbPairsArr,
                                         (GT_VOID*)&keyMaskGet.udbPairsArr,
                                         CPSS_PX_UDB_PAIRS_MAX_CNS * sizeof(CPSS_PX_UDB_PAIR_DATA_STC)))
                                         ? GT_TRUE : GT_FALSE;

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                                         "get another keyMask.udbPairsArr than was set: %d", dev);

            for (port = 0; port < 16; port++)
            {
                st = cpssPxIngressPortPacketTypeKeyGet(dev, port, &portKeyGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                UTF_VERIFY_EQUAL3_STRING_MAC(portKey.portUdbPairArr[udbpIndex].udbAnchorType,
                                             portKeyGet.portUdbPairArr[udbpIndex].udbAnchorType,
                                             "get another portUdbPairArr[%d].udbAnchorType than was set: %d, %d", udbpIndex, dev, port);
                UTF_VERIFY_EQUAL3_STRING_MAC(portKey.portUdbPairArr[udbpIndex].udbByteOffset,
                                             portKeyGet.portUdbPairArr[udbpIndex].udbByteOffset,
                                             "get another portUdbPairArr[%d].udbByteOffset than was set: %d, %d", udbpIndex, dev, port);
            }

        }
        /*
            1.4. Call with out of range udbpIndex
             Expected: NOT GT_OK.
        */
        udbpIndex = 4;

        st = cpssPxPortPfcPacketClassificationEnableSet(dev, packetType, udbpIndex, GT_TRUE);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, udbpIndex);
    }


    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    udbpIndex = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcPacketClassificationEnableSet(dev, packetType, udbpIndex, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcPacketClassificationEnableSet(dev, packetType, udbpIndex, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcSourcePortToPfcCounterSet)
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call with pfcCounterNum [0 / 127].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPfcSourcePortToPfcCounterGet with the same params.
    Expected: GT_OK and the same profileIndex.
    1.1.3. Call with out of range pfcCounterNum [256]
    Expected: NOT GT_OK
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM

*/
{

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_PFC_VALID_PHY_PORT_CNS ;
    GT_U32      pfcCounterNum = 0;
    GT_U32      pfcCounterNumGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
                Call with pfcCounter [0 ..127.
                Expected: GT_OK.*/
            for(pfcCounterNum = 0;pfcCounterNum < 127;(pfcCounterNum += 16))
            {
                st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pfcCounterNum);

                if(GT_OK == st)
                {
                    /*
                       1.1.2. Call cpssPxPortPfcSourcePortToPfcCounterGet.
                       Expected: GT_OK and the same pfcCounter.
                    */
                    st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNumGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                    "[cpssPxPortPfcProfileIndexGet]: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(pfcCounterNum, pfcCounterNumGet,
                                    "get another profileIndex value than was set: %d, %d", dev, port);
                }
            }

        }
        /*
            1.1.3. Call with out of range pfcCounterNum [256]
                      Expected: NOT GT_OK
        */
        pfcCounterNum   = BIT_7;

        st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, limit = %d", dev, port, pfcCounterNum);

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        pfcCounterNum = 7;
        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {

            /* 1.2.1. Call function for each non-active port */
            /* pfcCounterNum = 7    */
            st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        /* avbModeEnable == GT_TRUE    */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        /* pfcCounterNum = 7    */
        st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_PFC_VALID_PHY_PORT_CNS ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* pfcCounterNum = 7    */
        st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, port);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcSourcePortToPfcCounterSet(dev, port, pfcCounterNum);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_U32  *profileIndexPtr
)
*/


UTF_TEST_CASE_MAC(cpssPxPortPfcSourcePortToPfcCounterGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL profileIndexPtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    1.3. Call function with of bound value for port number.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port;
    GT_U32     pfcCounterNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextTxqPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call function with non-NULL enablePtr and retAvbModeEnable.
               Expected: GT_OK.
            */
            st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, pfcCounterNum);

            /*
               1.1.2. Call function with enablePtr [NULL] .
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);


        }

        /* 1.2. For all active devices go over all non available physical ports. */

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while (GT_OK == prvUtfNextTxqPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port    */
            /* enable == GT_TRUE                                */
            st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_PFC_VALID_PHY_PORT_CNS ;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcSourcePortToPfcCounterGet(dev, port, &pfcCounterNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcCounterGet)
{
/*
    1. Go over all active devices.
    1.1. Call with tcQueue [0 - 7] and pfcCounterNum [0 - 17]
    1.1.1. Call function with non-NULL profileIndexPtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8   tcQueue = 0;
    GT_U32  pfcCounterNum = 0;
    GT_U32  pfcCounterValue;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
            for(pfcCounterNum = 0; pfcCounterNum <  17; pfcCounterNum++)
            {
                st = cpssPxPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tcQueue, pfcCounterNum);
            }

        /* call with NULL pointer */
        st = cpssPxPortPfcCounterGet(dev, tcQueue, pfcCounterNum, NULL);
        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, tcQueue, pfcCounterNum);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcCounterGet(dev, tcQueue, pfcCounterNum, &pfcCounterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcQueueCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   tcQueue,
    OUT GT_U32  *cntPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcQueueCounterGet)
{
/*
    1. Go over all active devices.
    1.1. Call with tcQueue [0 - 7]
    1.1.1. Call function with non-NULL cntPtr
    Expected: GT_OK.
    1.1.2. Call function with cntPtr [NULL] .
    Expected: GT_BAD_PTR.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_U8       tcQueue = 0;
    GT_U32      cnt = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(tcQueue = 0; tcQueue < PORT_PFC_INVALID_TCQUEUE_CNS; tcQueue++)
        {
            st = cpssPxPortPfcQueueCounterGet(dev, tcQueue, &cnt);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call with wrong tcQueue [PORT_PFC_INVALID_TCQUEUE_CNS].
            Expected: NOT GT_OK.
        */
        tcQueue = PORT_PFC_INVALID_TCQUEUE_CNS;

        st = cpssPxPortPfcQueueCounterGet(dev, tcQueue, &cnt);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st, "%d, tcQueue = NULL", dev);

        tcQueue = 0;

        /*
            1.3. Call with wrong cnt [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortPfcQueueCounterGet(dev, tcQueue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, cnt = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcQueueCounterGet(dev, tcQueue, &cnt);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcQueueCounterGet(dev, tcQueue, &cnt);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssPxPortPfcXonMessageFilterEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcXonMessageFilterEnableSet)
{
/*
    1. Go over all active devices.
    1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK.
    1.2. Call cpssPxPortPfcXonMessageFilterEnableGet.
        Expected: GT_OK and the same enable.
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/
    GT_STATUS   st = GT_OK;

    GT_U8   dev;
    GT_BOOL enable;
    GT_BOOL enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (enable = GT_TRUE; enable <= GT_FALSE; enable++)
        {
             /*
               1.1. Call with enable.
               Expected: GT_OK.
            */

            st = cpssPxPortPfcXonMessageFilterEnableSet(dev, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

            /*
               1.2. Call cpssPxPortPfcXonMessageFilterEnableGet.
               Expected: GT_OK and the same enable options.
            */
            st = cpssPxPortPfcXonMessageFilterEnableGet(dev, &enableGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "[cpssPxPortPfcXonMessageFilterEnableGet]: %d, %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                            "get another enable value than was set: %d, %d", dev);
        }
    }


    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_FALSE;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcXonMessageFilterEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcXonMessageFilterEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPfcXonMessageFilterEnableGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL  *enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPfcXonMessageFilterEnableGet)
{
/*
    1. Go over all active devices.
    1.1. ITERATE_DEVICES_PHY_PORTS (PIPE)
    1.1.1. Call function with non-NULL enablePtr
    Expected: GT_OK.
    1.1.2. Call function with enablePtr [NULL] .
    Expected: GT_BAD_PTR.
    1.2. For all active devices go over all non available physical ports.
        Expected: GT_BAD_PARAM
    2. Call the function with not active devices
        Expected: GT_NOT_APPLICABLE_DEVICE
    3. Call function with out of bound value for device id
        Expected: GT_BAD_PARAM
*/

    GT_STATUS   st    = GT_OK;

    GT_U8   dev;
    GT_BOOL enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssPxPortPfcXonMessageFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.2. Call with enablePtr [NULL].
           Expected: GT_BAD_PTR
        */
        st = cpssPxPortPfcXonMessageFilterEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPfcXonMessageFilterEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPfcXonMessageFilterEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortTx suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortPfc)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcProfileIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcProfileIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcDbaAvailableBuffersSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcDbaAvailableBuffersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcProfileQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcProfileQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcCountingModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcCountingModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcGlobalDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcGlobalDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcGlobalQueueConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcGlobalQueueConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcSourcePortToPfcCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcSourcePortToPfcCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcTimerMapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcTimerMapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcTimerToQueueMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcTimerToQueueMapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcShaperToPortRateRatioSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcShaperToPortRateRatioGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcForwardEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcForwardEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcPacketClassificationEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcQueueCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcXonMessageFilterEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPfcXonMessageFilterEnableGet)



UTF_SUIT_END_TESTS_MAC(cpssPxPortPfc)



