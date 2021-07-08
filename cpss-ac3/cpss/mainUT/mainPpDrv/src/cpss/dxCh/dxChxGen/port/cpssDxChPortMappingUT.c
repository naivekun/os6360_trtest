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
* @file cpssDxChPortMappingUT.c
*
* @brief Unit tests for cpssDxChPortMapping, that provides
* CPSS implementation for Port interface mode configuration.
*
* @version   9
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrlUT.h>

/*#define UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS  64
#define UT_MAC_MAX_NUM_CNS                 72

#define UT_NOT_RELEVANT_PHY_PORT_CNS          1
#define UT_NOT_RELEVANT_MAPPING_TYPE_CNS      2
#define UT_NOT_RELEVANT_PORT_GROUP_CNS        4
#define UT_NOT_RELEVANT_IF_NUM_CNS            8
#define UT_NOT_RELEVANT_TXQ_PORT_CNS         16
#define UT_NOT_RELEVANT_TM_ENABLE_CNS        32
#define UT_NOT_RELEVANT_TM_PORT_CNS          64*/


#define UT_SOURCE_INDEX                       1
#define UT_DESTIN_INDEX                       2
#define UT_ACTIVE_INDEX                       2
#define UT_INTERFACE_NUM_OUT_OF_RANGE         72
#define UT_TXQ_PORT_NUM_OUT_OF_RANGE          72
#define UT_PHYS_PORT_NUM_OUT_OF_RANGE        256
#define UT_2_EQUELS_TM_PORT_INDEXES           3
#define UT_ILKN_BEGIN                       128
#define UT_ILKN_END                         191
#define UT_ILKN_EHTERNET_PORT_NUMBER         64
#define UT_ILKN_SOURCE_INDEX                101
#define UT_ILKN_DESTIN_INDEX                102
#define UT_ILKN_INTERFACE_NUM_OUT_OF_RANGE    65

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPhysicalPortMapSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC     *portMapArrayPtr
);
 */
/*UTF_TEST_CASE_MAC(cpssDxChPortPhysicalPortMapSet)*/


/* usefull data structures */



GT_STATUS prvHdrPortNumCpssDxCHPortMapShadowInit
(
    INOUT HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC *thisPtr
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
        cpssOsMemSet((GT_VOID *)thisPtr,0,sizeof(HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC));
        thisPtr->Size = sizeof(thisPtr->Data)/sizeof(thisPtr->Data[0]);
        thisPtr->Len = 0;
        thisPtr->Current = 0;
    }
    return rc;
}

GT_STATUS prvHdrPortNumCpssDxCHPortMapShadowAppend
(
    INOUT HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC *thisPtr,
    IN GT_PHYSICAL_PORT_NUM                           portNum,
    IN CPSS_DXCH_SHADOW_PORT_MAP_STC           *portMapPtr
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

typedef struct
{
    GT_U32                    Current;
    GT_U32                    Current1;
    GT_U32                    Len;
    GT_U32                    Size;
    CPSS_DXCH_PORT_MAP_STC    Data[UT_MAX_PORTS];
} HDR_CPSS_DXCH_PORT_MAP_STC;

GT_STATUS prvHdrCpssDxCHPortMapInit
(
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC *thisPtr
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
        cpssOsMemSet((GT_VOID *)thisPtr,0,sizeof(HDR_CPSS_DXCH_PORT_MAP_STC));
        thisPtr->Size = sizeof(thisPtr->Data)/sizeof(thisPtr->Data[0]);
        thisPtr->Len = 0;
        thisPtr->Current = 0;
    }
    return rc;
}

GT_STATUS prvHdrCpssDxCHPortMapAppend
(
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC *thisPtr,
    IN CPSS_DXCH_PORT_MAP_STC        *portMapPtr
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
            thisPtr->Data[thisPtr->Len] = *portMapPtr;
            thisPtr->Len++;
        }
    }
    return rc;
}

GT_STATUS prvHdrCpssDxCHPortMapPtrGet
(
    IN HDR_CPSS_DXCH_PORT_MAP_STC *thisPtr,
    IN GT_U32 index,
    OUT CPSS_DXCH_PORT_MAP_STC **structPtrPtr
)
{
    GT_STATUS rc = GT_FAIL;

    if(index < thisPtr->Size)
    {
        *structPtrPtr = &(thisPtr->Data)[index];
        rc = GT_OK;
    }
    return rc;
}


GT_STATUS prvHdrPortNumSpeedInterfaceInit
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

GT_STATUS prvHdrPortNumSpeedInterfaceAppend
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

GT_STATUS prvPortActivateDeactivate
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
    rc = cpssDxChPortModeSpeedSet(devNum,&initPortsBmp, Mode, ifMode, speed);
    return rc;
}

GT_STATUS prvPortsActivateDeactivate
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
        rc = prvPortActivateDeactivate(devNum, mode,
                                 portActivePtr->Data[portActivePtr->Current].portNum, portActivePtr->Data[portActivePtr->Current].ifMode,
                                 portActivePtr->Data[portActivePtr->Current].speed);
        if(GT_TRUE == mode)
        {
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"prvPortsActivateDeactivate : prvPortActivateDeactivate Activation Error : dev %d port %d",
                devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"prvPortsActivateDeactivate : prvPortActivateDeactivate Deactivation Error : dev %d port %d",
                devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        }
        if(GT_OK != rc)
        {
            break;
        }
    }
    return rc;
}

GT_VOID prvShadowtoMapTransform
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                        *portMapStruct
)
{
    portMapStruct->physicalPortNumber = portShadowPtr->Data[portShadowPtr->Current].portNum;
    portMapStruct->mappingType        = portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType;
    portMapStruct->portGroup          = portShadowPtr->Data[portShadowPtr->Current].portMap.portGroup;
    portMapStruct->interfaceNum       = portShadowPtr->Data[portShadowPtr->Current].portMap.macNum;
    portMapStruct->txqPortNumber      = portShadowPtr->Data[portShadowPtr->Current].portMap.txqNum;
    portMapStruct->tmEnable           = portShadowPtr->Data[portShadowPtr->Current].portMap.trafficManagerEn;
    portMapStruct->tmPortInd          = portShadowPtr->Data[portShadowPtr->Current].portMap.tmPortIdx;
}

GT_STATUS prvMapStructAppend
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                    *portMapPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_MAP_STC     portMapStruct;

    prvShadowtoMapTransform(portShadowPtr, /*OUT*/&portMapStruct);
    rc = prvHdrCpssDxCHPortMapAppend(/*INOUT*/portMapPtr, /*INT*/&portMapStruct);
    return rc;
}

GT_STATUS prvMappingBuild
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                    *portMapPtr
)
{
    GT_STATUS rc;

    rc = prvHdrCpssDxCHPortMapInit(/*INOUT*/portMapPtr);
    if (GT_OK != rc)
    {
        return rc;
    }


    for (portShadowPtr->Current = 0; portShadowPtr->Current < portShadowPtr->Len; portShadowPtr->Current++)
    {
        rc = prvMapStructAppend(portShadowPtr, /*OUT*/portMapPtr);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

GT_VOID PortMapStructILKNFilling
(
    IN   GT_U32                     portNum,
    OUT  CPSS_DXCH_PORT_MAP_STC     *PortMapStruct
)
{
        PortMapStruct->physicalPortNumber = (GT_PHYSICAL_PORT_NUM )portNum;
        PortMapStruct->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E;
        PortMapStruct->portGroup          = (GT_U32)0;
        PortMapStruct->interfaceNum       = (GT_U32)(portNum - 128);
        PortMapStruct->txqPortNumber      = GT_NA;
        PortMapStruct->tmEnable           = GT_TRUE;
        PortMapStruct->tmPortInd          = (GT_U32)portNum;
}

GT_STATUS prvILKNMappingBuild
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                    *portMapPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_MAP_STC     PortMapStruct;
    GT_PHYSICAL_PORT_NUM       portNum;

    rc = prvHdrCpssDxCHPortMapInit(/*INOUT*/portMapPtr);
    if(GT_OK != rc)
    {
         return rc;
    }
    for (portShadowPtr->Current = 0; portShadowPtr->Current < portShadowPtr->Len; portShadowPtr->Current++)
    {
        portNum = portShadowPtr->Data[portShadowPtr->Current].portNum;
        if(portNum != UT_ILKN_EHTERNET_PORT_NUMBER)
        {
            rc = prvMapStructAppend(portShadowPtr, /*OUT*/portMapPtr);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"prvBuildILKNMapping() : prvMapStructAppend() : port idx #%d num =%d",portShadowPtr->Current,portNum);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    for (portMapPtr->Current = 128; portMapPtr->Current <= 191; portMapPtr->Current++)
    {
        PortMapStructILKNFilling(portMapPtr->Current, /*OUT*/&PortMapStruct);
        rc = prvHdrCpssDxCHPortMapAppend(/*INOUT*/portMapPtr, /*IN*/&PortMapStruct);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return rc;
}

GT_STATUS prvTmDisableForAllPortsPrepare
(
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                   *portMapPtr
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_DXCH_PORT_MAP_STC    *structPtr;

    for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
    {
        rc = prvHdrCpssDxCHPortMapPtrGet(portMapPtr, portMapPtr->Current,/*OUT*/&structPtr);
        if(GT_OK == rc)
        {
            structPtr->tmEnable = GT_FALSE;
        }
        else
        {
            break;
        }
    }
    return rc;
}

GT_STATUS prvTmEnableForActivePortAndChangeTmPortIndPrepare
(
    IN    HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                    *portMapPtr
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_DXCH_PORT_MAP_STC    *prtMapPtr;

    for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
    {
        if(portMapPtr->Data[portMapPtr->Current].physicalPortNumber == portActivePtr->Data[portActivePtr->Current].portNum)
        {
            rc = prvHdrCpssDxCHPortMapPtrGet(portMapPtr, portMapPtr->Current, /*OUT*/&prtMapPtr);
            if(GT_OK == rc)
            {
                prtMapPtr->tmEnable = GT_TRUE;
                prtMapPtr->tmPortInd = (portMapPtr->Data[portMapPtr->Current].physicalPortNumber + 1) % 64;
            }
            else
            {
                break;
            }
        }
    }
    return rc;
}

GT_STATUS prvPortShadowMapGet
(
    IN  GT_U8                                          devNum,
    OUT HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr
)
{
   GT_PHYSICAL_PORT_NUM                             portNum ;
   GT_STATUS                                        rc;

   rc = prvHdrPortNumCpssDxCHPortMapShadowInit(/*INOUT*/portShadowPtr);
   if(GT_OK == rc)
   {
       for (portNum  = 0 ; portNum  < UT_MAX_PORTS; portNum ++)
       {
            CPSS_DXCH_DETAILED_PORT_MAP_STC *portShadowCurrentPtr;
            rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum ,/*OUT*/&portShadowCurrentPtr);
            if(GT_OK != rc)
            {
                break;
            }
            if(portShadowCurrentPtr->valid)
            {
                rc = prvHdrPortNumCpssDxCHPortMapShadowAppend(/*INOUT*/portShadowPtr, /*IN*/portNum , /*IN*/&portShadowCurrentPtr->portMap);
                if(GT_OK != rc)
                {
                    break;
                }
            }
       }
   }
   return rc;
}

GT_STATUS prvPortNumCpssDxchPortMapShadowCompare
(
    IN PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC *oldStruct,
    IN PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC *newStruct
)
{
   GT_STATUS                                        rc;

   rc = GT_OK;
   if(oldStruct->portNum                       != newStruct->portNum)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.ilknChannel      != newStruct->portMap.ilknChannel)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.macNum           != newStruct->portMap.macNum)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.mappingType      != newStruct->portMap.mappingType)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.portGroup        != newStruct->portMap.portGroup)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.rxDmaNum         != newStruct->portMap.rxDmaNum)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.tmPortIdx        != newStruct->portMap.tmPortIdx)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.trafficManagerEn != newStruct->portMap.trafficManagerEn)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.txDmaNum         != newStruct->portMap.txDmaNum)
   {
       rc = GT_FAIL;
   }
   else if(oldStruct->portMap.txqNum           != newStruct->portMap.txqNum)
   {
       rc = GT_FAIL;
   }
   return rc;
}

GT_STATUS PortsMappingWithInitialCompare
(
    IN GT_U8                                          devNum,
    IN HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowOldPtr
)
{
   GT_PHYSICAL_PORT_NUM                             portNum ;
   HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       portShadowNew;
   GT_STATUS                                        rc;

   rc = prvPortShadowMapGet( devNum, /*OUT*/&portShadowNew);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "PortsMappingWithInitialCompare : prvPortShadowMapGet", devNum );
   if(GT_OK == rc)
   {
       if(portShadowOldPtr->Len != portShadowNew.Len)
       {
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
                    "PortsMappingWithInitialCompare: different Lenght", devNum );
           rc = GT_FAIL;
       }
       else
       {
           for (portNum  = 0 ; (portNum  < portShadowOldPtr->Len) && (GT_OK == rc); portNum ++)
           {
               rc = prvPortNumCpssDxchPortMapShadowCompare(&portShadowOldPtr->Data[portNum], &portShadowNew.Data[portNum]);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
                    "PortsMappingWithInitialCompare: prvPortNumCpssDxchPortMapShadowCompare", devNum );
               if (GT_OK != rc)
               {
                   break;
               }
           }
       }
   }
   return rc;
}

GT_STATUS prvDefaultPortsMappingAndActivePortsParamSaving
(
    IN  GT_U8                                          devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr,
    OUT HDR_PORT_NUM_SPEED_INTERFACE_STC              *portActivePtr
)
{
   GT_STATUS                                     rc;

   rc = prvHdrPortNumSpeedInterfaceInit(/*INOUT*/portActivePtr);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDefaultPortsMappingAndActivePortsParamSaving : prvHdrPortNumSpeedInterfaceInit", devNum );
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

        if(CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E == portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType)
            continue;

        portNum = portShadowPtr->Data[portShadowPtr->Current].portNum;
        speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portShadowPtr->Data[portShadowPtr->Current].portMap.macNum);
        macNum = portShadowPtr->Data[portShadowPtr->Current].portMap.macNum;
        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            if(CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E == portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType)
            {
                rc = cpssDxChPortInterfaceModeGet(devNum, portNum , &ifMode);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvDefaultPortsMappingAndActivePortsParamSaving : cpssDxChPortInterfaceModeGet : dev %d port %d",
                     devNum, portShadowPtr->Data[portShadowPtr->Current].portNum );
                if(GT_OK != rc)
                {
                    break;
                }
                if(ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    rc = prvHdrPortNumSpeedInterfaceAppend( /*INOUT*/portActivePtr, /*IN*/portNum , /*IN*/speed, /*IN*/ifMode, /*IN*/macNum);
                    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvDefaultPortsMappingAndActivePortsParamSaving : prvHdrPortNumSpeedInterfaceAppend : dev %d port %d",
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

GT_BOOL prvIsILKNPresent
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    *portShadowPtr
)
{
     for ( portShadowPtr->Current  = 0 ;  portShadowPtr->Current  < portShadowPtr->Len;  portShadowPtr->Current ++)
     {
         if(CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType)
         {
             return GT_TRUE;
         }
     }
     return GT_FALSE;
}

GT_VOID prvMapFromShadowStructGet
(
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                           *portMapPtr
)
{
    portMapPtr->physicalPortNumber = portShadowPtr->Data[portShadowPtr->Current].portNum;
    portMapPtr->interfaceNum       = portShadowPtr->Data[portShadowPtr->Current].portMap.macNum;
    portMapPtr->mappingType        = portShadowPtr->Data[portShadowPtr->Current].portMap.mappingType;
    portMapPtr->portGroup          = portShadowPtr->Data[portShadowPtr->Current].portMap.portGroup;
    portMapPtr->tmPortInd          = portShadowPtr->Data[portShadowPtr->Current].portMap.tmPortIdx;
    portMapPtr->txqPortNumber      = portShadowPtr->Data[portShadowPtr->Current].portMap.txqNum;
    if(portMapPtr->tmPortInd == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        portMapPtr->tmEnable = GT_FALSE;
    }
    else
    {
        portMapPtr->tmEnable = GT_TRUE;
    }
}

GT_STATUS prvDefaultPortsMappingRestore
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc;
    CPSS_DXCH_PORT_MAP_STC                        portMapStruct;

    cpssOsMemSet(portMapPtr->Data, 0, sizeof(portMapPtr->Data));
    rc = prvHdrCpssDxCHPortMapInit(/*INOUT*/portMapPtr);
    if(GT_OK == rc)
    {
        for (portShadowPtr->Current = 0; portShadowPtr->Current < portShadowPtr->Len; portShadowPtr->Current++)
        {
            prvMapFromShadowStructGet(portShadowPtr, /*OUT*/&portMapStruct);
            prvHdrCpssDxCHPortMapAppend(/*INOUT*/portMapPtr, /*INT*/&portMapStruct);
        }
        rc = cpssDxChPortPhysicalPortMapSet(devNum, portMapPtr->Len, portMapPtr->Data);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s dev %d", "prvDefaultPortsMappingRestore : cpssDxChPortPhysicalPortMapSet : ", devNum);
    }
    return rc;
}

GT_STATUS prvSourceAndDestinPtrGet
(
       IN  HDR_CPSS_DXCH_PORT_MAP_STC                *portMapPtr,
       OUT CPSS_DXCH_PORT_MAP_STC                   **sourcePtrPtr,
       OUT CPSS_DXCH_PORT_MAP_STC                   **destinPtrPtr
)
{
    GT_STATUS                                     rc;

    rc = prvHdrCpssDxCHPortMapPtrGet(portMapPtr, UT_SOURCE_INDEX, /*OUT*/sourcePtrPtr);
    if(GT_OK == rc)
    {
        rc =  prvHdrCpssDxCHPortMapPtrGet(portMapPtr, UT_DESTIN_INDEX, /*OUT*/destinPtrPtr);
    }
    return rc;
}

GT_STATUS prvILKNSourceAndDestinPtrGet
(
       IN  HDR_CPSS_DXCH_PORT_MAP_STC                *portMapPtr,
       OUT CPSS_DXCH_PORT_MAP_STC                   **sourcePtrPtr,
       OUT CPSS_DXCH_PORT_MAP_STC                   **destinPtrPtr
)
{
    GT_STATUS                                     rc;

    rc = prvHdrCpssDxCHPortMapPtrGet(portMapPtr, UT_ILKN_SOURCE_INDEX, /*OUT*/sourcePtrPtr);
    if(GT_OK == rc)
    {
        rc =  prvHdrCpssDxCHPortMapPtrGet(portMapPtr, UT_ILKN_DESTIN_INDEX, /*OUT*/destinPtrPtr);
    }
    return rc;
}

GT_STATUS prvMappingBuildAndSourceAndDestinPtrGet
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                          **sourcePtrPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                          **destinPtrPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuild(portShadowPtr, /*OUT*/portMapPtr);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d", "prvMappingBuildAndSourceAndDestinPtrGet : prvMappingBuild : ", devNum );
    if(GT_OK == rc)
    {
        rc = prvSourceAndDestinPtrGet(portMapPtr, /*OUT*/sourcePtrPtr, /*OUT*/destinPtrPtr);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d", "prvMappingBuildAndSourceAndDestinPtrGet : prvSourceAndDestinPtrGet : ", devNum );
    }
    return rc;
}

GT_STATUS prvILKNMappingBuildAndILKNSourceAndDestinPtrGet
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                          **sourcePtrPtr,
    OUT CPSS_DXCH_PORT_MAP_STC                          **destinPtrPtr
)
{
    GT_STATUS                                     rc;

    rc = prvILKNMappingBuild(portShadowPtr, /*OUT*/portMapPtr);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d",
        "prvILKNMappingBuildAndILKNSourceAndDestinPtrGet : prvILKNMappingBuild", devNum );
    if(GT_OK == rc)
    {
        rc = prvILKNSourceAndDestinPtrGet(portMapPtr, /*OUT*/sourcePtrPtr, /*OUT*/destinPtrPtr);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d", "prvILKNMappingBuildAndILKNSourceAndDestinPtrGet : prvILKNSourceAndDestinPtrGet : ", devNum );
    }
    return rc;
}

GT_STATUS prvMapSetAndThatMappingIsWrongAndNotChangedCheck
(
    IN GT_U8                                             devNum,
    IN HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN GT_STATUS                                         statusToCheck
)
{
    GT_STATUS                                     rc;

    rc = cpssDxChPortPhysicalPortMapSet(devNum, portMapPtr->Len, portMapPtr->Data);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(statusToCheck, rc
        ,"%s :  dev %d", " : prvMapSetAndThatMappingIsWrongAndNotChangedCheck : cpssDxChPortPhysicalPortMapSet", devNum );
    if(rc != statusToCheck)
    {
        rc = GT_FAIL;
    }
    else
    {
         rc = PortsMappingWithInitialCompare( devNum, portShadowPtr);
         UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d", " : prvMapSetAndThatMappingIsWrongAndNotChangedCheck : PortsMappingWithInitialCompare", devNum );
    }
    return rc;
}

GT_STATUS prv2EquelsPhysPortsNumberCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->physicalPortNumber = sourcePtr->physicalPortNumber;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsILKNPhysPortsNumberCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowILKN_Ptr
)
{
    GT_STATUS                                     rc;

    rc = prvILKNMappingBuildAndILKNSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->physicalPortNumber = sourcePtr->physicalPortNumber;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowILKN_Ptr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsInterfacesNumCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->interfaceNum = sourcePtr->interfaceNum;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsILKNInterfacesNumCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowILKN_Ptr
)
{
    GT_STATUS                                     rc;

    rc = prvILKNMappingBuildAndILKNSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->interfaceNum = sourcePtr->interfaceNum;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowILKN_Ptr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsILKNTmIndexCheck
(
    IN    GT_U8                                             devNum,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowILKN_Ptr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    OUT   CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    OUT   CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvILKNMappingBuildAndILKNSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->tmPortInd = sourcePtr->tmPortInd;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowILKN_Ptr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prvInterfaceNumberILKNOutOfRangeCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowILKN_Ptr
)
{
    GT_STATUS                                     rc;

    rc = prvILKNMappingBuildAndILKNSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->interfaceNum = (GT_U32)UT_ILKN_INTERFACE_NUM_OUT_OF_RANGE;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowILKN_Ptr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsTxqPortNumberCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->txqPortNumber = sourcePtr->txqPortNumber;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}


GT_STATUS prvPhysPortsNumberOutOfRangeCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN HDR_CPSS_DXCH_PORT_MAP_STC                        *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->physicalPortNumber = (GT_PHYSICAL_PORT_NUM)UT_PHYS_PORT_NUM_OUT_OF_RANGE;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prvInterfaceNumberOutOfRangeCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        destinPtr->interfaceNum = (GT_U32)UT_INTERFACE_NUM_OUT_OF_RANGE;
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prvTxqPortNumberOutOfRangeCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;
    GT_U32  dpIndex;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0)
        {
            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, destinPtr->interfaceNum,&dpIndex, NULL/*localDmaNumPtr*/);
            if (rc != GT_OK)
            {
                return rc;
            }
            destinPtr->txqPortNumber = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp*(1+dpIndex);
        }
        else
        {
            destinPtr->txqPortNumber = (GT_U32)UT_TXQ_PORT_NUM_OUT_OF_RANGE;
        }
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prv2EquelsTmIndexWhileTmEnableCheck
(
    IN    GT_U8                                             devNum,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    OUT   CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    OUT   CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS                                     rc;

    rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
    if(GT_OK == rc)
    {
        sourcePtr->tmEnable = GT_TRUE;
        sourcePtr->tmPortInd = (GT_U32)UT_2_EQUELS_TM_PORT_INDEXES;
        destinPtr->tmEnable = GT_TRUE;
        destinPtr->tmPortInd = (GT_U32)(UT_2_EQUELS_TM_PORT_INDEXES);
        rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
    }
    return rc;
}

GT_STATUS prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate
(
    IN GT_U8                                             devNum,
    IN HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    IN GT_STATUS                                         statusToCheck
)
{
    GT_STATUS                                     rc;

    rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, statusToCheck);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvPortActivateDeactivate(devNum, GT_FALSE, portActivePtr->Data[portActivePtr->Current].portNum, portActivePtr->Data[portActivePtr->Current].ifMode,
        portActivePtr->Data[portActivePtr->Current].speed);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate : prvPortActivateDeactivate : dev %d port %d",
         devNum,portActivePtr->Data[portActivePtr->Current].portNum );
    return rc;
}

GT_STATUS prvPointersForChangeMappingForActivePortGet
(
    IN    HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN    HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    IN    GT_BOOL                                           differentTxqPortNumber,
    OUT   CPSS_DXCH_PORT_MAP_STC                           **sourcePtrPtr,
    OUT   CPSS_DXCH_PORT_MAP_STC                           **destinPtrPtr
)
{
    GT_STATUS                                     rc;

    rc = GT_OK;
    for(portMapPtr->Current = 0; (portMapPtr->Current < portMapPtr->Len) && (GT_OK == rc); portMapPtr->Current++)
    {
        if(portMapPtr->Data[portMapPtr->Current].physicalPortNumber == portActivePtr->Data[portActivePtr->Current].portNum)
        {
            rc =  prvHdrCpssDxCHPortMapPtrGet(portMapPtr, portMapPtr->Current, /*OUT*/sourcePtrPtr);
            if(GT_OK == rc)
            {
                for(portMapPtr->Current1 = portMapPtr->Current + 1; portMapPtr->Current1 < portMapPtr->Len; portMapPtr->Current1++)
                {
                    if(portMapPtr->Data[portMapPtr->Current1].mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
                    {
                        if((differentTxqPortNumber == GT_FALSE) ||
                            (portMapPtr->Data[portMapPtr->Current1].txqPortNumber !=
                                portMapPtr->Data[portMapPtr->Current].txqPortNumber))
                        {
                            rc =  prvHdrCpssDxCHPortMapPtrGet(portMapPtr, portMapPtr->Current1, destinPtrPtr);
                            return rc;
                        }
                    }
                }
                for(portMapPtr->Current1 = portMapPtr->Current - 1; portMapPtr->Current1 >= 0; portMapPtr->Current1--)
                {
                    if(portMapPtr->Data[portMapPtr->Current1].mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
                    {
                        if((differentTxqPortNumber == GT_FALSE) ||
                            (portMapPtr->Data[portMapPtr->Current1].txqPortNumber !=
                                portMapPtr->Data[portMapPtr->Current].txqPortNumber))
                        {
                            rc =  prvHdrCpssDxCHPortMapPtrGet(portMapPtr, portMapPtr->Current1, destinPtrPtr);
                            return rc;
                        }
                    }
                }
                return GT_FAIL;
            }
        }
    }
    return rc;
}

GT_STATUS prvPortActivateAndMappingBuild
(
    IN GT_U8                                             devNum,
    IN HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    IN HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc;

    rc = prvPortActivateDeactivate(devNum, GT_TRUE,
                                        portActivePtr->Data[portActivePtr->Current].portNum, portActivePtr->Data[portActivePtr->Current].ifMode,
                                        portActivePtr->Data[portActivePtr->Current].speed);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvPortActivateAndMappingBuild : prvPortActivateDeactivate : dev %d port %d", devNum,portActivePtr->Data[portActivePtr->Current].portNum );
    if(GT_OK == rc)
    {
        rc = prvMappingBuild(portShadowPtr, /*OUT*/portMapPtr);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,": prvPortActivateAndMappingBuild : prvPortActivateDeactivate : dev %d port %d", devNum,portActivePtr->Data[portActivePtr->Current].portNum );
    }
    return rc;
}

/*Activating every single port and turn on traffic manager - should be error */
GT_STATUS prvEverySinglePortActivateAndTMTurnOn
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    OUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc = GT_OK;

    for (portActivePtr->Current = 0; (portActivePtr->Current < portActivePtr->Len) && (GT_OK == rc); portActivePtr->Current++)
    {
        /*---------------------------------------------------------------
         * 1. Activate port for speed/If by cpssPortModeSpeedSet
         * 2. Build new mapping where current port is passed through TM
         * 3. Configure new mapping  --> error
         * 4. Deactivate port
         * 5. Check that mapping does not changed
         *---------------------------------------------------------------
         */


         rc = prvPortActivateAndMappingBuild(devNum, portShadowPtr, portActivePtr, portMapPtr);
         if(GT_OK == rc)
         {
             rc = prvTmDisableForAllPortsPrepare(/*INOUT*/portMapPtr);
             UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvEverySinglePortActivateAndTMTurnOn : prvTmDisableForAllPortsPrepare : activate port : dev %d port %d", devNum,portActivePtr->Data[portActivePtr->Current].portNum );
             if(GT_OK != rc)
             {
                break;
             }

             rc = prvTmEnableForActivePortAndChangeTmPortIndPrepare( portActivePtr, /*INOUT*/portMapPtr);
             UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc," : prvEverySinglePortActivateAndTMTurnOn : prvTmEnableForActivePortAndChangeTmPortIndPrepare : dev %d port %d", devNum,portActivePtr->Data[portActivePtr->Current].portNum );
             if(GT_OK != rc)
             {
                break;
             }
             rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate( devNum, portShadowPtr, portMapPtr, portActivePtr, GT_BAD_STATE);
         }
    }
    return rc;
}

/*Txq port number changing while exist active port - should be error*/
GT_STATUS prvEverySinglePortActivateAndHisTxqPortNumberChange
(
    IN    GT_U8                                             devNum,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN    HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc;
    GT_U32                                        temp;
    CPSS_DXCH_PORT_MAP_STC                       *sourcePtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
    CPSS_DXCH_PORT_MAP_STC                       *destinPtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
    GT_U32 dqIndex = 0, dqIndex1 = 0;

    rc = GT_OK;
    for (portActivePtr->Current = 0; (portActivePtr->Current < portActivePtr->Len) && (GT_OK == rc); portActivePtr->Current++)
    {
        rc = prvPortActivateAndMappingBuild(devNum, portShadowPtr, portActivePtr, portMapPtr);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,  ": prvEverySinglePortActivateAndHisTxqPortNumberChange : prvPortActivateAndMappingBuild : dev %d port %d",
            devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        if(GT_OK == rc)
        {
            rc = prvPointersForChangeMappingForActivePortGet(portMapPtr, portActivePtr, GT_TRUE, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
               ": prvEverySinglePortActivateAndHisTxqPortNumberChange : prvPointersForChangeMappingForActivePortGet : dev %d port %d",
               devNum,portActivePtr->Data[portActivePtr->Current].portNum );
            if(GT_OK == rc)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0 && PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0)
                {
                    dqIndex = portMapPtr->Data[portMapPtr->Current].txqPortNumber / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                    dqIndex1 = portMapPtr->Data[portMapPtr->Current1].txqPortNumber / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                }
                if (dqIndex != dqIndex1)
                {
                    continue;
                }
                temp = sourcePtr->txqPortNumber;
                sourcePtr->txqPortNumber = destinPtr->txqPortNumber;
                destinPtr->txqPortNumber = temp;
                rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate( devNum, portShadowPtr, portMapPtr, portActivePtr, GT_BAD_STATE);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
                     ": prvEverySinglePortActivateAndHisTxqPortNumberChange : prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate : dev %d port %d",
                   devNum,portActivePtr->Data[portActivePtr->Current].portNum );
            }
        }
        if(GT_OK != rc)
        {
            break;
        }
    }
    return rc;
}

/*Interface number changing while exist active port - should be error*/
GT_STATUS prvEverySinglePortActivateAndHisInterfaceNumberChange
(
    IN    GT_U8                                             devNum,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN    HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc;
    GT_U32                                        temp;
    CPSS_DXCH_PORT_MAP_STC                       *sourcePtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
    CPSS_DXCH_PORT_MAP_STC                       *destinPtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
    GT_U32 dqIndex = 0, dqIndex1 = 0;

    rc = GT_OK;
    for (portActivePtr->Current = 0; (portActivePtr->Current < portActivePtr->Len) && (GT_OK == rc); portActivePtr->Current++)
    {
        rc = prvPortActivateAndMappingBuild(devNum, portShadowPtr, portActivePtr, portMapPtr);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
            ": prvEverySinglePortActivateAndHisInterfaceNumberChange : prvPortActivateAndMappingBuild : dev %d port %d",
            devNum,portActivePtr->Data[portActivePtr->Current].portNum );
        if(GT_OK == rc)
        {
            rc = prvPointersForChangeMappingForActivePortGet(portMapPtr, portActivePtr, GT_FALSE, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
            UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
               ": prvEverySinglePortActivateAndHisInterfaceNumberChange : prvPointersForChangeMappingForActivePortGet : dev %d port %d",
               devNum,portActivePtr->Data[portActivePtr->Current].portNum );
            if(GT_OK == rc)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0 && PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0)
                {
                    dqIndex = portMapPtr->Data[portMapPtr->Current].txqPortNumber / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                    dqIndex1 = portMapPtr->Data[portMapPtr->Current1].txqPortNumber / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                }
                if (dqIndex != dqIndex1)
                {
                    continue;
                }
                temp = sourcePtr->interfaceNum;
                sourcePtr->interfaceNum = destinPtr->interfaceNum;
                destinPtr->interfaceNum = temp;
                rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate( devNum, portShadowPtr, portMapPtr, portActivePtr, GT_BAD_STATE);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
                   ": prvEverySinglePortActivateAndHisInterfaceNumberChange : prvMapSetAndThatMappingIsWrongAndNotChangedCheckAndPortDeactivate : dev %d port %d",
                   devNum,portActivePtr->Data[portActivePtr->Current].portNum );
           }
        }
        if(GT_OK != rc)
        {
            break;
        }
    }
    return rc;
}

GT_STATUS prvCurrentActivePortByIndexFind
(
    INOUT HDR_PORT_NUM_SPEED_INTERFACE_STC  *portActivePtr
)
{
    GT_STATUS                                     rc;

    rc = GT_FALSE;
    for(portActivePtr->Current = 0; portActivePtr->Current < portActivePtr->Len; portActivePtr->Current++)
    {
        if(portActivePtr->Current == UT_ACTIVE_INDEX)
        {
            rc = GT_OK;
            break;
        }
    }
    return rc;
}

GT_VOID prvOtherPortsInterfaceNumberChange
(
    IN     HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapTransformedPtr,
    INOUT  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
    {
        for(portMapTransformedPtr->Current = 0; portMapTransformedPtr->Current < portMapTransformedPtr->Len; portMapTransformedPtr->Current++)
        {
            if(portMapPtr->Data[portMapPtr->Current].physicalPortNumber == portMapTransformedPtr->Data[portMapTransformedPtr->Current].physicalPortNumber)
            {
                portMapPtr->Data[portMapPtr->Current].interfaceNum = portMapTransformedPtr->Data[portMapTransformedPtr->Current].interfaceNum;
                break;
            }
        }
    }
}

GT_STATUS prvOtherPortsInterfaceNumberTransform
(
    IN     GT_U8                                            devNum,
    IN     HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    INOUT  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
    GT_STATUS                                     rc;
    GT_U32                                        activePortIndex;
    GT_U32                                        interface0port = 0;
    GT_U32                                        previousCurrent;
    GT_U32                                        dpIndex = 0, dqIndex = 0;

    rc = GT_OK;
    activePortIndex = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
    {
        if(portMapPtr->Data[portMapPtr->Current].physicalPortNumber == portActivePtr->Data[portActivePtr->Current].portNum)
        {
            activePortIndex = portMapPtr->Current;
        }
    }
    if(activePortIndex != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
    {
        /* First pass is for non-cascade extender ports */
        interface0port = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
        previousCurrent = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
        for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
        {
            if(portMapPtr->Current == activePortIndex)
            {
                /* Skip active port */
                continue;
            }
            if(portMapPtr->Data[portMapPtr->Current].txqPortNumber == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                /* Skip extender cascade port */
                continue;
            }

            if(previousCurrent == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                /* Save for the very last entry found */
                interface0port = portMapPtr->Data[portMapPtr->Current].interfaceNum;
            }
            else
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0)
                {
                    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, portMapPtr->Data[portMapPtr->Current].interfaceNum,&dpIndex, NULL/*localDmaNumPtr*/);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    dqIndex = portMapPtr->Data[previousCurrent].txqPortNumber / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                }
                /* Update previous entry found */
                portMapPtr->Data[previousCurrent].interfaceNum = portMapPtr->Data[portMapPtr->Current].interfaceNum;
                if (dpIndex != dqIndex)
                {
                    continue;
                }
            }

            previousCurrent = portMapPtr->Current;
        }
        if(previousCurrent != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            /* Update very last entry found */
            portMapPtr->Data[previousCurrent].interfaceNum = interface0port;
        }


        /* Second pass is for cascade extender ports */
        interface0port = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
        previousCurrent = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
        for(portMapPtr->Current = 0; portMapPtr->Current < portMapPtr->Len; portMapPtr->Current++)
        {
            if(portMapPtr->Current == activePortIndex)
            {
                /* Skip active port */
                continue;
            }
            if(portMapPtr->Data[portMapPtr->Current].txqPortNumber != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                /* Skip non-extender cascade port */
                continue;
            }

            if(previousCurrent == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                /* Save for the very last entry found */
                interface0port = portMapPtr->Data[portMapPtr->Current].interfaceNum;
            }
            else
            {
                /* Update previous entry found */
                portMapPtr->Data[previousCurrent].interfaceNum = portMapPtr->Data[portMapPtr->Current].interfaceNum;
            }

            previousCurrent = portMapPtr->Current;
        }
        if(previousCurrent != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            /* Update very last entry found */
            portMapPtr->Data[previousCurrent].interfaceNum = interface0port;
        }
    }
    else
    {
        rc = GT_FAIL;
    }
    return rc;
}

GT_STATUS prvCpssDxCHDefaultPortMapbyPortNumGet
(
    IN  GT_PHYSICAL_PORT_NUM                              portNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    OUT PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC           *resultPtr
)
{
   GT_STATUS                                     rc;

   rc = GT_FALSE;
   for(portShadowPtr->Current = 0; portShadowPtr->Current < portShadowPtr->Len; portShadowPtr->Current++)
   {
       if(portShadowPtr->Data[portShadowPtr->Current].portNum == portNum)
       {
           *resultPtr = portShadowPtr->Data[portShadowPtr->Current];
           rc = GT_OK;
           break;
       }
   }
   return rc;
}

GT_STATUS prvMappingByEthernetMappingTypeFiltering
(
    IN   HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtrBefore,
    OUT  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtrAfter
)
{
    GT_STATUS                                     rc;

    rc = prvHdrCpssDxCHPortMapInit(/*INOUT*/portMapPtrAfter);
    if(GT_OK == rc)
    {
        for(portMapPtrBefore->Current = 0; portMapPtrBefore->Current < portMapPtrBefore->Len; portMapPtrBefore->Current++)
        {
            if(portMapPtrBefore->Data[portMapPtrBefore->Current].mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                rc = prvHdrCpssDxCHPortMapAppend(/*INOUT*/portMapPtrAfter, /*INT*/&portMapPtrBefore->Data[portMapPtrBefore->Current]);
                if(GT_OK != rc)
                {
                    break;
                }
            }
        }
    }
    return rc;
}

GT_STATUS prvDestinPortActivateAndOtherPortsInterfaceNumberChange
(
    IN    GT_U8                                             devNum,
    IN    HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN    HDR_PORT_NUM_SPEED_INTERFACE_STC                 *portActivePtr,
    INOUT HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr
)
{
   GT_STATUS                                        rc;
   static HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       portShadowNew;
   static HDR_CPSS_DXCH_PORT_MAP_STC                       portMapTransform;

   PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC           activePortShadowOld;
   PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC           activePortShadowNew;


   rc = prvCurrentActivePortByIndexFind(/*INOUT*/portActivePtr);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvCurrentActivePortByIndexFind", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvPortActivateAndMappingBuild(devNum, portShadowPtr, portActivePtr, portMapPtr);
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvMappingByEthernetMappingTypeFiltering(portMapPtr, /*OUT*/&portMapTransform);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : cpssDxChPortPhysicalPortMapSet() :  dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvMappingByEthernetMappingTypeFiltering", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvOtherPortsInterfaceNumberTransform(devNum, portActivePtr, /*INOUT*/&portMapTransform);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : cpssDxChPortPhysicalPortMapSet() :  dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvOtherPortsInterfaceNumberTransform", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   prvOtherPortsInterfaceNumberChange(/*INOUT*/&portMapTransform,portMapPtr);
   rc = cpssDxChPortPhysicalPortMapSet(devNum, portMapPtr->Len, portMapPtr->Data);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,
       "%s : dev %d", "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : cpssDxChPortPhysicalPortMapSet : ", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvPortShadowMapGet( devNum, /*OUT*/&portShadowNew);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvPortShadowMapGet", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvCpssDxCHDefaultPortMapbyPortNumGet(portActivePtr->Data[portActivePtr->Current].portNum, portShadowPtr, /*OUT*/&activePortShadowOld);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvCpssDxCHDefaultPortMapbyPortNumGet: activePortShadowOld", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvCpssDxCHDefaultPortMapbyPortNumGet(portActivePtr->Data[portActivePtr->Current].portNum, &portShadowNew, /*OUT*/&activePortShadowNew);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvCpssDxCHDefaultPortMapbyPortNumGet: activePortShadowNew", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvPortNumCpssDxchPortMapShadowCompare(&activePortShadowOld, &activePortShadowNew);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvPortNumCpssDxchPortMapShadowCompare", devNum );
   if(GT_OK != rc)
   {
       return rc;
   }
   rc = prvPortActivateDeactivate(devNum, GT_FALSE,
                                        portActivePtr->Data[portActivePtr->Current].portNum, portActivePtr->Data[portActivePtr->Current].ifMode,
                                        portActivePtr->Data[portActivePtr->Current].speed);
   UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s : dev %d",
       "prvDestinPortActivateAndOtherPortsInterfaceNumberChange : prvPortActivateDeactivate", devNum );
   return rc;
}

GT_STATUS prvInvalidDqIndexCheck
(
    IN  GT_U8                                             devNum,
    IN  HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC       *portShadowPtr,
    IN  HDR_CPSS_DXCH_PORT_MAP_STC                       *portMapPtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *sourcePtr,
    IN  CPSS_DXCH_PORT_MAP_STC                           *destinPtr
)
{
    GT_STATUS rc;
    GT_U32 dpIndex;

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp > 0)
    {
        rc = prvMappingBuildAndSourceAndDestinPtrGet(devNum, portShadowPtr, /*OUT*/portMapPtr, /*OUT*/&sourcePtr, /*OUT*/&destinPtr);
        if(GT_OK == rc)
        {
            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum, destinPtr->interfaceNum,&dpIndex, NULL/*localDmaNumPtr*/);
            if (rc != GT_OK)
            {
                return rc;
            }
            destinPtr->txqPortNumber = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp * (dpIndex +1);

            rc = prvMapSetAndThatMappingIsWrongAndNotChangedCheck(devNum, portShadowPtr, portMapPtr, GT_BAD_PARAM);
        }
        return rc;
    }

    return GT_OK;
}

/*------------------------------------------------------------------------------------------------------
* At the beginning saving default ports mapping and active ports and deactivating all avtive ports
* At the finishing restoring default mapping and active ports
* The testing is usefull for configuration 29.1 only
* The cpssDxChPortPhysicalPortMapSetUT() provided the following tests:
* For EHERNET mapping type:
*  1. Activating every single port and turn on traffic manager - should be error
*  2. Activating every single port and Txq port number changing - should be error
*  3. Activating every single port Interface number changing - should be error
*  4. Set Map 2 Equels Physical port numbers - should be error
*  5. Set Map 2 Equels Interface numbers - should be error
*  6. Set Map 2 Equels Txq port numbers - should be error
*  7. Set Map 2 Physical port number out of range - should be error
*  8. Set Map 2 Interface number out of range - should be error
*  9. Set Map 2 Txq port number out of range - should be error
* 10. Set Map 2 Equels TM Indexes when TM Enable - should be error
* 11. Activate port, change all other ports interface numbers - activated port mapping should not be changed
* For ILKN mapping type:
* 12. Set Map 2 Equels ILKN Physical port numbers - should be error
* 13. Set Map 2 Equels ILKN Interface numbers - should be error
* 14. Set Map 2 Equels ILKN TM Indexes - should be error
* 15. Set Map 2 Interface number out of range - should be error
* All tests after an error is occured checks that mapping is not changed
--------------------------------------------------------------------------------------------------------
*/
GT_VOID cpssDxChPortPhysicalPortMapSetUT(GT_VOID)
{
           GT_U8                                         devNum = 0;

           GT_STATUS                                     rc;
           GT_BOOL                                       is291;
           CPSS_DXCH_PORT_MAP_STC                       *sourcePtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
           CPSS_DXCH_PORT_MAP_STC                       *destinPtr = (CPSS_DXCH_PORT_MAP_STC*)NULL;
    static HDR_CPSS_DXCH_PORT_MAP_STC                    portMap;
    static HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    portShadow;
    static HDR_PORT_NUM_SPEED_INTERFACE_STC              portActive;
    static HDR_PORT_NUM_CPSS_DXCH_PORT_MAP_SHADOW_STC    portShadowILKN;

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

       /* Saving default mapping*/
       rc = prvPortShadowMapGet( devNum, /*OUT*/&portShadow);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"%s : dev %d", "prvPortShadowMapGet", devNum );

       is291 = (GT_BOOL)(!prvIsILKNPresent(&portShadow));

       if(is291)
       {

           /* Saving active ports*/
           rc = prvDefaultPortsMappingAndActivePortsParamSaving(devNum, &portShadow, /*OUT*/&portActive);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvDefaultPortsMappingAndActivePortsParamSaving()", devNum );
           if(GT_OK != rc)
           {
               goto restore_def_map;
           }

           /* Deactivation ports */
           rc = prvPortsActivateDeactivate(devNum, GT_FALSE, &portActive);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvPortsActivateDeactivate", devNum );
           if(GT_OK != rc)
           {
               goto restore_def_map;
           }

           /*Activating every single port and turn on traffic manager - should be error */
           rc = prvEverySinglePortActivateAndTMTurnOn(devNum, &portShadow, &portActive, /*OUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvEverySinglePortActivateAndTMTurnOn()", devNum );
           if(GT_OK != rc)
           {
               goto restore_def_map;
           }

           /*Activating every single port and Txq port number changing - should be error*/
           rc = prvEverySinglePortActivateAndHisTxqPortNumberChange(devNum, &portShadow, &portActive, /*INOUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvEverySinglePortActivateAndHisTxqPortNumberChange()", devNum );
           if(GT_OK != rc)
           {
               goto restore_def_map;
           }

           /*Activating every single port Interface number changing - should be error*/
           rc = prvEverySinglePortActivateAndHisInterfaceNumberChange(devNum, &portShadow, &portActive, /*INOUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvEverySinglePortActivateAndHisInterfaceNumberChange()", devNum );
           if(GT_OK != rc)
           {
               goto restore_def_map;
           }

           /*Set Map 2 Equals Physical port numbers - should be error*/
           rc = prv2EquelsPhysPortsNumberCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsPhysPortsNumberCheck()", devNum );
           if (GT_OK != rc)
           {
                goto restore_def_map;
           }

           /*Set Map 2 Equels Interface numbers - should be error*/
           rc = prv2EquelsInterfacesNumCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsInterfacesNumCheck()", devNum );
           if (GT_OK != rc)
           {
                goto restore_def_map;
           }

           /*Set Map 2 Equels Txq port numbers - should be error*/
           rc = prv2EquelsTxqPortNumberCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsTxqPortNumberCheck()", devNum );
           if (rc != GT_OK)
           {
                goto restore_def_map;
           }

           /*Set Map 2 Physical port number out of range - should be error*/
           rc = prvPhysPortsNumberOutOfRangeCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvPhysPortsNumberOutOfRangeCheck()", devNum );
           if (GT_OK != rc)
           {
                goto restore_def_map;
           }

           /*Set Map 2 Interface number out of range - should be error*/
           rc = prvInterfaceNumberOutOfRangeCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvInterfaceNumberOutOfRangeCheck()", devNum );
           if (GT_OK != rc)
           {
                goto restore_def_map;
           }

           if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
           {
                /* currently the TXQ queue is ignored by the CPSS ! and internally set by the CP*/
           }
           else
           {
               /*Set Map 2 Txq port number out of range - should be error*/
               rc = prvTxqPortNumberOutOfRangeCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvTxqPortNumberOutOfRangeCheck()", devNum );
               if (rc != GT_OK)
               {
                    goto restore_def_map;
               }
           }

           /*Set map 2 invalid DQ index (not equal to DP index) - should be error. Applicable for Bobcat3*/
           rc = prvInvalidDqIndexCheck(devNum, &portShadow, /*INOUT*/&portMap, /*OUT*/sourcePtr, /*OUT*/destinPtr);
           if(rc != GT_OK)
           {
               goto restore_def_map;
           }

           /*Set Map 2 Equels TM Indexes when TM Enable - should be error*/
           rc = prv2EquelsTmIndexWhileTmEnableCheck( devNum, &portShadow, /*INOUT*/&portMap, /*OUT*/sourcePtr, /*OUT*/destinPtr);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsTmIndexWhileTmEnableCheck()", devNum );
           if (GT_OK != rc)
           {
               goto restore_def_map;
           }

           /*Activate port, change all other ports interface numbers - activated port mapping should not be changed*/
           rc = prvDestinPortActivateAndOtherPortsInterfaceNumberChange( devNum, &portShadow, &portActive, /*INOUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvDestinPortActivateAndOtherPortsInterfaceNumberChange()", devNum );
           if (GT_OK != rc)
           {
                goto restore_def_map;
           }

           /*Prepare mapping for ILKN mapping type*/
           rc = prvILKNMappingBuild(&portShadow, /*OUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvILKNMappingBuild()", devNum );
           /*Set mapping for ILKN mapping type*/
           rc = cpssDxChPortPhysicalPortMapSet(devNum, portMap.Len, portMap.Data);

           if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_TRUE)
           {
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "cpssDxChPortPhysicalPortMapSet()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

               /*Prepare shadow for ILKN mapping type*/
               rc = prvPortShadowMapGet( devNum, /*OUT*/&portShadowILKN);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvPortShadowMapGet()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

               /*Set Map 2 Equels ILKN Physical port numbers - should be error*/
               rc = prv2EquelsILKNPhysPortsNumberCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr, &portShadowILKN);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsILKNPhysPortsNumberCheck()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

               /*Set Map 2 Equels ILKN Interface numbers - should be error*/
               rc = prv2EquelsILKNInterfacesNumCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr, &portShadowILKN);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsILKNInterfacesNumCheck()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

                /*Set Map 2 Equels ILKN TM Indexes - should be error*/
               rc = prv2EquelsILKNTmIndexCheck( devNum, &portShadow, &portShadowILKN, /*INOUT*/&portMap, /*OUT*/sourcePtr, /*OUT*/destinPtr);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prv2EquelsILKNTmIndexCheck()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

               /*Set Map 2 Interface number out of range - should be error*/
               rc = prvInterfaceNumberILKNOutOfRangeCheck( devNum, &portShadow, &portMap, sourcePtr, destinPtr, &portShadowILKN);
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc,"%s :  dev %d", "prvInterfaceNumberILKNOutOfRangeCheck()", devNum );
               if (GT_OK != rc)
               {
                    goto restore_def_map;
               }

           }
           else
           {
               UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_BAD_PARAM, rc,"%s :  dev %d", "cpssDxChPortPhysicalPortMapSet()", devNum );
               if (GT_BAD_PARAM != rc)
               {
                    goto restore_def_map;
               }
           }

restore_def_map:
           /* Restore default port mapping */
           rc = prvDefaultPortsMappingRestore(devNum, &portShadow, /*OUT*/&portMap);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"%s :  dev %d", "prvDefaultPortsMappingRestore()", devNum );

           /* Restore ports Activation*/
           rc = prvPortsActivateDeactivate(devNum, GT_TRUE, &portActive);
           UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"%s :  dev %d", "prvPortsActivateDeactivate()", devNum );
       }
    }



}

#if 0
GT_VOID cpssDxChPortPhysicalPortMapSetUT(GT_VOID)
{
/*
    ITERATE_DEVICES_PHY_PORTS (BobCat2)
    1.1. Call with valid portMapArrayPtr [non-NULL].
    Expected: GT_OK.
    1.1.1. Call with valid portMapArrayPtr [non-NULL].
    Expected: GT_OK.
    1.1.2. Call with invalid portMapArrayPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with max Ethernet MAC [71].
    Expected: GT_OK.
    1.1.4. Call with max+1 Ethernet MAC [72].
    Expected: GT_BAD_PARAM.
    1.1.5. Call with max Interlaken channel [63].
    Expected: GT_OK.
    1.1.6. Call with max+1 Interlaken channel [64].
    Expected: GT_BAD_PARAM.
    1.1.7. Call with wrong mappingType [CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E]
    Expected: GT_BAD_PARAM.
    1.1.8  Call with various tmPortInd [0, 71, 72, 127, 128, 191, 192]
    Expected [GT_OK, GT_OK, GT_OUT_OF_RANGE, GT_OUT_OF_RANGE, GT_OK, GT_OK, GT_OUT_OF_RANGE]

*/
    GT_STATUS                   st          =   GT_OK;
    GT_U8                       devNum      =   0;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      portMapArraySize = 0;
    static CPSS_DXCH_PORT_MAP_STC      portMapArray[256];
    static CPSS_DXCH_PORT_MAP_STC      portMapArrayGet[256];
    static GT_U8                       portFlags[256];
    GT_U32                      currIndex;
    GT_U32                      ilknPortBase;
    GT_BOOL                     isEqual;
    GT_U32                      ii;
    GT_U32                      tmPortInds[] = {0, 71, 72, 127, 128};
    GT_STATUS                   stTmPortInds[] =
        {GT_OK, GT_OK, GT_BAD_PARAM, GT_BAD_PARAM, GT_BAD_PARAM};

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        GM_NOT_SUPPORT_THIS_TEST_MAC;

        currIndex = 0;

        cpssOsMemSet(portMapArray, 0, sizeof(portMapArray));
        cpssOsMemSet(portMapArrayGet, 0, sizeof(portMapArrayGet));

        /* 1.1. For all active devices go over all available physical ports. */
        while( (GT_OK == prvUtfNextPhyPortGet(&portNum, GT_TRUE)) &&
               (currIndex < UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS) )
        {
            /*
                1.1. Call with valid portMapArrayPtr [non-NULL].
                Expected: GT_OK.
            */

            portMapArray[currIndex].physicalPortNumber = portNum;
            portMapArray[currIndex].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            portMapArray[currIndex].portGroup = 0;
            portMapArray[currIndex].interfaceNum = portNum;
            portMapArray[currIndex].txqPortNumber = portNum;
            portMapArray[currIndex].tmEnable = GT_FALSE;

            currIndex++;
        }
        portMapArraySize = currIndex;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portMapArraySize);


        st = cpssDxChPortPhysicalPortMapGet(devNum, 0, portMapArraySize, portMapArrayGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portMapArraySize);

        if (st == GT_OK)
        {
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)portMapArray, (GT_VOID*)portMapArrayGet,
                                         sizeof(portMapArray)))
                       ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                "got another portMapArrayGet than was set: %d, %d", devNum);
        }

        st = prvUtfNextPhyPortReset(&portNum, devNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        currIndex = 0;
        ilknPortBase = ((UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS / 4) * 3);

        cpssOsMemSet(portMapArray, 0, sizeof(portMapArray));
        cpssOsMemSet(portMapArrayGet, 0, sizeof(portMapArrayGet));
        cpssOsMemSet(portFlags, 0, sizeof(portFlags));

        /* 1.1.1. For all active devices go over all available physical ports. */
        while (currIndex < ilknPortBase)
        {
            /*
                1.1.1. Call with valid portMapArrayPtr [non-NULL].
                Expected: GT_OK.
            */
            if (GT_OK != prvUtfNextPhyPortGet(&portNum, GT_TRUE))
            {
                break;
            }
            portMapArray[currIndex].physicalPortNumber = portNum;
            portMapArray[currIndex].mappingType =
                CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            portMapArray[currIndex].portGroup = 0;
            portMapArray[currIndex].interfaceNum = portNum;
            portMapArray[currIndex].txqPortNumber = portNum;
            portMapArray[currIndex].tmEnable = GT_TRUE;
            portMapArray[currIndex].tmPortInd = currIndex;
            /* when TM enable TXQ overriden by CPSS */
            portFlags[currIndex] |=  UT_NOT_RELEVANT_TXQ_PORT_CNS;

            currIndex++;
        }
        while (currIndex < UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS)
        {
            /*
                1.1.1. Call with valid portMapArrayPtr [non-NULL].
                Expected: GT_OK.
            */

            if (GT_OK != prvUtfNextPhyPortGet(&portNum, GT_TRUE))
            {
                break;
            }
            portMapArray[currIndex].physicalPortNumber = portNum;
            portMapArray[currIndex].mappingType =
                CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E;
            portMapArray[currIndex].portGroup = 0;
            portMapArray[currIndex].interfaceNum = (currIndex - ilknPortBase);
            portMapArray[currIndex].txqPortNumber = portNum;
            portMapArray[currIndex].tmEnable = GT_TRUE;
            portMapArray[currIndex].tmPortInd = 128 + currIndex;
            /* when TM disable TM Port not relevant                               */
            /* interfaceNum used by Set as ILKN Channel, but not supported by Get */
            portFlags[currIndex] |= UT_NOT_RELEVANT_TXQ_PORT_CNS | UT_NOT_RELEVANT_IF_NUM_CNS;

            currIndex++;
        }
        portMapArraySize = currIndex;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portMapArraySize);


        st = cpssDxChPortPhysicalPortMapGet(devNum, 0, portMapArraySize, portMapArrayGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portMapArraySize);

        /* copy not relevant fields from "set data" to "get data" */

        for (ii = 0; (ii < portMapArraySize); ii++)
        {
            if (portFlags[ii] & UT_NOT_RELEVANT_PHY_PORT_CNS)
            {
                portMapArrayGet[ii].physicalPortNumber = portMapArray[ii].physicalPortNumber;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_MAPPING_TYPE_CNS)
            {
                portMapArrayGet[ii].mappingType = portMapArray[ii].mappingType;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_PORT_GROUP_CNS)
            {
                portMapArrayGet[ii].portGroup = portMapArray[ii].portGroup;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_IF_NUM_CNS)
            {
                portMapArrayGet[ii].interfaceNum = portMapArray[ii].interfaceNum;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_TXQ_PORT_CNS)
            {
                portMapArrayGet[ii].txqPortNumber = portMapArray[ii].txqPortNumber;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_TM_ENABLE_CNS)
            {
                portMapArrayGet[ii].tmEnable = portMapArray[ii].tmEnable;
            }
            if (portFlags[ii] & UT_NOT_RELEVANT_TM_PORT_CNS)
            {
                portMapArrayGet[ii].tmPortInd = portMapArray[ii].tmPortInd;
            }
        }


        if (st == GT_OK)
        {
            isEqual = (0 == cpssOsMemCmp((GT_VOID*)portMapArray, (GT_VOID*)portMapArrayGet,
                                         sizeof(portMapArray)))
                       ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual,
                "got another portMapArrayGet than was set: %d", devNum);
        }

        /*
            1.1.2. Call with invalid portMapArrayPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.1.3. Call with max Ethernet MAC [71].
            Expected: GT_OK.
        */
        portMapArray[0].interfaceNum = UT_MAC_MAX_NUM_CNS - 1;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.4. Call with max+1 Ethernet MAC [72].
            Expected: GT_BAD_PARAM.
        */
        portMapArray[0].interfaceNum = UT_MAC_MAX_NUM_CNS;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        portMapArray[0].interfaceNum = UT_MAC_MAX_NUM_CNS - 1;

        /*
            1.1.5. Call with max Interlaken channel [63].
            Expected: GT_OK.
        */
        portMapArray[1].interfaceNum = UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS - 1;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.1.6. Call with max+1 Interlaken channel [64].
            Expected: GT_BAD_PARAM.
        */
        portMapArray[1].interfaceNum = UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS;

        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        portMapArray[1].interfaceNum = UT_ILKN_CHANNEL_MAX_PHYS_PORT_CNS - 1;

        /*
            1.1.7. Call with wrong mappingType [CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E]
            Expected: GT_BAD_PARAM.
        */
        for ( ii = 0 ; ii <= CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E ; ii++ )
        {
            if( (CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E == ii) ||
                (CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == ii) )
            {
                continue;
            }

            portMapArray[0].mappingType = ii;

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, ii);
        }

        portMapArray[0].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        portMapArray[0].tmEnable    = GT_TRUE;

        /*
            1.1.8  Call with various tmPortInd [0, 71, 72, 127, 128]
            Expected [GT_OK, GT_OK, GT_OUT_OF_RANGE, GT_BAD_PARAM]
        */

        for ( ii = 0 ; ii < (sizeof(tmPortInds)/sizeof(tmPortInds[0])) ; ii++ )
        {
            portMapArray[0].tmPortInd = tmPortInds[ii];

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(stTmPortInds[ii], st, devNum, ii);
        }

        portMapArray[0].tmPortInd = 0;

#if 0

        /*
            1.1.3. Call with invalid mapping - different physical to the same MAC.
            Expected: GT_BAD_PARAM.
        */
        if(currIndex >= 3)
        {
            for(i = 0; i < 3; i++)
            {
                /*portMapArray[i].physicalPortNumber = portNum;*/
                /*portMapArray[i].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;*/
                /*portMapArray[i].portGroup = 0;*/
                portMapArray[i].interfaceNum = portMapArray[0].physicalPortNumber;
                /*portMapArray[i].txqPortNumber = portNum;*/
                /*portMapArray[i].tmEnable = GT_FALSE;*/
            }

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

            /* restore default 1:1 mapping */
            for(i = 0; i < 3; i++)
            {
                portMapArray[i].interfaceNum = portMapArray[i].physicalPortNumber;
            }

        }


        /*
            1.1.4. Call with invalid mapping - different physical to the same TXQ.
            Expected: GT_BAD_PARAM.
        */
        if(currIndex >= 3)
        {
            for(i = 0; i < 3; i++)
            {
                /*portMapArray[i].physicalPortNumber = portNum;*/
                /*portMapArray[i].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;*/
                /*portMapArray[i].portGroup = 0;*/
                /*portMapArray[i].interfaceNum = portNum;*/
                portMapArray[i].txqPortNumber = portMapArray[0].physicalPortNumber;
                /*portMapArray[i].tmEnable = GT_FALSE;*/
            }

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

            /* restore default 1:1 mapping */
            for(i = 0; i < 3; i++)
            {
                portMapArray[i].txqPortNumber = portMapArray[i].physicalPortNumber;
            }

        }


        /*
            1.1.5. Call with invalid mapping - .
            Expected: GT_BAD_PARAM.
        */
        if(currIndex >= 3)
        {
            for(i = 0; i < 3; i++)
            {
                /*portMapArray[i].physicalPortNumber = portNum;*/
                /*portMapArray[i].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;*/
                /*portMapArray[i].portGroup = 0;*/
                /*portMapArray[i].interfaceNum = portNum;*/
                portMapArray[i].txqPortNumber = portMapArray[0].physicalPortNumber;
                /*portMapArray[i].tmEnable = GT_FALSE;*/
            }

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

            /* restore default 1:1 mapping */
            for(i = 0; i < 3; i++)
            {
                portMapArray[i].txqPortNumber = portMapArray[i].physicalPortNumber;
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
            portMapArray[0].physicalPortNumber = portNum;
            portMapArray[0].mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            portMapArray[0].portGroup = 0;
            portMapArray[0].interfaceNum = portNum;
            portMapArray[0].txqPortNumber = portNum;
            portMapArray[0].tmEnable = GT_FALSE;

            portMapArraySize = 1;

            st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portMapArraySize);
        }

#endif
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArray);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

#endif
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortPhysicalPortMapGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_DXCH_PORT_MAP_STC    *portMapArrayPtr
);
 */
GT_VOID cpssDxChPortPhysicalPortMapGetUT(GT_VOID)
/*UTF_TEST_CASE_MAC(cpssDxChPortPhysicalPortMapGet)*/
{
/*
    ITERATE_DEVICES_PHY_PORTS (BobCat2)
    1.1.1. Call with invalid portMapArrayPtr [NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with firstPhysicalPortNumber + portMapArraySize >
                                                            (max phy port + 1)
    Expected: GT_FAIL.

*/

    GT_U8                       devNum      =   0;

    GT_STATUS                   st          =   GT_OK;
    static CPSS_DXCH_PORT_MAP_STC      portMapArrayGet[256];

    /* prepare device iterator to go through all applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    cpssOsBzero((GT_VOID*) &portMapArrayGet, sizeof(portMapArrayGet));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1.1. Call with invalid portMapArrayPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPortPhysicalPortMapGet(devNum, 0, 1, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.1.2. Call with firstPhysicalPortNumber + portMapArraySize >
                                                            (max phy port + 1)
            Expected: GT_FAIL.
        */

        st = cpssDxChPortPhysicalPortMapGet(devNum, 0,
                                            PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum) + 2,
                                            portMapArrayGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_FAIL, st, devNum);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator to go through all non-applicable devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                         UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                         UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E );

    /* Go over all non applicable devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPortPhysicalPortMapGet(devNum, 0, 1, portMapArrayGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3.Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortPhysicalPortMapGet(devNum, 0, 1, portMapArrayGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxChPortMapping suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortMapping)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPhysicalPortMapSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortPhysicalPortMapGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortMapping)


