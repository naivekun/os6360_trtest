#include <Copyright.h>

/**
********************************************************************************
* @file gtIMP.c
*
* @brief API definitions for handling IMP communication and debug.
*
* @version   5
********************************************************************************
*/
/********************************************************************************
* gtIMP.c
*
* DESCRIPTION:
*       API definitions for handling IMP communication and debug.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <gtIMP.h>
#include <gtDrvSwRegs.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <gtUtils.h>
#include <msApiInternal.h>

/*
 * Write to IMP Comm/Debug Register
 */
static GT_STATUS writeImpCommDebugReg
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U16            data
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          tmpData;   /* temporary Data storage */

    GT_U32      retryCount = 0; /* Counter for busy wait loops */

    /* Wait until the device is ready. */
    tmpData = 1;
    while(tmpData == 1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,15,1,&tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        /* the number of iterations does not exceed the limit */
        PRV_CPSS_MAX_SMI_PHY_DRV_NUM_ITERATIONS_CHECK_CNS((retryCount++));
    }

    retVal = prvCpssDrvHwSetAnyReg(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,data);
    if(retVal != GT_OK)
    {
           return retVal;
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvImpRun function
* @endinternal
*
* @brief   This routine is to run IMP with specified start address
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] addr                     - specified start address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note IMP should be stopped before calling this function.
*
*/
GT_STATUS prvCpssDrvImpRun
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U16           addr
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          tmpData;     /* temporary Data storage */

    /* Set address Hi byte */
    tmpData = (1 << 15) | (0x0B << 8) | ((addr >> 8) & 0xff);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
           return retVal;
    }
    /* Set address Lo byte */
    tmpData = (1 << 15) | (0x0A << 8) | ((addr >> 0) & 0xff);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
           return retVal;
    }

    /* Issue IMPOp Examine command - to set the PC */
    tmpData = (1 << 15) | (0x08 << 8) | 0x03;
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
           return retVal;
    }

    /* Issue IMPOp Run command */
    tmpData = (1 << 15) | (0x08 << 8) | 0x01;
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
           return retVal;
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvImpStop function
* @endinternal
*
* @brief   This routine is to stop IMP
*
* @param[in] dev                      - (pointer to) device driver structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpStop
(
    IN  GT_CPSS_QD_DEV   *dev
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          tmpData;     /* temporary Data storage */

    /* Issue IMPOp Stop command */
    tmpData = (1 << 15) | (0x08 << 8) | 0x02;

    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
           return retVal;
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvImpReset function
* @endinternal
*
* @brief   This routine is to reset imp
*
* @param[in] dev                      - (pointer to) device driver structure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If IMP was running when calling this function, it will start running from 0x0000
*
*/
GT_STATUS prvCpssDrvImpReset
(
    IN  GT_CPSS_QD_DEV   *dev
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          tmpData;     /* temporary Data storage */

    /* Issue IMPOp Reset command */
    tmpData = (1 << 15) | (0x08 << 8) | 0x0F;

    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvImpLoadToRAM function
* @endinternal
*
* @brief   This routine is to load data to memory
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] addr                     - start address
* @param[in] data                     - data size
* @param[in] data                     -  to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note IMP should be stopped before calling this function.
*
*/
GT_STATUS prvCpssDrvImpLoadToRAM
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U16           addr,
    IN  GT_U16           dataSize,
    IN  GT_U8*           data
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U16          tmpData;     /* temporary Data storage */
    GT_U16          i;

    /* Set address Hi byte */
    tmpData = (1 << 15) | (0x0B << 8) | ((addr >> 8) & 0xff);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }
    /* Set address Lo byte */
    tmpData = (1 << 15) | (0x0A << 8) | ((addr >> 0) & 0xff);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* Issue IMPOp Examine command */
    tmpData = (1 << 15) | (0x08 << 8) | 0x03;
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    for (i = 0; i < dataSize; i++)
    {
        /* Set data to Deposit */
        tmpData = (1 << 15) | (0x0C << 8) | data[i];
        retVal = writeImpCommDebugReg(dev,tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }

        /* Issue IMPOp Deposit Next command */
        tmpData = (1 << 15) | (0x08 << 8) | 0x06;
        retVal = writeImpCommDebugReg(dev,tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvImpWriteComm function
* @endinternal
*
* @brief   This routine is to write data to IMP Comm Interface
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] data                     -  (one character) to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvImpWriteComm
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8   data
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U32          timeOut;
    GT_U16          tmpData;     /* temporary Data storage */

    /* Write Data */
    tmpData = (1 << 15) | (0x02 << 8) | (data);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* Wait Done */
    tmpData = 0;
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */
    do
    {
        /* Get WrDataBsy bit */
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,4,1,&tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        if((tmpData == 1) &&
            (timeOut-- < 1))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while((tmpData == 1));

    return GT_OK;
}

/**
* @internal prvCpssDrvImpReadComm function
* @endinternal
*
* @brief   This routine is to read data from IMP Comm Interface
*
* @param[in] dev                      - (pointer to) device driver structure
* @param[in] dataPtr                  - data (one character) to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on timeout waiting for data
*
* @note This function is waiting for data for a short time in a busy loop.
*       Use this function only when there is certaintly that data
*       to be read really exists.
*
*/
GT_STATUS prvCpssDrvImpReadComm
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            *dataPtr
)
{
    GT_STATUS       retVal;    /* Functions return value */
    GT_U32          timeOut;
    GT_U16          tmpData;     /* temporary Data storage */

    /* Wait Ready */
    tmpData = 0;
    retVal = prvCpssDrvHwSetAnyReg (dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }
    timeOut = PRV_CPSS_QD_SMI_ACCESS_LOOP; /* initialize the loop count */
    do
    {
        /* Get RdDataRdy bit */
        retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,0,1,&tmpData);
        if(retVal != GT_OK)
        {
            return retVal;
        }
        if((tmpData == 0) &&
            (timeOut-- < 1))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }
    } while(tmpData == 0);

    /* Read Data */
    tmpData = (0x01 << 8);
    retVal = writeImpCommDebugReg(dev,tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }
    retVal = prvCpssDrvHwGetAnyRegField(dev,PRV_CPSS_GLOBAL2_DEV_ADDR,PRV_CPSS_QD_REG_IMP_COMM_DBG,0,8,&tmpData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    *dataPtr = (GT_U8) tmpData;
    return GT_OK;
}



