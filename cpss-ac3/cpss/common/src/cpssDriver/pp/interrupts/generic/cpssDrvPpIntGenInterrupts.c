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
* @file cpssDrvPpIntGenInterrupts.c
*
* @brief This file includes initialization function for the interrupts module,
* and low level interrupt handling (interrupt bits scanning).
*
* @version   3
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>


/*******************************************************************************
* internal definitions
*******************************************************************************/

#ifdef DRV_INTERRUPTS_DBG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

/*******************************************************************************
 * External usage variables
 ******************************************************************************/

/**
* @internal prvCpssDrvInterruptDeviceRemove function
* @endinternal
*
* @brief   This function removes a list of devices after Hot removal operation.
*
* @param[in] devNum                   - device Number to be removed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvInterruptDeviceRemove
(
    IN  GT_U8   devNum
)
{
    GT_STATUS       rc;         /* Function return value.           */
    GT_U32          portGroupId;

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {

        rc =   prvCpssDrvInterruptDisable(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intVecNum);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* Delete the node from the linked-list.    */
        rc = prvCpssDrvInterruptDisconnect(prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intVecNum,
                            prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].isrConnectionId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)


    return GT_OK;
}

/**
* @internal prvCpssDrvInterruptPpSR function
* @endinternal
*
* @brief   This is the Packet Processor interrupt service routine, it scans the
*         interrupts and enqueues each of them to the interrupt queues structure.
* @param[in] cookie                   - (devNum) The PP device number to scan.
*
* @retval 1                        - if interrupts where received from the given device
* @retval 0                        - otherwise.
*/
GT_U8 prvCpssDrvInterruptPpSR
(
    IN void *cookie
)
{
    GT_U8   devNum;         /* The Pp device number to scan.            */
    PRV_CPSS_DRV_INT_CTRL_STC *intCtrlPtr;
    GT_U32   portGroupId;            /* The port group ID number to scan .
                                   relevant to devices with multi-port-groups.
                                   for 'non multi-port-groups' device it will be
                                   PRV_CPSS_DRV_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS */
    PRV_CPSS_DRV_INT_ISR_COOKIE_STC        *isrCookieInfoPtr;/*pointer to format of the cookie of interrupt*/

    isrCookieInfoPtr = cookie;
    devNum = isrCookieInfoPtr->devNum;
    portGroupId = isrCookieInfoPtr->portGroupId;

    intCtrlPtr = &prvCpssDrvPpConfig[devNum]->intCtrl;

    return prvCpssDrvIntEvReqDrvnScan(devNum,portGroupId,
                   intCtrlPtr->portGroupInfo[portGroupId].intNodesPool,
                   intCtrlPtr->portGroupInfo[portGroupId].intMaskShadow,
                   intCtrlPtr->portGroupInfo[portGroupId].intScanRoot);
}

/**
* @internal isMgMapped function
* @endinternal
*
* @brief   check if the MG is mapped on this device
*
* @param[in] devNum                   - The device number
* @param[in] mgNum                    - the managment unit number
*
* @retval GT_TRUE                  - MG is mapped.
* @retval GT_FALSE                 - MG is not mapped
*/

static GT_BOOL isMgMapped
(
    IN GT_U8  devNum,
    IN GT_U32 mgNum
)
{
    switch (mgNum)
    {
        case 1:
            if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg1.start != 0)
            {
                return GT_TRUE;
            }
            break;

        case 2:
            if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg2.start != 0)
            {
                return GT_TRUE;
            }
            break;

        case 3:
            if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg3.start != 0)
            {
                return GT_TRUE;
            }
            break;

        /*default*/

    }
    return GT_FALSE;
}

/**
* @internal prvCpssDrvPpIntUnMappedMgSetSkip function
* @endinternal
*
* @brief   Set skip on registers that cant be acccessed.
*         For example: Not all MGs must be mapped
* @param[in] devNum                   - The device number
* @param[in] maskRegMapArr            - Array of register addresses
* @param[in] maskRegNums              - Number of mask registers
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntUnMappedMgSetSkip
(
    IN  GT_U8  devNum,
    IN  GT_U32 *maskRegMapArr,
    IN  GT_U32 maskRegNums
)
{

    GT_U32 i, mgNum;

    CPSS_NULL_PTR_CHECK_MAC(maskRegMapArr);

    for (i=0; i < maskRegNums; i++)
    {
        mgNum = PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(maskRegMapArr[i]);
        if (mgNum != 0  && isMgMapped(devNum, mgNum) == GT_FALSE )
        {
            maskRegMapArr[i] = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
        }
    }
    return GT_OK;
}

/********************************************************************************
* !!!!!!!!!!!!!!!!!!!!! FOR DEBUG PURPOSES ONLY !!!!!!!!!!!!!!!!!!!!!!!!!!
********************************************************************************/
/*#define DRV_INTERRUPTS_DBG*/
#ifdef DRV_INTERRUPTS_DBG

extern void     lkAddr (unsigned int addr);

void prvCpssDrvSubPrintIntScan(PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *pIntScanNode)
{
    GT_U8 i;

    cpssOsPrintf("isGpp = %d.\n",pIntScanNode->isGpp);
    if(pIntScanNode->isGpp == GT_TRUE)
    {
        /* This is a Gpp interrupt, call the attached function. */
        return;
    }

    /* Read the interrupt cause register.           */
    cpssOsPrintf("pIntScanNode->causeRegAddr = 0x%x.\n",pIntScanNode->causeRegAddr);
    cpssOsPrintf("pIntScanNode->pRegReadFunc = 0x%x:\n",pIntScanNode->pRegReadFunc);
    /*lkAddr(pIntScanNode->pRegReadFunc);*/

    /* Scan local interrupts (non-summary bits).    */
    cpssOsPrintf("pIntScanNode->nonSumBitMask = 0x%x.\n",pIntScanNode->nonSumBitMask);
    cpssOsPrintf("startIdx = %d, endIdx = %d.\n",pIntScanNode->startIdx,
             pIntScanNode->endIdx);

    /* Unmask the received interrupt bits if needed */
    cpssOsPrintf("pIntScanNode->maskRcvIntrEn = 0x%x.\n",pIntScanNode->maskRcvIntrEn);
    cpssOsPrintf("pIntScanNode->maskRegAddr = 0x%x.\n",pIntScanNode->maskRegAddr);
    cpssOsPrintf("pIntScanNode->pRegWriteFunc = 0x%x.\n",pIntScanNode->pRegWriteFunc);
    /*lkAddr(pIntScanNode->pRegWriteFunc);*/

    /* Clear the interrupts (if needed).            */
    cpssOsPrintf("pIntScanNode->rwBitMask = 0x%x.\n",pIntScanNode->rwBitMask);

    cpssOsPrintf("pIntScanNode->subIntrListLen = %d.\n",pIntScanNode->subIntrListLen);
    for(i = 0; i < pIntScanNode->subIntrListLen; i++)
    {
        cpssOsPrintf("----------------------------\n");
        cpssOsPrintf("pIntScanNode->subIntrScan[i]->bitNum = %d.\n",
                 pIntScanNode->subIntrScan[i]->bitNum);
        prvCpssDrvSubPrintIntScan(pIntScanNode->subIntrScan[i]);
    }

    return;
}

void prvCpssDrvPrintIntScan(GT_U8 devNum)
{
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *pIntScanNode;
    GT_U32  portGroupId = 0;

    pIntScanNode = prvCpssDrvPpConfig[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot;

    prvCpssDrvSubPrintIntScan(pIntScanNode);
    return;
}

#endif /*DRV_INTERRUPTS_DBG*/
