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
* @file cpssDrvPpIntGenInit.c
*
* @brief Includes driver level interrupts initialization code.
*
* @version   6
********************************************************************************
*/
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>

/*******************************************************************************
 * External usage variables
 ******************************************************************************/

/**
* @internal prvCpssDrvInterruptsInit function
* @endinternal
*
* @brief   Initialize the interrupts mechanism for a given device.
*
* @param[in] devNum                   - The device number to initialize the interrupts
*                                      mechanism for.
* @param[in] portGroupId              - The port group Id , supports CPSS_PORT_GROUP_UNAWARE_MODE_CNS.
* @param[in] intVecNum                - The interrupt vector number this device is connected
*                                      to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvInterruptsInit
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask
)
{
   GT_STATUS rc = GT_OK;

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    if(intVecNum == CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS)
    {
        /* no more needed for this device */
        return GT_OK;
    }

    PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC(devNum,portGroupId);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            rc = prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpInterruptInit(devNum,portGroupId,
                                intVecNum,intMask);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    }
    else
    {
        /* Call appropriate implementation by virtual function */
        rc = prvCpssDrvPpObjConfig[devNum]->genPtr->drvHwPpInterruptInit(devNum,portGroupId,
                            intVecNum,intMask);
    }

    return rc;
}

/**
* @internal prvCpssDrvInterruptsAfterPhase2 function
* @endinternal
*
* @brief   Interrupts initialization after phase2 was performed.
*
* @param[in] oldDevNum                - The device's old device number.
* @param[in] newDevNum                - The device's new device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*
* @note This initialization includes:
*       1. Passing all device numbers variables to the new device's devNuum.
*       2. Updating the mask registers of the interrupt mask registers that
*       cannot be written before start-init.
*
*/
GT_STATUS prvCpssDrvInterruptsAfterPhase2
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intNodesPoolReqDrvn;
    GT_STATUS   retVal;         /* Function's return value. */
    GT_U32      i;              /* Loops index.             */
    GT_U32      portGroupId;/* port group Id */

    /* fix warning */
    oldDevNum = oldDevNum;

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(newDevNum);

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(newDevNum,portGroupId)
    {
        /* Update all interrupt's related data structure, to    */
        /* include newDevNum instead of oldDevNum.              */
        intNodesPoolReqDrvn = prvCpssDrvPpConfig[newDevNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

        for(i = 0; i < prvCpssDrvPpConfig[newDevNum]->intCtrl.numOfIntBits; i++)
        {
            intNodesPoolReqDrvn[i].devNum = newDevNum;
        }

        /* Initialize the interrupt mask register's that can    */
        /* be modified only after phase2.                       */
        retVal = prvCpssDrvPpObjConfig[newDevNum]->genPtr->drvHwPpInitInMasks(newDevNum,portGroupId,
                                                                  GT_FALSE);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(newDevNum,portGroupId)

    return GT_OK;
}

/**
* @internal prvCpssDrvInterruptsRenumber function
* @endinternal
*
* @brief   renumber device number and update the Interrupts DB.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
* @param[in] oldDevNum                - The device's old device number.
* @param[in] newDevNum                - The device's new device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssDrvInterruptsRenumber
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intNodesPoolReqDrvn;
    GT_U32      ii;              /* Loops index.             */
    GT_U32      portGroupId;/* port group Id */

    /* fix warning */
    oldDevNum = oldDevNum;

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(newDevNum,portGroupId)
    {
        /* Update all interrupt's related data structure, to    */
        /* include newDevNum instead of oldDevNum.              */
        intNodesPoolReqDrvn = prvCpssDrvPpConfig[newDevNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

        prvCpssDrvPpConfig[newDevNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo.devNum = newDevNum;

        for(ii = 0; ii < prvCpssDrvPpConfig[newDevNum]->intCtrl.numOfIntBits; ii++)
        {
            intNodesPoolReqDrvn[ii].devNum = newDevNum;
        }
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(newDevNum,portGroupId)

    return GT_OK;
}
