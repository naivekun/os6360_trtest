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
* @file cpssDxChHwInitDeviceMatrixUT.c
*
* @brief Unit tests for Device Matrix
*
* @version   15
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfDeviceMatrix.h>
#include <utf/private/prvUtfDeviceMatrixData.h>
#include <common/tgfCommon.h>

#include <cpssCommon/cpssBuffManagerPool.h>

/* Include appDemo */
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#define SET_MATRIX_PTR_AND_SIZE_MAC(_ptrMatrixData,_matrixDataEntriesNum,_matrixTable) \
    _ptrMatrixData = &_matrixTable[0];                                          \
    _matrixDataEntriesNum = sizeof(_matrixTable)/sizeof(_matrixTable[0])

extern GT_STATUS appDemoDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
);

/**
* @struct PRV_UTF_DEVICE_MATRIX_DATA_OVERRIDE_STC
 *
 * @brief Entry format to override device matrix parameter value.
*/
typedef struct{

    /** @brief HW device ID
     *  fieldName  - field name to override
     */
    GT_U32 deviceId;

    GT_CHAR *fieldName;

    /** new field value (the override one) */
    GT_U32 fieldValue;

} PRV_UTF_DEVICE_MATRIX_DATA_OVERRIDE_STC;

/* Some device parameters used in the CPSS are changed\override in compare */
/* to the expected ones based on the device matrix file. Since the parsing */
/* of device matrix file which is the base of transforming it into the     */
/* parameters H file is unaware of these modification, the following array */
/* enables overriding device matrix values. */
PRV_UTF_DEVICE_MATRIX_DATA_OVERRIDE_STC prvUtfDeviceMatrixDataOverride[] =
{
    { 0xDF50, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xDFE4, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xDFF4, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xDF74, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xDF76, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xDF70, "routerNextHop"       ,   NA_TABLE_CNS    }, /* AlleyCat2 - no Router Next Hop - only PBR */
    { 0xDF34, "transmitDescr"       ,   0x1000          }, /* AlleyCat-WS 8Mb buffer memory - 4K descriptors*/
    { 0xE01D, "ingressPolicer"      ,   0x2000          }, /* Lion 98CX8226 */
    { 0xE01D, "egressPolicer"       ,   0x800           }, /* Lion 98CX8226 */
    { 0xE01D, "cnc"                 ,   0x4000          }, /* Lion 98CX8226 */
    { 0xE01D, "tunnelTermination"   ,   0x4000          }  /* Lion 98CX8226 */
};

/**
* @internal prvCpssDxChHwInitDeviceMatrixDisableInt function
* @endinternal
*
* @brief   Disable interrupts.
*
*/
static void prvCpssDxChHwInitDeviceMatrixDisableInt(void)
{
    PRV_CPSS_LOCK_BY_MUTEX;
    PRV_CPSS_INT_SCAN_LOCK();
    /* wait 1 sec to finish all interrupts processing */
    cpssOsTimerWkAfter(1000);
}

/**
* @internal prvCpssDxChHwInitDeviceMatrixEnableInt function
* @endinternal
*
* @brief   Enable interrupts.
*
*/
static void prvCpssDxChHwInitDeviceMatrixEnableInt(void)
{
    PRV_CPSS_INT_SCAN_UNLOCK();
    PRV_CPSS_UNLOCK_BY_MUTEX;
}

/**
* @internal prvCpssDxChHwDeviceMatrixOverride function
* @endinternal
*
* @brief   Override device parameters as retrieved fron device matrix file.
*
* @param[in] ptrMatrixData            - (pointer to) device parameters list to
*                                      override.
* @param[in] matrixDataEntriesNum     - number of entries(=devices) in data based
*                                      created based on the device matrix file
*                                       None
*/
static void prvCpssDxChHwDeviceMatrixOverride
(
    IN PRV_UTF_DEVICE_MATRIX_DATA_STC  *ptrMatrixData,
    IN GT_U32                          matrixDataEntriesNum
)
{
    GT_U32 ii, jj;

    for ( ii = 0 ;
          ii < sizeof(prvUtfDeviceMatrixDataOverride)/
                sizeof(prvUtfDeviceMatrixDataOverride[0]);
          ii++ )
    {
        for( jj = 0 ; jj < matrixDataEntriesNum ; jj ++ )
        {
            if( ptrMatrixData[jj].deviceId == prvUtfDeviceMatrixDataOverride[ii].deviceId )
            {
                if( 0 == osStrCmp("bufferMemory",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].bufferMemory =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("transmitDescr",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].transmitDescr =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("fdb",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].fdb =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("txQueues",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].txQueues =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("mcGroups",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].mcGroups =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("trunks",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].trunks =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("mstp",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].mstp =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("ingressAcls",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].ingressAcls =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("ingressPolicer",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].ingressPolicer =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("egressPolicer",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].egressPolicer =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("arp",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].arp =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("tunnelStart",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].tunnelStart =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("tunnelTermination",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].tunnelTermination =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("lpm",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].lpm =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("routerNextHop",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].routerNextHop =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("ipMll",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].ipMll =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("cnc",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].cnc =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("vlanTranslationSupport",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].vlanTranslationSupport = (GT_BOOL)
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("tr101Support",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].tr101Support = (GT_BOOL)
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("networkPorts",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].networkPorts =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("networkPortsType",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].networkPortsType = (PRV_CPSS_PORT_TYPE_ENT)
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else if( 0 == osStrCmp("stackPorts",
                                  prvUtfDeviceMatrixDataOverride[ii].fieldName) )
                {
                    ptrMatrixData[jj].stackPorts =
                                    prvUtfDeviceMatrixDataOverride[ii].fieldValue;
                }
                else
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, GT_FALSE,
                                                 "No match for field %s at line %d",
                                                 prvUtfDeviceMatrixDataOverride[ii].fieldName,
                                                 ii);
                }
                break;
            }
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(jj, matrixDataEntriesNum,
                                             "No match for deviceId 0x%x at line %d",
                                             prvUtfDeviceMatrixDataOverride[ii].deviceId,
                                             ii);
        }
    }
}

/**
* @internal prvCpssDxChHwDeviceMatrixCheckResults function
* @endinternal
*
* @brief   Check device parameters according to device matrix file.
*
* @param[in] deviceType               - device type
* @param[in] matrixDataPtr            - (pointer to) device parameters list to
*                                      verify.
*                                       None
*/
static void prvCpssDxChHwDeviceMatrixCheckResults
(
    IN CPSS_PP_DEVICE_TYPE             deviceType,
    IN PRV_UTF_DEVICE_MATRIX_DATA_STC  *matrixDataPtr
)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev = prvTgfDevNum;
    PRV_UTF_DEVICE_MATRIX_DATA1_STC *matrixData1Ptr; /* extra device parameters */
    GT_U32      jj,kk;
    GT_U32      mcGroups;
    GT_U32      tunnelTermination;
    GT_U32      isLpmInTcam;
    GT_U32      arp;                /*num of arp entries */
    GT_U32      networkPortsNum;
    GT_U32      ingressAclsDivider;
    GT_U32      ipMllDivider;
    GT_U32      perCoreDivider;
    CPSS_DXCH_CFG_DEV_INFO_STC  devInfo;
    GT_U32      perCoreDivider_plr;
    GT_U32      perCoreDivider_cnc;
    GT_U32      doublePhysicalPorts = 0;
    GT_U32      mstpDivider = 1;
    GT_U32      trunksDivider = 1;


    perCoreDivider = 1;
    ingressAclsDivider = 4;
    isLpmInTcam = 1;
    ipMllDivider = 2;

    st = cpssDxChCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Device get info for device 0x%x", deviceType);
    if( GT_OK != st )
    {
        return;
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
    {
        ingressAclsDivider = 1;/* the number are already in rules in DB */
        isLpmInTcam = 0;
        ipMllDivider = 1;/* the MLL are already in pairs in DB */
    }

    switch(devInfo.genDevInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION_E:
                perCoreDivider = 4;
                break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
                perCoreDivider = 8;
                break;
        default:
                break;
    }

    perCoreDivider_plr = perCoreDivider;
    perCoreDivider_cnc = perCoreDivider;

    /* override special 'per core' tables */
    switch(devInfo.genDevInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        /* currently Falcon as BC3 */
        CPSS_TBD_BOOKMARK_FALCON
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                perCoreDivider_plr = 2;
                perCoreDivider_cnc = 2;

                /* next added with indication that 'size' is multiple of pipes */
                matrixDataPtr->cnc            &= ~PIPE_INDICATION;
                matrixDataPtr->ingressPolicer &= ~PIPE_INDICATION;

                doublePhysicalPorts =
                    PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(dev,CPSS_DXCH_LION_TABLE_STG_EGRESS_E) ? 1 : 0;
                if(doublePhysicalPorts)
                {
                    mstpDivider = 2;
                    trunksDivider = 2;
                }

                break;
        default:
                break;
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
    {
        ipMllDivider = 2;/* the MLL are NOT in pairs in DB */
    }


    /* Buffer memory */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->bufferMemory,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.bufferMemory,
                                 "Buffer memory for device 0x%x",
                                 deviceType);

    /* Transmit descriptors */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->transmitDescr,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.transmitDescr,
                                 "Transmit descriptors for device 0x%x",
                                 deviceType);

    /* FDB*/
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->fdb,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb,
                                 "FDB size different for device 0x%x",
                                 deviceType);

    /* Transmit queues */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->txQueues,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.txQueuesNum,
                                 "Transmit queues for device 0x%x",
                                 deviceType);

    if(matrixDataPtr->mcGroups == VIDX_NUM_4K_INDICATION_CNS)
    {
        mcGroups = _4K;
    }
    else
    {
        if( 0x1000 == matrixDataPtr->mcGroups )
        {
            mcGroups = 0xFFF;
        }
        else
        {
            mcGroups = matrixDataPtr->mcGroups;
        }
    }

    /* MC\VIDX groups */
    UTF_VERIFY_EQUAL1_STRING_MAC(mcGroups,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.vidxNum,
                                 "MC groups for device 0x%x",
                                 deviceType);

    /* MSTP groups */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->mstp / mstpDivider,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.stgNum,
                                 "MSTP groups for device 0x%x",
                                 deviceType);

    /* Trunks */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->trunks / trunksDivider,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum,
                                 "Trunks for device 0x%x",
                                 deviceType);

    /* Ingress ACLs */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->ingressAcls/ingressAclsDivider,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.policyTcamRaws,
                                 "Ingress ACLs for device 0x%x",
                                 deviceType);

    /* Ingress policer */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->ingressPolicer/perCoreDivider_plr,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.policersNum,
                                 "Ingress policer for device 0x%x",
                                 deviceType);

    if(matrixDataPtr->egressPolicer == SHARED_ING_EGR_POLICER_INDICATION_CNS)
    {
        /*EPLR and IPLR are shared */
        matrixDataPtr->egressPolicer = matrixDataPtr->ingressPolicer;
    }

    /* Egress policer */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->egressPolicer/perCoreDivider_plr,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.egressPolicersNum,
                                 "Egress policer for device 0x%x",
                                 deviceType);

    arp = (matrixDataPtr->arp == ARP_FROM_TS_INDICATION_CNS) ?
                                ((matrixDataPtr->tunnelStart/perCoreDivider) * 4) :/* 4 ARP for each TS */
                                matrixDataPtr->arp;

    /* ARP */
    UTF_VERIFY_EQUAL1_STRING_MAC(arp,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerArp,
                                 "ARP entries for device 0x%x",
                                 deviceType);

    /* Tunnel start */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->tunnelStart/perCoreDivider,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelStart,
                                 "Tunnel start entries for device 0x%x",
                                 deviceType);

    if(matrixDataPtr->tunnelTermination == SINGLE_TCAM_INDICATION_CNS)
    {
        /* use info from IPCL about the TCAM */
        tunnelTermination = matrixDataPtr->ingressAcls/perCoreDivider;
    }
    else
    if(isLpmInTcam && (matrixDataPtr->lpm/4 >= matrixDataPtr->tunnelTermination) )
    {
        if(devInfo.genDevInfo.devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            tunnelTermination = matrixDataPtr->tunnelTermination;
        }
        else
        {
            tunnelTermination = matrixDataPtr->lpm/4;
        }
    }
    else
    {
        tunnelTermination = matrixDataPtr->tunnelTermination/perCoreDivider;
    }

    if(isLpmInTcam)
    {
        /* Router TCAM entries */
        UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->lpm/4,
                                     PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.router,
                                     "Router entries for device 0x%x",
                                     deviceType);
    }

    /* Tunnel Termination entries */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelTermination,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.tunnelTerm,
                                 "Tunnel Termination entries for device 0x%x",
                                 deviceType);

    /* Router Next Hop */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->routerNextHop,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.routerNextHop,
                                 "Router Next Hop entries for device 0x%x",
                                 deviceType);

    /* IP MLL\MLL pairs */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->ipMll/ipMllDivider,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.mllPairs,
                                 "IP MLL pairs for device 0x%x",
                                 deviceType);

    /* CNC */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->cnc/perCoreDivider_cnc,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.cncBlocks *
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.cncBlockNumEntries,
                                 "CNC for device 0x%x",
                                 deviceType);

    /* Vlan translation support */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->vlanTranslationSupport,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.vlanTranslationSupported,
                                 "Vlan translation support for device 0x%x",
                                 deviceType);

    /* TR101 support */
    UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->tr101Support,
                                 PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.tr101Supported,
                                 "TR101 support for device 0x%x",
                                 deviceType);


    if((CPSS_PP_FAMILY_DXCH_XCAT_E == devInfo.genDevInfo.devFamily) ||
       (CPSS_PP_FAMILY_DXCH_XCAT3_E == devInfo.genDevInfo.devFamily) ||
       (CPSS_PP_FAMILY_DXCH_XCAT2_E == devInfo.genDevInfo.devFamily))
    {
        /* networkPorts */
        networkPortsNum = 0;
        /* loop over 24lsb of exiting ports bitmap to find out the network ports number */
        for(jj = 0 ; jj < 24 ; jj++)
        {
            if( devInfo.genDevInfo.existingPorts.ports[0] & (1 << jj) )
            {
                networkPortsNum++;
            }
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->networkPorts,
                                     networkPortsNum,
                                     "Number of network ports for device 0x%x",
                                     deviceType);

        /* networkPortsType */
        /* check network port type based on port 0 type */
        UTF_VERIFY_EQUAL1_STRING_MAC(matrixDataPtr->networkPortsType,
                                     PRV_CPSS_DXCH_PORT_TYPE_MAC(dev, 0),
                                     "Network ports type for device 0x%x",
                                     deviceType);
    }

    matrixData1Ptr = matrixDataPtr->data1InfoPtr;

    if(matrixData1Ptr)
    {
        if(matrixData1Ptr->check_existingPorts == GT_TRUE)
        {
            /* compare the existing ports bmp */
            for(kk = 0 ; kk < CPSS_MAX_PORTS_BMP_NUM_CNS ; kk++)
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(
                    matrixData1Ptr->existingPorts.ports[kk],
                    devInfo.genDevInfo.existingPorts.ports[kk],
                    "expected in range of ports [%d..%d] bmp[0x%8.8x] got [0x%8.8x] \n",
                    (kk * 32), /*start port*/
                    (kk * 32) + 31,/*end port*/
                    matrixData1Ptr->existingPorts.ports[kk],/*expected*/
                    devInfo.genDevInfo.existingPorts.ports[kk] /*actual*/
                    );
            }
        }
    }
}

/**
* @internal prvCpssDxChHwBobCat2DeviceMatrixTest function
* @endinternal
*
* @brief   Test Bobcat2 parameters according to device matrix file.
*
* @param[in] matrixDataPtr            - (pointer to) device parameters list to
*                                      verify.
* @param[in] matrixDataEntriesNum     - number of matrix entries
*                                       None
*/
static void prvCpssDxChHwBobCat2DeviceMatrixTest
(
    IN PRV_UTF_DEVICE_MATRIX_DATA_STC  *matrixDataPtr,
    IN GT_U32                          matrixDataEntriesNum
)
{
    GT_STATUS               st = GT_OK;
    GT_U32                  ii;
    CPSS_PP_DEVICE_TYPE     deviceType = 0x11AB;

    for( ii = 0 ; ii < matrixDataEntriesNum ; ii++ )
    {
        U32_SET_FIELD_MAC(deviceType, 16, 16, (matrixDataPtr->deviceId));
        /* Override device ID by value from device matrix */
        st = appDemoDebugDeviceIdSet(prvTgfDevNum, deviceType);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDrvDebugDeviceIdSet: entry %d, device 0x%x",
                                     ii, deviceType);
        if( GT_OK != st )
        {
            break;
        }

        /* Reset system for new device ID */
        st = prvTgfResetAndInitSystem();
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvTgfResetAndInitSystem: entry %d, device 0x%x",
                                     ii, deviceType);
        if( GT_OK != st )
        {
            break;
        }

        /* Verify device info according to matrix data */
        prvCpssDxChHwDeviceMatrixCheckResults(deviceType, matrixDataPtr);

        matrixDataPtr++;

        /* wait until the reset is done */
        osTimerWkAfter(10);
    }
    appDemoDebugDeviceIdReset();
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwInitDeviceMatrixCheckTest
(
    IN GT_U8    devNum
)
*/
static void cpssDxChHwInitDeviceMatrixCheckTest(IN GT_U8 dev)
{
    GT_STATUS   st = GT_OK;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   ppPhase1Params;
    PRV_UTF_DEVICE_MATRIX_DATA_STC  *ptrMatrixData;
    GT_U32                          matrixDataEntriesNum;
    CPSS_PP_DEVICE_TYPE                 deviceType;
    GT_U32          ii;
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr = PRV_CPSS_PP_MAC(dev);
    GT_U32  queue;/*rx/tx queue*/
    GT_BOOL is_ASIC_SIMULATION;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );

    if( GT_OK != prvUtfNextDeviceGet(&dev, GT_TRUE) )
    {
        return;
    }

    st = cpssDxChCfgDevInfoGet(dev, &devInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    st = appDemoDxChPhase1ParamsGet(&ppPhase1Params);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

#ifdef ASIC_SIMULATION
    is_ASIC_SIMULATION = GT_TRUE;
#else
    is_ASIC_SIMULATION = GT_FALSE;
#endif


    switch(devInfo.genDevInfo.devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForXcat);
                break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForXcat3);
                break;
        case CPSS_PP_FAMILY_DXCH_LION_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForLion);
                break;
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForXcat2);
                break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForLion2);
                break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForBobcat2);

                if(PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                {
                    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) == GT_TRUE)
                    {
                       SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForCetus);
                    }
                    else
                    {
                        SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForCaelum);
                    }
                }

                if(prvTgfResetModeGet() == GT_TRUE && is_ASIC_SIMULATION == GT_TRUE)
                {
                    /* Test Bobcat2 matrix devices */
                    prvCpssDxChHwBobCat2DeviceMatrixTest(ptrMatrixData, matrixDataEntriesNum);
                    return;
                }

                break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForAldrin);

            if(prvTgfResetModeGet() == GT_TRUE  && is_ASIC_SIMULATION == GT_TRUE)
            {
                /* Test Aldrin matrix devices */
                prvCpssDxChHwBobCat2DeviceMatrixTest(ptrMatrixData, matrixDataEntriesNum);
                return;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForAC3X);

            if(prvTgfResetModeGet() == GT_TRUE  && is_ASIC_SIMULATION == GT_TRUE)
            {
                /* Test AC3X matrix devices */
                prvCpssDxChHwBobCat2DeviceMatrixTest(ptrMatrixData, matrixDataEntriesNum);
                return;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForBobcat3);

            if(prvTgfResetModeGet() == GT_TRUE  && is_ASIC_SIMULATION == GT_TRUE)
            {
                /* Test Aldrin matrix devices */
                prvCpssDxChHwBobCat2DeviceMatrixTest(ptrMatrixData, matrixDataEntriesNum);
                return;
            }

            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            SET_MATRIX_PTR_AND_SIZE_MAC(ptrMatrixData,matrixDataEntriesNum,prvUtfDeviceMatrixDataForAldrin2);

            if(prvTgfResetModeGet() == GT_TRUE  && is_ASIC_SIMULATION == GT_TRUE)
            {
                /* Test Aldrin matrix devices */
                prvCpssDxChHwBobCat2DeviceMatrixTest(ptrMatrixData, matrixDataEntriesNum);
                return;
            }

            break;
        default: UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Unsupported device family");
                return;
    }

    /* update matrix values with override ones */
    prvCpssDxChHwDeviceMatrixOverride(ptrMatrixData, matrixDataEntriesNum);

    /* Disable interrupts */
    prvCpssDxChHwInitDeviceMatrixDisableInt();

    /* clear buffers pools due to buffers allocated but not yet freed by former UTs*/
    for(queue = 0; queue < NUM_OF_TX_QUEUES; queue++)
    {
        if(devPtr->intCtrl.txDescList[queue].poolId)
        {
            st = cpssBmPoolReCreate(devPtr->intCtrl.txDescList[queue].poolId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
    }

    for( ii = 0 ; ii < matrixDataEntriesNum ; ii++ )
    {
        st = cpssDxChCfgDevRemove(dev);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChCfgDevRemove: entry %d, device 0x%x",
                                     ii, (ptrMatrixData->deviceId));

        if( GT_OK != st )
        {
            break;
        }

        deviceType = 0x11AB;
        U32_SET_FIELD_MAC(deviceType, 16, 16, (ptrMatrixData->deviceId));
        st = prvCpssDrvDebugDeviceIdSet(dev, deviceType);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDrvDebugDeviceIdSet: entry %d, device 0x%x",
                                     ii, deviceType);
        if( GT_OK != st )
        {
            break;
        }

        st = cpssDxChHwPpPhase1Init(&ppPhase1Params, &deviceType);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChHwPpPhase1Init: entry %d, device 0x%x",
                                     ii, deviceType);
        if( GT_OK != st )
        {
            break;
        }

        prvCpssDxChHwDeviceMatrixCheckResults(deviceType, ptrMatrixData);

        ptrMatrixData++;
    }

    if(is_ASIC_SIMULATION == GT_TRUE)
    {
        /* Enable interrupts */
        prvCpssDxChHwInitDeviceMatrixEnableInt();
    }
}
extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

UTF_TEST_CASE_MAC(cpssDxChHwInitDeviceMatrixCheckTest)
{
    GT_U8       dev;
    GT_U32      boardIdx;
    GT_U32      boardRevId;
    GT_U32      reloadEeprom;

    /* the test too hard for GM because it use multiple system
       resets each of them allocates more and more not freeable memory.
       There is no added value to check the test on GM also. */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip test for systems with TM because it's already done without TM */
    PRV_TGF_SKIP_TEST_WHEN_TM_USED_MAC;

    cpssInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip multidevice boards */
    if((boardIdx == 30) /* BC2 x 6 */ ||
       (boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    if(prvUtfIsAc3xWithPipeSystem()) /* DXPX Board */
    {
        SKIP_TEST_MAC;
    }

    /* reduce run of logic per single device of the family */
    prvUtfSetSingleIterationPerFamily();

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssDxChHwInitDeviceMatrixCheckTest(dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChHwInitDeviceMatrix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChHwInitDeviceMatrix)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwInitDeviceMatrixCheckTest)

UTF_SUIT_END_TESTS_MAC(cpssDxChHwInitDeviceMatrix)

