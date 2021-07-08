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
* @file cpssPxPortCtrlUT.c
*
* @brief Unit tests for cpssPxPortCtrl, that provides
* CPSS implementation for Port configuration and control facility.
*
* @version   137
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortCtrlUT.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>

/* defines */

/* Default valid value for port id */
#define PORT_CTRL_VALID_PHY_PORT_CNS  0
#define PORT_STEP_FOR_REDUCED_TIME_CNS  0

static GT_STATUS internal_checkIsCgMac(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isCgMacPtr
)
{
    GT_U32  portMacNum; /* MAC number */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    *isCgMacPtr = (PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_CG_E);

    return GT_OK;
}

static GT_BOOL checkIsCgMac(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS   rc;
    GT_BOOL     isCgMac;

    rc = internal_checkIsCgMac(devNum,portNum,&isCgMac);
    if(rc == GT_OK)
    {
        return isCgMac;
    }

    return GT_FALSE;
}

GT_STATUS prvPxPortActivateDeactivate
(
    IN GT_U8                         devNum,
    IN GT_BOOL                       Mode,
    IN GT_PHYSICAL_PORT_NUM          portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN CPSS_PORT_SPEED_ENT           speed
)
{
    GT_STATUS   rc;                    /* return code */
    CPSS_PORTS_BMP_STC initPortsBmp,   /* bitmap of ports to init */
                      *initPortsBmpPtr;/* pointer to bitmap */

    initPortsBmpPtr = &initPortsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
    CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr, portNum);
    rc = cpssPxPortModeSpeedSet(devNum,initPortsBmpPtr, Mode, ifMode, speed);
    return rc;
}


GT_STATUS prvPxPortsActivateDeactivate
(
    IN GT_U8                              devNum,
    IN GT_BOOL                            mode,
    IN HDR_PORT_NUM_SPEED_INTERFACE_STC  *portActivePtr
)
{
    GT_STATUS                                 rc;

    rc = GT_OK;
    for (portActivePtr->Current = 0; portActivePtr->Current < portActivePtr->Len; portActivePtr->Current++)
    {
        rc = prvPxPortActivateDeactivate(devNum, mode,
                                 portActivePtr->Data[portActivePtr->Current].portNum, portActivePtr->Data[portActivePtr->Current].ifMode,
                                 portActivePtr->Data[portActivePtr->Current].speed);
        if(GT_TRUE == mode)
        {
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"prvPxPortsActivateDeactivate : prvPxPortActivateDeactivate Activation Error : dev %d port %d",
                devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"prvPxPortsActivateDeactivate : prvPxPortActivateDeactivate Deactivation Error : dev %d port %d",
                devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        }
        if(GT_OK != rc)
        {
            break;
        }
    }
    return rc;
}

GT_STATUS prvHdrPortNumCpssPxPortMapShadowAppend
(
    INOUT HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC *thisPtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum,
    IN CPSS_PX_SHADOW_PORT_MAP_STC           *portMapPtr
)
{
    GT_STATUS   rc;

    rc = GT_OK;
    if (thisPtr == NULL)
    {
        rc = GT_BAD_PTR;
    }
    if(GT_OK == rc)
    {
        if (thisPtr->Len >= thisPtr->Size)
        {
            rc = GT_NO_RESOURCE;
        }
        else
        {
            thisPtr->Data[thisPtr->Len].portNum = portNum;
            thisPtr->Data[thisPtr->Len].portMap = *portMapPtr;
            thisPtr->Len++;
        }
    }
    return rc;
}

GT_STATUS prvHdrPortNumCpssPxPortMapShadowInit
(
    INOUT HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC *thisPtr
)
{
    GT_STATUS   rc;

    rc = GT_OK;
    if (thisPtr == NULL)
    {
        rc = GT_BAD_PTR;
    }
    else
    {
        cpssOsMemSet((GT_VOID *)thisPtr,0,sizeof(HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC));
        thisPtr->Size = sizeof(thisPtr->Data)/sizeof(thisPtr->Data[0]);
        thisPtr->Len = 0;
        thisPtr->Current = 0;
    }
    return rc;
}

GT_STATUS prvPxPortShadowMapGet
(
    IN  GT_U8                                          devNum,
    OUT HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC    *portShadowPtr
)
{
   GT_PHYSICAL_PORT_NUM                             portNum ;
   GT_STATUS                                        rc;

   rc = prvHdrPortNumCpssPxPortMapShadowInit(/*INOUT*/portShadowPtr);
   if(GT_OK == rc)
   {
       for (portNum  = 0 ; portNum  < UT_MAX_PORTS; portNum ++)
       {
            CPSS_PX_DETAILED_PORT_MAP_STC *portShadowCurrentPtr;
            rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum ,/*OUT*/&portShadowCurrentPtr);
            if(GT_OK != rc)
            {
                break;
            }
            if(portShadowCurrentPtr->valid)
            {
                rc = prvHdrPortNumCpssPxPortMapShadowAppend(/*INOUT*/portShadowPtr, /*IN*/portNum , /*IN*/&portShadowCurrentPtr->portMap);
                if(GT_OK != rc)
                {
                    break;
                }
            }
       }
   }
   return rc;
}

GT_STATUS prvcpssPxPortAutoNegAdvertismentConfigSetCheck
(
    GT_U8                dev,
    GT_PHYSICAL_PORT_NUM port,
    GT_BOOL              link,
    CPSS_PORT_DUPLEX_ENT duplex_mode,
    CPSS_PORT_SPEED_ENT  speed,
    GT_CHAR              *info
)
{
    GT_STATUS                                st  = GT_OK;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC  portAnAdvertisment;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC  portAnAdvertismentGet;

    portAnAdvertisment.link = link;
    portAnAdvertisment.duplex = duplex_mode;
    portAnAdvertisment.speed = speed;
    st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegAdvertismentConfigSet %s: Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    st = cpssPxPortAutoNegAdvertismentConfigGet(dev, port, &portAnAdvertismentGet);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegAdvertismentConfigGet %s: Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.link, portAnAdvertismentGet.link,
                "cpssPxPortAutoNegMasterModeEnableSet wrong setting or getting port link - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.duplex, portAnAdvertismentGet.duplex,
                "cpssPxPortAutoNegMasterModeEnableSet wrong setting or getting port duplex mode - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(portAnAdvertisment.speed, portAnAdvertismentGet.speed,
                "cpssPxPortAutoNegMasterModeEnableSet wrong setting or getting port speed - %s : Device %d   Port %d",info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    return GT_OK;
}

GT_STATUS prvCpssPxIsMapAndNotCPUCheck
(
    GT_U8                     dev,
    GT_PHYSICAL_PORT_NUM               port
)
{
    GT_STATUS                          st;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMap;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

    /* Checking for port mapping */
    st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapPtr);
    if (st != GT_OK)
    {
        return GT_FAIL;
    }
    if (portMapPtr->valid == GT_FALSE)
    {
        return GT_FAIL;
    }
    if (portMapPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
    {
        return GT_FAIL;
    }
    return GT_OK;
}

GT_STATUS prvCpssPxPortAutoNegMasterModeEnableSetCheck
(
    GT_U8                dev,
    GT_PHYSICAL_PORT_NUM port,
    GT_BOOL              enable,
    GT_CHAR             *info
)
{
    GT_BOOL            isEnable;
    GT_STATUS          st = GT_OK;
    st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegMasterModeEnableSet %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &isEnable);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegMasterModeEnableGet %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(enable, isEnable, "cpssPxPortAutoNegMasterModeEnableSet wrong setting or getting %s: Device %d   Port %d", info, dev, port);
    if(GT_OK != st)
    {
        return st;
    }
    return st;
}

GT_STATUS prvHdrPxPortNumSpeedInterfaceInit
(
    INOUT HDR_PORT_NUM_SPEED_INTERFACE_STC *thisPtr
)
{
    GT_STATUS   rc;

    rc = GT_OK;
    if (thisPtr == NULL)
    {
        rc = GT_BAD_PTR;
    }
    else
    {
        cpssOsMemSet((GT_VOID *)thisPtr,0,sizeof(HDR_PORT_NUM_SPEED_INTERFACE_STC));
        thisPtr->Size = sizeof(thisPtr->Data)/sizeof(thisPtr->Data[0]);
        thisPtr->Len = 0;
        thisPtr->Current = 0;
    }
    return rc;
}

GT_STATUS prvHdrPxPortNumSpeedInterfaceAppend
(
    INOUT HDR_PORT_NUM_SPEED_INTERFACE_STC *thisPtr,
    IN    GT_PHYSICAL_PORT_NUM              portNum,
    IN    CPSS_PORT_SPEED_ENT               speed,
    IN    CPSS_PORT_INTERFACE_MODE_ENT      ifMode,
    IN    GT_U32                            macNum
)
{
    GT_STATUS   rc;

    rc = GT_OK;
    if (thisPtr == NULL)
    {
        rc = GT_BAD_PTR;
    }
    else
    {
         if (thisPtr->Len >= thisPtr->Size)
         {
             rc = GT_NO_RESOURCE;
         }
         else
         {
             thisPtr->Data[thisPtr->Len].portNum = portNum;
             thisPtr->Data[thisPtr->Len].ifMode = ifMode;
             thisPtr->Data[thisPtr->Len].speed = speed;
             thisPtr->Data[thisPtr->Len].macNum = macNum;
             thisPtr->Len++;
         }
    }
    return rc;
}

GT_STATUS prvDefaultPxPortsMappingAndActivePortsParamSaving
(
    IN  GT_U8                                          devNum,
    IN  HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr
)
{
   GT_STATUS                                     rc;

   rc = prvHdrPxPortNumSpeedInterfaceInit(/*INOUT*/portActivePtr);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDefaultPxPortsMappingAndActivePortsParamSaving : prvHdrPxPortNumSpeedInterfaceInit", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   for ( portShadowPtr->Current  = 0 ;  portShadowPtr->Current  < portShadowPtr->Len;  portShadowPtr->Current ++)
   {
        GT_PHYSICAL_PORT_NUM               portNum ;
        CPSS_PORT_SPEED_ENT                speed;
        OUT CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
        GT_U32                             macNum;
        portNum = portShadowPtr->Data[portShadowPtr->Current].portNum;
        speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portShadowPtr->Data[portShadowPtr->Current].portMap.macNum);
        macNum = portShadowPtr->Data[portShadowPtr->Current].portMap.macNum;
        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            if(CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E == portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType)
            {
                rc = cpssPxPortInterfaceModeGet(devNum, portNum , &ifMode);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvDefaultPxPortsMappingAndActivePortsParamSaving : cpssPxPortInterfaceModeGet : dev %d port %d",
                     devNum, portShadowPtr->Data[portShadowPtr->Current].portNum );
                if(GT_OK != rc)
                {
                    break;
                }
                if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    rc = prvHdrPxPortNumSpeedInterfaceAppend( /*INOUT*/portActivePtr, /*IN*/portNum , /*IN*/speed, /*IN*/ifMode, /*IN*/macNum);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvDefaultPxPortsMappingAndActivePortsParamSaving : prvHdrPxPortNumSpeedInterfaceAppend : dev %d port %d",
                         devNum, portShadowPtr->Data[portShadowPtr->Current].portNum );
                    if(GT_OK != rc)
                    {
                        break;
                    }
                }
            }
        }
   }
   return rc;
}

/* Save active ports parameters */
GT_STATUS prvCpssPxSaveActivePortsParam
(
    GT_U8                                          dev,
    HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr
)
{
    GT_STATUS                                            st;
    static HDR_PORT_NUM_CPSS_PX_PORT_MAP_SHADOW_STC    portShadow;

    /* Saving default mapping*/
    st = prvPxPortShadowMapGet( dev, /*OUT*/&portShadow);
    if(GT_NOT_APPLICABLE_DEVICE == st)
    {
        return st;
    }
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortShadowMapGet", dev);
    if(GT_OK != st)
    {
        return GT_FAIL;
    }

    /* Saving active ports*/
    st = prvDefaultPxPortsMappingAndActivePortsParamSaving(dev, &portShadow, /*OUT*/portActivePtr);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvDefaultPortsMappingAndActivePortsParamSaving", dev);
    if(GT_OK != st)
    {
         return GT_FAIL;
    }

    return GT_OK;
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacSaLsbSet
(
    IN GT_U8            devNum,
    IN GT_U8            portNum,
    IN GT_U8            macSaLsb
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacSaLsbSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with macSaLsb [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortMacSaLsbGet with non-NULL macSaLsb.
    Expected: GT_OK and macSaLsb the same as just written.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8                   macSaLsb    = 0;
    GT_U8                   macSaLsbGet = 0;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with macSaLsb [0 / 250].
               Expected: GT_OK.
            */
            /* iterate with macSaLsb = 0 */
            macSaLsb = 0;

            st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
            if (st != GT_NOT_SUPPORTED)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, macSaLsb);

                /*
                    1.1.2. Call cpssPxPortMacSaLsbGet with non-NULL macSaLsb.
                    Expected: GT_OK and macSaLsb the same as just written.
                */
                st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsbGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortMacSaLsbGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(macSaLsb, macSaLsbGet,
                                             "got another macSaLsb as was written: %d, %d", dev, port);
            }

            /* iterate with macSaLsb = 250 */
            macSaLsb = 250;

            st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
            if (st != GT_NOT_SUPPORTED)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, macSaLsb);
            }

            /*
                1.1.2. Call cpssPxPortMacSaLsbGet with non-NULL macSaLsb.
                Expected: GT_OK and macSaLsb the same as just written.
            */
            st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsbGet);
            if (st != GT_NOT_SUPPORTED)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortMacSaLsbGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(macSaLsb, macSaLsbGet,
                                             "got another macSaLsb as was written: %d, %d", dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        macSaLsb = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    macSaLsb = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssPxPortMacSaLsbSet(dev, port, macSaLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacSaLsbGet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    OUT GT_U8    *macSaLsb
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacSaLsbGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-NULL macSaLsb.
    Expected: GT_OK.
    1.1.2. Call with macSaLsb [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS               st  = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8                   macSaLsb = 0;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with non-NULL macSaLsb.
                Expected: GT_OK.
            */
            st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsb);
            if (st != GT_NOT_SUPPORTED)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with macSaLsb [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortMacSaLsbGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, macSaLsbPtr = NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsb);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsb);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacSaLsbGet(dev, port, &macSaLsb);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with state [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    1.1.2. Call cpssPxPortEnableGet.
    Expected: GT_OK and the same enable.
*/

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssPxPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssPxPortEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "[cpssPxPortEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);

            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssPxPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call cpssPxPortEnableGet.
               Expected: GT_OK and the same enable.
            */
            st = cpssPxPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "[cpssPxPortEnableGet]: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* enable == GT_TRUE  */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEnableGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortLinkStatusGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    OUT GT_BOOL   *isLinkUpPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortLinkStatusGet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with non-null isLinkUpPtr.
    Expected: GT_OK.
    1.1.2. Call with isLinkUpPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st    = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null isLinkUpPtr.
               Expected: GT_OK.
            */
            st = cpssPxPortLinkStatusGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isLinkUpPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortLinkStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortLinkStatusGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortLinkStatusGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
        st = cpssPxPortLinkStatusGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortLinkStatusGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortLinkStatusGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForceLinkPassEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForceLinkPassEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortForceLinkPassEnableGet with non NULL statePtr.
    Expected: GT_OK and the same state.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE */
            state = GT_FALSE;

            st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssPxPortForceLinkPassEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssPxPortForceLinkPassEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssPxPortForceLinkPassEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);

            /* Call function with enable = GT_TRUE */
            state = GT_TRUE;

            st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssPxPortForceLinkPassEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssPxPortForceLinkPassEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssPxPortForceLinkPassEnableGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                       "get another state than was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE     */
            st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortForceLinkPassEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForceLinkPassEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForceLinkPassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with not NULL statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     state = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not NULL statePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortForceLinkPassEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statePtr = NULL", dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForceLinkPassEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForceLinkDownEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForceLinkDownEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with state [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortForceLinkDownEnableGet with non NULL statePtr.
    Expected: GT_OK and the same state.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                 isCgMac;

    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isCgMac  = checkIsCgMac(dev, port);
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK.
            */

            /* Call function with enable = GT_FALSE] */
            state = GT_FALSE;

            st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
            if(isCgMac)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }

            /*
                1.1.2. Call cpssPxPortForceLinkDownEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssPxPortForceLinkDownEnableGet(dev, port, &stateGet);
            if(isCgMac)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssPxPortForceLinkDownEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                           "get another state than was set: %d, %d", dev, port);
            }

            /* Call function with enable = GT_TRUE] */
            state = GT_TRUE;

            st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
            if(isCgMac)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }

            /*
                1.1.2. Call cpssPxPortForceLinkDownEnableGet with non NULL statePtr.
                Expected: GT_OK and the same state.
            */
            st = cpssPxPortForceLinkDownEnableGet(dev, port, &stateGet);
            if(isCgMac)
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssPxPortForceLinkDownEnableGet: %d, %d", dev, port);

                /* verifying values */
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                           "get another state than was set: %d, %d", dev, port);
            }
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        /* state == GT_TRUE */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortForceLinkDownEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForceLinkDownEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL   *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForceLinkDownEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (Px)
    1.1.1. Call with not NULL statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS               st   = GT_OK;
    GT_U8                   dev  = 0;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                 isCgMac;

    GT_BOOL     state = GT_FALSE;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            isCgMac  = checkIsCgMac(dev, port);

            /*
                1.1.1. Call with not NULL statePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
            if(isCgMac)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with statePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortForceLinkDownEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, statePtr = NULL", dev);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForceLinkDownEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMruSet
(
    IN  GT_U8  devNum,
    IN  GT_U8  portNum,
    IN  GT_U32 mruSize
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMruSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS (Px)
    1.1.1. Call with mruSize [0, 10304].
    Expected: GT_OK and the same value.
    1.1.2. Check that odd values are unsupported. Call with mruSize [1, 16383].
    Expected: NOT GT_OK.
    1.1.3. Call with mruSize [16384] (out of range).
    Expected: NOT GT_OK.
    1.2. Call with port [PRV_CPSS_PX_CPU_DMA_NUM_CNS = 63] and valid other parameters.
    Expected: GT_OK.
    */

    GT_STATUS st      = GT_OK;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM port    = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32    size    = 0;
    GT_U32    sizeGet = 1;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with mruSize [0, 10304/10240, 2048, 1522].
               Expected: GT_OK and the same value.
            */

            /* mruSize = 0 */
            size = 0;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /*checking value*/
            st = cpssPxPortMruGet(dev, port, &sizeGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
            UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                     "got another size then was set: %d, %d", dev, port);
            /* mruSize = 10304 */
            size = 10304;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /*checking value*/
            st = cpssPxPortMruGet(dev, port, &sizeGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
            UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                     "got another size then was set: %d, %d", dev, port);

            /* mruSize = 2048 */
            size = 2048;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /*checking value*/
            st = cpssPxPortMruGet(dev, port, &sizeGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
            UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                     "got another size then was set: %d, %d", dev, port);

            /* mruSize = 1522 */
            size = 1522;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /*checking value*/
            st = cpssPxPortMruGet(dev, port, &sizeGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
            UTF_VERIFY_EQUAL2_STRING_MAC(size, sizeGet,
                     "got another size then was set: %d, %d", dev, port);

            /* 1.1.2. Check that odd values are unsupported.
                      Call with mruSize [1, 16383].
               Expected: NOT GT_OK.
            */

            /* mruSize = 1 */
            size = 1;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /* mruSize = 16383 */
            size = 16383;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);

            /* 1.1.3. Call with mruSize [16384] (out of range).
               Expected: NOT GT_OK.
            */
            size = 16384;

            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, size);
        }

        size = 500;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortMruSet(dev, port, size);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMruSet(dev, port, size);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMruSet(dev, port, size);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    size = 500;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMruSet(dev, port, size);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, size == 500 */

    st = cpssPxPortMruSet(dev, port, size);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMruGet
(
    IN   GT_U8                        devNum,
    IN   GT_U8                        portNum,
    OUT  GT_U32                      *mruSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMruGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with not null mruSizePtr.
        Expected: GT_OK.
        1.1.2. Call with null mruSizePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                        dev;
    GT_STATUS                    st     = GT_OK;
    GT_PHYSICAL_PORT_NUM         port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                       mruSize;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not null mruSizePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortMruGet(dev, port, &mruSize);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call with null mruSizePtr [NULL].
                Expected: GT_BAD_PTR
            */
            st = cpssPxPortMruGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMruGet(dev, port, &mruSize);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMruGet(dev, port, &mruSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortMruGet(dev, port, &mruSize);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMruGet(dev, port, &mruSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMruGet(dev, port, &mruSize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInternalLoopbackEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInternalLoopbackEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortInternalLoopbackEnableGet.
    Expected: GT_OK and enablePtr the same as was written.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    GT_BOOL     state  = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT             speed = CPSS_PORT_SPEED_NA_E;
#ifndef ASIC_SIMULATION
    GT_BOOL                         isLinkUp;
#endif

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssPxPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            /* Call function with enable = GT_FALSE] */
            enable = GT_FALSE;

            st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* 1.1.2. Call cpssPxPortInternalLoopbackEnableGet.
               Expected: GT_OK and enablePtr the same as was written.
            */
            st = cpssPxPortInternalLoopbackEnableGet(dev, port, &state);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssPxPortInternalLoopbackEnableGet: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssPxPortInternalLoopbackEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);
            }
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE] */
            enable = GT_TRUE;

            st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* 1.1.2. Call cpssPxPortInternalLoopbackEnableGet.
               Expected: GT_OK and enablePtr the same as was written.
            */
            st = cpssPxPortInternalLoopbackEnableGet(dev, port, &state);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {
                UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssPxPortInternalLoopbackEnableGet: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssPxPortInternalLoopbackEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, state,
                                         "get another enable value than was set: %d, %d", dev, port);
#ifndef ASIC_SIMULATION
                /* wait 10mSec before link check to let it stabilize */
                cpssOsTimerWkAfter(10);

                st = cpssPxPortLinkStatusGet(dev, port, &isLinkUp);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssPxPortLinkStatusGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isLinkUp,
                                         "link is down although defined loopback: %d, %d", dev, port);
#endif
            }
        }

#ifndef ASIC_SIMULATION
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.3 Go over all available physical ports and check link,
           in previous loop on all ports was loopback enabled, so I expect all
           ports be in link up */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssPxPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
                continue;

            st = cpssPxPortLinkStatusGet(dev, port, &isLinkUp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "cpssPxPortLinkStatusGet_2: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, isLinkUp,
                                         "link is down although loopback enabled on it: %d, %d", dev, port);

            /* disable loopback to avoid surprises in other tests. */
            enable = GT_FALSE;
            st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }
#endif

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortInternalLoopbackEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInternalLoopbackEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInternalLoopbackEnableGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT             speed = CPSS_PORT_SPEED_NA_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            if (st != GT_OK)
            {
                continue;
            }

            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                st = cpssPxPortSpeedGet(dev, port, &speed);
                if (st != GT_OK)
                {
                    continue;
                }
            }

            /* 1.1.1. Call with non-null enablePtr.
                Expected: GT_OK.
            */
            st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
            if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortInternalLoopbackEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortInternalLoopbackEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesTxEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            enable [GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesTxEnableGet
    Expected: GT_OK and the same enable.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORTS_BMP_STC    portsBmp;

    GT_BOOL               enable    = GT_FALSE;
    GT_BOOL               enableGet = GT_FALSE;
    GT_BOOL     supported;
    CPSS_PX_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPortsActivateDeactivate", dev);
            if(GT_OK != st)
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
#endif

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            st = cpssPxPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }


            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);

            if(st != GT_OK)
            {
                continue;
            }


            /* 1.1.1 */
            enable    = GT_FALSE;
            st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            st = cpssPxPortSerdesTxEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet, "get another enable :%d,%d", dev, port);
            if(GT_OK != st)
            {
                 isError = GT_TRUE;
                 goto restore;
            }

            /* 1.1.1 */
            enable    = GT_TRUE;
            st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            st = cpssPxPortSerdesTxEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

#ifndef ASIC_SIMULATION
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(enable, enableGet, "get another enable :%d,%d", dev, port);
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
#endif


#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            if(GT_TRUE == canSave)
            {
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                                  portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesTxEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesTxEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.1.2. Call with NULL enablePtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL               enable    = GT_FALSE;
    CPSS_PORTS_BMP_STC    portsBmp;
    GT_BOOL supported;
    CPSS_PX_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
            if(GT_OK != st)
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
#endif

    /* 1. Go over all active devices. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            st = cpssPxPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);

                if(st != GT_OK)
                {
                    continue;
                }


            /* 1.1.1 */
            st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            st = cpssPxPortSerdesTxEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            if(GT_TRUE == canSave)
            {
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                                  portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

       }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesTxEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesManualTxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesManualTxConfigSet)
{
/*
    ITERATE_DEVICE_PHY_PORT
    1.1.1. Call with valid values
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesConfigGet with not NULL serdesCfgPtr.
    Expected: GT_OK and the same serdesCfgPtr as was set.
    1.1.3. Call with values out of range
    Expected: NOT GT_OK.
    1.1.5. Call with serdesCfg [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               laneNum;

    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32                          startSerdes;
    GT_U32                          numOfSerdesLanes;
    GT_BOOL              isSimulation;

    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesCfg;
    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesCfgGet;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));
    cpssOsBzero((GT_VOID*) &serdesCfgGet, sizeof(serdesCfgGet));
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    #ifdef ASIC_SIMULATION
          isSimulation = GT_TRUE;
    #else
          isSimulation = GT_FALSE;
    #endif


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devFamily = PRV_CPSS_PX_PP_MAC(dev)->genInfo.devFamily;
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            GT_BOOL                            valid;
            static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
            CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

            valid = GT_FALSE;
            st = cpssPxPortPhysicalPortDetailedMapGet(dev, port, /*OUT*/portMapShadowPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if(GT_TRUE == portMapShadowPtr->valid)
            {
                st = prvUtfPortInterfaceModeGet(dev, port, &ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);

                if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    valid = GT_FALSE;
                }
                else /* (numOfSerdesLanes > 0) */
                {
                    valid = GT_TRUE;
                }
            }


            if (valid == GT_TRUE)
            {

                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                       &startSerdes,
                                                       &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                        1.1.5. Call with serdesCfg [NULL].
                        Expected: GT_BAD_PTR.
                */
                st = cpssPxPortSerdesManualTxConfigSet(dev, port, startSerdes, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                             "%d, %d, serdesCfgPtr = NULL", dev, port);

                for(laneNum = 0; laneNum < numOfSerdesLanes; laneNum++)
                {
                    /* call with first group of the values */
                    serdesCfg.txAmp = 0;
                    serdesCfg.emph0 = 0;
                    serdesCfg.emph1 = 0;


                    st = cpssPxPortSerdesManualTxConfigSet(dev, port, laneNum, &serdesCfg);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                            1.1.2. Call cpssPxPortSerdesConfigGet with not NULL serdesCfgPtr.
                            Expected: GT_OK and the same serdesCfgPtr as was set.
                    */
                    st = cpssPxPortSerdesManualTxConfigGet(dev, port, laneNum, &serdesCfgGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "cpssPxPortSerdesConfigGet: %d, %d", dev, port);

                    if(GT_FALSE == isSimulation)
                    {
                        if(GT_OK == st)
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmp, serdesCfgGet.txAmp,
                                                         "get another serdesCfg.txAmp than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmp, serdesCfgGet.txAmp);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph0, serdesCfgGet.emph0,
                                                         "get another serdesCfg.emph0 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph0, serdesCfgGet.emph0);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph1, serdesCfgGet.emph1,
                                                         "get another serdesCfg.emph1 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph1, serdesCfgGet.emph1);
                        }
                    }

                    /* call with second group of the values */
                    serdesCfg.txAmp = 10;
                    serdesCfg.emph0 = 11;
                    serdesCfg.emph1 = 11;


                    st = cpssPxPortSerdesManualTxConfigSet(dev, port, laneNum, &serdesCfg);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                            1.1.2. Call cpssPxPortSerdesConfigGet with not NULL serdesCfgPtr.
                            Expected: GT_OK and the same serdesCfgPtr as was set.
                    */
                    st = cpssPxPortSerdesManualTxConfigGet(dev, port, laneNum, &serdesCfgGet);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "cpssPxPortSerdesConfigGet: %d, %d", dev, port);

                    if(GT_FALSE == isSimulation)
                    {
                        if(GT_OK == st)
                        {
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.txAmp, serdesCfgGet.txAmp,
                                                         "get another serdesCfg.txAmp than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.txAmp, serdesCfgGet.txAmp);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph0, serdesCfgGet.emph0,
                                                         "get another serdesCfg.emph0 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph0, serdesCfgGet.emph0);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.emph1, serdesCfgGet.emph1,
                                                         "get another serdesCfg.emph1 than was set:dev-%d,port-%d,set-%d,get-%d",
                                                         dev, port, serdesCfg.emph1, serdesCfgGet.emph1);
                        }
                    }
                }

                /*
                        1.1.3. Call with serdesCfg->txAmp [32] out of range, and other params from 1.1.1.
                        Expected: NOT GT_OK.
                */
                serdesCfg.txAmp = 32;
                st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) */
                {
                    st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesManualTxConfigSet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesManualTxConfigGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesManualTxConfigGet)
{
/*
    ITERATE_DEVICE_PHY_PORT
    1.1.1. Call with not NULL serdesCfgPtr.
    Expected: GT_OK.
    1.1.2. Call with serdesCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    GT_PHYSICAL_PORT_NUM                port;
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SERDES_TX_CONFIG_STC serdesCfg;

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            GT_BOOL                            valid;
                static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
                CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

                valid = GT_FALSE;
                st = cpssPxPortPhysicalPortDetailedMapGet(dev, port, /*OUT*/portMapShadowPtr);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                if(GT_TRUE == portMapShadowPtr->valid)
                {
                    st = prvUtfPortInterfaceModeGet(dev, port, &ifMode);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", dev, port);

                    if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                    {
                        valid = GT_FALSE;
                    }
                    else /* (numOfSerdesLanes > 0) */
                    {
                        valid = GT_TRUE;
                    }
                }


            if (valid == GT_TRUE)
            {
                /*
                        1.1.1. Call with not NULL serdesCfgPtr.
                        Expected: GT_OK.
                */
                st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
                if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    continue;
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                /*
                        1.1.2. Call with serdesCfgPtr [NULL].
                        Expected: GT_BAD_PTR.
                */
                st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, serdesCfgPtr = NULL", dev, port);
            }
            else
            {
                st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesManualTxConfigGet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesManualRxConfigSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum,
    IN CPSS_PORT_SERDES_RX_CONFIG_STC  *serdesRxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesManualRxConfigSet)
{
/*
    ITERATE_DEVICE_PHY_PORT (Lion2 and above)
    1.1.1. Call with valid values
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesConfigGet with not NULL serdesCfgPtr.
    Expected: GT_OK and the same serdesCfgPtr as was set.
    1.1.3. Call with values out of range
    Expected: NOT GT_OK.
    1.1.5. Call with serdesCfg [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    GT_U32               laneNum;
    GT_BOOL              isSimulation;

    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32                          startSerdes;
    GT_U32                          numOfSerdesLanes;

    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesCfg;
    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesCfgGet;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));
    cpssOsBzero((GT_VOID*) &serdesCfgGet, sizeof(serdesCfgGet));
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    #ifdef ASIC_SIMULATION
          isSimulation = GT_TRUE;
    #else
          isSimulation = GT_FALSE;
    #endif

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", dev, port);

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                continue;
            }

                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);


            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes,
                                                   &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.5. Call with serdesCfg [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSerdesManualRxConfigSet(dev, port, startSerdes, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                "%d, %d, serdesCfgPtr = NULL", dev, port);

            for(laneNum = 0; laneNum < numOfSerdesLanes; laneNum++)
            {
                /* call with first group of the values */
                serdesCfg.sqlch = 68;
                serdesCfg.ffeRes = 0;
                serdesCfg.ffeCap = 0;
                serdesCfg.dcGain = 0;
                serdesCfg.bandWidth = 0;
                serdesCfg.loopBandwidth = 0;


                st = cpssPxPortSerdesManualRxConfigSet(dev, port, laneNum, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssPxPortSerdesConfigGet with not NULL serdesCfgPtr.
                    Expected: GT_OK and the same serdesCfgPtr as was set.
                */
                st = cpssPxPortSerdesManualRxConfigGet(dev, port, laneNum, &serdesCfgGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortSerdesManualRxConfigSet: %d, %d", dev, port);

                if(GT_FALSE == isSimulation)
                {
                     if (GT_OK == st)
                     {
                         UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.sqlch, serdesCfgGet.sqlch,
                             "get another serdesCfg.sqlch than was set:dev-%d,port-%d,set-%d,get-%d",
                             dev, port, serdesCfg.sqlch, serdesCfgGet.sqlch);
                         UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeRes, serdesCfgGet.ffeRes,
                             "get another serdesCfg.ffeRes than was set:dev-%d,port-%d,set-%d,get-%d",
                             dev, port, serdesCfg.ffeRes, serdesCfgGet.ffeRes);
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeCap, serdesCfgGet.ffeCap,
                            "get another serdesCfg.ffeCap than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeCap, serdesCfgGet.ffeCap);

                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.dcGain, serdesCfgGet.dcGain,
                                "get another serdesCfg.dcGain than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.dcGain, serdesCfgGet.dcGain);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.bandWidth, serdesCfgGet.bandWidth,
                                "get another serdesCfg.bandWidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.bandWidth, serdesCfgGet.bandWidth);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth,
                                "get another serdesCfg.loopBandwidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth);
                    }
                }

                /* call with second group of the values */
                serdesCfg.sqlch = 31;
                serdesCfg.ffeRes = 7;
                serdesCfg.ffeCap = 15;
                serdesCfg.dcGain = 99;
                serdesCfg.bandWidth = 13;
                serdesCfg.loopBandwidth = 1;


                st = cpssPxPortSerdesManualRxConfigSet(dev, port, laneNum, &serdesCfg);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssPxPortSerdesManualRxConfigGet with not NULL serdesCfgPtr.
                    Expected: GT_OK and the same serdesCfgPtr as was set.
                */
                st = cpssPxPortSerdesManualRxConfigGet(dev, port, laneNum, &serdesCfgGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortSerdesManualRxConfigGet: %d, %d", dev, port);

                if(GT_FALSE == isSimulation)
                {
                    if (GT_OK == st)
                    {
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeRes, serdesCfgGet.ffeRes,
                            "get another serdesCfg.ffeRes than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeRes, serdesCfgGet.ffeRes);
                        UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.ffeCap, serdesCfgGet.ffeCap,
                            "get another serdesCfg.ffeCap than was set:dev-%d,port-%d,set-%d,get-%d",
                            dev, port, serdesCfg.ffeCap, serdesCfgGet.ffeCap);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.dcGain, serdesCfgGet.dcGain,
                                "get another serdesCfg.dcGain than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.dcGain, serdesCfgGet.dcGain);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.bandWidth, serdesCfgGet.bandWidth,
                                "get another serdesCfg.bandWidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.bandWidth, serdesCfgGet.bandWidth);
                            UTF_VERIFY_EQUAL4_STRING_MAC(serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth,
                                "get another serdesCfg.loopBandwidth than was set:dev-%d,port-%d,set-%d,get-%d",
                                dev, port, serdesCfg.loopBandwidth, serdesCfgGet.loopBandwidth);
                    }
                }
            }

            /*
                1.1.3. Call with serdesCfg->txAmp [32] out of range, and other params from 1.1.1.
                Expected: NOT GT_OK.
            */

            serdesCfg.sqlch = 311;
            st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesManualRxConfigSet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesManualRxConfigGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  GT_U32                               laneNum,
    OUT CPSS_PORT_SERDES_TX_CONFIG_STC  *serdesTxCfgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesManualRxConfigGet)
{
/*
    ITERATE_DEVICE_PHY_PORT
    1.1.1. Call with not NULL serdesCfgPtr.
    Expected: GT_OK.
    1.1.2. Call with serdesCfgPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SERDES_RX_CONFIG_STC serdesCfg;

    cpssOsBzero((GT_VOID*) &serdesCfg, sizeof(serdesCfg));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet: %d, %d",
                                         dev, port);
            /*
                1.1.1. Call with not NULL serdesCfgPtr.
                Expected: GT_OK.
            */
            st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
            if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call with serdesCfgPtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, serdesCfgPtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesManualRxConfigGet(dev, port, 0, &serdesCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesPowerStatusSet
(
    IN  GT_SW_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesPowerStatusSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS
    1.1.1. Call with
    Expected: GT_OK.
    */

    GT_STATUS   st     = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT                 devFamily;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL                 powerUp = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT     speed;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                           CPSS_PORT_DIRECTION_TX_E /
                                           CPSS_PORT_DIRECTION_BOTH_E],
                              and powerUp [GT_TRUE / GT_FALSE].
               Expected: GT_OK.
            */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet: %d, %d",
                                         dev, port);

            st = cpssPxPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortSpeedGet: %d, %d",
                                         dev, port);

            /*call with powerUp = GT_FALSE */
            powerUp = GT_FALSE;

            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                   powerUp);


            if( portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* the function is supported for XG ports only */
                if (portType != PRV_CPSS_PORT_XG_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }


            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                   powerUp);

            /* CH3 Gig ports does not supports this API */
            if(portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* the function is supported for XG ports only */
                if (portType != PRV_CPSS_PORT_XG_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                   powerUp);


            if (portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* the function is supported for XG ports only */
                if (portType != PRV_CPSS_PORT_XG_E)
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
            }


            /*call with powerUp = GT_TRUE */
            powerUp = GT_TRUE;


            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                  powerUp);


            if(portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }


            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                   powerUp);

            if (portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            }

            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                   powerUp);


            if (portType == PRV_CPSS_PORT_FE_E)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                    /* the function is supported for XG ports only */
                    if (portType != PRV_CPSS_PORT_XG_E)
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            }
        }


        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                                  powerUp);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }


        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                               powerUp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                     */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                              powerUp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                               powerUp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesPowerStatusSet(dev, port,
                                           powerUp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}



/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesTuningSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
           laneBmp[0/0xFFFF/0xFFFFFFFF]
           serdesFrequency [CPSS_DXCH_PORT_SERDES_SPEED_1_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_4_25_E/
                            CPSS_DXCH_PORT_SERDES_SPEED_12_5_E]

           tuneValuesPtr->sqlch[0],
           tuneValuesPtr->txEmphAmp[0/15],
           tuneValuesPtr->txAmp[0/31],
           tuneValuesPtr->txAmpAdj[0/1],
           tuneValuesPtr->txEmphEn[GT_TRUE/GT_FALSE],
           tuneValuesPtr->txEmph1[0],
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesTuningGet.
    Expected: GT_OK and the same tuneValuesPtr.
    1.1.3. Call with out of range serdesFrequency.
    Expected: GT_BAD_PARAM and the same mode.
    1.1.4. Call with NULL tuneValuesPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneBmp   = 0;
    GT_U32                laneNum   = 0;
    CPSS_PORTS_BMP_STC    portsBmp;

    CPSS_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;
    CPSS_PORT_SERDES_TUNE_STC    tuneValues;
    CPSS_PORT_SERDES_TUNE_STC    tuneValuesGet;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  startSerdes;    /* first serdes of port */
    GT_U32  serdesNumPerPort;/* number of serdeses occupied now by port */
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    CPSS_PX_DETAILED_PORT_MAP_STC                      portMap;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
       st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);

       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
           st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
           if(GT_OK != st)
           {
               isError = GT_TRUE;
               goto restore;
           }
        }
#endif

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        lanesNumInDev = 16;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,&portMap);
            if (st != GT_OK)
            {
                return;
            }
            if (portMap.valid == GT_FALSE)
            {
                continue;
            }
            if (portMap.portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                continue;
            }

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 sedes */
                                          CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                          CPSS_PORT_SPEED_1000_E);
            if(GT_OK != st)
            {
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }



                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }


            st = prvCpssPxPortIfModeSerdesNumGet(dev, port,
                                                   CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                                   &startSerdes, &serdesNumPerPort);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.1 */
            laneBmp   = 1;
            serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;
            cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
            tuneValues.txEmphAmp = 15;
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum   = 0;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphAmp, tuneValuesGet.txEmphAmp,
                "get another  tuneValues.txEmphAmp value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
            tuneValues.txEmphAmp = 0;

            /* 1.1.1 */
            laneBmp   = 0xFFFF;
            tuneValues.txAmpAdj = 1;
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 8;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                    "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 15;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txAmpAdj, tuneValuesGet.txAmpAdj,
                    "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            tuneValues.txAmpAdj = 0;

            /* 1.1.1 */
            laneBmp   = 0xFFFFFFFF;
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 16;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

           }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }


            /* 1.1.1 */
            laneBmp   = 0xFFFFFFFF;
            tuneValues.txEmphEn = GT_TRUE;
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 0;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 8;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                    "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }

            /* 1.1.2 */
            cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
            laneNum = 16;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValuesGet);
            if((startSerdes+laneNum) < lanesNumInDev)
            {
                UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(tuneValues.txEmphEn, tuneValuesGet.txEmphEn,
                    "get another  tuneValues.txEmphEn value that expected - %d,%d ", dev, port);
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
                if((GT_OK == st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

            }

            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues),
                                serdesFrequency);

            /* 1.1.4. */
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, NULL);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_BAD_PTR, st, dev, port);
            if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
            /* Deactivate tested port */
            if(GT_TRUE == canSave)
            {
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, port);
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesTuningSet(dev, port, laneBmp, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/* UTF_TEST_CASE_MAC(cpssPxPortSerdesTuningGet) */
/*----------------------------------------------------------------------------*/
#define PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG 0
UTF_TEST_CASE_MAC(cpssPxPortSerdesTuningGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
           laneNum[0/max value for each dev and port]
           serdesFrequency [CPSS_PORT_SERDES_SPEED_1_25_E/
                            CPSS_PORT_SERDES_SPEED_4_25_E/
                            CPSS_PORT_SERDES_SPEED_12_5_E]
           and not NULL tuneValuesPtr.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum.
    Expected: NOT GT_OK.
    1.1.3. Call with out of range serdesFrequency.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with NULL tuneValuesPtr.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                serdesNumPerPort;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E;

    CPSS_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;
    CPSS_PORT_SERDES_TUNE_STC    tuneValues;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
        GT_U32 portMacNum;
        GT_U32 portIdx;
    #endif

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        lanesNumInDev = 16;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
            cpssOsPrintf("\n    Process port:");
            portIdx = 0;
        #endif
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet:%d,%d", dev, port);

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
                portMacNum = portMapArray[0].interfaceNum;
            #endif

            #if (PORT_SERDES_TUNING_GET_UT_PORT_SEQ_DEBUG == 1)
                if (0 == (portIdx % 20))
                {
                    cpssOsPrintf("\n");
                }
                portIdx ++;
                cpssOsPrintf("%3d",portMacNum);
            #endif
            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &serdesNumPerPort);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (serdesNumPerPort == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
            maxLaneNum = serdesNumPerPort - 1;
            }

            /* 1.1.1 */
            laneNum = 0;
            serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_PORT_SERDES_SPEED_4_25_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_PORT_SERDES_SPEED_12_5_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            laneNum = maxLaneNum;
            serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_PORT_SERDES_SPEED_4_25_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            serdesFrequency = CPSS_PORT_SERDES_SPEED_12_5_E;

            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;

            /* 1.1.2 */
            laneNum = lanesNumInDev + 1;
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3. */
            laneNum = 0;
            UTF_ENUMS_CHECK_MAC(cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues),
                                serdesFrequency);

            /* 1.1.4. */
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesTuningGet(dev, port, laneNum, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesLaneTuningSet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  laneNum;
    GT_U32                                portGroupId         = 0;
    CPSS_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;
    GT_U32                            serdesFreq = 0;
    CPSS_PORT_SERDES_TUNE_STC    tuneValues;
    CPSS_PORT_SERDES_TUNE_STC    tuneValuesGet;


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        lanesNumInDev = 16;
         /* Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            for(laneNum = 0; laneNum < lanesNumInDev; laneNum++)
            {
                for(serdesFreq = 0; serdesFreq < CPSS_PORT_SERDES_SPEED_NA_E; serdesFreq++)
                {
                    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)serdesFreq;
                    cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
                    cpssOsMemSet(&tuneValuesGet, 0, sizeof(tuneValuesGet));
                        tuneValues.BW = 14;
                        tuneValues.DC = 99;
                        tuneValues.HF = 14;
                        tuneValues.LB = 1;
                        tuneValues.LF = 4;
                        tuneValues.sqlch = 99;
                        tuneValues.txAmp = 6;
                        tuneValues.txEmph1 = 10;
                        tuneValues.txEmphAmp = 11;
                        st = cpssPxPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        st = cpssPxPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValuesGet);
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.BW, tuneValuesGet.BW,
                            "get another  tuneValues.BW value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.DC, tuneValuesGet.DC,
                            "get another  tuneValues.DC value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.HF, tuneValuesGet.HF,
                            "get another  tuneValues.HF value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.LB, tuneValuesGet.LB,
                            "get another  tuneValues.LB value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.LF, tuneValuesGet.LF,
                            "get another  tuneValues.LF value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.sqlch, tuneValuesGet.sqlch,
                            "get another  tuneValues.sqlch value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txAmp, tuneValuesGet.txAmp,
                            "get another  tuneValues.txAmp value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmph1, tuneValuesGet.txEmph1,
                            "get another  tuneValues.txEmph1 value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(tuneValues.txEmphAmp, tuneValuesGet.txEmphAmp,
                            "get another  tuneValues.txEmphAmp value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, tuneValuesGet.txAmpAdj,
                            "get another  tuneValues.txAmpAdj value that expected - %d,%d ", dev, portGroupId, laneNum);
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, tuneValuesGet.txAmpShft,
                            "get another  tuneValues.txAmpShft value that expected - %d,%d ", dev, portGroupId, laneNum);
                }
            }

            /* serdesFrequency is not valid */
            st = cpssPxPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, laneNum);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;

        /* portGroupId is not valid */
        st = cpssPxPortSerdesLaneTuningSet(dev, portGroupId, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, 0);

        /* pointer  is not valid */
        st = cpssPxPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesLaneTuningSet(dev, 0, 0, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesLaneTuningGet)
{
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32  lanesNumInDev; /* number of serdes lanes in current device */
    GT_U32  laneNum;
    GT_U32                                portGroupId         = 0;
    CPSS_PORT_SERDES_SPEED_ENT   serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;
    GT_U32                            serdesFreq = 0;
    CPSS_PORT_SERDES_TUNE_STC    tuneValues;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        lanesNumInDev = 16;
         /* Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            for(laneNum = 0; laneNum < lanesNumInDev; laneNum++)
            {
                for(serdesFreq = 0; serdesFreq < CPSS_PORT_SERDES_SPEED_NA_E; serdesFreq++)
                {
                    serdesFrequency = (CPSS_PORT_SERDES_SPEED_ENT)serdesFreq;
                    cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));
                    st = cpssPxPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    if((GT_NOT_IMPLEMENTED != st) && (GT_NOT_INITIALIZED != st))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                    }
                    st = cpssPxPortSerdesLaneTuningSet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                    st = cpssPxPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portGroupId, laneNum);
                }
            }

            /* serdesFrequency is not valid */
            st = cpssPxPortSerdesLaneTuningGet(dev, portGroupId, laneNum, serdesFrequency, &tuneValues);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, laneNum);
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        serdesFrequency = CPSS_PORT_SERDES_SPEED_1_25_E;

        /* portGroupId is not valid */
        st = cpssPxPortSerdesLaneTuningGet(dev, portGroupId, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId, 0);

        /* pointer  is not valid */
        st = cpssPxPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, &tuneValues);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesLaneTuningGet(dev, 0, 0, serdesFrequency, &tuneValues);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesResetStateSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesResetStateSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet: %d, %d",
                                         dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            /*
               1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssPxPortSerdesResetStateSet(dev, port, enable);
            if (IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssPxPortSerdesResetStateSet(dev, port, enable);
            if (IS_PORT_FE_E(portType))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortSerdesResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        /* enable == GT_TRUE  */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortSerdesResetStateSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesAutoTune)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters portTuningMode
            [CPSS_PX_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_E/
             CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E].
    Expected: GT_OK.
    1.1.2. Call with out of range portTuningMode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORTS_BMP_STC  portsBmp;
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT portTuningMode =
        CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
    GT_BOOL supported;
    GT_BOOL                                              isError = GT_FALSE;
#ifndef GM_USED
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              canSave = GT_TRUE;
#endif
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifndef GM_USED
        /* Save active ports parameters */
        st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_NOT_APPLICABLE_DEVICE == st)
        {
            canSave = GT_FALSE;
        }
        else if(GT_OK != st)
        {
           return;
        }

        /* Deactivation ports */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }
        }
#endif

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceSpeedSupportGet(dev, port,
                                                      CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                                      CPSS_PORT_SPEED_10000_E,
                                                      &supported);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }


            if(!supported)
            {
                continue;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, port);
            st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                          /* configure mode using 1 serdes */
                                          CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                          CPSS_PORT_SPEED_10000_E);

                if(st != GT_OK)
                {
#ifndef GM_USED
                    goto port_power_down;
#else
                    continue;
#endif
                }


            /* 1.1.1 */

            portTuningMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E;
            st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            portTuningMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E;
            st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_NO_RETURN_MAC(GT_OK, st, dev, port);
            if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
            {
                isError = GT_TRUE;
                goto restore;
            }

            portTuningMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E;
            st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
            /* fail - here is legal result of tuning process,
               in simulation always will fail */
            if((st != GT_OK) && (st != GT_FAIL))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "cpssPxPortSerdesAutoTune(TX_TRAINING_STATUS) FAILED:%d,%d\n",
                              dev, port);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }

            portTuningMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E;
            st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
            /* fail - here is legal result of tuning process,
               in simulation always will fail */
            if((st != GT_OK) && (st != GT_FAIL))
            {
                GT_CHAR     errorMsg[256]; /* error message string */
                cpssOsSprintf(errorMsg,
                              "cpssPxPortSerdesAutoTune(RX_TRAINING) FAILED:%d,%d\n",
                              dev, port);
                (GT_VOID)utfFailureMsgLog(errorMsg, NULL, 0);
            }

            /* 1.1.2. */
            UTF_ENUMS_CHECK_MAC(cpssPxPortSerdesAutoTune
                                (dev, port, portTuningMode),
                                 portTuningMode);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/

#ifndef GM_USED
port_power_down:
            /* Deactivate tested port */
            if(GT_TRUE == canSave)
            {
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                              CPSS_PORT_INTERFACE_MODE_SR_LR_E,
                                              CPSS_PORT_SPEED_10000_E);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                              portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
#endif

        }

restore:
#ifndef GM_USED
        /*  Restore ports Activation with initial parameters */
        if(GT_TRUE == canSave)
        {
            st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPxPortsActivateDeactivate()", dev );
            if((GT_OK != st) || (GT_TRUE == isError))
            {
                return;
            }
        }
#else
        if(GT_TRUE == isError)
        {
            return;
        }
#endif

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesAutoTune(dev, port, portTuningMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEomBaudRateGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    serdesNum,
    OUT GT_U32                   *baudRatePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEomBaudRateGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with serdesNum[0 / maxSerdesNum/2 / maxSerdesNum] and
        valid baudRatePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range serdesNum[maxSerdesNum + 1] and other
        valid parameters same as 1.1.1
    Expected: GT_BAD_PARAM.
    1.1.3. Call with invalid baudRatePtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    CPSS_PORT_INTERFACE_MODE_ENT ifMode  = CPSS_PORT_INTERFACE_MODE_SR_LR_E;

    GT_STATUS                   st               = GT_OK;
    GT_U8                       devNum           = 0;
    GT_PHYSICAL_PORT_NUM        portNum          = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_U32                      serdesNum        = 0;
    GT_U32                      maxSerdesNum     = 0;
    GT_U32                      startSerdes      = 0;
    GT_U32                      numOfSerdesLanes = 0;
    GT_U32                      baudRate         = 0;




    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            GT_BOOL                            valid;


            static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
            CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

            valid = GT_FALSE;
            st = cpssPxPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapShadowPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (GT_TRUE == portMapShadowPtr->valid)
            {
                st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    valid = GT_FALSE;
                }
                else /* (numOfSerdesLanes > 0) */
                {
                    valid = GT_TRUE;
                }
            }


            if (valid == GT_TRUE)
            {
                st = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                                        &startSerdes, &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssPxPortIfModeSerdesNumGet() : : dev = %d port = %d",devNum, portNum);

                /* Get maxSerdesNum for specific devNum and portNum*/
                maxSerdesNum = numOfSerdesLanes - 1;
                /*
                    1.1.1. Call with serdesNum [0] and valid baudRatePtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = 0;
                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.1. Call with serdesNum [maxSerdesNum/2] and
                                valid baudRatePtr [non-NULL].
                        Expected: GT_OK.
                */
                serdesNum = maxSerdesNum/2;
                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.1. Call with serdesNum [maxSerdesNum] and
                                valid baudRatePtr [non-NULL].
                        Expected: GT_OK.
                */
                serdesNum = maxSerdesNum;
                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                        1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and
                                other valid parameters same as 1.1.1
                        Expected: GT_BAD_PARAM.
                */
                serdesNum = maxSerdesNum + 1;
                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                    serdesNum);

                /* restore valid values */
                serdesNum = 0;

                /*
                    1.1.3. Call with invalid baudRatePtr [NULL] and other
                            valid parameters same as 1.1.1.
                    Expected: GT_BAD_PTR.
                */

                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum, serdesNum);
            }
            else
            {
                serdesNum = 0;
                st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum, serdesNum);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum,
                                                                    &baudRate);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    serdesNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEomBaudRateGet(devNum, portNum, serdesNum, &baudRate);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesAutoTuneStatusGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *rxTuneStatusPtr,
    OUT CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT *txTuneStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesAutoTuneStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid rxTuneStatusPtr [non-NULL] and
        valid txTuneStatusPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid rxTuneStatusPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with invalid txTuneStatusPtr [NULL] and other
        valid parameters same as 1.1.1..
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st          =   GT_OK;
    GT_U8                       devNum      =   0;
    GT_PHYSICAL_PORT_NUM        portNum     =   PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus =
                                        CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;
    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus =
                                        CPSS_PORT_SERDES_AUTO_TUNE_PASS_E;


    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1.  go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            /*
                1.1.1. Call with valid rxTuneStatusPtr [non-NULL] and
                    valid txTuneStatusPtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
            if(GT_NOT_INITIALIZED != st)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,portNum);
            }

            /*
                1.1.2. Call with invalid rxTuneStatusPtr [NULL] and other
                    valid parameters same as 1.1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                          NULL, &txTuneStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,portNum);

            /*
                1.1.3. Call with invalid txTuneStatusPtr [NULL] and other
                    valid parameters same as 1.1.1..
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                          &rxTuneStatus, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,portNum);
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */


    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesAutoTuneStatusGet(devNum, portNum,
                                                 &rxTuneStatus, &txTuneStatus);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesPolaritySet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneBmp[0/0xFFFF/0xFFFFFFFF]
            invertTx [GT_FALSE/GT_TRUE]
            invertRx [GT_FALSE/GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesPolarityGet
    Expected: GT_OK and the same mode.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneBmp   = 0;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    GT_U32                maxLaneBmp;

    GT_BOOL               invertTx    = GT_FALSE;
    GT_BOOL               invertRx    = GT_FALSE;
    GT_BOOL               invertTxGet = GT_FALSE;
    GT_BOOL               invertRxGet = GT_FALSE;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

            /*  find max value of laneNum  */
            maxLaneNum = 0;

                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);
            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /*  max relevant bitmap of lane numbers  */
            maxLaneBmp = 0;
            maxLaneBmp   = (1 << (maxLaneNum+1)) - 1;

            /* 1.1.1 */
            laneBmp   = 1;
            invertTx  = GT_FALSE;
            invertRx  = GT_FALSE;


            st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = 0;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode) && ((port & 0x3) > 0))
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);

            /* 1.1.1 */
            laneBmp   = maxLaneBmp;
            invertTx  = GT_TRUE;
            invertRx  = GT_TRUE;
            st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = 0;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);
#endif
            /* 1.1.2 */
            laneNum   = maxLaneNum;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTxGet, &invertRxGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(invertTx, invertTxGet, "get another invertTx that expected:%d,%d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(invertRx, invertRxGet, "get another invertRx that expected:%d,%d", dev, port);
#endif
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesPolaritySet(dev, port, laneBmp, invertTx, invertRx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesPolarityGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneNum[0/maxNumber of laneNum]
            and not NULL invertTx, invertRx.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum
    Expected: NOT GT_OK.
    1.1.3. Call with NULL invertTx
    Expected: GT_BAD_PTR.
    1.1.4. Call with NULL invertRx
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;

    GT_BOOL               invertTx    = GT_FALSE;
    GT_BOOL               invertRx    = GT_FALSE;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /* 1.1.1 */
            laneNum = 0;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1 */
            laneNum = maxLaneNum;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum = maxLaneNum +1;
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            laneNum = 0;

            /* 1.1.3 */
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, NULL, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

            /* 1.1.4 */
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesPolarityGet(dev, port, laneNum, &invertTx, &invertRx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesLoopbackModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            mode [CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E/
                  CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E/
                  CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortSerdesLoopbackModeGet
    Expected: GT_OK and the same mode.
    1.1.3. Call with out of range mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */


    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT mode = CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT modeGet = CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }

                st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);



            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);



            /* 1.1.1 */
            mode = CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E;

            st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum = 0;
            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &modeGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
            /*  Verify values  */
            UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "get another mode that expected:%d,%d", dev, port);
#endif
            /* 1.1.1 */
            mode = CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E;
            st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
            if(0 == numOfSerdesLanes)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* 1.1.2 */
            laneNum = 0;
            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &modeGet);
            if(0 == numOfSerdesLanes)
            {
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
#ifndef ASIC_SIMULATION
                /*  Verify values  */
                UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet, "get another mode that expected:%d,%d", dev, port);
#endif
            }


            /* 1.1.3. */
            UTF_ENUMS_CHECK_MAC(cpssPxPortSerdesLoopbackModeSet(dev, port, mode),
                                mode);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesLoopbackModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesLoopbackModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS
    1.1.1. Call with valid parameters
            laneNum[0/max Value for current device and port]
            and not NULL modePtr.
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum
    Expected: NOT GT_OK.
    1.1.3. Call with NULL modePtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                 dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                laneNum   = 0;
    GT_U32                startSerdes;    /* first serdes of port */
    GT_U32                numOfSerdesLanes;/* number of serdeses occupied now by port */
    GT_U32                maxLaneNum;
    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT mode = CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        if(prvUtfIsGmCompilation())
        {/* GM doesn't support cpssPxPortPhysicalPortMapGet and other low level
            features */
            SKIP_TEST_MAC
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            st = cpssPxPortPhysicalPortMapGet(dev, port, 1, portMapArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = prvCpssPxPortIfModeSerdesNumGet(dev, port, ifMode,
                                                   &startSerdes, &numOfSerdesLanes);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet:%d,%d", dev, port);

            if (numOfSerdesLanes == 0)
            {
                maxLaneNum = 0;
            }
            else
            {
                maxLaneNum = numOfSerdesLanes - 1;
            }

            /* 1.1.1 */
            laneNum   = 0;

            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2 */
            laneNum   = maxLaneNum;
            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.3 */
            laneNum   = maxLaneNum + 1;

            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            laneNum   = 0;

            /* 1.1.4 */
            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
#ifdef ASIC_SIMULATION
            port += PORT_STEP_FOR_REDUCED_TIME_CNS;
#endif /*ASIC_SIMULATION*/
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1.  */
            st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. */
        port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSerdesLoopbackModeGet(dev, port, laneNum, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortEomDfeResGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  serdesNum,
    OUT GT_U32                 *dfeResPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortEomDfeResGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion2)
    1.1.1. Call with serdesNum [0 / maxSerdesNum/2 / maxSerdesNum] and
        valid dfeResPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with invalid dfeResPtr [NULL] and other
        valid parameters same as 1.1.1.
    Expected: GT_BAD_PTR.
*/

    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR_E;

    GT_STATUS                st                 = GT_OK;
    GT_U8                    devNum             = 0;
    GT_PHYSICAL_PORT_NUM     portNum            = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                   serdesNum          = 0;
    GT_U32                   maxSerdesNum       = 0;
    GT_U32                   startSerdes        = 0;
    GT_U32                   numOfSerdesLanes   = 0;
    GT_U32                   dfeRes[CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS] = {0};

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            GT_BOOL                            valid;

            static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
            CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

            valid = GT_FALSE;
            st = cpssPxPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapShadowPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (GT_TRUE == portMapShadowPtr->valid)
            {
                st = prvUtfPortInterfaceModeGet(devNum,portNum,&ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet", devNum, portNum);
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    valid = GT_FALSE;
                }
                else /* (numOfSerdesLanes > 0) */
                {
                    valid = GT_TRUE;
                }
            }



            if (valid == GT_TRUE)
            {
                st = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                                &startSerdes, &numOfSerdesLanes);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvCpssPxPortIfModeSerdesNumGet() : : dev = %d port = %d",devNum, portNum);
                maxSerdesNum = numOfSerdesLanes - 1;

                /*
                    1.1.1. Call with serdesNum [0] and valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = 0;
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.1. Call with serdesNum [maxSerdesNum/2] and
                            valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = maxSerdesNum/2;
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.1. Call with serdesNum [maxSerdesNum] and
                            valid dfeResPtr [non-NULL].
                    Expected: GT_OK.
                */
                serdesNum = maxSerdesNum;
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, portNum, serdesNum);

                /*
                    1.1.2. Call with out of range serdesNum [maxSerdesNum + 1] and
                            other valid parameters same as 1.1.1.
                    Expected: GT_BAD_PARAM.
                */
                serdesNum = maxSerdesNum + 1;
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                serdesNum);

                /* restore valid values */
                serdesNum = 0;

                /*
                    1.1.3. Call with invalid dfeResPtr [NULL] and other
                            valid parameters same as 1.1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, NULL);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum,
                                                                    serdesNum);
            }
            else
            {
                serdesNum = 0;
                st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum,
                                                                    serdesNum);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        serdesNum = 0;

        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        serdesNum = 0;
        portNum = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    serdesNum = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortEomDfeResGet(devNum, portNum, serdesNum, dfeRes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

static GT_STATUS prvCpssPxPortEyeResultMemoryFree
(
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
{
    if(NULL != eye_resultsPtr->matrixPtr)
    {
        cpssOsFree(eye_resultsPtr->matrixPtr);
        eye_resultsPtr->matrixPtr = NULL;
    }
    if(NULL != eye_resultsPtr->vbtcPtr)
    {
        cpssOsFree(eye_resultsPtr->vbtcPtr);
        eye_resultsPtr->vbtcPtr = NULL;
    }
    if(NULL != eye_resultsPtr->hbtcPtr)
    {
        cpssOsFree(eye_resultsPtr->hbtcPtr);
        eye_resultsPtr->hbtcPtr = NULL;
    }
    return GT_OK;
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSerdesEyeMatrixGet
(
    IN  GT_U8                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_U32                                serdesNum,
    IN  CPSS_PX_PORT_SERDES_EYE_INPUT_STC  *eye_inputPtr,
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC *eye_resultsPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSerdesEyeMatrixGet)
{
    GT_STATUS                                 st       = GT_OK;
    GT_U8                                     dev      = 0;
    GT_PHYSICAL_PORT_NUM                      port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32                                    serdes   = 0;
    GT_U32                                    portMacNum = 0;
    GT_U32                                    numOfSerdesLanes = 0;
    GT_U32                                    startSerdes = 0;
    OUT CPSS_PX_PORT_SERDES_EYE_RESULT_STC  eye_results;
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC       eye_input;
   /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    if (cpssDeviceRunCheck_onEmulator()) 
    {
        SKIP_TEST_MAC
    }

    eye_results.matrixPtr = NULL;
    eye_results.vbtcPtr = NULL;
    eye_results.hbtcPtr = NULL;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. For all active devices go over all available physical ports. */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
                st = prvCpssPxPortPhysicalPortMapCheckAndConvert(dev, port, PRV_CPSS_PX_PORT_TYPE_MAC_E, &portMacNum);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortPhysicalPortMapCheckAndConvert: device: %d, port: %d", dev, port);


            st = prvCpssPxPortIfModeSerdesNumGet(dev, port,
                            PRV_CPSS_PX_PORT_IFMODE_MAC(dev, portMacNum),
                            &startSerdes, &numOfSerdesLanes);
            if(GT_OK == st)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssPxPortIfModeSerdesNumGet: device: %d, port: %d", dev, port);
                for(serdes = 0; serdes < numOfSerdesLanes; serdes++)
                {
                    eye_input.min_dwell_bits = 100000;
                    eye_input.max_dwell_bits = 100000000;
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
#ifndef ASIC_SIMULATION
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#else
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_NOT_SUPPORTED, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#endif
#ifndef ASIC_SIMULATION
                    if((NULL == eye_results.matrixPtr) || (NULL == eye_results.vbtcPtr) || (NULL == eye_results.hbtcPtr)
                        || (0 == eye_results.x_points) || (0 == eye_results.y_points))
                    {
                        st = GT_FAIL;
                        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FAIL, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    }
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, NULL, &eye_results);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, NULL);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PTR, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 10000;
                    eye_input.max_dwell_bits = 100000000;
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 100000;
                    eye_input.max_dwell_bits = 1000000000;
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
                    eye_input.min_dwell_bits = 1000000;
                    eye_input.max_dwell_bits = 100000;
                    st = cpssPxPortSerdesEyeMatrixGet(dev, port, serdes, &eye_input, &eye_results);
                    prvCpssPxPortEyeResultMemoryFree(&eye_results);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_BAD_PARAM, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d, serdes: &d", dev, port, serdes);
#endif
                }
            }
        }

#ifndef ASIC_SIMULATION
        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        eye_input.min_dwell_bits = 100000;
        eye_input.max_dwell_bits = 100000000;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
            prvCpssPxPortEyeResultMemoryFree(&eye_results);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "cpssPxPortSerdesEyeMatrixGet: device: %d, port: %d", dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssPxPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
        prvCpssPxPortEyeResultMemoryFree(&eye_results);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* 1.4. For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = CPSS_CPU_PORT_NUM_CNS;
       st = cpssPxPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
       prvCpssPxPortEyeResultMemoryFree(&eye_results);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

#endif

    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,UTF_NONE_FAMILY_E);
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
        prvCpssPxPortEyeResultMemoryFree(&eye_results);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortSerdesEyeMatrixGet(dev, port, 0, &eye_input, &eye_results);
    prvCpssPxPortEyeResultMemoryFree(&eye_results);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortDuplexModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
*/
GT_VOID cpssPxPortDuplexModeSetUT(GT_VOID)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with dMode [CPSS_PORT_FULL_DUPLEX_E].
    Expected: GT_OK.
    1.1.2. Call with dMode [CPSS_PORT_HALF_DUPLEX_E].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED for 10Gbps ports.
    1.1.3. Call with wrong enum values dMode
    Expected: GT_BAD_PARAM
    */

    GT_STATUS            st   = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DUPLEX_ENT mode = CPSS_PORT_FULL_DUPLEX_E;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PX_PORT_MAP_STC   portMapArray[1];

    cpssOsBzero((GT_VOID*) &portMapArray, sizeof(portMapArray));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /* 1.1.1. Call with dMode [CPSS_PORT_FULL_DUPLEX_E].
               Expected: GT_OK.
            */

            /* Call with mode [CPSS_PORT_FULL_DUPLEX_E] */
            mode = CPSS_PORT_FULL_DUPLEX_E;

            st = cpssPxPortDuplexModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /* 1.1.2. Call with dMode [CPSS_PORT_HALF_DUPLEX_E].
               Expected: GT_OK for non 10Gpbs ports and GT_NOT_SUPPORTED otherwise.
            */

            /* Call with mode [CPSS_PORT_HALF_DUPLEX_E] */
            mode = CPSS_PORT_HALF_DUPLEX_E;

            st = cpssPxPortDuplexModeSet(dev, port, mode);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }

            /*
               1.1.3. Call with wrong enum values dMode
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortDuplexModeSet
                                (dev, port, mode),
                                mode);
        }

        mode = CPSS_PORT_FULL_DUPLEX_E;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* dMode [CPSS_PORT_FULL_DUPLEX_E]               */
            st = cpssPxPortDuplexModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* dMode == CPSS_PORT_FULL_DUPLEX_E */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortDuplexModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_PORT_FULL_DUPLEX_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortDuplexModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, dMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssPxPortDuplexModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortDuplexModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortDuplexModeGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with non-null dModePtr.
    Expected: GT_OK.
    1.1.2. Call with dModePtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS            st   = GT_OK;

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DUPLEX_ENT mode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null dModePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortDuplexModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with dModePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortDuplexModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }


        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortDuplexModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortDuplexModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortDuplexModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortDuplexModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSpeedGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSpeedGet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with non-null speedPtr.
    Expected: GT_OK.
    1.1.2. Call with speedPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS           st    = GT_OK;

    GT_U8               dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_SPEED_ENT speed;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null speedPtr.
               Expected: GT_OK.
            */
            st = cpssPxPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with speedPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortSpeedGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortSpeedGet(dev, port, &speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSpeedGet(dev, port, &speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSpeedGet(dev, port, &speed);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSpeedGet(dev, port, &speed);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortDuplexAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortDuplexAutoNegEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with state [GT_FALSE].
        Expected: GT_OK.
        1.1.2. Call with state [GT_TRUE].
        Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
        1.1.3. Call cpssPxPortDuplexAutoNegEnableGet with not null state.
        Expected: GT_OK and the same value.
    */

    GT_U8                  dev;
    GT_STATUS              st       = GT_OK;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                stateGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE].
               Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* 1.1.2. Call with state [GT_TRUE].
               Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = prvUtfPortTypeGet(dev, port, &portType);

            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);

            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }

            /*
                1.1.3. Call cpssPxPortDuplexAutoNegEnableGet with not null state.
                Expected: GT_OK and the same value.
            */
            /*Call with state [GT_FALSE] */
            state = GT_FALSE;

            st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                     "got another state then was set: %d, %d", dev, port);

            /*Call with state [GT_TRUE] */
            state = GT_TRUE;

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);

            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
                st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &stateGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                         "got another state then was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
        }

        state = GT_TRUE;

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE  */
            st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortDuplexAutoNegEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortDuplexAutoNegEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortDuplexAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (Px)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortDuplexAutoNegEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortDuplexAutoNegEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state,
    IN  GT_BOOL   pauseAdvertise
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowCntrlAutoNegEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with state [GT_FALSE] and pauseAdvertise [GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call with state [GT_TRUE] and pauseAdvertise [GT_FALSE and GT_TRUE].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
    */

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                pause    = GT_FALSE;
    GT_BOOL                stateGet = GT_FALSE;
    GT_BOOL                pauseGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE], pauseAdvertise [GT_TRUE].
               Expected: GT_OK and the same values.
            */
            state = GT_FALSE;
            pause = GT_FALSE;

            st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, pause);
            /* checking values*/
            st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                     "got another state then was set: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                     "got another pause then was set: %d, %d", dev, port);

            /* Getting port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
               1.1.2. Call with state [GT_TRUE]
               pauseAdvertise [GT_FALSE and GT_TRUE].
               Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise..
            */

            /* Call function with pauseAdvertise = GT_FALSE] */
            state = GT_TRUE;
            pause = GT_FALSE;

            st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, pause);
                /* checking values*/
                st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                         "got another state then was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                         "got another pause then was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state, pause);
            }

            /* Call function with pauseAdvertise = GT_TRUE] */
            pause = GT_TRUE;

            st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, state, pause);
                st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &stateGet, &pauseGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                         "got another state then was set: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(pause, pauseGet,
                         "got another pause then was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state, pause);
            }
        }

        state = GT_TRUE;
        pause = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    pause = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE, pause == GT_TRUE */

    st = cpssPxPortFlowCntrlAutoNegEnableSet(dev, port, state, pause);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableGet
(
    IN   GT_U8       devNum,
    IN   GT_U8       portNum,
    OUT  GT_BOOL     *statePtr,
    OUT  GT_BOOL     *pauseAdvertisePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowCntrlAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state;
    GT_BOOL     pause;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, NULL, &pause);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);

            st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortFlowCntrlAutoNegEnableGet(dev, port, &state, &pause);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSpeedAutoNegEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSpeedAutoNegEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with state [GT_FALSE].
    Expected: GT_OK.
    1.1.2. Call with state [GT_TRUE ].
    Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
*/

    GT_STATUS              st       = GT_OK;

    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [GT_FALSE].
                Expected: GT_OK.
            */
            state = GT_FALSE;

            st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* Getting port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            /*
                1.1.2. Call with state [GT_TRUE ].
                Expected: GT_OK for non 10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
            if(PRV_CPSS_PORT_XG_E > portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, state);
            }
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE     */
            st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortSpeedAutoNegEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortSpeedAutoNegEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortSpeedAutoNegEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortSpeedAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortSpeedAutoNegEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortSpeedAutoNegEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortSpeedAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortSpeedAutoNegEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortSpeedAutoNegEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowControlEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowControlEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with state [CPSS_PORT_FLOW_CONTROL_DISABLE_E/
                            CPSS_PORT_FLOW_CONTROL_RX_TX_E  /
                            CPSS_PORT_FLOW_CONTROL_RX_ONLY_E/
                            CPSS_PORT_FLOW_CONTROL_TX_ONLY_E]
    Expected: GT_OK for XG ports and non GT_OK when not XG ports with
              last two values.
    1.1.2. Call cpssPxPortFlowControlEnableGet with the same params.
    Expected: GT_OK and the same value.
    1.1.3. Call with wrong enum value state.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st     = GT_OK;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    GT_U8     dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    CPSS_PORT_FLOW_CONTROL_ENT state  = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with state [CPSS_PORT_FLOW_CONTROL_DISABLE_E/
                                        CPSS_PORT_FLOW_CONTROL_RX_TX_E  /
                                        CPSS_PORT_FLOW_CONTROL_RX_ONLY_E/
                                        CPSS_PORT_FLOW_CONTROL_TX_ONLY_E]
                Expected: GT_OK for XG ports and non GT_OK when not XG ports with
                          last two values.
            */

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_DISABLE_E */
            state = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

            st = cpssPxPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssPxPortFlowControlEnableGet returns actual
                Flow control status but not previouse settings of
                cpssPxPortFlowControlEnableSet.
                The status depends on a lot of other configurations like
                flow control autonegatiation, type of PHY,
                PHY - MAC autonegatiation and so on.
                The status always disabled in simulation.
            */

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_DISABLE_E */
            state = CPSS_PORT_FLOW_CONTROL_RX_TX_E;

            st = cpssPxPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E */
            state = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E;

            st = cpssPxPortFlowControlEnableSet(dev, port, state);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* Call function with state = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E */
            state = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E;

            st = cpssPxPortFlowControlEnableSet(dev, port, state);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            state = CPSS_PORT_FLOW_CONTROL_DISABLE_E;

            /*
                1.1.3. Call with wrong enum value state.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortFlowControlEnableSet
                                (dev, port, state),
                                state);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available physical
           ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssPxPortFlowControlEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowControlEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowControlEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortFlowControlEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowControlEnableGet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT  *statePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowControlEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with non-null statePtr.
    Expected: GT_OK.
    1.1.2. Call with statePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS            st    = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_FLOW_CONTROL_ENT  state;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null statePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortFlowControlEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with statePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortFlowControlEnableGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowControlEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowControlEnableGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortFlowControlEnableGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPeriodicFcEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPeriodicFcEnableSet)
{
    /*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with enable [GT_FALSE and GT_TRUE]. Expected: GT_OK.
    */

    GT_STATUS   st        = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enable    = GT_FALSE;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get type of port */
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortPeriodicFcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortPeriodicFcEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            st = cpssPxPortPeriodicFcEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortPeriodicFcEnableGet: %d, %d", dev, port);

            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);

        }

        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortPeriodicFcEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPeriodicFcEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPeriodicFcEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS (Px)
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortPeriodicFcEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortPeriodicFcEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortPeriodicFcEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPeriodicFcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPeriodicFcEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPeriodicFcEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortBackPressureEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   state
)
*/
UTF_TEST_CASE_MAC(cpssPxPortBackPressureEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS ()
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
        1.1.2. Call cpssPxPortBackPressureEnableGet with not null state.
        Expected: GT_OK for non-10Gbps ports and the same value.
                  GT_NOT_SUPPORTED otherwise.
    */

    GT_STATUS              st       = GT_OK;
    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                state    = GT_FALSE;
    GT_BOOL                stateGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_FALSE;

            st = cpssPxPortBackPressureEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            /*
                1.1.1. Call with state [GT_FALSE and GT_TRUE].
                Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED otherwise.
            */
            state = GT_TRUE;

            st = cpssPxPortBackPressureEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            /*
                1.1.2. Call cpssPxPortBackPressureEnableGet with not null state.
                Expected: GT_OK for non-10Gbps ports and the same value.
                          GT_NOT_SUPPORTED otherwise.
            */
            state = GT_FALSE;
            st = cpssPxPortBackPressureEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*checking value*/
            st = cpssPxPortBackPressureEnableGet(dev, port, &stateGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
             "cpssPxPortBackPressureEnableGet: got another state then was set: %d, %d",
                                         dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. For all active devices go over all non available
            physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE  */
            st = cpssPxPortBackPressureEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */

        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortBackPressureEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBackPressureEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortBackPressureEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortBackPressureEnableGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_BOOL  *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortBackPressureEnableGet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS ()
        1.1.1. Call with non-null enablePtr.
        Expected: GT_OK.
        1.1.2. Call with enablePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                  dev;
    GT_STATUS              st       = GT_OK;
    GT_PHYSICAL_PORT_NUM   port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                enable   = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /* 1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */

            st = cpssPxPortBackPressureEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* 1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortBackPressureEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortBackPressureEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortBackPressureEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortBackPressureEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortBackPressureEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortCrcCheckEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_BOOL     enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortCrcCheckEnableSet)
{
    /*
        ITERATE_DEVICES_PHY_CPU_PORTS ()
        1.1.1. Call with state [GT_FALSE and GT_TRUE].
        Expected: GT_OK.
        1.1.2. Call cpssPxPortCrcCheckEnableGet with not null state.
        Expected: GT_OK and the same value.
    */
    GT_U8       dev;
    GT_STATUS   st       = GT_OK;
    GT_PHYSICAL_PORT_NUM port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     state    = GT_FALSE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with state [GT_FALSE and GT_TRUE].
               Expected: GT_OK and the same value.
            */

            /* Call function with enable = GT_FALSE] */
            state = GT_FALSE;

            st = cpssPxPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /* Call function with enable = GT_TRUE] */
            state = GT_TRUE;

            st = cpssPxPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*
                1.1.2. Call cpssPxPortCrcCheckEnableGet with not null state.
                Expected: GT_OK and the same value.
            */
            state = GT_FALSE;

            st = cpssPxPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);

            /*checking value*/
            st = cpssPxPortCrcCheckEnableGet(dev, port, &stateGet);

            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, state);
            UTF_VERIFY_EQUAL2_STRING_MAC(state, stateGet,
                     "got another state then was set: %d, %d", dev, port);
        }

        state = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state == GT_TRUE */
            st = cpssPxPortCrcCheckEnableSet(dev, port, state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* state == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortCrcCheckEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    state = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortCrcCheckEnableSet(dev, port, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortCrcCheckEnableSet(dev, port, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXGmiiModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortXGmiiModeSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with mode
    [CPSS_PORT_XGMII_LAN_E, CPSS_PORT_XGMII_WAN_E].
    Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. For 10Gbps port call with mode [3].
    Expected: GT_BAD_PARAM.
    */

    GT_STATUS                st       = GT_OK;
    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XGMII_MODE_ENT mode     = CPSS_PORT_XGMII_LAN_E;
    PRV_CPSS_PORT_TYPE_ENT   portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with mode [CPSS_PORT_XGMII_LAN_E /
                                      CPSS_PORT_XGMII_WAN_E].
               Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED for others.
            */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* mode = CPSS_PORT_XGMII_LAN_E */
            mode = CPSS_PORT_XGMII_LAN_E;
            st = cpssPxPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /* mode = CPSS_PORT_XGMII_WAN_E */
            mode = CPSS_PORT_XGMII_WAN_E;

            st = cpssPxPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

            /*
               1.1.2. For 10Gbps port call with wrong enum values mode.
               Expected: GT_BAD_PARAM.
            */
            if(PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_ENUMS_CHECK_MAC(cpssPxPortXGmiiModeSet
                                    (dev, port, mode),
                                    mode);
            }
        }

        mode = CPSS_PORT_XGMII_LAN_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* mode = CPSS_PORT_XGMII_LAN_E                 */
            st = cpssPxPortXGmiiModeSet(dev, port, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* mode = CPSS_PORT_XGMII_LAN_E */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXGmiiModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    mode = CPSS_PORT_XGMII_LAN_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXGmiiModeSet(dev, port, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, mode = CPSS_PORT_XGMII_LAN_E               */

    st = cpssPxPortXGmiiModeSet(dev, port, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXGmiiModeGet
(
    IN  GT_U8                     devNum,
    IN  GT_U8                     portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT *modePtr
)
*/
GT_VOID cpssPxPortXGmiiModeGetUT(GT_VOID)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with non-null modePtr.
        Expected: GT_OK.
        1.1.2. Call with modePtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                    dev;
    GT_STATUS                st       = GT_OK;
    GT_PHYSICAL_PORT_NUM     port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XGMII_MODE_ENT mode;
    PRV_CPSS_PORT_TYPE_ENT   portType = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null modePtr.
               Expected: GT_OK.
            */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = cpssPxPortXGmiiModeGet(dev, port, &mode);

            if(PRV_CPSS_PORT_XG_E <= portType)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, mode);
            }
            /* 1.1.2. Call with modePtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortXGmiiModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortXGmiiModeGet(dev, port, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXGmiiModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXGmiiModeGet(dev, port, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXGmiiModeGet(dev, port, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/* Default valid value for ipg */
#define PORT_IPG_VALID_CNS  8

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortIpgSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U32  ipg
)
*/
UTF_TEST_CASE_MAC(cpssPxPortIpgSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    Expected: GT_NOT_SUPPORTED XG ports GT_OK for others.
    */

    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      ipg      = PORT_IPG_VALID_CNS;
    GT_U32      ipgGet;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    GT_U32      validPorts[1];
    GT_PHYSICAL_PORT_NUM    firstPortNum;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
       /* get ports with MAC */
       prvUtfValidPortsGet(dev ,UTF_GENERIC_PORT_ITERATOR_TYPE_MAC_E,
           0  /*start port */, &validPorts[0] , 1);
       firstPortNum = validPorts[0];

       ipg      = PORT_IPG_VALID_CNS;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1.
               GT_NOT_SUPPORTED XG ports GT_OK for others.
            */
            st = cpssPxPortIpgSet(dev, port, ipg);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortIpgSet(dev, port, ipg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.4 check for valid and invalid IPG values */
        port = firstPortNum;
        /* skip XG ports */

        st = prvUtfPortTypeGet(dev, port, &portMacType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        if(portMacType >= PRV_CPSS_PORT_XG_E)
            continue;

        ipg  = PORT_IPG_VALID_CNS;

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* ipg  = PORT_IPG_VALID_CNS(8), valid for all Px devices */
        st = cpssPxPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssPxPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssPxPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 15, valid for all devices */
        ipg = 15;

        st = cpssPxPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssPxPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                     "cpssPxPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                     "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 16, valid  */
        ipg = 16;

        st = cpssPxPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssPxPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                 "cpssPxPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                 "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 511, valid  */
        ipg = 511;

        st = cpssPxPortIpgSet(dev, port, ipg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

        st = cpssPxPortIpgGet(dev, port, &ipgGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                 "cpssPxPortIpgGet: %d, %d", dev, port);
        UTF_VERIFY_EQUAL2_STRING_MAC(ipg, ipgGet,
                                 "get another enable value than was set: %d, %d", dev, port);

        /* ipg  = 512, invalid */
        ipg = 512;

        st = cpssPxPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OUT_OF_RANGE, st, dev, port, ipg);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;
    ipg  = PORT_IPG_VALID_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortIpgSet(dev, port, ipg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0 */

    st = cpssPxPortIpgSet(dev, port, ipg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortIpgGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT  GT_U32  *ipgPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortIpgGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    Expected: GT_NOT_SUPPORTED XG ports GT_OK for others.
    */

    GT_STATUS   st       = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port     = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      ipg;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /* 1.1.1.
               GT_NOT_SUPPORTED XG ports GT_OK for others.
            */
            st = cpssPxPortIpgGet(dev, port, &ipg);
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                if(GE_NOT_SUPPORTED(portTypeOptions))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    /* 1.1.2. Call with speedPtr [NULL].
                    Expected: GT_BAD_PTR.
                    */
                    st = cpssPxPortIpgGet(dev, port, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortIpgGet(dev, port, &ipg);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortIpgGet(dev, port, &ipg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0 */

    st = cpssPxPortIpgGet(dev, port, &ipg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortExtraIpgSet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    IN  GT_U8       number
)
*/
UTF_TEST_CASE_MAC(cpssPxPortExtraIpgSet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with number [2].
    Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    */

    GT_STATUS   st        = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8       number    = 2;
    GT_U8       numberGet = 1;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with number [2].
               Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssPxPortExtraIpgSet(dev, port, number);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);

            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);

                st = cpssPxPortExtraIpgGet(dev, port, &numberGet);

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);
                UTF_VERIFY_EQUAL2_STRING_MAC(number, numberGet,
                         "got another number then was set: %d, %d", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* number == 2 */

            st = cpssPxPortExtraIpgSet(dev, port, number);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* number == 2 */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortExtraIpgSet(dev, port, number);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    /* number == 2 */
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortExtraIpgSet(dev, port, number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, number == 2 */

    st = cpssPxPortExtraIpgSet(dev, port, number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortExtraIpgGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    OUT  GT_U8    *numberPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortExtraIpgGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with non-null numberPtr.
        Expected: GT_OK.
        1.1.2. Call with numberPtr [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8       dev;
    GT_STATUS   st     = GT_OK;
    GT_PHYSICAL_PORT_NUM  port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U8       number = 1;
    PRV_CPSS_PORT_TYPE_ENT      portMacType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /* 1.1.1. Call with non-null numberPtr.
               Expected: GT_OK.
            */
            st = cpssPxPortExtraIpgGet(dev, port, &number);

            if(portMacType < PRV_CPSS_PORT_XG_E)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, number);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, number);
            }

            /* 1.1.2. Call with numberPtr [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortExtraIpgGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortExtraIpgGet(dev, port, &number);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortExtraIpgGet(dev, port, &number);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortExtraIpgGet(dev, port, &number);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortExtraIpgGet(dev, port, &number);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *isLocalFaultPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortXgmiiLocalFaultGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null isLocalFaultPtr.
    Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call with isLocalFaultPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS               st    = GT_OK;
    GT_U8                   dev;
    GT_BOOL                 state = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;
    GT_PHYSICAL_PORT_NUM port  = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            /* 1.1.1. Call with non-null isLocalFaultPtr.
               Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssPxPortXgmiiLocalFaultGet(dev, port, &state);
            if(portMacType < PRV_CPSS_PORT_XG_E)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isLocalFaultPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortXgmiiLocalFaultGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortXgmiiLocalFaultGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgmiiLocalFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgmiiLocalFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXgmiiLocalFaultGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_U8       portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortXgmiiRemoteFaultGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null isRemoteFaultPtr.
    Expected: GT_OK  for 10-Gbps ports and GT_NOT_SUPPORTED for others.
    1.1.2. Call with isRemoteFaultPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS               st    = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port  = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                 state = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portMacType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.1. Call with non-null isRemoteFaultPtr.
               Expected: GT_OK for 10-Gbps ports and GT_NOT_SUPPORTED for others.
            */
            st = cpssPxPortXgmiiRemoteFaultGet(dev, port, &state);
            if(portMacType < PRV_CPSS_PORT_XG_E)
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with isRemoteFaultPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortXgmiiRemoteFaultGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortXgmiiRemoteFaultGet(dev, port, &state);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgmiiRemoteFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgmiiRemoteFaultGet(dev, port, &state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXgmiiRemoteFaultGet(dev, port, &state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacStatusGet
(
    IN  GT_U8                      devNum,
    IN  GT_U8                      portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacStatusGet)
{
    /*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null portMacStatusPtr.
    Expected: GT_OK.
    1.1.2. Call with portMacStatusPtr [NULL].
    Expected: GT_BAD_PTR.
    */

    GT_STATUS                st     = GT_OK;
    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_MAC_STATUS_STC status;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with non-null portMacStatusPtr.
               Expected: GT_OK.
            */
            st = cpssPxPortMacStatusGet(dev, port, &status);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* 1.1.2. Call with portMacStatusPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortMacStatusGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortMacStatusGet(dev, port, &status);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacStatusGet(dev, port, &status);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacStatusGet(dev, port, &status);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacStatusGet(dev, port, &status);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInbandAutoNegEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_U8    portNum,
    IN  GT_BOOL  enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInbandAutoNegEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED for 10Gbps.
*/
    GT_STATUS              st        = GT_OK;
    GT_U8                  dev;
    GT_PHYSICAL_PORT_NUM   port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL                enable    = GT_FALSE;
    GT_BOOL                enableGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT portType  = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* 1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK for non-10Gbps ports and GT_NOT_SUPPORTED for 10Gbps.
            */

            /* enable = GT_FALSE */
            enable = GT_FALSE;
            st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            st = cpssPxPortInbandAutoNegEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInbandAutoNegEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);

            /* enable = GT_TRUE */
            enable = GT_TRUE;

            st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            st = cpssPxPortInbandAutoNegEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInbandAutoNegEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable value than was set: %d, %d", dev, port);
        }

        enable = GT_TRUE;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* state = GT_TRUE */
            st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, state == GT_TRUE */

    st = cpssPxPortInbandAutoNegEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortAttributesOnPortGet
(
    IN    GT_U8                     devNum,
    IN    GT_U8                     portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortAttributesOnPortGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with non-null portAttributSetArrayPtr.
    Expected: GT_OK.
    1.1.2. Call with portAttributSetArrayPtr [NULL].
    Expected: GT_BAD_PTR.
*/

    GT_STATUS                st  = GT_OK;
    GT_U8                    dev;
    GT_PHYSICAL_PORT_NUM     port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_ATTRIBUTES_STC attributes;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null portAttributSetArrayPtr.
               Expected: GT_OK.
            */
            st = cpssPxPortInterfaceModeGet(dev,port,&ifMode);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            st = cpssPxPortAttributesOnPortGet(dev, port, &attributes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with portAttributSetArrayPtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortAttributesOnPortGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortAttributesOnPortGet(dev, port, &attributes);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortAttributesOnPortGet(dev, port, &attributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortAttributesOnPortGet(dev, port, &attributes);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortAttributesOnPortGet(dev, port, &attributes);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_U8                    portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPreambleLengthSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E] and length [8, 4]
    Expected: GT_OK.
    1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                CPSS_PORT_DIRECTION_BOTH_E] and length [4]
    Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED  for others.
    1.1.3. Call cpssPxPortPreambleLengthGet with the same parameters
    as in 1.1.1 and 1.1.2.
    Expected: GT_OK and the same value.
    1.1.4. Call with direction [4] and length [4, 8]
    Expected: GT_BAD_PARAM.
    1.1.5. Call with direction [CPSS_PORT_DIRECTION_TX_E] and length [1, 2]
    Expected: NON GT_OK.
*/

    GT_STATUS               rc, st       = GT_OK;
    GT_U8                   dev;
    GT_PHYSICAL_PORT_NUM    port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_DIRECTION_ENT direction = CPSS_PORT_DIRECTION_RX_E;
    GT_U32                  length    = 0;
    GT_U32                  lengthGet = 1;
    PRV_CPSS_PORT_TYPE_ENT  portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    GT_STATUS               expSt; /* expected status */
    GT_BOOL                 valid;
    CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /*
                1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E], length [8, 4]
               Expected: GT_OK.
            */

            direction = CPSS_PORT_DIRECTION_TX_E;

            /* Call with length = 8 */
            length = 8;

            st = cpssPxPortPreambleLengthSet(dev, port, direction, length);

            if(PRV_CPSS_PORT_GE_E == portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                /*
                    1.1.3. Call cpssPxPortPreambleLengthGet with the same parameters
                    as in 1.1.1 and 1.1.2.
                    Expected: GT_OK and the same value.
                */
                st = cpssPxPortPreambleLengthGet(dev, port, direction, &lengthGet);

                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                UTF_VERIFY_EQUAL2_STRING_MAC(length, lengthGet,
                                 "cpssPxPortPreambleLengthGet: %d, %d", dev, port);
            }

            /* Call with length = 4 */
            length = 4;

            st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
            if(PRV_CPSS_PORT_GE_E == portType)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                /*
                    1.1.3. Call cpssPxPortPreambleLengthGet with the same parameters
                    as in 1.1.1 and 1.1.2.
                    Expected: GT_OK and the same value.
                */
                st = cpssPxPortPreambleLengthGet(dev, port, direction, &lengthGet);

                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);

                UTF_VERIFY_EQUAL2_STRING_MAC(length, lengthGet,
                                 "cpssPxPortPreambleLengthGet: %d, %d", dev, port);
            }

            /*
                1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E /
                                           CPSS_PORT_DIRECTION_BOTH_E], length [4]
               Expected: GT_OK for 10Gbps ports and GT_NOT_SUPPORTED  for others.
            */

            /* Call with direction = CPSS_PORT_DIRECTION_RX_E */
            direction = CPSS_PORT_DIRECTION_RX_E;
            length = 4;


            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            valid = portMapShadow.valid;

            if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
            {
                st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /* Call with direction = CPSS_PORT_DIRECTION_BOTH_E */
            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
            rc = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
            valid = portMapShadow.valid;

            if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
            {
                    UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
            }
            /*
                1.1.3. Call with wrong enum values direction , length [4]
               Expected: GT_BAD_PARAM.
            */
            /* length == 4; */
            UTF_ENUMS_CHECK_MAC(cpssPxPortPreambleLengthSet
                                (dev, port, direction, length),
                                direction);

            /*
               1.1.4. Call with direction [CPSS_PORT_DIRECTION_TX_E], length [1, 2]
               Expected: NON GT_OK.

            */

            direction = CPSS_PORT_DIRECTION_TX_E;
            expSt = GT_OK;

            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/&portMapShadow);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            valid = portMapShadow.valid;

            if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
            {
                /* Call with length = 1 */
                length = 1;
                st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
                UTF_VERIFY_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);

                /* Call with length = 2 */
                length = 2;
                st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
                UTF_VERIFY_EQUAL4_PARAM_MAC(expSt, st, dev, port, direction, length);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        length = 4;
        direction = CPSS_PORT_DIRECTION_TX_E;

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* direction == CPSS_PORT_DIRECTION_TX_E */

            st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    length = 4;
    direction = CPSS_PORT_DIRECTION_TX_E;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, direction == CPSS_PORT_DIRECTION_TX_E, length == 4 */

    st = cpssPxPortPreambleLengthSet(dev, port, direction, length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPreambleLengthGet
(
    IN  GT_U8                    devNum,
    IN  GT_U8                    portNum,
    IN  CPSS_PORT_DIRECTION_ENT  direction,
    OUT GT_U32                  *length
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPreambleLengthGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E].
    Expected: GT_OK for GE ports and not GT_OK for others.
    1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E].
    Expected: GT_OK for XG 10Gbps ports and not GT_OK for others.
    1.1.3. Call with direction [CPSS_PORT_DIRECTION_BOTH_E].
    Expected: GT_BAD_PARAM.
    1.1.4. Call with wrong enum values direction.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with length [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_U8                           dev;
    GT_STATUS                       st        = GT_OK;
    GT_PHYSICAL_PORT_NUM            port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT          portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PORT_DIRECTION_ENT         direction = CPSS_PORT_DIRECTION_RX_E;
    GT_U32                          length;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_PX_DETAILED_PORT_MAP_STC   *portMapShadowPtr = &portMapShadow;
    GT_STATUS                       rc;
    GT_BOOL                         valid;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(dev, port, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInterfaceModeGet: %d, %d",
                                         dev, port);
            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with direction [CPSS_PORT_DIRECTION_TX_E].
                Expected: GT_OK
            */

            direction = CPSS_PORT_DIRECTION_TX_E;

            st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
            if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
            }

            /*
                1.1.2. Call with direction [CPSS_PORT_DIRECTION_RX_E].
                Expected: GT_OK for XG ports and GT_NOT_SUPPORTED for others.
            */

            direction = CPSS_PORT_DIRECTION_RX_E;

            st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);


            rc = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, port);
            valid = portMapShadowPtr->valid;
            if(ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                valid = GT_FALSE;
            }

            if (valid == GT_TRUE) /* GE mac shall fail , XLG pass OK, total result -- OK */
            {
                UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
                /*we don't call get for direction = CPSS_PORT_DIRECTION_BOTH_E */
            }
            else
            {
                /* port is not mapped */
                UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, direction, length);
            }

            /*
                1.1.3. Call with direction [CPSS_PORT_DIRECTION_BOTH_E].
                Expected: GT_BAD_PARAM.
            */

            direction = CPSS_PORT_DIRECTION_BOTH_E;

            st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, dev, port, direction, length);


            /*
                1.1.4. Call with wrong enum values direction.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortPreambleLengthGet
                                (dev, port, direction, &length),
                                direction);

            /*
               1.1.5. Call with  NULL lenghtGet [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPreambleLengthGet(dev, port, direction, NULL);
            UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, dev, port, direction, length);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_OK */
        /* for CPU port number.                                            */
        direction = CPSS_PORT_DIRECTION_TX_E;
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPreambleLengthGet(dev, port, direction, &length);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacSaBaseSet)
{
/*
    ITERATE_DEVICES ()
    1.1. Call with mac {arEther[10, 20, 30, 40, 50, 60]}.
    Expected: GT_OK.
    1.2. Call cpssPxPortMacSaBaseGet with not-NULL macPtr.
    Expected: GT_OK and the same mac as was set.
    1.3. Call with mac[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st  = GT_OK;
    GT_U8           dev;
    GT_ETHERADDR    mac;
    GT_ETHERADDR    macGet;
    GT_BOOL         isEqual = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &mac, sizeof(mac));
    cpssOsBzero((GT_VOID*) &macGet, sizeof(macGet));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with mac {arEther[10, 20, 30, 40, 50, 0]}.
            Expected: GT_OK.
        */
        mac.arEther[0] = 10;
        mac.arEther[1] = 20;
        mac.arEther[2] = 30;
        mac.arEther[3] = 40;
        mac.arEther[4] = 50;
        mac.arEther[5] = 0;

        st = cpssPxPortMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssPxPortMacSaBaseGet with not-NULL macPtr.
            Expected: GT_OK and the same mac as was set.
        */
        st = cpssPxPortMacSaBaseGet(dev, &macGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssPxPortMacSaBaseGet: %d", dev);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &mac, (GT_VOID*) &macGet, sizeof(mac))) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another mac then was set: %d", dev);

        /*
            1.3. Call with mac[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortMacSaBaseSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mac = NULL" ,dev);
    }

    mac.arEther[0] = 10;
    mac.arEther[1] = 20;
    mac.arEther[2] = 30;
    mac.arEther[3] = 40;
    mac.arEther[4] = 50;
    mac.arEther[5] = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacSaBaseSet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacSaBaseSet(dev, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacSaBaseGet)
{
/*
    ITERATE_DEVICES ()
    1.1. Call with not-NULL mac
    Expected: GT_OK.
    1.2. Call with mac[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st  = GT_OK;
    GT_U8           dev;
    GT_ETHERADDR    mac;

    cpssOsBzero((GT_VOID*) &mac, sizeof(mac));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not-NULL mac
            Expected: GT_OK.
        */
        st = cpssPxPortMacSaBaseGet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with mac[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssPxPortMacSaBaseGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mac = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacSaBaseGet(dev, &mac);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortMacSaBaseGet(dev, &mac);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPaddingEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPaddingEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT ()
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPaddingEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable - GT_FALSE */

                enable = GT_FALSE;

                st = cpssPxPortPaddingEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                  1.1.2. Call cpssPxPortPaddingEnableSet with not-NULL enablePtr.
                  Expected: GT_OK and the same enable as was set.
                */
                st = cpssPxPortPaddingEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortPaddingEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
            /* iterate with enable - GT_TRUE */
            enable = GT_TRUE;

            st = cpssPxPortPaddingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssPxPortPaddingEnableSet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssPxPortPaddingEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortPaddingEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortPaddingEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPaddingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPaddingEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPaddingEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPaddingEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPaddingEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT ()
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssPxPortPaddingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call enablePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPaddingEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortPaddingEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPaddingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPaddingEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPaddingEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortExcessiveCollisionDropEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortExcessiveCollisionDropEnableSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT ()
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;

            st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {/* this is GE specific feature */
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }

            /*
                1.1.2. Call cpssPxPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortExcessiveCollisionDropEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);

            /* iterate with enable - GT_TRUE */
            enable = GT_TRUE;

            st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /*
                1.1.2. Call cpssPxPortExcessiveCollisionDropEnableGet with not-NULL enablePtr.
                Expected: GT_OK and the same enable as was set.
            */
            st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortExcessiveCollisionDropEnableGet: %d, %d", dev, port);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortExcessiveCollisionDropEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortExcessiveCollisionDropEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortExcessiveCollisionDropEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (Px)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portTypeOptions;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeOptionsGet(dev,port,&portTypeOptions);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enable);
            if(GE_NOT_SUPPORTED(portTypeOptions))
            {/* this is GE specific feature */
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }

            /*
                1.1.2. Call enablePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortExcessiveCollisionDropEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInBandAutoNegBypassEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInBandAutoNegBypassEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT ()
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (!IS_PORT_XG_E(portType))
            {
                /* Set Inband Auto-Negotiation */
                st = cpssPxPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssPxPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);

                /*
                    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                    Expected: GT_OK.
                */
                /* iterate with enable - GT_FALSE */
                enable = GT_FALSE;

                st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                    1.1.2. Call cpssPxPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
                    Expected: GT_OK and the same enable as was set.
                */
                st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInBandAutoNegBypassEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);

                /* iterate with enable - GT_TRUE */
                enable = GT_TRUE;

                st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                /*
                    1.1.2. Call cpssPxPortInBandAutoNegBypassEnableGet with not-NULL enablePtr.
                    Expected: GT_OK and the same enable as was set.
                */
                st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enableGet);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortInBandAutoNegBypassEnableGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortInBandAutoNegBypassEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInBandAutoNegBypassEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInBandAutoNegBypassEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (Px)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_BOOL     enable    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (!IS_PORT_XG_E(portType))
            {
                /* Set Inband Auto-Negotiation */
                st = cpssPxPortInbandAutoNegEnableSet(dev, port, GT_TRUE);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                           "cpssPxPortInbandAutoNegEnableSet: %d, %d, GT_TRUE", dev, port);
                /*
                    1.1.1. Call with not-NULL enable.
                    Expected: GT_OK.
                */
                st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enable);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call enablePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortInBandAutoNegBypassEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortIpgBaseGet
(
    IN   GT_U8                        devNum,
    IN   GT_U8                        portNum,
    OUT  CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBase
)
*/
UTF_TEST_CASE_MAC(cpssPxPortIpgBaseGet)
{
    /*
        ITERATE_DEVICES_PHY_PORTS ()
        1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                    CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
        Expected: GT_OK.
        1.1.2. Call with  [NULL].
        Expected: GT_BAD_PTR
    */
    GT_U8                        dev;
    GT_STATUS                    st        = GT_OK;
    GT_PHYSICAL_PORT_NUM         port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_XG_FIXED_IPG_ENT   ipgBase;
    PRV_CPSS_PORT_TYPE_ENT       portType  = PRV_CPSS_PORT_NOT_EXISTS_E;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /*
               1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                           CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
               Expected: GT_OK.
            */
            st = cpssPxPortIpgBaseGet(dev, port, &ipgBase);

            if(PRV_CPSS_PORT_XG_E <= portType)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            /* 1.1.2. Call with  [NULL].
               Expected: GT_BAD_PTR
            */
            st = cpssPxPortIpgBaseGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortIpgBaseGet(dev, port, &ipgBase);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortIpgBaseGet(dev, port, &ipgBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortIpgBaseGet(dev, port, &ipgBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortIpgBaseGet(dev, port, &ipgBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortIpgBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
*/
UTF_TEST_CASE_MAC(cpssPxPortIpgBaseSet)
{
/*
    ITERATE_DEVICES_PHY_PORT ( )
    1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
    Expected: GT_OK.
    1.1.2. Call cpssPxPortIpgBaseSet with not-NULL ipgBase.
    Expected: GT_OK and the same ipgBase as was set.
*/
    GT_STATUS                   st        = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    PRV_CPSS_PORT_TYPE_ENT      portType  = PRV_CPSS_PORT_NOT_EXISTS_E;
    CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase   = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;
    CPSS_PORT_XG_FIXED_IPG_ENT  ipgBaseGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);
            /*
                1.1.1. Call with ipgBase = [CPSS_PORT_XG_FIXED_IPG_8_BYTES_E /
                                            CPSS_PORT_XG_FIXED_IPG_12_BYTES_E]
                Expected: GT_OK.
            */
            ipgBase = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

            st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssPxPortIpgBaseSet: %d, %d, GT_TRUE", dev, port);

            ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

            st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssPxPortIpgBaseSet: %d, %d, GT_TRUE", dev, port);

            /*
                1.1.2. Call cpssPxPortIpgBaseGet with not-NULL ipgBase.
                Expected: GT_OK and the same ipgBase as was set.
            */
            st = cpssPxPortIpgBaseGet(dev, port, &ipgBaseGet);

            if(PRV_CPSS_PORT_XG_E <= portType)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                       "cpssPxPortIpgBaseGet: %d, %d", dev, port);
                UTF_VERIFY_EQUAL2_STRING_MAC(ipgBase, ipgBaseGet,
                    "got another ipgBase then was set: %d, %d", dev, port);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            /*  1.1.3. Call with wrong enum values ipgBase.   */
            /*  Expected: GT_BAD_PARAM.                     */
            if(PRV_CPSS_PORT_XG_E == portType)
            {
                UTF_ENUMS_CHECK_MAC(cpssPxPortIpgBaseSet
                                    (dev, port, ipgBase),
                                    ipgBase);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port    = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

        st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port    = PORT_CTRL_VALID_PHY_PORT_CNS;
    ipgBase = CPSS_PORT_XG_FIXED_IPG_12_BYTES_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    ipgBase = CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

    st = cpssPxPortIpgBaseSet(dev, port, ipgBase);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowControlModeSet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    IN  CPSS_PX_PORT_FC_MODE_ENT  fcMode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowControlModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with fcMode [CPSS_PX_PORT_FC_MODE_802_3X_E /
                             CPSS_PX_PORT_FC_MODE_PFC_E /
                             CPSS_PX_PORT_FC_MODE_LL_FC_E]
    Expected: GT_OK.
    1.1.2. Call with cpssPxPortFlowControlModeGet with the same fcMode.
    Expected: GT_OK and the same fcMode.
    1.1.3. Call with wrong enum values fcMode
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_PHYSICAL_PORT_NUM        port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_FC_MODE_ENT    fcMode = CPSS_PX_PORT_FC_MODE_802_3X_E;
    CPSS_PX_PORT_FC_MODE_ENT    fcModeGet;
    static CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with fcMode [CPSS_PX_PORT_FC_MODE_802_3X_E /
                                         CPSS_PX_PORT_FC_MODE_PFC_E /
                                         CPSS_PX_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            st = cpssPxPortPhysicalPortDetailedMapGet(dev,port,/*OUT*/portMapShadowPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            fcMode = CPSS_PX_PORT_FC_MODE_802_3X_E;

            st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssPxPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssPxPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
                1.1.1. Call with fcMode [CPSS_PX_PORT_FC_MODE_802_3X_E /
                                         CPSS_PX_PORT_FC_MODE_PFC_E /
                                         CPSS_PX_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            fcMode = CPSS_PX_PORT_FC_MODE_PFC_E;

            st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssPxPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssPxPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
                1.1.1. Call with fcMode [CPSS_PX_PORT_FC_MODE_802_3X_E /
                                         CPSS_PX_PORT_FC_MODE_PFC_E /
                                         CPSS_PX_PORT_FC_MODE_LL_FC_E]
               Expected: GT_OK.
            */

            fcMode = CPSS_PX_PORT_FC_MODE_LL_FC_E;

            st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, fcMode);

            if (GT_OK == st)
            {
                /*
                    1.1.2. Call with cpssPxPortFlowControlModeGet with the same fcMode.
                    Expected: GT_OK and the same fcMode.
                */

                st = cpssPxPortFlowControlModeGet(dev, port, &fcModeGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, fcMode);

                UTF_VERIFY_EQUAL2_STRING_MAC(fcMode, fcModeGet,
                            "get another fcMode value than was set: %d, %d", dev, port);
            }

            /*
               1.1.3. Call with wrong enum values fcMode
               Expected: GT_BAD_PARAM
            */
            UTF_ENUMS_CHECK_MAC(cpssPxPortFlowControlModeSet
                                (dev, port, fcMode),
                                fcMode);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* fcMode [CPSS_PORT_FULL_DUPLEX_E]               */
            st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, fcMode == CPSS_PORT_FULL_DUPLEX_E */

    st = cpssPxPortFlowControlModeSet(dev, port, fcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortFlowControlModeGet
(
    IN  GT_U8                 devNum,
    IN  GT_U8                 portNum,
    OUT CPSS_PX_PORT_FC_MODE_ENT  *fcModePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortFlowControlModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (Lion)
    1.1.1. Call with non-null fcModePtr.
    Expected: GT_OK.
    1.1.2. Call with fcModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS            st   = GT_OK;

    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_FC_MODE_ENT fcMode;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null fcModePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortFlowControlModeGet(dev, port, &fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with fcModePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortFlowControlModeGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortFlowControlModeGet(dev, port, &fcMode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortFlowControlModeGet(dev, port, &fcMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortFlowControlModeGet(dev, port, &fcMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortFlowControlModeGet(dev, port, &fcMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortMacResetStateSet
(
    IN  GT_U8     devNum,
    IN  GT_U8     portNum,
    IN  GT_BOOL   enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortMacResetStateSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS ()
    1.1.1. Call with enable [GT_FALSE and GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with enable [GT_FALSE and GT_TRUE].
               Expected: GT_OK.
            */

            /* Call function with enable = GT_TRUE */
            enable = GT_TRUE;
            st = cpssPxPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

            /* Call function with enable = GT_FALSE */
            enable = GT_FALSE;

            st = cpssPxPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE    */
            st = cpssPxPortMacResetStateSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE  */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortMacResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortMacResetStateSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, enable == GT_TRUE */

    st = cpssPxPortMacResetStateSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForward802_3xEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForward802_3xEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS            st     = GT_OK;
    GT_U8                dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL              enable = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxPortForward802_3xEnableGet(dev, port, &enable);
            if(!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortForward802_3xEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortForward802_3xEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForward802_3xEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForward802_3xEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForward802_3xEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForward802_3xEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForward802_3xEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortForward802_3xEnableGet.
    Expected: GT_OK and the same enable value as was set.
*/
    GT_STATUS   st     = GT_OK;

    GT_U8                 dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL               enable    = GT_FALSE;
    GT_BOOL               enableGet = GT_TRUE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable [GT_FALSE] */

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            enable = GT_FALSE;
            st = cpssPxPortForward802_3xEnableSet(dev, port, enable);
            if(!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortForward802_3xEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortForward802_3xEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;
            st = cpssPxPortForward802_3xEnableSet(dev, port, enable);

            if (!IS_PORT_XG_E(portType))
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortForward802_3xEnableGet(dev, port, &enableGet);

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortForward802_3xEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssPxPortForward802_3xEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForward802_3xEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForward802_3xEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForward802_3xEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForwardUnknownMacControlFramesEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS()
    1.1.1. Call with non-null enablePtr.
    Expected: GT_OK.
    1.1.2. Call with enablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port   = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable = GT_FALSE;

    /* Prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
               1.1.1. Call with non-null enablePtr.
               Expected: GT_OK.
            */
            st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
               1.1.2. Call with enablePtr [NULL].
               Expected: GT_BAD_PTR.
            */
            st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssPxPortForwardUnknownMacControlFramesEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS (xCat2)
    1.1.1. Call with enable [GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortForwardUnknownMacControlFramesEnableGet.
    Expected: GT_OK and the same enable value as was set.
*/
    GT_STATUS   st     = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port      = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_TRUE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /*
                1.1.1. Call with enable [GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            /* Call function with enable [GT_FALSE] */
            enable = GT_FALSE;
            st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortForwardUnknownMacControlFramesEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);

            /* Call function with enable [GT_TRUE] */
            enable = GT_TRUE;
            st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.2. Call cpssPxPortForward802_3xEnableGet.
                Expected: GT_OK and the same enable value as was set.
            */
            st = cpssPxPortForwardUnknownMacControlFramesEnableGet(dev, port, &enableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssPxPortForwardUnknownMacControlFramesEnableGet: %d", dev);
            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                    "get another enable than was set: %d, %d", dev, port);
        }

        /*
           1.2. For all active devices go over all non available
           physical ports.
        */
        enable = GT_TRUE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            /* enable == GT_TRUE */
            st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* enable == GT_TRUE */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    enable = GT_TRUE;
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortForwardUnknownMacControlFramesEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPeriodicFlowControlCounterSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPeriodicFlowControlCounterSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with value [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPeriodicFlowControlCounterSet with non-NULL value.
    Expected: GT_OK and value the same as just written.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      value    = 0;
    GT_U32      valueGet = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* there is no MAC/PCS/RXDMA/TXDMA in GM */
        GM_NOT_SUPPORT_THIS_TEST_MAC;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* 1.1.1. Call with value [0 / 250].
               Expected: GT_OK.
            */
            /* iterate with value = 0 */
            value = 0;
            st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, value);

            /*
                1.1.2. Call cpssPxPortPeriodicFlowControlCounterGet with non-NULL value.
                Expected: GT_OK and value the same as just written.
            */
            st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &valueGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortPeriodicFlowControlCounterGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                         "got another value as was written: %d, %d", dev, port);

            /* iterate with value = 250 */
            value = 250;
            st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, value);

            /*
                1.1.2. Call cpssPxPortPeriodicFlowControlCounterGet .
                Expected: GT_OK and valueGet the same as just written.
            */
            st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &valueGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortPeriodicFlowControlCounterGet: %d, %d", dev, port);

            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(value, valueGet,
                                         "got another value as was written: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        value = 0;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    value = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssPxPortPeriodicFlowControlCounterSet(dev, port, value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPeriodicFlowControlCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPeriodicFlowControlCounterGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with value [0 / 250].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPeriodicFlowControlCounterGet with non-NULL value.
    Expected: GT_OK and value the same as just written.
*/
    GT_STATUS   st       = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM   port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32      value    = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        GM_NOT_SUPPORT_THIS_TEST_MAC;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &value);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortPeriodicFlowControlCounterGet: %d, %d", dev, port);
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        value = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &value);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/

    value = 0;
    port     = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &value);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssPxPortPeriodicFlowControlCounterGet(dev, port, &value);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortVosOverrideControlModeGet
(
    IN   GT_U8       devNum,
    OUT  GT_BOOL     *vosOverridePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortVosOverrideControlModeGet)
{
    GT_U8    devNum;
    GT_BOOL  vosOverride;
    GT_STATUS st;

    vosOverride = GT_FALSE;

    /* prepare not applicable device iterator  */
    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Setting VOS override mode to true */
        vosOverride = GT_TRUE;
        st = cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, vosOverride, vosOverride);
        /* Setting VOS override mode to false */
        vosOverride = GT_FALSE;
        st = cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, vosOverride, vosOverride);
    }

    /* Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortVosOverrideControlModeSet
(
    IN   GT_U8       devNum,
    IN  GT_BOOL     vosOverride
)
*/
UTF_TEST_CASE_MAC(cpssPxPortVosOverrideControlModeSet)
{
    GT_U8    devNum;
    GT_BOOL  vosOverride;
    GT_STATUS st;

    vosOverride = GT_FALSE;

    /* prepare not applicable device iterator  */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortVosOverrideControlModeSet(devNum, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* Setting VOS override mode to true */
        vosOverride = GT_TRUE;
        st = cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, vosOverride, vosOverride);
        /* Setting VOS override mode to false */
        vosOverride = GT_FALSE;
        st = cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        st = cpssPxPortVosOverrideControlModeGet(devNum, &vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, vosOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FALSE, vosOverride, vosOverride);
    }

    /* Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssPxPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          value
)*/
UTF_TEST_CASE_MAC(cpssPxPortXlgReduceAverageIPGSet)
{
    GT_STATUS                       st   = GT_OK;
    GT_U8                           dev  = 0;
    GT_PHYSICAL_PORT_NUM            port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_U32                          valueSource = 0x0000000C;
    GT_U32                          valueResult = 0;

    /* prepare not applicable device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

#ifdef GM_USED
        /* test is not ready for GM for Bobk */
        continue;
#endif

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            if(CPSS_PORT_INTERFACE_MODE_NA_E != portIfMode)
            {
                st = cpssPxPortXlgReduceAverageIPGSet(dev, port, valueSource);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(valueSource, valueResult, dev, port);
                st = cpssPxPortXlgReduceAverageIPGSet(dev, port, 0);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                UTF_VERIFY_EQUAL2_PARAM_MAC(0, valueResult, dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortXlgReduceAverageIPGSet(dev, port, valueSource);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXlgReduceAverageIPGSet(dev, port, valueSource);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXlgReduceAverageIPGSet(dev, port, valueSource);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXlgReduceAverageIPGSet(dev, port, valueSource);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*GT_STATUS cpssPxPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U16                          value
)*/
UTF_TEST_CASE_MAC(cpssPxPortXlgReduceAverageIPGGet)
{
    GT_STATUS                       st   = GT_OK;
    GT_U8                           dev  = 0;
    GT_PHYSICAL_PORT_NUM            port = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_U32                          valueResult = 0;

    /* prepare not applicable device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
        /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            if(CPSS_PORT_INTERFACE_MODE_NA_E != portIfMode)
            {
                st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                st = cpssPxPortXlgReduceAverageIPGGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available
           physical ports.
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    /* 2. For not active devices check that function returns non GT_OK.*/

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXlgReduceAverageIPGGet(dev, port, &valueResult);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPcsLoopbackModeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT      *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPcsLoopbackModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with valid modePtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid modePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                            st      = GT_OK;
    GT_U8                                devNum  = 0;
    GT_PHYSICAL_PORT_NUM                 portNum = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT   mode;
    CPSS_PORT_INTERFACE_MODE_ENT         ifMode;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssPxPortInterfaceModeGet: %d, %d", devNum, portNum);

            if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
            {
                continue;
            }
            /*
                1.1.1. Call with valid modePtr [non-NULL].
                Expected: GT_OK.
            */
            st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

            /*
                1.1.2. Call with invalid modePtr [NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
        }

        /*
            1.2. For all active devices go over all non available
                physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortPcsLoopbackModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    mode
)
*/
UTF_TEST_CASE_MAC(cpssPxPortPcsLoopbackModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with mode [CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E,
                           CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E,
                           CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortPcsLoopbackModeGet().
    Expected: GT_OK and the same parameters value as was set in 1.1.1.
    1.1.3. Call with out of range enum value mode and other parameters as 1.1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS            st         = GT_OK;
    GT_U8                devNum     = 0;
    GT_PHYSICAL_PORT_NUM portNum    = PORT_CTRL_VALID_PHY_PORT_CNS;
    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    mode    = 0;
    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT    modeRet = 0;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PX_DETAILED_PORT_MAP_STC  portMapShadow;
    GT_BOOL valid;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssPxPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            valid = portMapShadow.valid;

            if(valid == GT_TRUE)
            {
                st = prvUtfPortTypeGet(devNum, portNum, &portType);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d", devNum, portNum);

                st = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                        "cpssPxPortInterfaceModeGet: %d, %d", devNum, portNum);

                if(CPSS_PORT_INTERFACE_MODE_NA_E == ifMode)
                {
                    continue;
                }
                /*
                    1.1.1. Call with mode [CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E;

                st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
                if(portType == PRV_CPSS_PORT_GE_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, devNum, portNum);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                    /*
                        1.1.2. Call cpssPxPortPcsLoopbackModeGet().
                        Expected: GT_OK and the same parameters value as was set.
                    */
                    st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                    /* verifying values */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);
                }

                /*
                    1.1.1. Call with mode [CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E;

                st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*
                    1.1.2. Call cpssPxPortPcsLoopbackModeGet().
                    Expected: GT_OK and the same parameters value as was set.
                */
                st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);

                /*
                    1.1.1. Call with mode [CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E,
                                           CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E].
                    Expected: GT_OK.
                */
                mode = CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E;

                st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /*
                    1.1.2. Call cpssPxPortPcsLoopbackModeGet().
                    Expected: GT_OK and the same parameters value as was set.
                */
                st = cpssPxPortPcsLoopbackModeGet(devNum, portNum, &modeRet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

                /* verifying values */
                UTF_VERIFY_EQUAL2_PARAM_MAC(mode, modeRet, devNum, portNum);

                /*
                    1.1.3. Call with out of range enum value mode
                        and other parameters as 1.1.1.
                    Expected: GT_BAD_PARAM.
                */
                UTF_ENUMS_CHECK_MAC(cpssPxPortPcsLoopbackModeSet
                                    (devNum, portNum, mode),
                                    mode);
            }
        }

        /*
            1.2. For all active devices go over all non available
                 physical ports.
        */
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&portNum, GT_FALSE))
        {
            st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portNum = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);

        st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortPcsLoopbackModeSet(devNum, portNum, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgLanesSwapEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_BOOL    enable
)
*/
/* UTF_TEST_CASE_MAC(cpssPxPortXgLanesSwapEnableSet) */
GT_VOID cpssPxPortXgLanesSwapEnableSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT ()
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortXgLanesSwapEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            CPSS_PORT_SPEED_ENT speed;

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            st = cpssPxPortSpeedGet(dev,port,/*OUT*/&speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if (speed != CPSS_PORT_SPEED_NA_E)
            {
                /* Supported for XAUI or HyperG.Stack ports only */
                if((IS_PORT_XG_E(portType)) && (IS_XAUI_SUPPORTED_ON_PORT_MAC(port)))
                {
                    /*
                        1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                        Expected: GT_OK.
                    */
                    /* iterate with enable - GT_FALSE */
                    enable = GT_FALSE;

                    st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }

                    /*
                        1.1.2. Call cpssPxPortPaddingEnableSet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enableGet);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortXgLanesSwapEnableGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                    }

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }


                    /*
                        1.1.2. Call cpssPxPortXgLanesSwapEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enableGet);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                    {
                        UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssPxPortXgLanesSwapEnableGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet, "got another enable then was set: %d, %d", dev, port);
                    }
                }
            }
        }

        enable = GT_FALSE;
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXgLanesSwapEnableSet(dev, port, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgLanesSwapEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssPxPortXgLanesSwapEnableGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT ()
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_BOOL     enable    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    GT_PHYSICAL_PORT_NUM    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /* Supported for XAUI or HyperG.Stack ports only */
            if((IS_PORT_XG_E(portType)) && (IS_XAUI_SUPPORTED_ON_PORT_MAC(port)))
            {
                /*
                    1.1.1. Call with not-NULL enable.
                    Expected: GT_OK.
                */
                st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enable);
                if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                /*
                    1.1.2. Call enablePtr[NULL].
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortXgLanesSwapEnableGet(dev, port, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortXgLanesSwapEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgPscLanesSwapSet
(
    IN GT_U8 devNum,
    IN GT_U8 portNum,
    IN GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    IN GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
*/
/* UTF_TEST_CASE_MAC(cpssPxPortXgPscLanesSwapSet) */
GT_VOID cpssPxPortXgPscLanesSwapSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with rxSerdesLaneArr [0,1,2,3 / 0,1,2,3],
                     txSerdesLaneArr [0,1,2,3 / 3,2,1,0].
    Expected: GT_OK.
    1.1.2. Call cpssPxPortXgPscLanesSwapGet with not NULL pointers.
    Expected: GT_OK and the same params as was set.
    1.1.3. Call with out of range rxSerdesLaneArr [0] [4]
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.4. Call with out of range txSerdesLaneArr [0] [4]
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.5. Call with rxSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.6. Call with txSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                     and other params from 1.1.
    Expected: NOT GT_OK.
    1.1.7. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.1.8. Call with txSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;

    GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];

    GT_U32 rxSerdesLaneArrGet[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArrGet[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_BOOL checkSuccess;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            CPSS_PORT_SPEED_ENT speed;


            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            st = cpssPxPortSpeedGet(dev,port,/*OUT*/&speed);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if ((speed != CPSS_PORT_SPEED_NA_E) && (IS_XAUI_SUPPORTED_ON_PORT_MAC(port)))
            {
                /* Supported for XAUI or HyperG.Stack ports only! */
                if((portType == PRV_CPSS_PORT_XG_E) || (portType == PRV_CPSS_PORT_XLG_E))
                {
                    /*
                        1.1.1. Call with rxSerdesLaneArr [0,1,2,3 / 0,1,2,3],
                                         txSerdesLaneArr [0,1,2,3 / 3,2,1,0].
                        Expected: GT_OK.
                    */
                    /* iterate with 0 */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 3;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                        if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                        {
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        }


                    /*
                        1.1.2. Call cpssPxPortXgPscLanesSwapGet with not NULL pointers.
                        Expected: GT_OK and the same params as was set.
                    */
                    checkSuccess = GT_FALSE;
                    st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArrGet, txSerdesLaneArrGet);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        checkSuccess = GT_TRUE;
                    }

                    if(checkSuccess)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        /* verifuing values */
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[0], rxSerdesLaneArrGet[0],
                                                     "got another rxSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[1], rxSerdesLaneArrGet[1],
                                                     "got another rxSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[2], rxSerdesLaneArrGet[2],
                                                     "got another rxSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[3], rxSerdesLaneArrGet[3],
                                                     "got another rxSerdesLaneArr[3] than was set: %d, %d", dev, port);

                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[0], txSerdesLaneArrGet[0],
                                                     "got another txSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[1], txSerdesLaneArrGet[1],
                                                     "got another txSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[2], txSerdesLaneArrGet[2],
                                                     "got another txSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[3], txSerdesLaneArrGet[3],
                                                     "got another txSerdesLaneArr[3] than was set: %d, %d", dev, port);
                    }

                    /* iterate with 3 */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    txSerdesLaneArr[0] = 3;
                    txSerdesLaneArr[1] = 2;
                    txSerdesLaneArr[2] = 1;
                    txSerdesLaneArr[3] = 0;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }

                    /*
                        1.1.2. Call cpssPxPortXgPscLanesSwapGet with not NULL pointers.
                        Expected: GT_OK and the same params as was set.
                    */
                    st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArrGet, txSerdesLaneArrGet);
                    if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    else
                    {
                        checkSuccess = GT_TRUE;
                    }

                    if(checkSuccess)
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        /* verifuing values */
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[0], rxSerdesLaneArrGet[0],
                                                     "got another rxSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[1], rxSerdesLaneArrGet[1],
                                                     "got another rxSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[2], rxSerdesLaneArrGet[2],
                                                     "got another rxSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(rxSerdesLaneArr[3], rxSerdesLaneArrGet[3],
                                                     "got another rxSerdesLaneArr[3] than was set: %d, %d", dev, port);

                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[0], txSerdesLaneArrGet[0],
                                                     "got another txSerdesLaneArr[0] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[1], txSerdesLaneArrGet[1],
                                                     "got another txSerdesLaneArr[1] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[2], txSerdesLaneArrGet[2],
                                                     "got another txSerdesLaneArr[2] than was set: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(txSerdesLaneArr[3], txSerdesLaneArrGet[3],
                                                     "got another txSerdesLaneArr[3] than was set: %d, %d", dev, port);
                    }

                    /*
                        1.1.3. Call with out of range rxSerdesLaneArr [0] [4]
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    rxSerdesLaneArr[0] = 4;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, rxSerdesLaneArr[0] = %d",
                                                     dev, port, rxSerdesLaneArr[0]);

                    rxSerdesLaneArr[0] = 0;

                    /*
                        1.1.4. Call with out of range txSerdesLaneArr [0] [4]
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    txSerdesLaneArr[0] = 4;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "%d, %d, txSerdesLaneArr[0] = %d",
                                                     dev, port, txSerdesLaneArr[0]);

                    txSerdesLaneArr[0] = 3;

                    /*
                        1.1.5. Call with rxSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 2;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, rxSerdesLaneArr[2] = %d, rxSerdesLaneArr[3] = %d",
                                                     dev, port, rxSerdesLaneArr[2], rxSerdesLaneArr[3]);

                    rxSerdesLaneArr[0] = 0;
                    rxSerdesLaneArr[1] = 1;
                    rxSerdesLaneArr[2] = 2;
                    rxSerdesLaneArr[3] = 3;

                    /*
                        1.1.6. Call with txSerdesLaneArr [0, 1, 2, 2] (same SERDES lane)
                                         and other params from 1.1.
                        Expected: NOT GT_OK.
                    */
                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 2;

                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                    UTF_VERIFY_NOT_EQUAL4_STRING_MAC(GT_OK, st, "%d, %d, txSerdesLaneArr[2] = %d, txSerdesLaneArr[3] = %d",
                                                     dev, port, txSerdesLaneArr[2], txSerdesLaneArr[3]);

                    txSerdesLaneArr[0] = 0;
                    txSerdesLaneArr[1] = 1;
                    txSerdesLaneArr[2] = 2;
                    txSerdesLaneArr[3] = 3;

                    /*
                        1.1.3. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssPxPortXgPscLanesSwapSet(dev, port, NULL, txSerdesLaneArr);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, rxSerdesLaneArr = NULL", dev, port);

                    /*
                        1.1.4. Call with txSerdesLaneArr [NULL] and other params from 1.1.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, txSerdesLaneArr = NULL", dev, port);
                }
            }
        }

        /*
            1.2. For all active devices go over all non available physical
            ports.
        */
        rxSerdesLaneArr[0] = 0;
        rxSerdesLaneArr[1] = 1;
        rxSerdesLaneArr[2] = 2;
        rxSerdesLaneArr[3] = 3;

        txSerdesLaneArr[0] = 0;
        txSerdesLaneArr[1] = 1;
        txSerdesLaneArr[2] = 2;
        txSerdesLaneArr[3] = 3;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    rxSerdesLaneArr[0] = 0;
    rxSerdesLaneArr[1] = 1;
    rxSerdesLaneArr[2] = 2;
    rxSerdesLaneArr[3] = 3;

    txSerdesLaneArr[0] = 0;
    txSerdesLaneArr[1] = 1;
    txSerdesLaneArr[2] = 2;
    txSerdesLaneArr[3] = 3;

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssPxPortXgPscLanesSwapSet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortXgPscLanesSwapGet
(
    IN GT_U8 devNum,
    IN GT_U8 portNum,
    OUT GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS],
    OUT GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS]
)
*/
UTF_TEST_CASE_MAC(cpssPxPortXgPscLanesSwapGet)
{
/*
    ITERATE_DEVICES_PHY_PORTS ()
    1.1.1. Call with not NULL rxSerdesLaneArr and txSerdesLaneArr;
    Expected: GT_OK.
    1.1.2. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.1.3. Call with txSerdesLaneArr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = PORT_CTRL_VALID_PHY_PORT_CNS;
    GT_U32 rxSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    GT_U32 txSerdesLaneArr[CPSS_PX_PORT_XG_PSC_LANES_NUM_CNS];
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            /* get port type */
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                         "prvUtfPortTypeGet: %d, %d", dev, port);

            /* Supported for XAUI or HyperG.Stack ports only! */
            if(((portType == PRV_CPSS_PORT_XG_E) || (portType == PRV_CPSS_PORT_XLG_E)) && (IS_XAUI_SUPPORTED_ON_PORT_MAC(port)))
            {
                /*
                    1.1.1. Call with not NULL rxSerdesLaneArr and txSerdesLaneArr;
                    Expected: GT_OK.
                */
                st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
                if(prvUtfPortMacModuloCalc(dev, port, 2) != 0)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }


                /*
                    1.1.2. Call with rxSerdesLaneArr [NULL] and other params from 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortXgPscLanesSwapGet(dev, port, NULL, txSerdesLaneArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, rxSerdesLaneArr = NULL", dev, port);

                /*
                    1.1.3. Call with txSerdesLaneArr [NULL] and other params from 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, NULL);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%D, %d, txSerdesLaneArr = NULL", dev, port);
            }
        }

        /* 1.2. For all active devices go over all non available physical
           ports.
        */
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        /* macSaLsb == 0 */
        port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    /* port == 0, macSaLsb == 0 */

    st = cpssPxPortXgPscLanesSwapGet(dev, port, rxSerdesLaneArr, txSerdesLaneArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortAutoNegAdvertismentConfigGet
(
    IN   GT_SW_DEV_NUM                             devNum,
    IN   GT_PHYSICAL_PORT_NUM                     portNum,
    OUT  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)*/
UTF_TEST_CASE_MAC(cpssPxPortAutoNegAdvertismentConfigGet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC              portAnAdvertisment;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Save active ports parameters */
        st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
         {
            return;
        }

        /* Deactivation ports */
        st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
        if(GT_OK != st)
        {
            isError = GT_TRUE;
            goto restore;
        }

        /* For all active devices go over all available physical ports. */
        for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
        {

            if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
            {
                /* Activate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for Bad pointer */
                st = cpssPxPortAutoNegAdvertismentConfigGet(dev, portActive.Data[portActive.Current].portNum, NULL);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PTR, st,"Bad Pointer   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* For all active devices go over all non available physical ports. */
                st = prvCpssPxIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                 /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                st = cpssPxPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                     /* Getting Advertisement Config */
                    st = cpssPxPortAutoNegAdvertismentConfigGet(dev,portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegAdvertismentConfigGet : Device %d  Port %d", dev, portActive.Data[portActive.Current].portNum);
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                            isError = GT_TRUE;
                            goto restore;
                    }
                }

                /* Deactivate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, portActive.Data->portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
        }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPxPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssPxPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssPxPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
       st = cpssPxPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortAutoNegAdvertismentConfigGet(dev,port, &portAnAdvertisment);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortAutoNegAdvertismentConfigSet
(
    IN   GT_SW_DEV_NUM                               devNum,
    IN   GT_PHYSICAL_PORT_NUM                portNum,
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *portAnAdvertismentPtr
)*/
UTF_TEST_CASE_MAC(cpssPxPortAutoNegAdvertismentConfigSet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC              portAnAdvertisment;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

       /* Save active ports parameters */
       st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
       st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
       if(GT_OK != st)
       {
           isError = GT_TRUE;
           goto restore;
       }

       /* For all active devices go over all available physical ports. */
       for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
       {

           if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
           {

                /* Activate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_TRUE,
                                               portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                               portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for port mapping */
                st = prvCpssPxIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                portAnAdvertisment.link = GT_TRUE;
                portAnAdvertisment.duplex = CPSS_PORT_FULL_DUPLEX_E;
                portAnAdvertisment.speed = CPSS_PORT_SPEED_1000_E;
                st = cpssPxPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

             /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegAdvertismentConfigSet(dev, portActive.Data[portActive.Current].portNum, &portAnAdvertisment);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    /* Setting Advertisment Configuration and checking */
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_100_E, " link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_10_E, " link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_100_E, " link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_TRUE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_10_E, " link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " no link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_100_E, " no link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_FULL_DUPLEX_E, CPSS_PORT_SPEED_10_E, " no link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_1000_E, " no link  full duplex  1000 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_100_E, " no link  full duplex  100 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                    st = prvcpssPxPortAutoNegAdvertismentConfigSetCheck(dev, portActive.Data[portActive.Current].portNum,
                        GT_FALSE, CPSS_PORT_HALF_DUPLEX_E, CPSS_PORT_SPEED_10_E, " no link  full duplex  10 Mbps ");
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }
                /* Deactivate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
           }
      }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPxPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
       st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

   }

   port = PORT_CTRL_VALID_PHY_PORT_CNS;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
   /* Go over all non active devices. */
   while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
   {
       st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
   }

   /* Call function with out of bound value for device id */
   dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
   st = cpssPxPortAutoNegAdvertismentConfigSet(dev, port, &portAnAdvertisment);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*GT_STATUS cpssPxPortAutoNegMasterModeEnableGet
(
    IN   GT_SW_DEV_NUM            devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL                 *enablePtr
)*/
UTF_TEST_CASE_MAC(cpssPxPortAutoNegMasterModeEnableGet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    GT_BOOL                                              enable;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;

    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /* Save active ports parameters */
        st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
        if(GT_OK != st)
        {
            return;
        }

        /* Deactivation ports */
        st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
        if(GT_OK != st)
        {
            isError = GT_TRUE;
            goto restore;
        }

        /* For all active devices go over all available physical ports. */
        for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
        {

            if((portActive.Data[portActive.Current].macNum > 47) || (0 == portActive.Data[portActive.Current].macNum % 4))
            {
                /* Activate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* Checking for Bad pointer */
                st = cpssPxPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, NULL);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PTR, st,"Bad Pointer   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                if((GT_BAD_PTR != st) && (GT_FALSE == utfContinueFlagGet()))
                {
                    isError = GT_TRUE;
                    goto restore;
                }

                /* For all active devices go over all non available physical ports. */
                st = prvCpssPxIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
                if (st != GT_OK)
                {
                    continue;
                }

                 /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
                st = cpssPxPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data[portActive.Current].ifMode)
                {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
                }
                else if((portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_10_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data[portActive.Current].speed != CPSS_PORT_SPEED_1000_E))
                {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d",
                       dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

                /* Set port interface QSGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d",
                        dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 2500 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
                if(GT_OK == st)
                {
                    st = cpssPxPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Set port interface SGMII and port speed 1000 Mbps */
                st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);

                if(GT_OK == st)
                {
                     /* Getting Master Mode */
                    st = cpssPxPortAutoNegMasterModeEnableGet(dev, portActive.Data[portActive.Current].portNum, &enable);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "cpssPxPortAutoNegMasterModeEnableGet : Device %d  Port %d",
                        dev, portActive.Data[portActive.Current].portNum);
                    if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
                }

                /* Deactivate tested port */
                st = prvPxPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                    portActive.Data[portActive.Current].portNum );
                if(GT_OK != st)
                {
                    isError = GT_TRUE;
                    goto restore;
                }
            }
        }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPxPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &enable);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                         */
       port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &enable);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                            */
       port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
       st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &enable);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssPxPortAutoNegMasterModeEnableGet(dev, port, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*static GT_STATUS prvCpssPxAutoNegMasterModeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)*/
UTF_TEST_CASE_MAC(cpssPxPortAutoNegMasterModeEnableSet)
{
    GT_STATUS                                            st;
    GT_U8                                                dev;
    GT_PHYSICAL_PORT_NUM                                 port;
    CPSS_PORTS_BMP_STC                                   portsBmp;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    GT_BOOL                                              isError = GT_FALSE;

    /* there is no MAC/PCS/RXDMA/TXDMA in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

       /* Save active ports parameters */
       st = prvCpssPxSaveActivePortsParam(dev, /*OUT*/&portActive);
       if(GT_OK != st)
       {
           return;
       }

       /* Deactivation ports */
       st = prvPxPortsActivateDeactivate(dev, GT_FALSE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st," %s :  dev %d", "prvPxPortsActivateDeactivate", dev);
       if(GT_OK != st)
       {
           isError = GT_TRUE;
           goto restore;
       }

       for (portActive.Current = 0; portActive.Current < portActive.Len; portActive.Current++)
       {

           if((portActive.Data[portActive.Current].macNum > 16) || (0 == portActive.Data[portActive.Current].macNum % 4))
           {
              /* Activate tested port */
               st = prvPxPortActivateDeactivate(dev, GT_TRUE,
                                            portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                            portActive.Data[portActive.Current].speed);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Activate Port dev %d :  port %d", dev,
                   portActive.Data[portActive.Current].portNum );
               if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
               {
                   isError = GT_TRUE;
                   goto restore;
               }

               /* Checking for port mapping */
               st = prvCpssPxIsMapAndNotCPUCheck(dev, portActive.Data[portActive.Current].portNum);
               if (st != GT_OK)
               {
                   continue;
               }

               /* Testing for not supporting not SGMII port interface and port speed not one of 10, 100 or 1000 Mbps */
               st = cpssPxPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
               if(CPSS_PORT_INTERFACE_MODE_SGMII_E != portActive.Data->ifMode)
               {
                     UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d",
                         dev, portActive.Data[portActive.Current].portNum );
                     if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                     {
                         isError = GT_TRUE;
                         goto restore;
                     }
               }
               else if((portActive.Data->speed != CPSS_PORT_SPEED_10_E) && (portActive.Data->speed != CPSS_PORT_SPEED_100_E) &&
                    (portActive.Data->speed != CPSS_PORT_SPEED_1000_E))
               {
                   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Speed   dev %d :  port %d",
                       dev, portActive.Data[portActive.Current].portNum );
                   if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                       isError = GT_TRUE;
                       goto restore;
                   }
               }

               CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
               CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portActive.Data[portActive.Current].portNum);

               /* Set port interface QSGMII and port speed 1000 Mbps */
               st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_QSGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
               if(GT_OK == st)
               {
                    st = cpssPxPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,"Not Valid Port Interface   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
               }

               /* Set port interface SGMII and port speed 2500 Mbps */
               st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_2500_E);
               if(GT_OK == st)
               {
                    st = cpssPxPortAutoNegMasterModeEnableSet(dev, portActive.Data[portActive.Current].portNum, GT_TRUE);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_NOT_SUPPORTED, st,
                        "Not Valid Port Speed   dev %d :  port %d", dev, portActive.Data[portActive.Current].portNum );
                    if((GT_NOT_SUPPORTED != st) && (GT_FALSE == utfContinueFlagGet()))
                    {
                        isError = GT_TRUE;
                        goto restore;
                    }
               }

               /* Set port interface SGMII and port speed 1000 Mbps */
               st = cpssPxPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                              CPSS_PORT_INTERFACE_MODE_SGMII_E,
                                              CPSS_PORT_SPEED_1000_E);
               if(GT_OK == st)
               {
                   /* Setting Master Mode enable and checking */
                   st = prvCpssPxPortAutoNegMasterModeEnableSetCheck( dev, portActive.Data[portActive.Current].portNum, GT_TRUE, "enable");
                   if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                        isError = GT_TRUE;
                        goto restore;
                   }

                   /* Setting Master Mod disable and checking */
                   st = prvCpssPxPortAutoNegMasterModeEnableSetCheck( dev, portActive.Data[portActive.Current].portNum, GT_FALSE, "disable");
                   if((GT_OK != st) && (GT_FALSE == utfContinueFlagGet()))
                   {
                        isError = GT_TRUE;
                        goto restore;
                   }
               }
               /* Deactivate tested port */
               st = prvPxPortActivateDeactivate(dev, GT_FALSE,
                                 portActive.Data[portActive.Current].portNum, portActive.Data[portActive.Current].ifMode,
                                 portActive.Data[portActive.Current].speed);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st,"Cannot Deactivate Port dev %d :  port %d", dev,
                   portActive.Data[portActive.Current].portNum );
               if(GT_OK != st)
               {
                   isError = GT_TRUE;
                   goto restore;
               }
           }
       }

restore:
       /*  Restore ports Activation with initial parameters */
       st = prvPxPortsActivateDeactivate(dev, GT_TRUE, &portActive);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"%s :  dev %d", "prvPxPortsActivateDeactivate()", dev );
       if(GT_TRUE == isError)
       {
           return;
       }

       /* For all active devices go over all non available physical ports. */
       st = prvUtfNextMacPortReset(&port, dev);
       UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
       while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
       {
           /* Call function for each non-active port */
           st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
           UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
       }

       /* For active device check that function returns GT_BAD_PARAM */
       /* for out of bound value for port number.                    */
       port = UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
       st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

       /* For active device check that function returns GT_BAD_PARAM */
       /* for CPU port number.                                       */
       port = PRV_CPSS_PX_CPU_DMA_NUM_CNS;
       st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

   }

   port = PORT_CTRL_VALID_PHY_PORT_CNS;

   /* prepare device iterator */
   PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
   /* Go over all non active devices. */
   while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
   {
       st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
       st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_FALSE);
       UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
   }

   /* Call function with out of bound value for device id */
   dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
   st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_TRUE);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
   st = cpssPxPortAutoNegMasterModeEnableSet(dev, port, GT_FALSE);
   UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssPxPortInbandAutoNegRestart
(
    IN  GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
*/
UTF_TEST_CASE_MAC(cpssPxPortInbandAutoNegRestart)
{
    GT_STATUS               st          =   GT_OK;
    GT_U8                   devNum      =   0;
    GT_PHYSICAL_PORT_NUM    portNum     =   PORT_CTRL_VALID_PHY_PORT_CNS;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum,UTF_NONE_FAMILY_E);


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.*/

        portNum =UTF_CPSS_PX_MAX_PHYSICAL_PORTS_NUM_CNS(devNum);


        st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = PRV_CPSS_PX_CPU_DMA_NUM_CNS;

        st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* restore valid values */
    portNum = PORT_CTRL_VALID_PHY_PORT_CNS;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssPxPortInbandAutoNegRestart(devNum, portNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxPortCtrl suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxPortCtrl)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortLinkStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForceLinkPassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForceLinkPassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForceLinkDownEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForceLinkDownEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMruSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMruGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInternalLoopbackEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInternalLoopbackEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesAutoTuneStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesPolaritySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesPolarityGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesLoopbackModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesLoopbackModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEomDfeResGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesEyeMatrixGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesTxEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesTxEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesManualTxConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesManualTxConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesManualRxConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesManualRxConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesPowerStatusSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesTuningSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesTuningGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesLaneTuningSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesLaneTuningGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesResetStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSerdesAutoTune)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortEomBaudRateGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacSaLsbSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacSaLsbGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortDuplexModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortDuplexModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSpeedGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortDuplexAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortDuplexAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowCntrlAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowCntrlAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSpeedAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortSpeedAutoNegEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowControlEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowControlEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPeriodicFcEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPeriodicFcEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBackPressureEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortBackPressureEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortCrcCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXGmiiModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXGmiiModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortIpgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortIpgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortExtraIpgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortExtraIpgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgmiiLocalFaultGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgmiiRemoteFaultGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInbandAutoNegEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortAttributesOnPortGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPreambleLengthSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPreambleLengthGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacSaBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacSaBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPaddingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPaddingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortExcessiveCollisionDropEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortExcessiveCollisionDropEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInBandAutoNegBypassEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInBandAutoNegBypassEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortIpgBaseGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortIpgBaseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowControlModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortFlowControlModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortMacResetStateSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForward802_3xEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForward802_3xEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForwardUnknownMacControlFramesEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortForwardUnknownMacControlFramesEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPeriodicFlowControlCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPeriodicFlowControlCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortVosOverrideControlModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortVosOverrideControlModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXlgReduceAverageIPGSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXlgReduceAverageIPGGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPcsLoopbackModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortPcsLoopbackModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgLanesSwapEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgLanesSwapEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgPscLanesSwapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortXgPscLanesSwapGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortAutoNegAdvertismentConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortAutoNegAdvertismentConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortAutoNegMasterModeEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortAutoNegMasterModeEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssPxPortInbandAutoNegRestart)
UTF_SUIT_END_TESTS_MAC(cpssPxPortCtrl)


