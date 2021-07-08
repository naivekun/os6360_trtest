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
* @file appDemoPxPortUtils.c
*
* @brief App demo Px PORT API.
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/private/prvCpssPxInfoEnhanced.h>
#include <cpss/px/port/cpssPxPortStat.h>

/* include the external services */
#include <cmdShell/common/cmdExtServices.h>

/**
* @internal util_appDemoPxDeviceCoreClockGet function
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
static GT_STATUS util_appDemoPxDeviceCoreClockGet
(
    IN  GT_SW_DEV_NUM  dev,
    OUT GT_U32         *coreClockPtr
)
{
    /* device must be active */
    PRV_CPSS_PX_DEV_CHECK_MAC(dev);

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
* @internal util_appDemoPxTimeStampsRateGet function
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
static GT_U32 util_appDemoPxTimeStampsRateGet
(
    IN  GT_SW_DEV_NUM   dev,
    IN  GT_U32          startTimeStamp,
    IN  GT_U32          endTimeStamp,
    IN  GT_U32          eventCount
)
{
    GT_STATUS rc;           /* return code*/
    GT_U32    coreClock;    /* device core clock in HZ */
    double    diff;         /* difference between end and start time stamps */
    double    rate;         /* rate of events */
    GT_U32    rateOut;      /* integer value of rate */

    /* get core closk in HZ */
    rc = util_appDemoPxDeviceCoreClockGet(dev,&coreClock);
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
* @internal util_appDemoPxPortFWSRatesTableGet function
* @endinternal
*
* @brief   Calculates and stores RX/TX rate for all ports
*
* @param[out] rateDbPtr                - (pointer to) DB of rates
*
* @retval GT_OK                    - on success
*/
GT_STATUS util_appDemoPxPortFWSRatesTableGet
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

        rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(
            devNum, portNum, PRV_CPSS_PX_PORT_TYPE_MAC_E, &macPortNum);
        if(rc != GT_OK)
        {
            portNum++;
            continue;
        }

        rc = cpssPxPortMacCounterGet(
            devNum, portNum, CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxCfgLastRegAccessTimeStampGet(devNum, &startTimeStampRxArr[portNum]);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxPortMacCounterGet(
            devNum, portNum, CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxCfgLastRegAccessTimeStampGet(devNum, &startTimeStampTxArr[portNum]);
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

        rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(
            devNum, portNum, PRV_CPSS_PX_PORT_TYPE_MAC_E, &macPortNum);
        if(rc != GT_OK)
        {
            portNum++;
            continue;
        }

        rc = cpssPxPortMacCounterGet(
            devNum, portNum, CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxCfgLastRegAccessTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        rxRate = util_appDemoPxTimeStampsRateGet(
            devNum, startTimeStampRxArr[portNum], endTimeStamp, cntrValue.l[0]);

        rc = cpssPxPortMacCounterGet(
            devNum, portNum, CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E, &cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssPxCfgLastRegAccessTimeStampGet(devNum, &endTimeStamp);
        if (rc != GT_OK)
        {
            return rc;
        }

        txRate = util_appDemoPxTimeStampsRateGet(
            devNum, startTimeStampTxArr[portNum], endTimeStamp, cntrValue.l[0]);

        rateDbPtr[portNum].rxPacketRate = rxRate;
        rateDbPtr[portNum].txPacketRate = txRate;

        portNum++;
    }

    return GT_OK;
}

/**
* @internal appDemoPxPortPacketRateGet function
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
GT_STATUS appDemoPxPortPacketRateGet
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

    rc = util_appDemoPxPortFWSRatesTableGet(devNum, CPSS_MAX_PORTS_NUM_CNS, rateDb);
    if(GT_OK != rc)
    {
        return rc;
    }

    *rxPacketRate = rateDb[portNum].rxPacketRate;
    *txPacketRate = rateDb[portNum].txPacketRate;

    return GT_OK;
}


