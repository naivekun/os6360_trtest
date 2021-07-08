#include <Copyright.h>

/**
********************************************************************************
* @file gtPortRmon.c
*
* @brief API definitions for RMON counters
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtPortRmon.c
*
* DESCRIPTION:
*       API definitions for RMON counters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtPortRmon.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>
#include <msApiInternal.h>

/* Definitions for MIB Counter */
/*
#define GT_STATS_NO_OP               0x0
#define GT_STATS_FLUSH_ALL           0x1
#define GT_STATS_FLUSH_PORT          0x2
#define GT_STATS_READ_COUNTER        0x4
#define GT_STATS_CAPTURE_PORT        0x5
*/

/*
 *    Type definition for MIB counter operation
*/
typedef enum
{
    STATS_FLUSH_ALL    = 1,         /* Flush all counters for all ports */
    STATS_FLUSH_PORT   = 2,         /* Flush all counters for a port */
    STATS_READ_COUNTER = 4,         /* Read a specific counter from a port */
    STATS_READ_ALL     = 5,         /* Read all counters from a port */
} GT_STATS_OPERATION;

/****************************************************************************/
/* STATS operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS statsOperationPerform
(
    IN   GT_CPSS_QD_DEV          *dev,
    IN   GT_STATS_OPERATION      statsOp,
    IN   GT_U8                   port,
    IN   GT_CPSS_STATS_COUNTERS  counter,
    OUT  GT_VOID                 *statsData
);

static GT_STATUS statsReadCounter
(
    IN   GT_CPSS_QD_DEV  *dev,
    IN   GT_U16          port,
    IN   GT_U32          counter,
    OUT  GT_U32          *statsData
);

/**
* @internal prvCpssDrvGstatsFlushAll function
* @endinternal
*
* @brief   Flush All RMON counters for all ports.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGstatsFlushAll
(
    IN  GT_CPSS_QD_DEV    *dev
)
{
    GT_STATUS  retVal;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsFlushAll Called.\n"));

    retVal = statsOperationPerform(dev,STATS_FLUSH_ALL,0,0,NULL);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}


/**
* @internal prvCpssDrvGstatsFlushPort function
* @endinternal
*
* @brief   Flush All RMON counters for a given port.
*
* @param[in] port                     - the logical  number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsFlushPort
(
    IN  GT_CPSS_QD_DEV     *dev,
    IN  GT_CPSS_LPORT      port
)
{
    GT_STATUS    retVal;
    GT_U8        hwPort;         /* physical port number         */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsFlushPort Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = statsOperationPerform(dev,STATS_FLUSH_PORT,hwPort,0,NULL);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGstatsGetPortCounter function
* @endinternal
*
* @brief   This routine gets a specific counter of the given port
*
* @param[in] port                     - the logical  number.
* @param[in] counter                  - the  which will be read
*
* @param[out] statsData                - points to 32bit data storage for the MIB counter
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetPortCounter
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_STATS_COUNTERS  counter,
    OUT GT_U32                  *statsData
)
{
    GT_STATUS    retVal;
    GT_U8        hwPort;         /* physical port number         */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsGetPortCounter Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = statsOperationPerform(dev,STATS_READ_COUNTER,hwPort,counter,(GT_VOID*)statsData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}


/**
* @internal prvCpssDrvGstatsGetPortAllCounters function
* @endinternal
*
* @brief   This routine gets all RMON counters of the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] statsCounterSet          - points to GT_CPSS_STATS_COUNTER_SET for the MIB counters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetPortAllCounters
(
    IN  GT_CPSS_QD_DEV             *dev,
    IN  GT_CPSS_LPORT              port,
    OUT GT_CPSS_STATS_COUNTER_SET  *statsCounterSet
)
{
    GT_STATUS    retVal;
    GT_U8        hwPort;         /* physical port number         */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsGetPortAllCounters Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = statsOperationPerform(dev,STATS_READ_ALL,hwPort,0,(GT_VOID*)statsCounterSet);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;

}

/**
* @internal prvCpssDrvGstatsGetHistogramMode function
* @endinternal
*
* @brief   This routine gets the Histogram Counters Mode.
*
* @param[out] mode                     - Histogram Mode (GT_CPSS_COUNT_RX_ONLY, GT_CPSS_COUNT_TX_ONLY,
*                                      and GT_CPSS_COUNT_RX_TX)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsGetHistogramMode
(
    IN  GT_CPSS_QD_DEV          *dev,
    OUT GT_CPSS_HISTOGRAM_MODE  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsGetHistogramMode Called.\n"));

    if(mode == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Mode).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the Histogram mode bit.                */
    retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_GLOBAL_CONTROL2,6,2,&data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Software definition starts from 0 ~ 3, while hardware supports the values from 1 to 3 */
    switch(data)
    {
        case 0x1:
            *mode = GT_CPSS_COUNT_RX_ONLY;
            break;
        case 0x2:
            *mode = GT_CPSS_COUNT_TX_ONLY;
            break;
        case 0x3:
            *mode = GT_CPSS_COUNT_RX_TX;
            break;
        default:
            PRV_CPSS_DBG_INFO(("Failed (Bad Mode).\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGstatsSetHistogramMode function
* @endinternal
*
* @brief   This routine sets the Histogram Counters Mode.
*
* @param[in] mode                     - Histogram Mode (GT_CPSS_COUNT_RX_ONLY, GT_CPSS_COUNT_TX_ONLY,
*                                      and GT_CPSS_COUNT_RX_TX)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGstatsSetHistogramMode
(
    IN GT_CPSS_QD_DEV   *dev,
    IN GT_CPSS_HISTOGRAM_MODE        mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstatsSetHistogramMode Called.\n"));

    switch (mode)
    {
        case GT_CPSS_COUNT_RX_ONLY:
            data = 1;
            break;
        case GT_CPSS_COUNT_TX_ONLY:
            data = 2;
            break;
        case GT_CPSS_COUNT_RX_TX:
            data = 3;
            break;
        default:
            PRV_CPSS_DBG_INFO(("Failed (Bad Mode).\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*data = (GT_U16)mode;*/

    /* Set the Histogram mode bit.                */
    retVal = prvCpssDrvHwSetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_GLOBAL_CONTROL2,6,2,data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/**
* @internal statsOperationPerform function
* @endinternal
*
* @brief   This function is used by all stats control functions, and is responsible
*         to write the required operation into the stats registers.
* @param[in] statsOp                  - The stats operation bits to be written into the stats
*                                      operation register.
* @param[in] port                     -  number
* @param[in] counter                  -  to be read if it's read operation
*
* @param[out] statsData                - points to the data storage where the MIB counter will be saved.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

static GT_STATUS statsOperationPerform
(
    IN   GT_CPSS_QD_DEV          *dev,
    IN   GT_STATS_OPERATION      statsOp,
    IN   GT_U8                   port,
    IN   GT_CPSS_STATS_COUNTERS  counter,
    OUT  GT_VOID                 *statsData
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
    GT_U32  tmpCounter;
    GT_U32  startCounter;
    GT_U32  lastCounter;
    GT_U16  portNum;
    GT_U8   bank;
    GT_U32  retryCount = 0;             /* Counter for busy wait loops */


    prvCpssDrvGtSemTake(dev,dev->statsRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    portNum = (port + 1) << 5;

    /* Wait until the stats in ready. */
    data = (1 << 15);
    while((data & (1 << 15)) != 0)
    {
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,&data);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
            return retVal;
        }
        /* check that the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    data &= 0xfff;
    /* Set the STAT Operation register */
    switch (statsOp)
    {
        case STATS_FLUSH_ALL:
            data |= (1 << 15) | (STATS_FLUSH_ALL << 12);
            retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,data);

            prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
            return retVal;

        case STATS_FLUSH_PORT:
            data &= 0xc1f;
            data |= (1 << 15) | (STATS_FLUSH_PORT << 12) | portNum ;
            retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,data);

            prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
            return retVal;

        case STATS_READ_COUNTER:
            retVal = statsReadCounter(dev, portNum, (GT_U32)counter, (GT_U32*)statsData);
            if(retVal != GT_OK)
            {
                prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
                return retVal;
            }
            break;

        case STATS_READ_ALL:
            {
                int bankSize = 2;

                data &= 0xc1f;
                data |= (1 << 15) | (STATS_READ_ALL << 12) | (portNum & 0x03e0);
                retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,data);

                /* Wait until the stats in ready. */
                retryCount = 0;
                do
                {
                    retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,&data);
                    if(retVal != GT_OK)
                    {
                        prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
                        return retVal;
                    }
                    /* check that the number of iterations does not exceed the limit */
                    PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
                } while((data & (1<<15)) != 0);

                for(bank=0; bank<bankSize; bank++)
                {
                    lastCounter = (bank==0)?(GT_U32)CPSS_STATS_Late : (GT_U32)CPSS_STATS_OutMGMT;
                    startCounter = (bank==0)?(GT_U32)CPSS_STATS_InGoodOctetsLo : (GT_U32)CPSS_STATS_InDiscards;

                    if(bank==1)
                    {
                        statsData = (GT_U32 *)statsData + CPSS_STATS_Late +1;
                    }

                    for(tmpCounter=startCounter; tmpCounter<=lastCounter; tmpCounter++)
                    {
                        if((tmpCounter == CPSS_STATS_Single)         ||
                           (tmpCounter  > CPSS_STATS_InDiscards))
                        {
                            /* the CPSS not use it ... save 1/2 of the time ! */
                            *((GT_U32*)statsData+tmpCounter-startCounter) = 0;
                            continue;
                        }

                        retVal = statsReadCounter(dev, 0, tmpCounter,((GT_U32*)statsData+tmpCounter-startCounter));
                        if(retVal != GT_OK)
                        {
                            prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
                            return retVal;
                        }
                    }
                }
            }
            break;

        default:

            prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemGive(dev,dev->statsRegsSem);
    return GT_OK;
}

#ifdef ASIC_SIMULATION
static GT_U32 directAccessMode =  0;
void debug_set_directAccessMode(GT_U32  newValue)
{
    if(directAccessMode == newValue)
    {
        return;
    }

    cpssOsPrintf("debug_set_directAccessMode : oldMode[%d] , newValue[%d]",
        directAccessMode,newValue);

    directAccessMode = newValue;
}
void smemSohoDirectAccess(
    IN  GT_U8    deviceNumber,
    IN  GT_U32   DevAddr,
    IN  GT_U32   RegAddr,
    IN  GT_U32 * memPtr,
    IN  GT_BOOL   doRead/*GT_TRUE - read , GT_FALSE - write*/
);
#endif
/**
* @internal statsReadCounter function
* @endinternal
*
* @brief   This function is used to read a captured counter.
*
* @param[in] port                     -  number
* @param[in] counter                  -  to be read if it's read operation
*
* @param[out] statsData                - points to the data storage where the MIB counter will be saved.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
*
* @note If Semaphore is used, Semaphore should be acquired before this function call.
*
*/
static GT_STATUS statsReadCounter
(
    IN   GT_CPSS_QD_DEV   *dev,
    IN   GT_U16           port,
    IN   GT_U32           counter,
    OUT  GT_U32           *statsData
)
{
    GT_STATUS   retVal;         /* Functions return value.            */
    GT_U16      data;           /* Data to be set into the  register. */
    GT_U16      counter3_2;     /* Counter Register Bytes 3 & 2       */
    GT_U16      counter1_0;     /* Counter Register Bytes 1 & 0       */
    GT_U32      retryCount = 0; /* Counter for busy wait loops */

#ifdef ASIC_SIMULATION
    if(directAccessMode)
    {
        GT_U32  _32_data;
        GT_U32  _32_counter3_2;
        GT_U32  _32_counter1_0;
        GT_U32  DevAddr,RegAddr;
        /********trigger operation*****/
        _32_data = (GT_U16)((1 << 15) | (STATS_READ_COUNTER << 12) | port | (counter&0x1f) );
        if (counter & GT_CPSS_TYPE_BANK)
        {
            _32_data |= (1<<10);
        }
        DevAddr = PRV_CPSS_GLOBAL1_DEV_ADDR;
        RegAddr = PRV_CPSS_QD_REG_STATS_OPERATION;
        smemSohoDirectAccess(dev->devNum,DevAddr,RegAddr,&_32_data,GT_FALSE/*write*/);
        /*******************/

        /* read the counter (high) */
        RegAddr = PRV_CPSS_QD_REG_STATS_COUNTER3_2;
        smemSohoDirectAccess(dev->devNum,DevAddr,RegAddr,&_32_counter3_2,GT_TRUE/*read*/);
        /* read the counter (low) */
        RegAddr = PRV_CPSS_QD_REG_STATS_COUNTER1_0;
        smemSohoDirectAccess(dev->devNum,DevAddr,RegAddr,&_32_counter1_0,GT_TRUE/*read*/);

        *statsData = (_32_counter3_2 << 16) | _32_counter1_0;
        return GT_OK;
    }
#endif


    data = (GT_U16)((1 << 15) | (STATS_READ_COUNTER << 12) | port | (counter&0x1f) );
    if (counter & GT_CPSS_TYPE_BANK)
    {
        data |= (1<<10);
    }

    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,data);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    data = 1;
    while(data == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        /* check that the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_COUNTER3_2,&counter3_2);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL1_DEV_ADDR,PRV_CPSS_QD_REG_STATS_COUNTER1_0,&counter1_0);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    *statsData = (counter3_2 << 16) | counter1_0;

    return GT_OK;

}


