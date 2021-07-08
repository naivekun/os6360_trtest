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
* @file cpssHwInit.c
*
* @brief Includes CPSS level basic HW initialization functions.
*
* @version   43
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChCpssHwInitLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

#define BASE_ADDR_NOT_USED_CNS 0xffffffff

/* memory compare for specific field in DB */
#define MEM_COMP_FROM_SAVED_DB_TO_DEVICE_PARAM_MAC(fieldName)                                   \
    if(cpssOsMemCmp(&sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fieldName,          \
                    &phase2InfoPtr->fieldName,                                                  \
                    sizeof(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fieldName)))  \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "field[%s] is not with same values that given in previous initialization",\
            #fieldName);                                                                        \
    }

/**
* @internal prvCpssPpConfigDevDbInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         1. first call for device or device did HW reset :
*         function set the info about the device regarding DMA allocations
*         given by the application.
*         2. else the function check that the info given now match the info given
*         of first time after the HW reset.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] phase2InfoPtr            - (pointer to) phase2 init info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_STATE             - the new info it not the same info that was
*                                       set for this device (according to the bus
*                                       base address) before the 'shutdown'
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbInfoSet
(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_PP_PHASE2_INIT_INFO_STC *phase2InfoPtr
)
{
    /* validate the pointers */
    CPSS_NULL_PTR_CHECK_MAC(phase2InfoPtr);

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if(! PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* all current info relate to DMA so currently not relevant to non
        PCI/PEX devices */
        return GT_OK;
    }

    if(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid == GT_FALSE ||
       sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset == GT_TRUE)
    {
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.auqCfg         = (phase2InfoPtr->auqCfg);
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqUseSeparate = phase2InfoPtr->fuqUseSeparate;
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqCfg         = (phase2InfoPtr->fuqCfg);

        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid = GT_TRUE;

        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.netIfCfg = (phase2InfoPtr->netIfCfg);
/*        reInitializationDbPtr[devNum].deviceDidHwReset = GT_FALSE; -->
         the setting to GT_FALSE need to come from prvCpssPpConfigDevDbHwResetSet */

        /* we are done with saving the info (for possible use after shutdown) */
        return GT_OK;
    }

    /* we need to check that the DMA sizes not changed from the previous
       initialization , because we can't modify HW values */
    MEM_COMP_FROM_SAVED_DB_TO_DEVICE_PARAM_MAC(auqCfg);

    if(phase2InfoPtr->fuqUseSeparate == GT_TRUE)
    {
        if(GT_TRUE == sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqUseSeparate)
        {
            MEM_COMP_FROM_SAVED_DB_TO_DEVICE_PARAM_MAC(fuqCfg);
        }
        else
        {
            /* we now use the FU queue even we not used it before , it is valid
               state */
            /*so we need to update the initial DB now */
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqUseSeparate = GT_TRUE;
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqCfg         = (phase2InfoPtr->fuqCfg);
        }
    }
    else
    {
        /* we not care that there was DMA from FU queue and there non now */
        /* because the PP will not access this DMA any more */
        /* BUT we stay with the initial state of
           reInitializationDbPtr[devNum].dmaInfo.fuqUseSeparate
           because if there was before DMA for FU queue we need to remember that*/
        /* so no check need to be done */
    }

    /* check that the Network interface configuration did not changed from the */
    /* previous initialization */
    MEM_COMP_FROM_SAVED_DB_TO_DEVICE_PARAM_MAC(netIfCfg);

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbPrvNetInfQueueInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the Rx descriptors list per queue private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] netIfNum                 - The network interface number.
* @param[in] rxDescPtr                - (pointer to) the start of Rx descriptors block.
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfQueueInfoSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       netIfNum,
    IN GT_U32                       queueNum,
    IN PRV_CPSS_RX_DESC_STC         *rxDescPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr
)
{

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if (sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid == GT_FALSE)
    {
        /* no meaning in non- DMA device */
        return GT_OK;
    }

    sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[(netIfNum*8)+queueNum].rxDescBlock = rxDescPtr;
    /* At this stage the value of "freeDescNum" is equal to the total number of existing */
    /* descriptors, a value that is used later after SW restart to allocate the required */
    /* memory for the SW Rx descriptors.                                                 */
    sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[(netIfNum*8)+queueNum].totalDescNum = rxDescListPtr[(netIfNum*8)+queueNum].freeDescNum;


    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfMultiNetIfNumberGet function
* @endinternal
*
* @brief   private (internal) function to :
*         build the information about all the network interfaces according to phase1 and phase2 params
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The device number.
*
* @param[out] numOfNetIfsPtr           - (pointer to) the number of network intrfaces according to port mapping
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssDxChNetIfMultiNetIfNumberGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *numOfNetIfsPtr
)
{
    GT_PHYSICAL_PORT_NUM cpuPortNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_STATUS rc;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /*BC3/Aldrin2/Sip6*/
    {
        rc = prvCpssDxChPortMappingCPUPortGet(devNum, /*OUT*/cpuPortNumArr, numOfNetIfsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* the mapping is enough . no special address space needed */
            return GT_OK;
        }

        if(*numOfNetIfsPtr > 1)
        {
            /* check that MG1-3 are mapped for management access */
            if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg1.start == 0)
            {
                *numOfNetIfsPtr = 1;
            }
            else if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg2.start == 0)
            {
                *numOfNetIfsPtr = 2;
            }
            else if (prvCpssDrvPpConfig[devNum]->hwInfo[0].resource.mg3.start == 0)
            {
                *numOfNetIfsPtr = 3;
            }
       }
    }
    else
    {
        *numOfNetIfsPtr =1;
    }
    return GT_OK;

}

/**
* @internal prvCpssPpConfigPortToPhymacObjectBind function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - error
*/
GT_STATUS prvCpssPpConfigPortToPhymacObjectBind
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr

)
{
    GT_U32 portMacNum; /* MAC number */

    /* validate devNum and portNum */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /*NOTE : NULL pointer allow to 'clean' the DB of the port.*/

    /* Get PHY MAC object ptr */

    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portMacObjPtr = macDrvObjPtr;

    return GT_OK;
}


