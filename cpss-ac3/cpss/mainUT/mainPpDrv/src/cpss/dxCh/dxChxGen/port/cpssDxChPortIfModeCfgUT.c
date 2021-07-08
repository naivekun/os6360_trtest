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
* @file cpssDxChPortIfModeCfgUT.c
*
* @brief Unit tests for cpssDxChPortIfModeCfg, that provides
* CPSS implementation for Port interface mode configuration.
*
* @version   7
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>

/* defines */
/* Default valid value for port id */
#define PORT_VALID_PHY_PORT_CNS  0

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

extern GT_STATUS appDemoIsTmSupported
(
    IN   GT_U8 devNum,
    OUT  GT_BOOL *isTmSupported
);

typedef struct
{
     GT_PHYSICAL_PORT_NUM portNumArr[16];
     GT_BOOL              extModeStateArr[16];
     GT_U32               size;
     GT_U32               len;
}portExtModeStateList_STC;

GT_STATUS portExtModeStateList_Init
(
    INOUT portExtModeStateList_STC *thisPtr
)
{
    GT_STATUS   st = GT_OK;

    if (thisPtr == NULL)
    {
        st = GT_BAD_PTR;
    }
    else
    {
        cpssOsMemSet((GT_VOID *)thisPtr,0,sizeof(portExtModeStateList_STC));
        thisPtr->size = sizeof(thisPtr->portNumArr);
        thisPtr->len = 0;
    }
    return st;
}

GT_STATUS portExtModeStateList_LenGet
(
    INOUT portExtModeStateList_STC  *thisPtr,
    OUT GT_U32                      *lenPtr
)
{
    GT_STATUS   st = GT_OK;

    if (thisPtr == NULL)
    {
        st = GT_BAD_PTR;
    }
    else
    {
        *lenPtr = thisPtr->len;
    }
    return st;
}

GT_STATUS portExtModeStateList_Append
(
    INOUT portExtModeStateList_STC  *thisPtr,
    IN    GT_PHYSICAL_PORT_NUM      port,
    IN    GT_BOOL                   state
)
{
    GT_STATUS   st = GT_OK;

    if (thisPtr == NULL)
    {
        st = GT_BAD_PTR;
    }
    else
    {
         if (thisPtr->len >= thisPtr->size)
         {
             st = GT_NO_RESOURCE;
         }
         else
         {
             thisPtr->portNumArr[thisPtr->len] = port;
             thisPtr->extModeStateArr[thisPtr->len] = state;
             thisPtr->len++;
         }
    }
    return st;
}

GT_STATUS portExtModeStateList_Get
(
    IN portExtModeStateList_STC *thisPtr,
    IN GT_U32                   index,
    OUT GT_PHYSICAL_PORT_NUM    *port,
    OUT GT_BOOL                 *state
)
{
    GT_STATUS st = GT_FAIL;

    if(index < thisPtr->len)
    {
        *port = (thisPtr->portNumArr)[index];
        *state = (thisPtr->extModeStateArr)[index];
        st = GT_OK;
    }

    return st;
}

GT_STATUS portExtModeStateList_IndexGet
(
    IN portExtModeStateList_STC *thisPtr,
    IN GT_PHYSICAL_PORT_NUM     port,
    OUT GT_U32                  *index
)
{
    GT_U32 ind;

    if (thisPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    ind = 0;
    while (thisPtr->portNumArr[ind] != port)
    {
        ind++;
        if (ind > thisPtr->len)
        {
            return GT_FAIL;
        }
    }
    *index = ind;
    return GT_OK;
}

GT_STATUS portExtModeStateList_SetStateArr
(
    IN portExtModeStateList_STC *thisPtr,
    IN GT_BOOL                   state
)
{
    GT_U32 index;

    if (thisPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    for (index =0; index < thisPtr->len; index++)
    {
        thisPtr->extModeStateArr[index] = state;
    }
    return GT_OK;
}

/*--------------------------------------------------------------------------*/
/* Build port extended mode state list */
GT_STATUS prvBuildPortExtModeStateList
(
    GT_U8                       dev,
    GT_BOOL                     enable,
    IN portExtModeStateList_STC *portExtModeStateListPtr
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  localPort; /* number of port in local core */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_STATUS               st = GT_OK;

    st = portExtModeStateList_Init(/*INOUT*/portExtModeStateListPtr);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if(GT_OK != st)
    {
       return st;
    }

    st = prvUtfDeviceFamilyGet(dev, &devFamily);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if(GT_OK != st)
    {
         return st;
    }

    st = prvUtfNextPhyPortReset(&portNum, dev);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if(GT_OK != st)
    {
       return st;
    }

    while (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE))
    {
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev, portNum);

        /* Check if port can be in exteneded mode */
        if (((devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)&& ((localPort == 25) || (localPort == 27)))||
          ((devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&& ((localPort == 9) || (localPort == 11))))
        {
            /* Append port to extended mode sate list */
            st = portExtModeStateList_Append(/*INOUT*/portExtModeStateListPtr, /*IN*/portNum, /*IN*/enable);
            if (GT_OK != st)
            {
                return st;
            }
        }
    }
    return st;
}

/*--------------------------------------------------------------------------*/
/* Get ports extended mode state */
GT_STATUS prvGetPortsExtendedModeState
(
    GT_U8                       dev,
    IN portExtModeStateList_STC *portExtModeStateListPtr
)
{
    GT_U32      index;
    GT_BOOL     enableGet;
    GT_STATUS   st = GT_OK;

    for ( index = 0; index < portExtModeStateListPtr->len; index++)
    {
        st = cpssDxChPortExtendedModeEnableGet(dev, portExtModeStateListPtr->portNumArr[index], &enableGet);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"cpssDxChPortExtendedModeEnableGet: %d, %d",
                                               dev, portExtModeStateListPtr->portNumArr[index]);
        if(GT_OK != st)
        {
            break;
        }
        portExtModeStateListPtr->extModeStateArr[index] = enableGet;
    }
    return st;
}

/*--------------------------------------------------------------------------*/
/* Set ports extended mode state */
GT_STATUS prvSetPortsExtendedModeState
(
    GT_U8                       dev,
    IN portExtModeStateList_STC *portExtModeStateListPtr
)
{
    GT_U32                index = 0;
    GT_PHYSICAL_PORT_NUM  port = 0;
    GT_BOOL               enable = GT_FALSE, enableGet = GT_FALSE;
    GT_STATUS             st = GT_OK;

    for ( index = 0; index < portExtModeStateListPtr->len; index++)
    {
        /* Get port and state to config */
        st = portExtModeStateList_Get(/*IN*/portExtModeStateListPtr, /*IN*/index, /*OUT*/&port, /*OUT*/&enable);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st,"portExtModeStateList_Get: index = %d", index);
        if(GT_OK != st)
        {
            break;
        }

        /* Enable/Disable extended mode on port */
        st = cpssDxChPortExtendedModeEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st,
                                               "cpssDxChPortExtendedModeEnableSet: dev = %d, port = %d enable = %d",
                                               dev, port, enable);
        if(GT_OK != st)
        {
            break;
        }

        /* Check setting state */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
                                               "cpssDxChPortExtendedModeEnableGet: dev = %d, port = %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet,
                                               "get another enable value than was set: dev = %d, port = %d", dev, port);
        if(GT_OK != st)
        {
            break;
        }
    }

    return st;
}

/*--------------------------------------------------------------------------*/
/* Check Setting Enable/Disable exteded mode on ports */
GT_STATUS prvCheckPortsExtendedModeState
(
    GT_U8       dev,
    IN portExtModeStateList_STC *portExtModeStateListPtr

)
{
    GT_U32                index;
    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL               enable, enableGet;
    GT_STATUS             st = GT_OK;

    port = 0;
    enable = GT_FALSE;
    enableGet = GT_FALSE;

    for ( index = 0; index < portExtModeStateListPtr->len; index++)
    {
        /* Get port and extended mode to config */
        st = portExtModeStateList_Get(/*IN*/portExtModeStateListPtr, /*IN*/index, /*OUT*/&port, /*OUT*/&enable);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st,"portExtModeStateList_Get: index = %d", index);
        if(GT_OK != st)
        {
            break;
        }

        /* Check port's extended mode */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, &enableGet);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,
                                               "cpssDxChPortExtendedModeEnableGet: dev = %d, port = %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet,
                                              "current state of extended mode diffenent than was set: port = %d, currentState = %d",
                                               dev, port);
        if(GT_OK != st)
        {
            break;
        }
    }
    return st;
}

/*--------------------------------------------------------------------------*/
/* Check Enable/Disable exteded mode on all available physical ports */
GT_STATUS prvCheckPortsExtendedModeSet
(
    GT_U8                       dev,
    GT_BOOL                     enable,
    IN portExtModeStateList_STC *portExtModeStateListPtr
)
{
    GT_PHYSICAL_PORT_NUM    port;
    GT_U32                  localPort;  /* number of port in GOP */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;  /* device family */
    GT_U32                  index = 0;
    GT_STATUS               st = GT_OK;

    st = prvUtfDeviceFamilyGet(dev, &devFamily);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if(GT_OK != st)
    {
         return st;
    }

    st = prvUtfNextPhyPortReset(&port, dev);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);
    if(GT_OK != st)
    {
         return st;
    }

    while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
    {
       localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev, port);

        /* Enable/Disable extended mode on port */
        st = cpssDxChPortExtendedModeEnableSet(dev, port, enable);

         /* Check if port is can be in exteneded mode */
        if (((devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)&& ((localPort != 25) && (localPort != 27)))||
          ((devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&& ((localPort != 9) && (localPort != 11))))
        {
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_BAD_PARAM, st, "cpssDxChPortExtendedModeEnableSet on port = %d", port);
            if(GT_BAD_PARAM != st)
            {
                return GT_FAIL;
            }
            else
            {
                continue;
            }
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChPortExtendedModeEnableSet on port = %d", port);
            if(GT_OK != st)
            {
                return st;
            }

            /* Get index of this port in the extModeStateArr[] array */
            st = portExtModeStateList_IndexGet(/*IN*/portExtModeStateListPtr, /*IN*/port, /*OUT*/&index);
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st, "portExtModeStateList_IndexGet: port = %d", port);
            if(GT_OK != st)
            {
                return st;
            }

            portExtModeStateListPtr->extModeStateArr[index] = enable;

            /* Check if exteneded mode on ports set as expected */
            st = prvCheckPortsExtendedModeState(/*IN*/dev, /*IN*/portExtModeStateListPtr);
            UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st, "prvCheckPortsExtendedModeState: dev  = %d", dev);
            if(GT_OK != st)
            {
                return st;
            }

            /* Revert state on this port */
            portExtModeStateListPtr->extModeStateArr[index] = !enable;
            st = cpssDxChPortExtendedModeEnableSet(dev, port, !enable);
            if(GT_OK != st)
            {
                return st;
            }
        }
    }
    return st;
}

/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssDxChPortExtendedModeEnableSet) */
GT_VOID cpssDxChPortExtendedModeEnableSetUT(GT_VOID)
{
/*
    1. Go over all active devices and check configuration of
       cpssDxChPortExtendedModeEnableSet with state [GT_FALSE and GT_TRUE],
       all available physical ports.
    2. For not active devices check
       cpssDxChPortExtendedModeEnableSet returns non GT_OK
    3. check cpssDxChPortExtendedModeEnableSet with
       out of bound value for device id rerurns GT_BAD_PARAM.
*/

    GT_STATUS                           st = GT_OK;
    GT_U8                               devNum;
    static portExtModeStateList_STC     currentPortExtModeStateList;
    static portExtModeStateList_STC     portExtModeStateList;
    CPSS_PP_FAMILY_TYPE_ENT             devFamily;
    GT_BOOL                             enable;
    GT_PHYSICAL_PORT_NUM                port;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(devNum, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* Saving the current extended mode of ports */
        enable = GT_FALSE;
        st = prvBuildPortExtModeStateList(devNum, /*IN*/enable, /*OUT*/&currentPortExtModeStateList);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvBuildPortExtModeStateList: dev = %d enable = %d", devNum, enable);
        st = prvGetPortsExtendedModeState(devNum, /*INOUT*/&currentPortExtModeStateList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvGetPortsExtendedModeState: dev = %d", devNum);

        /* 1.1 Check enable exteded mode */
        /* Disableing exteded mode on all available physical ports */
        st = prvBuildPortExtModeStateList(devNum, /*IN*/enable, /*OUT*/&portExtModeStateList);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvBuildPortExtModeStateList: dev = %d enable = %d", devNum, enable);
        st = prvSetPortsExtendedModeState(devNum, /*INOUT*/&portExtModeStateList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvSetPortsExtendedModeState: dev = %d", devNum );

        /* Checking enableing exteded mode on all available physical ports */
        enable = GT_TRUE;
        st = prvCheckPortsExtendedModeSet(devNum , /*IN*/enable, /*IN*/&portExtModeStateList);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCheckPortsExtendedModeSet: dev = %d enable = %d", devNum, enable);

        /* 1.2 Checking disable extneded mode */
        /* Enableing extneded mode in all available physical ports */
        st = portExtModeStateList_SetStateArr(&portExtModeStateList, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"portExtModeStateList_SetStateArr: enable = %d", enable);
        st = prvSetPortsExtendedModeState(devNum, &portExtModeStateList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvSetPortsExtendedModeState: dev = %d", devNum );

       /* Checking disableing extneded mode on all available physical ports */
        enable = GT_FALSE;
        st = prvCheckPortsExtendedModeSet(devNum , enable, &portExtModeStateList);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCheckPortsExtendedModeSet: dev = %d enable = %d", devNum, enable);

        /* Restore previous ports extended mode */
        st = prvSetPortsExtendedModeState(devNum, &currentPortExtModeStateList);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvSetPortsExtendedModeState: dev = %d", devNum );
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E |
                                     UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E |
                                     UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortExtendedModeEnableSet(devNum, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssDxChPortExtendedModeEnableSet(devNum, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssDxChPortExtendedModeEnableGet) */
GT_VOID cpssDxChPortExtendedModeEnableGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_PHYSICAL_PORT_NUM  port;
    GT_BOOL     enable    = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                            UTF_CH2_E | UTF_CH3_E |
                                            UTF_XCAT_E | UTF_LION_E |
                                            UTF_XCAT2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            port = 25;
        }
        else
        {
            port = 9;
        }

        /* 1.1.1 */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        /* 1.1.2 */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        if(devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            port = 27;
        }
        else
        {
            port = 11;
        }

        /* 1.1.1 */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        /* 1.1.2 */
        st = cpssDxChPortExtendedModeEnableGet(dev, port, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                            UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortExtendedModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortExtendedModeEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



extern GT_STATUS appDemoAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
);




GT_BOOL prv_PORT_MODE_SET_UT_DEBUG_PRINT = GT_FALSE;
GT_STATUS PORT_MODE_SET_UT_DEBUG_PRINT_SET(GT_U32 status)
{
    if (status == 0)
    {
        prv_PORT_MODE_SET_UT_DEBUG_PRINT = GT_FALSE;
    }
    else
    {
        prv_PORT_MODE_SET_UT_DEBUG_PRINT = GT_TRUE;
    }
    return GT_OK;
}

GT_STATUS cpssDxChPortModeSpeedSetUT_ResourcePrint(GT_U8 dev)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_STATUS st;

    st = prvUtfDeviceFamilyGet(dev, &devFamily);
    if (GT_OK != st)
    {
        return st;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        appDemoAldrinPortGobalResourcesPrint(dev);
    }
    return GT_OK;
}



typedef struct
{
    GT_BOOL                         powerUp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
}UTF_SPEED_IF_CONF_STC;


UTF_SPEED_IF_CONF_STC prv_speed_If_List[] =
{
      { GT_FALSE, CPSS_PORT_INTERFACE_MODE_SGMII_E,        CPSS_PORT_SPEED_1000_E  }
     ,{ GT_FALSE, CPSS_PORT_INTERFACE_MODE_SGMII_E,        CPSS_PORT_SPEED_1000_E  }  /* repeat twice */
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E, CPSS_PORT_SPEED_100_E   }
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,   CPSS_PORT_SPEED_1000_E  }
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_QSGMII_E,       CPSS_PORT_SPEED_40000_E }
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,  CPSS_PORT_SPEED_16000_E }
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_HGL_E,          CPSS_PORT_SPEED_75000_E }
     ,{ GT_TRUE,  CPSS_PORT_INTERFACE_MODE_ILKN12_E,       CPSS_PORT_SPEED_100G_E  }
};
/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssDxChPortModeSpeedSet) */
GT_VOID cpssDxChPortModeSpeedSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS
    1.1 Call with correct parameters
                portsBmp,
                powerUp,
                ifMode,
                speed.
    Expected: GT_OK.
    1.2 Call with out of range ifMode.
    Expected: GT_BAD_PARAM.
    1.3 Call with out of range speed.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_PORTS_BMP_STC              portsBmp;
    GT_BOOL                         powerUp   = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode    = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
    CPSS_PORT_SPEED_ENT             speed     = CPSS_PORT_SPEED_10_E;
    CPSS_PORT_INTERFACE_MODE_ENT    ifModeOld;
    CPSS_PORT_SPEED_ENT             speedOld;
    GT_BOOL                         supported;
    GT_U32                          portNum;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    GT_U32 i;
    GT_U32 tempPortNum;

    static PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC groupResorcesStatus;
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC *groupResorcesStatusPtr;

    /* set value to avoid compilation warnings  */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);
    if (GT_TRUE == prvUtfIsGmCompilation())
    {
        PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(
            UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E , CPSS-6062);
    }

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);


        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if ( (CPSS_PP_FAMILY_DXCH_BOBCAT3_E == PRV_CPSS_PP_MAC(dev)->devFamily) ||
                 (CPSS_PP_FAMILY_DXCH_ALDRIN2_E == PRV_CPSS_PP_MAC(dev)->devFamily) )
            {
                tempPortNum=portNum;
                if (!(tempPortNum%4==0) && ((portNum<24) || ((portNum>=256) && (portNum<(256+24)))))
                {
                    continue;
                }
            }
            st = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }
            if (PRV_CPSS_SIP_5_CHECK_MAC(dev)) /* BC2 and higher */
            {
                groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;

                groupResorcesStatus = *groupResorcesStatusPtr;
            }

            if (prv_PORT_MODE_SET_UT_DEBUG_PRINT == GT_TRUE)
            {
                cpssOsPrintf("\nStart Processing port %d",portNum);
                st = cpssDxChPortModeSpeedSetUT_ResourcePrint(dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortModeSpeedSetUT_ResourcePrint: %d", dev);
            }


            st = prvUtfDeviceFamilyGet(dev, &devFamily);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

            if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == devFamily) &&
                (CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E == PRV_CPSS_PP_MAC(dev)->devSubFamily))
            {
                if(CPSS_NULL_PORT_NUM_CNS == portMap.portMap.macNum)
                {
                    continue;
                }
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

            st = cpssDxChPortInterfaceModeGet(dev, portNum, &ifModeOld);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

            st = cpssDxChPortSpeedGet(dev, portNum, &speedOld);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);

            for (i = 0 ; i < sizeof(prv_speed_If_List)/sizeof(prv_speed_If_List[0]); i++)
            {
                powerUp   = prv_speed_If_List[i].powerUp;
                ifMode    = prv_speed_If_List[i].ifMode;
                speed     = prv_speed_If_List[i].speed;

                if (prv_PORT_MODE_SET_UT_DEBUG_PRINT == GT_TRUE)
                {
                    static GT_CHAR *powerUpStr[] =
                    {
                        "Dn", "Up"
                    };
                    cpssOsPrintf("\n   Power %s IF=%s Speed=%s",powerUpStr[powerUp],CPSS_IF_2_STR(ifMode), CPSS_SPEED_2_STR(speed));
                }
                st = cpssDxChPortInterfaceSpeedSupportGet(dev, portNum, ifMode,
                                                                speed, &supported);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceSpeedSupportGet dev=%d portNum=%d speed=%s If=%s",dev, portNum, CPSS_IF_2_STR(ifMode), CPSS_SPEED_2_STR(speed));

                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed);
                if(supported)
                {

                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, ifMode, speed);

                    if (prv_PORT_MODE_SET_UT_DEBUG_PRINT == GT_TRUE)
                    {
                        st = cpssDxChPortModeSpeedSetUT_ResourcePrint(dev);
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortModeSpeedSetUT_ResourcePrint: %d", dev);
                    }

                }
                else
                {
                    if (powerUp == GT_TRUE)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
                    }
                }
            }
            /* 1.2 */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed),
                                                         ifMode);

            /* 1.3 */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed),
                                                          speed);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifModeOld) ||
                                            (CPSS_PORT_SPEED_NA_E == speedOld))
            {
                continue;
            }

            st = cpssDxChPortInterfaceSpeedSupportGet(dev, portNum, ifModeOld,
                                                        speedOld, &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, portNum);
            if (ifModeOld != CPSS_PORT_INTERFACE_MODE_NA_E )
            {
                st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE, ifModeOld, speedOld);
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, portNum, ifModeOld, speedOld);
            }

            /* check that total resource at beginning of port test and at the end are same */
            if (PRV_CPSS_SIP_5_CHECK_MAC(dev))  /* BC2 and higher */
            {
                groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;
                st = cpssOsMemCmp(&groupResorcesStatus,groupResorcesStatusPtr,sizeof(groupResorcesStatus));
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "global resource comparison failed dev %d port %d",dev, portNum);
            }

            if (prv_PORT_MODE_SET_UT_DEBUG_PRINT == GT_TRUE)
            {
                st = cpssDxChPortModeSpeedSetUT_ResourcePrint(dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortModeSpeedSetUT_ResourcePrint: %d", dev);
                cpssOsPrintf("\nEnd Processing port %d",portNum);
            }

        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortModeSpeedSet(dev, &portsBmp, powerUp, ifMode, speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
)
 */
UTF_TEST_CASE_MAC(cpssDxChPortFecModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2, SIP5)
    1.1.1. Call with valid modePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st          =   GT_OK;
    GT_U8                       devNum      =   0;
    GT_PHYSICAL_PORT_NUM        portNum     =   PORT_VALID_PHY_PORT_CNS;
    CPSS_DXCH_PORT_FEC_MODE_ENT mode        = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
    GT_BOOL                     supported;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                      CPSS_PORT_INTERFACE_MODE_KR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            if(!supported)
            {
                continue;
            }

            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            st = cpssDxChPortSpeedGet(devNum, portNum, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) ||
                                                (CPSS_PORT_SPEED_NA_E == speed))
            {/* port not initialized - skip */
                continue;
            }
            /*
                1.1.1. Call with valid modePtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortFecModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFecModeGet(devNum, portNum, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
)
 */
UTF_TEST_CASE_MAC(cpssDxChPortFecModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2, SIP5)
    1.1.1. Call with mode [CPSS_DXCH_PORT_FEC_MODE_ENABLED_E/
                           CPSS_DXCH_PORT_FEC_MODE_DISABLED_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChPortFecModeGet().
    Expected: GT_OK and same mode as was set in 1.1.1.
    1.1.3. Call with mode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st      = GT_OK;
    GT_U8                       devNum  = 0;
    GT_PHYSICAL_PORT_NUM        portNum = PORT_VALID_PHY_PORT_CNS;

    CPSS_DXCH_PORT_FEC_MODE_ENT mode    = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
    CPSS_DXCH_PORT_FEC_MODE_ENT modeGet = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
    GT_BOOL                     supported;
    CPSS_PORTS_BMP_STC              portsBmp;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;

    /* there is no MAC/PCS/SERDES/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            CPSS_TBD_BOOKMARK_FALCON
            SKIP_TEST_MAC;
        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
            {
                st = cpssDxChPortFecModeSet(devNum, portNum, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
                continue;
            }

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                      CPSS_PORT_INTERFACE_MODE_KR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            if(!supported)
            {
                continue;
            }

            st = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPortSpeedGet(devNum, portNum, &speed);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

            st = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum, CPSS_PORT_SPEED_10000_E);
            (GT_VOID)prvCpssDxChPortPizzaArbiterIfDelete(devNum, portNum);
            if(st != GT_OK)
            {
                goto restoreFecTest;
            }

            st = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                          CPSS_PORT_INTERFACE_MODE_KR_E,
                                          CPSS_PORT_SPEED_10000_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.1.1. Call with mode [CPSS_DXCH_PORT_FEC_MODE_ENABLED_E].
                Expected: GT_OK.
            */
            mode = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;

            st = cpssDxChPortFecModeSet(devNum, portNum, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortFecModeGet().
                Expected: GT_OK and same mode as was set in 1.1.1.
            */
            st = cpssDxChPortFecModeGet(devNum, portNum, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeGet, devNum, portNum);

            /*
                1.1.1. Call with mode [CPSS_DXCH_PORT_FEC_MODE_DISABLED_E].
                Expected: GT_OK.
            */
            mode = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;

            st = cpssDxChPortFecModeSet(devNum, portNum, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call cpssDxChPortFecModeGet().
                Expected: GT_OK and same mode as was set in 1.1.1.
            */
            st = cpssDxChPortFecModeGet(devNum, portNum, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /* verifying values */
            UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeGet, devNum, portNum);

restoreFecTest:
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                                              ifMode, speed);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            }

            /*
                1.1.3. Call with mode[wrong enum values].
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortFecModeSet
                                (devNum, portNum, modeGet),
                                modeGet);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortFecModeSet(devNum, portNum, modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortFecModeSet(devNum, portNum, modeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortFecModeSet(devNum, portNum, modeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortFecModeSet(devNum, portNum, modeGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortInterfaceSpeedSupportGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
 */
UTF_TEST_CASE_MAC(cpssDxChPortInterfaceSpeedSupportGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (ALL devices)
    1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E /
                             CPSS_PORT_INTERFACE_MODE_100BASE_FX_E /
                             CPSS_PORT_INTERFACE_MODE_ILKN24_E],
                     speed [CPSS_PORT_SPEED_10_E /
                            CPSS_PORT_SPEED_20000_E /
                            CPSS_PORT_SPEED_50000_E] and
                     valid supportedPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid ifMode [CPSS_PORT_INTERFACE_MODE_NA_E] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with ifMode [wrong enum values] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with invalid speed [CPSS_PORT_SPEED_NA_E] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with speed [wrong enum values] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.6. Call with invalid supportedPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                    st        = GT_OK;
    GT_U8                        devNum    = 0;
    GT_PHYSICAL_PORT_NUM         portNum   = PORT_VALID_PHY_PORT_CNS;

    CPSS_PORT_INTERFACE_MODE_ENT ifMode    =
                                      CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
    CPSS_PORT_SPEED_ENT          speed     = CPSS_PORT_SPEED_10_E;
    GT_BOOL                      supported = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E],
                                 speed [CPSS_PORT_SPEED_10_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            speed  = CPSS_PORT_SPEED_10_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E],
                                 speed [CPSS_PORT_SPEED_20000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            speed  = CPSS_PORT_SPEED_20000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E],
                                 speed [CPSS_PORT_SPEED_50000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
            speed  = CPSS_PORT_SPEED_50000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                   ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_100BASE_FX_E],
                                 speed [CPSS_PORT_SPEED_10_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
            speed  = CPSS_PORT_SPEED_10_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_100BASE_FX_E],
                                 speed [CPSS_PORT_SPEED_20000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
            speed  = CPSS_PORT_SPEED_20000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_100BASE_FX_E],
                                 speed [CPSS_PORT_SPEED_50000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
            speed  = CPSS_PORT_SPEED_50000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_ILKN24_E],
                                 speed [CPSS_PORT_SPEED_10_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_ILKN24_E;
            speed  = CPSS_PORT_SPEED_10_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_ILKN24_E],
                                 speed [CPSS_PORT_SPEED_20000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_ILKN24_E;
            speed  = CPSS_PORT_SPEED_20000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.1. Call with ifMode [CPSS_PORT_INTERFACE_MODE_ILKN24_E],
                                 speed [CPSS_PORT_SPEED_50000_E],
                                 valid supportedPtr [non-NULL].
                Expected: GT_OK.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_ILKN24_E;
            speed  = CPSS_PORT_SPEED_50000_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, portNum, ifMode, speed);

            /*
                1.1.2. Call with invalid ifMode [CPSS_PORT_INTERFACE_MODE_NA_E]
                    and other valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /*
                1.1.3. Call with ifMode [wrong enum values] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortInterfaceSpeedSupportGet
                                (devNum, portNum, ifMode,speed, &supported),
                                ifMode);

            /* restore valid values */
            ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;

            /*
                1.1.4. Call with invalid speed [CPSS_PORT_SPEED_NA_E] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
            */
            speed  = CPSS_PORT_SPEED_NA_E;

            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

            /*
                1.1.5. Call with speed [wrong enum values] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PARAM.
             */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortInterfaceSpeedSupportGet
                                (devNum, portNum, ifMode,speed, &supported),
                                speed);

            /* restore valid values */
            speed = CPSS_PORT_SPEED_50000_E;

            /*
                1.1.6. Call with invalid supportedPtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                          ifMode, speed, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_VALID_PHY_PORT_CNS;
    ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;
    speed = CPSS_PORT_SPEED_50000_E;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                    ifMode, speed, &supported);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*GT_STATUS cpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
)*/
UTF_TEST_CASE_MAC(cpssDxChPortRefClockSourceOverrideEnableSet)
{
    GT_STATUS                       st  = GT_OK;                     /* function return value */
    GT_U8                           dev  = 0;                        /* device number */
    GT_PHYSICAL_PORT_NUM            port = PORT_VALID_PHY_PORT_CNS;  /* port number */
    GT_BOOL                         overrideEnableGet;               /* oerride Enable flag */
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSourceGet;               /* refference clock source */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E);
    /* Go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* For all active devices go over all available physical ports */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_FALSE, CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* Call testing function with overrideEnable = GT=FALSE */
            st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_FALSE, CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnableGet, &refClockSourceGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, overrideEnableGet," Set overrideEnable as GT_FALSE but Get as GT_TRUE : %d %d", dev, port);

            /* Call testing function with overrideEnable = GT_TRUE and refClockSource = CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E*/
            st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnableGet, &refClockSourceGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, overrideEnableGet," Set overrideEnable as GT_TRUE but Get as GT_FALSE : %d %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E, refClockSourceGet,
                " Set refClockSourceGete as CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E but Get another : %d %d", dev, port);

            /* Call testing function with overrideEnable = GT_TRUE and refClockSource = CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E*/
            st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnableGet, &refClockSourceGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, overrideEnableGet," Set overrideEnable as GT_TRUE but Get as GT_FALSE : %d %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E, refClockSourceGet,
                " Set refClockSourceGete as CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E but Get another : %d %d", dev, port);

            /* Call testing function with overrideEnable = GT_TRUE and refClockSource = CPSS_PORT_REF_CLOCK_SOURCE_LAST_E*/
            /* Expects return GT_BAD_PARAM */
            st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_LAST_E);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                " Set refClockSourceGete as CPSS_PORT_REF_CLOCK_SOURCE_LAST_E but not retun GT_BAD_PARAM : %d %d ",  dev, port);
        }

        /* For all active devices go over all non available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* Call function for each non-active port */
            st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                       */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port = PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortRefClockSourceOverrideEnableSet(dev, port, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}


/*GT_STATUS cpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPortRefClockSourceOverrideEnableGet)
{
    GT_STATUS                       st  = GT_OK;                    /* function return value */
    GT_U8                           dev  = 0;                       /* device number */
    GT_PHYSICAL_PORT_NUM            port = PORT_VALID_PHY_PORT_CNS; /* port number */
    GT_BOOL                         overrideEnable;                 /* oerride Enable flag */
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;                 /* refference clock source */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E);
     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            if(prvCpssDxChPortRemotePortCheck(dev, port))
            {
                st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                continue;
            }

            /* Get vith non NULL pointers */
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Get vith overrideEnable is NULL - expects return GT_BAD_PTR */
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, NULL, &refClockSource);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* Get vith refClockSource is NULL - expects return GT_BAD_PTR */
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
        }
        /* For all active devices go over all non available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* Call function for each non-active port */
            st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                    */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                       */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port = PORT_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_BOBCAT2_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPortRefClockSourceOverrideEnableGet(dev, port, &overrideEnable, &refClockSource);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortResourceTmBandwidthSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortResourceTmBandwidthSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (BobK, Bobcat2)
    1.1. Call with value [0, 0xCCC00, 0xFFFFFFFF / 25 ].
    * BobK family *
    Expected: GT_BAD_STATE    if TM Disable.
              0                         : GT_OK
              0xCCC00, 0xFFFFFFFF / 25  : GT_OUT_OF_RANGE .

    * Otherwise *
    Expected: GT_NOT_SUPPORTED.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev;
    GT_U32      value           = 0;
    GT_U32      valueGet        = 0;
    GT_U32      boardIdx        = 0;
    GT_U32      boardRevId      = 0;
    GT_U32      reloadEeprom    = 0;
    GT_BOOL     tmSupported     = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);
    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_CAELUM_E, CPSS-6064);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* iterate with value = 0 */
        value = 0;
        st = cpssDxChPortResourceTmBandwidthSet(dev, value);
        cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
        appDemoIsTmSupported(dev, &tmSupported);
        if (boardRevId != 2)
        {
            tmSupported = GT_FALSE;
        }
        if(tmSupported == GT_FALSE) /* if tm NOT Supported */
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, value);
        }
        else /* if tm Supported */
        {
            /* BobK family */
            /* 1.1. Call with value = 0.
            Expected: GT_OK if TM Enable,
                      GT_BAD_STATE    otherwise.
            */
            if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) || (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, value);
                /*
                1.2. Call cpssDxChPortResourceTmBandwidthGet with non-NULL value.
                Expected: GT_OK and value the same as just written.
                */
                st = cpssDxChPortResourceTmBandwidthGet(dev, &valueGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortResourceTmBandwidthGet: %d", dev);
                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(value, valueGet,
                                                "got another value as was written: %d", dev);
            }
            /* Bobcat2 family */
            /* 1.1. Call with value = 0.
            Expected: GT_BAD_PARAM.
            */
            else if(PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, value);
            }
            /* Otherwise */
            /* 1.1. Call with value = 0.
            Expected: GT_NOT_SUPPORTED.
            */
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, value);
            }
        }

        /* iterate with value = 0xFFFFFFFF / 25 */
        value = 0xFFFFFFFF / 25 ;
        st = cpssDxChPortResourceTmBandwidthSet(dev, value);
        cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
        appDemoIsTmSupported(dev, &tmSupported);
        if (boardRevId != 2)
        {
            tmSupported = GT_FALSE;
        }
        if(tmSupported == GT_FALSE) /* if tm NOT Supported */
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, value);
        }
        else /* if tm Supported */
        {
            /* BobK family */
            /* 1.1. Call with value = 0xFFFFFFFF / 25.
            Expected: GT_OUT_OF_RANGE if TM Enable,
                      GT_BAD_STATE    otherwise.
            */
            if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) || (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, value);
            }
            /* Bobcat2 family */
            /* 1.1. Call with value = 0xFFFFFFFF / 25.
                Expected: GT_BAD_STATE.
            */
            else if(PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, value);
            }
            /* Otherwise */
            /* 1.1. Call with value = 0xFFFFFFFF / 25.
                Expected: GT_NOT_SUPPORTED.
            */
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, value);
            }
        }

        /* iterate with value = 0xCCC00 */
        value = 0xCCC00;
        st = cpssDxChPortResourceTmBandwidthSet(dev, value);
        cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);
        appDemoIsTmSupported(dev, &tmSupported);
        if (boardRevId != 2)
        {
            tmSupported = GT_FALSE;
        }
        if(tmSupported == GT_FALSE) /* if tm NOT Supported */
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, value);
        }
        else /* if tm Supported */
        {
            /* BobK family */
            /* 1.1. Call with value = 0xCCC00.
            Expected: GT_OUT_OF_RANGE if TM Enable,
                      GT_BAD_STATE    otherwise.
            */
            if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) || (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev)))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, value);
            }
            /* Bobcat2 family */
            /* 1.1. Call with value = 0xCCC00.
            Expected: GT_BAD_STATE.
            */
            else if(PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, dev, value);
            }
            /* Otherwise */
            /* 1.1. Call with value = 0xCCC00.
            Expected: GT_NOT_SUPPORTED.
            */
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, value);
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortResourceTmBandwidthGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *tmBandwidthMbpsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPortResourceTmBandwidthGet)
{

/*
    ITERATE_DEVICES_PHY_PORTS (BobK, Bobcat2)
    1. Call with devNum
    * BobK family *
    Expected: GT_OK.
    * Bobcat2 family *
    Expected: GT_NOT_IMPLEMENTED
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev;
    GT_U32      value           = 0;
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC3X_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPortResourceTmBandwidthGet(dev, &value);
        /* BobK family */
        if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) || (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev)))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }
        /* Bobcat2 family */
        else if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_IMPLEMENTED, st, dev, value);
        }
        /* Otherwise */
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, value);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E | UTF_XCAT2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortResourceTmBandwidthGet(dev, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortResourceTmBandwidthGet(dev, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortIfModeCfg suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortIfModeCfg)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExtendedModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortExtendedModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortModeSpeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFecModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortFecModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInterfaceSpeedSupportGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRefClockSourceOverrideEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortRefClockSourceOverrideEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortResourceTmBandwidthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortResourceTmBandwidthGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortIfModeCfg)


