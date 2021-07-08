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
* @file cpssDxChPortInterlakenUT.c
*
* @brief Unit tests for cpssDxChPortInterlaken, that provides
* CPSS implementation for Interlaken Port configuration.
*
* @version   13
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitBW.h>

/* defines */

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChPortInterlakenCounterGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORTS (BobCat2)
    1.1.1. Call with counterId[CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_PKT_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_CRC_ERR_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BURST_SIZE_ERR_E/
                               CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_ERR_E],
                    laneNum[0/7] (relevant only for
                                          CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E/
                                          CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E/
                                          CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E)
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum[8] and
           counterId[CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E/
                     CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E/
                     CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E]
    Expected: NOT GT_OK.
    1.1.3. Call with out of range laneNum[8] and
           counterId[CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E/
                     CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_CRC_ERR_E/
                     CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_ERR_E]
    Expected: GT_OK.
    1.1.4. Call with wrong enum value counterId
    Expected: GT_BAD_PARAM.
    1.1.5. Call with NULL counterPtr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = 0;
    PRV_CPSS_PORT_TYPE_ENT portType;


    CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT              counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E;
    GT_U32                                           laneNum = 0;
    GT_U64                                           counter = {{0}};


    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E
                                         | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E  | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            if(portType != PRV_CPSS_PORT_ILKN_E)
            {
                continue;
            }

            /*   1.1.1.   */
            laneNum = 5;
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_PKT_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            laneNum = 7;
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_CRC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            laneNum = 0;
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BURST_SIZE_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.1.   */
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.2.   */
            laneNum = 8;
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_DIAG_CRC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_WORD_SYNC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*   1.1.3.   */
            laneNum = 8;
            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_BYTE_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_CRC_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_RX_ALIGNMENT_ERR_E;

            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*  1.1.4. */
            UTF_ENUMS_CHECK_MAC(cpssDxChPortInterlakenCounterGet(dev, port,
                                counterId, laneNum, &counter),
                                counterId);

            /*   1.1.5.   */
            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, port);

        }

        counterId = CPSS_PORT_INTERLAKEN_COUNTER_ID_BLK_TYPE_ERR_E;

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_GET_MAX_PHY_PORTS_NUM_MAC(dev);

        st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */

        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    port = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E
                                     | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPortInterlakenCounterGet(dev, port, counterId, laneNum, &counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/* Pay attention: tested only under cpssInitSystem 29,2,0 */
/* UTF_TEST_CASE_MAC(cpssDxChPortIlknChannelSpeedSet) */
GT_STATUS IsThereAnyDefinedIlknPort
(
    IN  GT_U8 dev,
    OUT GT_BOOL *thereisIlknPtr
)
{
    GT_STATUS st;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_U32 j;

    for(j = 0; j < 256; j++)
    {
        st = cpssDxChPortPhysicalPortDetailedMapGet(dev,j,/*OUT*/portMapShadowPtr);
        if (st != GT_OK)
        {
            return st;
        }
        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E)
            {
                /* cpssOsPrintf("\n  interlaken channels found %d\n",j); */
                *thereisIlknPtr = GT_TRUE;
                return GT_OK;
            }

        }
    }
    *thereisIlknPtr = GT_FALSE;
    return GT_OK;
}

static GT_STATUS deleteAllChannels
(
    IN  GT_U8                 dev,
    IN  GT_PHYSICAL_PORT_NUM  basePhysPortNum,
    OUT GT_U32               *failedChannelIdPtr
)
{
    GT_STATUS rc;
    GT_U32  i;
    GT_U32  actualSpeed;
    for (i = 0 ; i < 64 ; i++)
    {
        if (failedChannelIdPtr != NULL)
        {
            *failedChannelIdPtr = i;
        }
        rc = cpssDxChPortIlknChannelSpeedSet(dev,basePhysPortNum+i,CPSS_PORT_DIRECTION_BOTH_E, 0, &actualSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if (failedChannelIdPtr != NULL)
    {
        *failedChannelIdPtr = (GT_U32)~0;
    }
    return GT_OK;
}


#define PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(e, r)                                            \
    do {                                                                                 \
        GT_BOOL err = utfEqualVerify(e, r, __LINE__, __FILE__);                          \
        if (GT_FALSE == err) {                                                           \
            err = utfFailureMsgLog((const GT_CHAR *)NULL, (const GT_UINTPTR *)NULL, 0);  \
            if (GT_FALSE == err) return r;                                               \
        }                                                                                \
    } while(0)


GT_STATUS cpssDxChPortIlknChannelSpeedResolutionSetUT_removePort
(
    IN GT_U8 dev,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT       ifMode,
    IN CPSS_PORT_SPEED_ENT                speed
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);
    rc =  cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,ifMode,speed);
    PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    return GT_OK;
}

GT_STATUS cpssDxChPortIlknChannelSpeedResolutionSetUT_configurePort
(
    IN GT_U8 dev,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT       ifMode,
    IN CPSS_PORT_SPEED_ENT                speed
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC portsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);
    rc =  cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,ifMode,speed);
    PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    return GT_OK;
}


GT_STATUS cpssDxChPortIlknChannelSpeedResolutionSetUT_removeBW
(
    IN    GT_U8  dev,
    IN    GT_U32 bw2Free,
    INOUT CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS],
    INOUT CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS],
    INOUT GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_STATUS                          rc;
    GT_U32                             totalBWMbps;
    GT_U32                             unitBWGbps;
    GT_PHYSICAL_PORT_NUM               portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;
    GT_U32                             maxPortNum;

    rc = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, /*OUT*/&maxPortNum);
    PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /*-----------------------------------------------*/
    /* collect info about Network ports and total BW */
    /*-----------------------------------------------*/
    totalBWMbps = 0;
    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        speedArr[portNum]  = CPSS_PORT_SPEED_NA_E;
        ifModeArr[portNum] = CPSS_PORT_INTERFACE_MODE_NA_E;
        toDeleteArr[portNum] = GT_FALSE;

        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapShadowPtr);
        PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (portMapShadowPtr->valid == GT_TRUE)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                rc = cpssDxChPortSpeedGet(dev,portNum,    /*OUT*/&speedArr[portNum]);
                PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifModeArr[portNum]);
                PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                /* NA interface */
                if (ifModeArr[portNum] != CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    switch (speedArr[portNum])
                    {
                        case CPSS_PORT_SPEED_10_E   : totalBWMbps += 1000;   break;
                        case CPSS_PORT_SPEED_100_E  : totalBWMbps += 1000;   break;
                        case CPSS_PORT_SPEED_1000_E : totalBWMbps += 1000;   break;
                        case CPSS_PORT_SPEED_2500_E : totalBWMbps += 2500;   break;
                        case CPSS_PORT_SPEED_5000_E : totalBWMbps += 5000;   break;
                        case CPSS_PORT_SPEED_10000_E: totalBWMbps += 10000;  break;
                        case CPSS_PORT_SPEED_20000_E: totalBWMbps += 20000;  break;
                        case CPSS_PORT_SPEED_40000_E: totalBWMbps += 40000;  break;
                        default:
                        {
                            PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
                        }
                    }
                }
            }
        }
    }
    /*---------------------------------------------------------*/
    /* cselect ports to remove, first 40G, then 20G, than 10G  */
    /*---------------------------------------------------------*/
    rc = prvCpssDxChPortDynamicPizzaArbiterIfUnitBWGet(dev,CPSS_DXCH_PA_UNIT_RXDMA_0_E, &unitBWGbps);
    PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    if (totalBWMbps > unitBWGbps*1000 - bw2Free)
    {
        for (portNum = 0; portNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; portNum++)
        {
            if (speedArr[portNum] == CPSS_PORT_SPEED_40000_E)
            {
                toDeleteArr[portNum] = GT_TRUE;

                rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_removePort(dev,portNum,ifModeArr[portNum],speedArr[portNum]);
                PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                totalBWMbps -= 40000;
                if (totalBWMbps < unitBWGbps*1000 - bw2Free)
                {
                    return GT_OK;
                }
            }
        }
        for (portNum = 0; portNum < maxPortNum; portNum++)
        {
            if (speedArr[portNum] == CPSS_PORT_SPEED_20000_E)
            {
                toDeleteArr[portNum] = GT_TRUE;

                rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_removePort(dev,portNum,ifModeArr[portNum],speedArr[portNum]);
                PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                totalBWMbps -= 20000;
                if (totalBWMbps < unitBWGbps*1000 - bw2Free)
                {
                    return GT_OK;
                }
            }
        }
        for (portNum = 0; portNum < maxPortNum; portNum++)
        {
            if (speedArr[portNum] == CPSS_PORT_SPEED_10000_E)
            {
                toDeleteArr[portNum] = GT_TRUE;

                rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_removePort(dev,portNum,ifModeArr[portNum],speedArr[portNum]);
                PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

                totalBWMbps -= 10000;
                if (totalBWMbps < unitBWGbps*1000 - bw2Free)
                {
                    return GT_OK;
                }
            }
        }
        PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,GT_FAIL);
        return GT_FAIL;
    }
    return GT_OK;
}


GT_STATUS cpssDxChPortIlknChannelSpeedResolutionSetUT_restoreBW
(
    IN GT_U8  dev,
    IN CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS],
    IN CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS],
    IN GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    GT_U32                             maxPortNum;

    rc = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E, /*OUT*/&maxPortNum);
    PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);


    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        if (toDeleteArr[portNum] == GT_TRUE)
        {
            rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_configurePort(dev,portNum,ifModeArr[portNum],speedArr[portNum]);
            PRV_UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }
    return GT_OK;
}

#define __ILKN_UT_DEBUG  0

#if  (__ILKN_UT_DEBUG == 1)
    #ifdef __cplusplus
    extern "C" {
    #endif /* __cplusplus */
        extern GT_STATUS gtBobcat2PortMappingDump
        (
            IN  GT_U8  dev
        );

        GT_STATUS gtBobcat2PortPizzaArbiterIfUnitStateDump
        (
            IN  GT_U8  devNum,
            IN  GT_U32 portGroupId,
            IN  CPSS_DXCH_PA_UNIT_ENT unit
        );

    #ifdef __cplusplus
    }
    #endif /* __cplusplus */
#endif

GT_VOID cpssDxChPortIlknChannelSpeedSetUT(GT_VOID)
{
    GT_STATUS               st   = GT_OK;
    GT_U8                   dev;
    GT_U32                  j;
    GT_BOOL                 isThereIlkn;
    GT_U32                  speed, speedSum, actualSpeed, speedGet;
    GT_U32                  ifBandwidth;
    static                  CPSS_PORTS_BMP_STC      portsBmp;

    static CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\ncpssDxChPortIlknChannelSpeedSetUT() ...");
    #endif

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        #if (1 == __ILKN_UT_DEBUG)
            gtBobcat2PortMappingDump(dev);
            gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_RXDMA_0_E);
        #endif

        st = cpssDxChPortIlknChannelSpeedResolutionSetUT_removeBW(dev,40000,speedArr,ifModeArr,toDeleteArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* cpssOsPrintf("\nTest start dev %d ... \n",(GT_U32)dev); */
        st = IsThereAnyDefinedIlknPort(dev, &isThereIlkn);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (isThereIlkn == GT_FALSE)
        {
            prvUtfSkipTestsSet();
            /* cpssOsPrintf("\nTest exits\n"); */
            return ;
        }
        /* cpssOsPrintf("Done\n"); */


        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, 128);
        st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,
                                        CPSS_PORT_INTERFACE_MODE_ILKN4_E,
                                        CPSS_PORT_SPEED_20000_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortModeSpeedSet(%d)",
                                        128);
        ifBandwidth = 20000;

        for(speed = 40; speed <= ifBandwidth; speed+=600)
        {
            for(j = 0, speedSum = 0; (j < 64) && (speedSum+speed <
                                                            ifBandwidth); j++)
            {
                st = cpssDxChPortIlknChannelSpeedSet(dev, 128+j,
                                                     CPSS_PORT_DIRECTION_RX_E,
                                                     speed, &actualSpeed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortIlknChannelSpeedSet(%d, CPSS_PORT_DIRECTION_RX_E,speed=%d)",
                                             128+j, speed);

                st = cpssDxChPortIlknChannelSpeedGet(dev, 128+j,
                                                     CPSS_PORT_DIRECTION_RX_E,
                                                     &speedGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortIlknChannelSpeedGet(%d, CPSS_PORT_DIRECTION_RX_E)",
                                             128+j);
                UTF_VERIFY_EQUAL3_STRING_MAC(actualSpeed, speedGet,
                                             "channel=%d,CPSS_PORT_DIRECTION_RX_E,actualSpeed=%d,speedGet=%d",
                                             128+j, actualSpeed, speedGet);

                st = cpssDxChPortIlknChannelSpeedSet(dev, 128+j,
                                                     CPSS_PORT_DIRECTION_TX_E,
                                                     speed, &actualSpeed);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortIlknChannelSpeedSet(%d, CPSS_PORT_DIRECTION_TX_E,speed=%d)",
                                             128+j, speed);

                st = cpssDxChPortIlknChannelSpeedGet(dev, 128+j,
                                                     CPSS_PORT_DIRECTION_TX_E,
                                                     &speedGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                             "cpssDxChPortIlknChannelSpeedGet(%d, CPSS_PORT_DIRECTION_TX_E)",
                                             128+j);
                UTF_VERIFY_EQUAL3_STRING_MAC(actualSpeed, speedGet,
                                             "channel=%d,CPSS_PORT_DIRECTION_TX_E,actualSpeed=%d,speedGet=%d",
                                             128+j, actualSpeed, speedGet);

                speedSum+=actualSpeed;
            } /* for(j = 0; */

            /* delete all channels */
            st = deleteAllChannels(dev,128,/*OUT*/&j);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                            "cpssDxChPortIlknChannelSpeedSet(%d, CPSS_PORT_DIRECTION_BOTH_E,speed=0)",
                                            128+j);

            utfPrintKeepAlive();
        } /* for(speed = step, */

        st = cpssDxChPortIlknChannelSpeedResolutionSetUT_restoreBW(dev,speedArr,ifModeArr,toDeleteArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,
                                        CPSS_PORT_INTERFACE_MODE_ILKN4_E,
                                        CPSS_PORT_SPEED_20000_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChPortModeSpeedSet(%d) Power Off",
                                        128);

    } /* while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE)) */
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\nDone\n");
    #endif

}







GT_VOID cpssDxChPortIlknChannelSpeedResolutionSetUT(GT_VOID)
{
    GT_U8              dev = 0;
    GT_STATUS          rc;
    GT_U32             i;
    CPSS_PORTS_BMP_STC portsBmp;
    GT_U32             actualSpeed;
    GT_U32             speed2configure = 0;
    GT_U32             speedResolution;
    GT_U32             estIlknIFBW;
    GT_BOOL            isThereIlkn;

    static CPSS_PORT_SPEED_ENT               speedArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static CPSS_PORT_INTERFACE_MODE_ENT     ifModeArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];
    static GT_BOOL                        toDeleteArr[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E  | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT2_E, CPSS-6059);

    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\ncpssDxChPortIlknChannelSpeedResolutionSetUT() ...");
    #endif


    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        #if (1 == __ILKN_UT_DEBUG)

            /* gtBobcat2PortMappingDump(dev); */
            cpssOsPrintf("\nBefore BW remove : \n");
            gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_RXDMA_0_E);
        #endif
        rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_removeBW(dev,40000,speedArr,ifModeArr,toDeleteArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nAfter BW remove : \n");
            gtBobcat2PortPizzaArbiterIfUnitStateDump(dev,0,CPSS_DXCH_PA_UNIT_RXDMA_0_E);
        #endif

        rc = IsThereAnyDefinedIlknPort(dev, &isThereIlkn);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if (isThereIlkn == GT_FALSE)
        {
            prvUtfSkipTestsSet();
            /* cpssOsPrintf("\nTest exits\n"); */
            return ;
        }

        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nSet IF speed 40G ...");
        #endif
        /* configure intlaken IF */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,128);
        rc = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_TRUE,CPSS_PORT_INTERFACE_MODE_ILKN8_E,CPSS_PORT_SPEED_40000_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif

        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure single channel 41G , shall faul...");
        #endif
        /* configure channel above IF speed */
        rc = cpssDxChPortIlknChannelSpeedSet(dev,128,CPSS_PORT_DIRECTION_BOTH_E, 41000, &actualSpeed);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK,rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif

        /* configure channel below the resolution */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure single channel below resolution 35M , shall faul...");
        #endif
        rc = cpssDxChPortIlknChannelSpeedSet(dev,128,CPSS_PORT_DIRECTION_BOTH_E, 35, &actualSpeed);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK,rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif


        /* configure channel not in quants of resolution */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure single channel not in quants of resolution 41M , shall faul...");
        #endif
        rc = cpssDxChPortIlknChannelSpeedSet(dev,130,CPSS_PORT_DIRECTION_BOTH_E, 41, &actualSpeed);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK,rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif

        /* configure resolution not in quants of resolution */
        /* configure channel not in quants of resolution */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure single channel not in quants of resolution 625 , shall faul...");
        #endif
        speed2configure = 625;
        rc = cpssDxChPortIlknChannelSpeedSet(dev,128,CPSS_PORT_DIRECTION_BOTH_E, speed2configure, &actualSpeed);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK,rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif


        /* configure valid resolution 625Mbps */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure resolution 625M ...");
        #endif
        speedResolution = 625;
        rc = cpssDxChPortIlknChannelSpeedResolutionSet(dev,speedResolution,/*OUT*/&estIlknIFBW);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif

        /* configuure 64 equal channels */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure channels 625M : ");
        #endif

        for (i = 0 ; i < 64 ; i++)
        {
            #if (1 == __ILKN_UT_DEBUG)
                cpssOsPrintf(" %2d",i);
            #endif

            rc = cpssDxChPortIlknChannelSpeedSet(dev,128+i,CPSS_PORT_DIRECTION_BOTH_E, speed2configure, &actualSpeed);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

            rc = cpssDxChPortIlknChannelSpeedGet(dev,128+i,CPSS_PORT_DIRECTION_TX_E, &actualSpeed);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            if (actualSpeed != speed2configure)
            {
                rc = GT_FAIL;
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            }

            /*gtBobcat2PortPizzaArbiterIfStateDump(dev,0);*/

            rc = cpssDxChPortIlknChannelEnableSet(dev,128+i,CPSS_PORT_DIRECTION_BOTH_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nDone");
        #endif

        rc = deleteAllChannels(dev,128,/*OUT*/NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        /*---------------------*/
        /*---------------------*/
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure channels 10M : ");
        #endif

        speedResolution = 10;
        rc = cpssDxChPortIlknChannelSpeedResolutionSet(dev,speedResolution,/*OUT*/&estIlknIFBW);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure channels 1000M : ");
        #endif
        speed2configure = 1000;
        for (i = 0 ; i < 40 ; i++)
        {
            #if (1 == __ILKN_UT_DEBUG)
                cpssOsPrintf(" %2d",i);
            #endif
            rc = cpssDxChPortIlknChannelSpeedSet(dev,128+i,CPSS_PORT_DIRECTION_BOTH_E, speed2configure, &actualSpeed);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nDone");
        #endif

        /* shall get no resources */
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("\nConfigure channels 40 1000M shall get no resources ... ");
        #endif
        rc = cpssDxChPortIlknChannelSpeedSet(dev,128+40,CPSS_PORT_DIRECTION_BOTH_E, speed2configure, &actualSpeed);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf("Done");
        #endif

        /*--------------------*/
        /* delete all chanels */
        /*--------------------*/
        rc = deleteAllChannels(dev,128,/*OUT*/NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = cpssDxChPortIlknChannelSpeedResolutionSetUT_restoreBW(dev,speedArr,ifModeArr,toDeleteArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = cpssDxChPortModeSpeedSet(dev, &portsBmp, GT_FALSE,CPSS_PORT_INTERFACE_MODE_ILKN8_E,CPSS_PORT_SPEED_40000_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\nConfigure non applicable device :");
    #endif
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                     UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                     UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* go over all non active devices */
    speed2configure = 1000;
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        #if (1 == __ILKN_UT_DEBUG)
            cpssOsPrintf(" %2d,",(GT_U32)dev);
        #endif
        rc = cpssDxChPortIlknChannelSpeedResolutionSet(dev,speed2configure,/*OUT*/&estIlknIFBW);
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf(" \nDone");
    #endif


    /* 3. Call function with out of bound value for device id.*/
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\nTry to configure non existing device %d... ",PRV_CPSS_MAX_PP_DEVICES_CNS);
    #endif
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPortIlknChannelSpeedResolutionSet(dev,speed2configure,/*OUT*/&estIlknIFBW);
    UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_OK, rc);
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("Done\n");
    #endif
    #if (1 == __ILKN_UT_DEBUG)
        cpssOsPrintf("\nDone\n");
    #endif

}



/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPortInterlaken suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPortInterlaken)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortInterlakenCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIlknChannelSpeedSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPortIlknChannelSpeedResolutionSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPortInterlaken)

