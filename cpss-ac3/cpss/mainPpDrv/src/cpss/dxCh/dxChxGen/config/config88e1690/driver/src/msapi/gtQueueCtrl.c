#include <Copyright.h>

/**
********************************************************************************
* @file gtQueueCtrl.c
*
* @brief API definitions for Switch Queue Control, including per-port setting & global setting
*
* @version   /
********************************************************************************
*/
/********************************************************************************
* gtQueueCtrl.c
*
* DESCRIPTION:
*       API definitions for Switch Queue Control, including per-port setting & global setting
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtQueueCtrl.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>


/**
* @internal prvCpssDrvGprtSetQueueCtrl function
* @endinternal
*
* @brief   Set Queue control data to the Queue Control register.
*         The register(pointer) of Queue control are:
*         GT_CPSS_QUEUE_CFG_PORT_SCHEDULE
*         GT_CPSS_QUEUE_CFG_FILTER_Q_EN
*         GT_CPSS_QUEUE_CFG_PORT_SCRATCH_REG,   (acratch0 - 1 inc: 1)
*         GT_CPSS_QUEUE_CFG_H_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_Y_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_BEGIN  (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_END   (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_IN_Q_COUNT      (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END
*         GT_CPSS_QUEUE_CFG_IN_PORT_COUNT
* @param[in] point                    - Pointer to the Queue control register.
* @param[in] data                     - Queue Control  written to the register
* @param[in] point                    to by the point above.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetQueueCtrl
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_U8             point,
    IN  GT_U8             data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        hwPort;         /* the physical port number     */
    GT_U8        phyAddr;
    GT_U16       tmpData;
    int count=0x10;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetQueueCtrl Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    do {
        retVal = prvCpssDrvHwGetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_Q_CONTROL, &tmpData);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if((count--)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (tmpData&0x8000);

    tmpData =  (GT_U16)((1 << 15) | ((point&0x7f) << 8) | data);

    retVal = prvCpssDrvHwSetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_Q_CONTROL, tmpData);
    if(retVal != GT_OK)
    {
         PRV_CPSS_DBG_INFO(("Failed.\n"));
         prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
         return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetQueueCtrl function
* @endinternal
*
* @brief   Get Queue control data from the Queue Control register.
*         The register(pointer) of Queue control are:
*         GT_CPSS_QUEUE_CFG_PORT_SCHEDULE
*         GT_CPSS_QUEUE_CFG_FILTER_Q_EN
*         GT_CPSS_QUEUE_CFG_PORT_SCRATCH_REG,   (acratch0 - 1 inc: 1)
*         GT_CPSS_QUEUE_CFG_H_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_Y_Q_LIMIT_REG     (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_BEGIN  (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_PFC_THRESHOLD_END   (Q0 - Q7 inc: 2)
*         GT_CPSS_QUEUE_CFG_IN_Q_COUNT      (Q0 - Q7 inc: 1)
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_BEGIN
*         GT_CPSS_QUEUE_CFG_FC_THRESHOLD_END
*         GT_CPSS_QUEUE_CFG_IN_PORT_COUNT
* @param[in] point                    - Pointer to the Queue control register.
*
* @param[out] data                     - Queue Control  written to the register
* @param[out] point                    to by the point above.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetQueueCtrl
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_U8             point,
    OUT  GT_U8            *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        hwPort;         /* the physical port number     */
    GT_U8        phyAddr;
    GT_U16       tmpData;
    int count=0x10;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetQueueCtrl Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    do {
        retVal = prvCpssDrvHwGetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_Q_CONTROL, &tmpData);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if((count--)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (tmpData&0x8000);

    tmpData =  (GT_U16)((point&0x7f) << 8);
    retVal = prvCpssDrvHwSetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_Q_CONTROL, tmpData);
    if(retVal != GT_OK)
    {
         PRV_CPSS_DBG_INFO(("Failed.\n"));
         prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
         return retVal;
    }

    retVal = prvCpssDrvHwGetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_Q_CONTROL, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }
    *data = (GT_U8)(tmpData&0xff);

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetPortSched function
* @endinternal
*
* @brief   This routine sets Port Scheduling Mode.
*         When usePortSched is enablied, this mode is used to select the Queue
*         controller's scheduling on the port as follows:
*         GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1 - use 33,25,17,12,6,3,2,1
*         weighted fair scheduling
*         GT_CPSS_PORT_SCHED_STRICT_PRI_1 - use a strict priority scheme
* @param[in] port                     - the logical  number
* @param[in] mode                     - GT_CPSS_PORT_SCHED_MODE enum type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetPortSched
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_CPSS_PORT_SCHED_MODE        mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortSched Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(mode)
    {
        case GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1:
            data = 0;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7:
            data = 1;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7_6:
            data = 2;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5:
            data = 3;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4:
            data = 4;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3:
            data = 5;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3_2:
            data = 6;
            break;
        case GT_CPSS_PORT_SCHED_STRICT_PRI_1:
            data = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvGprtSetQueueCtrl(dev, port,0,(GT_U8)data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPortSched function
* @endinternal
*
* @brief   This routine gets Port Scheduling Mode.
*         When usePortSched is enablied, this mode is used to select the Queue
*         controller's scheduling on the port as follows:
*         GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1 - use 33,25,17,12,6,3,2,1
*         weighted fair scheduling
*         GT_CPSS_PORT_SCHED_STRICT_PRI_1 - use a strict priority scheme
* @param[in] port                     - the logical  number
*
* @param[out] mode                     - GT_CPSS_PORT_SCHED_MODE enum type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortSched
(
    IN  GT_CPSS_QD_DEV            *dev,
    IN  GT_CPSS_LPORT             port,
    OUT GT_CPSS_PORT_SCHED_MODE   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8 datac;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetPortSched Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvGprtGetQueueCtrl(dev,port,0, (GT_U8 *)&datac);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    switch(datac)
    {
        case 0x0:
            *mode = GT_CPSS_PORT_SCHED_WEIGHTED_RRB_1;
            break;
        case 0x1:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7;
            break;
        case 0x2:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7_6;
            break;
        case 0x3:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5;
            break;
        case 0x4:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4;
            break;
        case 0x5:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3;
            break;
        case 0x6:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI7_6_5_4_3_2;
            break;
        case 0x7:
            *mode = GT_CPSS_PORT_SCHED_STRICT_PRI_1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}


