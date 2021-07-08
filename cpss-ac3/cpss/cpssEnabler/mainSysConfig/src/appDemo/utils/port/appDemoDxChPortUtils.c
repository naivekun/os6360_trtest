/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDxChPortUtils.c
*
* @brief App demo DxCh PORT API.
*
* @version   3
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <silicon/alleycat3/mvHwsAlleycat3PortIf.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

extern GT_STATUS prvCpssDxChPortImplementWaSGMII2500
(
    IN  GT_U8                           devNum,
    GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal util_appDemoDxChDeviceCoreClockGet function
* @endinternal
*
* @brief   Get core clock in Herz of device
*
* @param[in] dev                      -   device id of ports
*
* @param[out] coreClockPtr             - (pointer to)core clock in Herz
*
* @retval GT_OK                    -  Get revision of device OK
* @retval GT_BAD_PARAM             -  Invalid device id
* @retval GT_BAD_PTR               -  Null pointer
*/
static GT_STATUS util_appDemoDxChDeviceCoreClockGet
(
    IN GT_U8          dev,
    OUT GT_U32       *coreClockPtr
)
{
    /* device must be active */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(dev);

    /* check for null pointer */
    CPSS_NULL_PTR_CHECK_MAC(coreClockPtr);

    switch (PRV_CPSS_PP_MAC(dev)->coreClock)
    {
        case 222: *coreClockPtr = 222222222; break;
        case 167: *coreClockPtr = 166666667; break;
        case 362: *coreClockPtr = 362500000; break;
        default: *coreClockPtr = PRV_CPSS_PP_MAC(dev)->coreClock * 1000000;
            break;
    }
    return GT_OK;
}


/**
* @internal util_appDemoDxChTimeStampsRateGet function
* @endinternal
*
* @brief   Get rate of events by VNT Time Stamps.
*
* @param[in] dev                      - device number
* @param[in] startTimeStamp           - start counting time stamp
* @param[in] endTimeStamp             - end counting time stamp
* @param[in] eventCount               - events count between end and start time stamps
*                                       events rate in events per second
*/
static GT_U32 util_appDemoDxChTimeStampsRateGet
(
    IN  GT_U8          dev,
    IN  GT_U32         startTimeStamp,
    IN  GT_U32         endTimeStamp,
    IN  GT_U32         eventCount
)
{
    GT_STATUS rc;           /* return code*/
    GT_U32    coreClock;    /* device core clock in HZ */
    double    diff;         /* difference between end and start time stamps */
    double    rate;         /* rate of events */
    GT_U32    rateOut;      /* integer value of rate */

    /* get core closk in HZ */
    rc = util_appDemoDxChDeviceCoreClockGet(dev,&coreClock);
    if (rc != GT_OK)
    {
        return 0xFFFFFFFF;
    }

    /* calculate difference between end and start */
    if (startTimeStamp > endTimeStamp)
    {
       /* take care of wraparound of end time */
       diff = 0xFFFFFFFFU;
       diff = diff + 1 + endTimeStamp - startTimeStamp;
    }
    else
       diff = endTimeStamp - startTimeStamp;

   /* time stamp is in device core clocks.
     event rate is number of events divided by time between them.
     the time is core clock period multiplied on number of core clocks.
     rate = eventCount / time, where time = diff * (1/coreClock) */
   rate = eventCount;
   rate = (rate * coreClock) / diff;

   /* convert float point value to unsigned integer */
   rateOut = (GT_U32)rate;

   return rateOut;
}


/**
* @internal util_appDemoDxChPortFWSRatesTableGet function
* @endinternal
*
* @brief   Calculates and stores RX/TX rate for all ports
*
* @param[out] rateDbPtr                - (pointer to) DB of rates
*
* @retval GT_OK                    - on success
*/
GT_STATUS util_appDemoDxChPortFWSRatesTableGet
(
    IN  GT_U8 devNum,
    IN  GT_U32 arrSize,
    OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
)
{
    GT_U64      cntrValue;  /* value of counters */
    GT_PHYSICAL_PORT_NUM       portNum;              /* port number */
    GT_STATUS   rc;                   /* return code */
    GT_U32      startTimeStampRxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for RX */
    GT_U32      startTimeStampTxArr[CPSS_MAX_PORTS_NUM_CNS]; /* VNT time stamp start for TX */
    GT_U32      endTimeStamp;   /* VNT time stamp start */
    GT_U32      rxRate;         /* VNT time stamp based RX rate */
    GT_U32      txRate;         /* VNT time stamp based TX rate */
    GT_U32      macPortNum;     /* MAC port number */

    cpssOsMemSet(rateDbPtr, 0, arrSize * sizeof(CMD_PORT_FWS_RATE_STC));
    cpssOsMemSet(startTimeStampRxArr, 0, sizeof(startTimeStampRxArr));
    cpssOsMemSet(startTimeStampTxArr, 0, sizeof(startTimeStampTxArr));

    portNum = 0;

    while (1)
    {
        if (portNum >= arrSize)
        {
            break;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macPortNum);
            if(rc != GT_OK)
            {
                portNum++;
                continue;
            }
        }
        else
        {
            if(PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) != GT_TRUE)
            {
                portNum++;
                continue;
            }
        }

        rc = cpssDxChMacCounterGet(devNum, portNum, CPSS_GOOD_UC_PKTS_RCV_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChMacCounterGet(devNum, portNum, CPSS_GOOD_UC_PKTS_SENT_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &startTimeStampTxArr[portNum]);
        if (rc != GT_OK)
        {
            return rc;
        }

        portNum++;
    }

    /* sleep */
    cpssOsTimerWkAfter(1000);

    portNum = 0;

    /* For device go over all available physical ports. */
    while (1)
    {
        if (portNum >= arrSize)
        {
            break;
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macPortNum);
            if(rc != GT_OK)
            {
                portNum++;
                continue;
            }
        }
        else
        {
            if(PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum) != GT_TRUE)
            {
                portNum++;
                continue;
            }
        }


        rc = cpssDxChMacCounterGet(devNum, portNum, CPSS_GOOD_UC_PKTS_RCV_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        rxRate = util_appDemoDxChTimeStampsRateGet(devNum, startTimeStampRxArr[portNum], endTimeStamp, cntrValue.l[0]);

        rc = cpssDxChMacCounterGet(devNum, portNum, CPSS_GOOD_UC_PKTS_SENT_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChVntLastReadTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        txRate = util_appDemoDxChTimeStampsRateGet(devNum, startTimeStampTxArr[portNum], endTimeStamp, cntrValue.l[0]);

        rateDbPtr[portNum].rxPacketRate = rxRate;
        rateDbPtr[portNum].txPacketRate = txRate;

        portNum++;
    }

    return GT_OK;
}


/**
* @internal appDemoDxChPortPacketRateGet function
* @endinternal
*
* @brief   This function calculates RX/TX packet rate on specific port.
*
* @param[in] rxPacketRate             - (pointer to) RX packet rate
* @param[in] txPacketRate             - (pointer to) TX packet rate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS appDemoDxChPortPacketRateGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *rxPacketRate,
    OUT GT_U32                  *txPacketRate
)
{
    GT_STATUS rc;
    static CMD_PORT_FWS_RATE_STC rateDb[CPSS_MAX_PORTS_NUM_CNS];

    CPSS_NULL_PTR_CHECK_MAC(rxPacketRate);
    CPSS_NULL_PTR_CHECK_MAC(txPacketRate);

    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    rc = util_appDemoDxChPortFWSRatesTableGet(devNum, CPSS_MAX_PORTS_NUM_CNS, rateDb);
    if(GT_OK != rc)
    {
        return rc;
    }

    *rxPacketRate = rateDb[portNum].rxPacketRate;
    *txPacketRate = rateDb[portNum].txPacketRate;

    return GT_OK;
}

/**
* @internal appDemoDxChPortMicroInitConfigSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; xCat3; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Falcon.
*
* @param[in] devNum                   - physical device number
*                                      portsBmp  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To be used with MicroInit tool.
*
*/
GT_STATUS appDemoDxChPortMicroInitConfigSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc = GT_OK;

    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_PORT_STANDARD    portMode;
    PRV_CPSS_DXCH_PORT_STATE_STC  originalPortStateStc;
    GT_U32                  firstInQuadruplet;  /* mac number of first port in quadruplet */
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    GT_U32                  curLanesList[96];


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E |CPSS_XCAT2_E|
            CPSS_LION_E);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &originalPortStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }
        originalPortStateStc.portEnableState = GT_TRUE;
        if (CPSS_PP_FAMILY_DXCH_XCAT3_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            /* Set FCA interface width */
            rc = prvCpssDxChPortBcat2FcaBusWidthSet(devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortBcat2FcaBusWidthSet, portNum = %d\n", portNum);
            }
            /* Set PTP interface width */
            rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortBcat2PtpInterfaceWidthSelect, portNum = %d\n", portNum);
            }

            /* set the default Tx and CTLE(Rx) parameters on port - done in
                mvHwsAvagoSerdesArrayPowerCtrlImpl/mvHwsAvagoSerdesPowerCtrlImpl */

            /* set P Sync Bypass */
            CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, 0, portMacMap, portMode));

            /* Configures the DP/Core Clock Selector on port */
            CHECK_STATUS(mvHwsClockSelectorConfig(devNum, 0, portMacMap, portMode));

            /* Configure MAC/PCS */
            CHECK_STATUS(mvHwsPortModeCfg(devNum, 0, portMacMap, portMode));

            /* Un-Reset the port */
            CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, portMacMap, portMode));

        }
        else
        {
            if(CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode)
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                firstInQuadruplet = portNum & 0xFFFFFFFC;
            }
            else
            {
                firstInQuadruplet = portNum;
            }
            /* rebuild active lanes list according to current configuration (redundancy) */
            CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, 0, firstInQuadruplet, portMode, curLanesList));

            /* power up the serdes */
            CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, 0, firstInQuadruplet, portMode, refClock, PRIMARY_LINE_SRC, &(curLanesList[0])));

            /* set the default Tx parameters on port */
            CHECK_STATUS(mvHwsPortTxDefaultsSet(devNum, 0, firstInQuadruplet, portMode));

            /* set P Sync Bypass */
            CHECK_STATUS(mvHwsAlleycat3PortPsyncBypassCfg(devNum, 0, firstInQuadruplet, portMode));

            /* Configures the DP/Core Clock Selector on port */
            CHECK_STATUS(mvHwsClockSelectorConfig(devNum, 0, firstInQuadruplet, portMode));

            /* Configure MAC/PCS */
            CHECK_STATUS(mvHwsPortModeCfg(devNum, 0, firstInQuadruplet, portMode));

            /* Un-Reset the port */
            CHECK_STATUS(mvHwsPortStartCfg(devNum, 0, firstInQuadruplet, portMode));
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        if (CPSS_PP_FAMILY_DXCH_XCAT3_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
            rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum,ifMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChHwRegAddrPortMacUpdate, portNum = %d\n", portNum);
            }
        }
        else
        {
            /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
            rc = prvCpssDxChHwXcat3RegAddrPortMacUpdate(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChHwXcat3RegAddrPortMacUpdate, portNum = %d\n", portNum);
            }
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        rc = cpssDxChPortMruSet(devNum, portNum, 1522);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in cpssDxChPortMruSet, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortStateRestore(devNum, portNum, &originalPortStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"appDemoDxChPortMicroInitConfigSet: error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
        }
    }

    return rc;
}


