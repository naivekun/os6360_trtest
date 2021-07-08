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
* @file prvCpssHwMultiPortGroups.c
*
* @brief Private API definition for multi-port-groups devices utilities.
*
* @version   18
********************************************************************************
*/

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssCommon/private/prvCpssMath.h>

/* yes ... include to DXCH */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal prvCpssMultiPortGroupsBmpCheckSpecificUnit function
* @endinternal
*
* @brief   Check port group bmp awareness for specific unit in the device.
*         check and return 'bad param' when device is multi-port group device ,
*         but portGroupsBmp indicate non active port groups
*         for non-multi port groups device --> function updates the portGroupsBmp
*         to CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*         for example:
*         the Bobcat3 is multi-port group device , but the FDB unit is single one, so
*         the portGroupsBmp from the application should be ignored (like in BC2 / xcat3)
*         NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
*         PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] portGroupsBmpPtr         - (pointer to) the port groups bitmap.
* @param[in] unitId                   - is one of PRV_CPSS_DXCH_UNIT_ENT
* @param[in,out] portGroupsBmpPtr         - (pointer to) the updated port groups bitmap.
*
* @retval GT_OK                    - the device supports the IN portGroupsBmpPtr
*                                       and updated the OUT portGroupsBmpPtr
* @retval GT_BAD_PARAM             - the IN portGroupsBmpPtr is illegal for the specific unit
*                                       in the device
*/
GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificUnit(
    IN    GT_U8                     devNum ,
    INOUT GT_PORT_GROUPS_BMP       *portGroupsBmpPtr,
    IN    PRV_CPSS_DXCH_UNIT_ENT    unitId
)
{
    GT_PORT_GROUPS_BMP portGroupsBmp = *portGroupsBmpPtr;
    GT_PORT_GROUPS_BMP temp_portGroupsBmp;
    GT_BOOL didError;
    GT_U32 regAddr;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       !PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* this macro may update the portGroupsBmp to 'CPSS_PORT_GROUP_UNAWARE_MODE_CNS' */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    }
    else
    {
        regAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&didError);
        if(didError == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The unitId[%d] is not valid for device[%d]", unitId , devNum);
        }

        if(GT_FALSE ==
            prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,&temp_portGroupsBmp,
                NULL,NULL))
        {
            /* there is single instance of the unit */
            portGroupsBmp = temp_portGroupsBmp;
        }
        else
        {
            /* check that the temp_portGroupsBmp hold the bits that the IN *portGroupsBmpPtr requires */
            if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            {
                /* update the BMP according to actual unit support */
                portGroupsBmp = temp_portGroupsBmp;
            }
            else
            if(portGroupsBmp & (~temp_portGroupsBmp))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Multi-port group device [%d], port group bitmap [0x%8.8x] not valid for unitId[%d] (the valid bitmap is [0x%8.8x])\n",
                    devNum, portGroupsBmp, unitId, temp_portGroupsBmp);
            }
        }
    }

    *portGroupsBmpPtr = portGroupsBmp;

    return GT_OK;
}

/**
* @internal prvCpssMultiPortGroupsBmpCheckSpecificTable function
* @endinternal
*
* @brief   Check port group bmp awareness for specific table in the device.
*         check and return 'bad param' when device is multi-port group device ,
*         but portGroupsBmp indicate non active port groups
*         for non-multi port groups device --> function updates the portGroupsBmp
*         to CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*         for example:
*         the Bobcat3 is multi-port group device , but the ARP/TS table is single one,
*         although the HA unit is 'per pipe'. So the portGroupsBmp from the application
*         should be ignored (like in BC2 / xcat3)
*         NOTE: the MACRO relevant to SIP5 port groups devices. other devices are treated as :
*         PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in,out] portGroupsBmpPtr         - (pointer to) the port groups bitmap.
*                                      tableId - is one of CPSS_DXCH_TABLE_ENT
* @param[in,out] portGroupsBmpPtr         - (pointer to) the updated port groups bitmap.
*
* @retval GT_OK                    - the device supports the IN portGroupsBmpPtr
*                                       and updated the OUT portGroupsBmpPtr
* @retval GT_BAD_PARAM             - the IN portGroupsBmpPtr is illegal for the specific table
*                                       in the device
*/
GT_STATUS prvCpssMultiPortGroupsBmpCheckSpecificTable(
    IN    GT_U8                     devNum ,
    INOUT GT_PORT_GROUPS_BMP       *portGroupsBmpPtr,
    IN    GT_U32 /*CPSS_DXCH_TABLE_ENT*/   tableId
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC   *directTablePtr;
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *indirectTablePtr;
    GT_PORT_GROUPS_BMP portGroupsBmp = *portGroupsBmpPtr;
    GT_PORT_GROUPS_BMP temp_portGroupsBmp;
    GT_U32 regAddr;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       !PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* this macro may update the portGroupsBmp to 'CPSS_PORT_GROUP_UNAWARE_MODE_CNS' */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    }
    else
    {
        /* validity check */
        if((GT_U32)tableId >=  PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableId);

        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            indirectTablePtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->readTablePtr);
            regAddr = indirectTablePtr->controlReg;
        }
        else
        {
            directTablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
            regAddr = directTablePtr->baseAddress;
        }

        if(GT_FALSE ==
            prvCpssDuplicatedMultiPortGroupsGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,&temp_portGroupsBmp,
                NULL,NULL))
        {
            /* there is single instance of the table */
            portGroupsBmp = temp_portGroupsBmp;
        }
        else
        {
            /* check that the temp_portGroupsBmp hold the bits that the IN *portGroupsBmpPtr requires */
            if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            {
                /* update the BMP according to actual unit support */
                portGroupsBmp = temp_portGroupsBmp;
            }
            else
            if(portGroupsBmp & (~temp_portGroupsBmp))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "Multi-port group device [%d], port group bitmap [0x%8.8x] not valid for table[%d] (the valid bitmap is [0x%8.8x])\n",
                    devNum, tableId , portGroupsBmp , temp_portGroupsBmp);
            }
        }
    }

    *portGroupsBmpPtr = portGroupsBmp;

    return GT_OK;
}


#define ERROR_CODE_ON_GT_U32_CNS 0xFFFFFFFF

typedef struct{
    GT_U32  local_macPortNum;
    GT_U32  portGroupId;
}PHYSICAL_PORT_INFO_STC;

/**
* @internal sip5GlobalPhysicalPortInfoGet function
* @endinternal
*
* @brief   sip5 : convert 'global' physical port number to 'local' MAC port number
*         and port group ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] global_physicalPortNum   - the global physical port number.
*
* @param[out] physicalPortInfoPtr      - (pointer to) physical port info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5GlobalPhysicalPortInfoGet(
    IN GT_U8 devNum ,
    IN GT_U32 global_physicalPortNum,
    OUT PHYSICAL_PORT_INFO_STC   *physicalPortInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  global_macPortNum,local_macPortNum,portGroupId;

    physicalPortInfoPtr->portGroupId = ERROR_CODE_ON_GT_U32_CNS;
    physicalPortInfoPtr->local_macPortNum = ERROR_CODE_ON_GT_U32_CNS;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "the device [%d] must be 'sip5'",
            devNum);
    }

    if(!PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        physicalPortInfoPtr->portGroupId = 0;
        physicalPortInfoPtr->local_macPortNum = global_physicalPortNum;
        /* no conversion needed */
        return GT_OK;
    }

    /*
        1. convert physical port to MAC ports
        2. convert MAC port to local MAC port
    */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, global_physicalPortNum,
        PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &global_macPortNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(ERROR_CODE_ON_GT_U32_CNS/* return error value */,
            "the device [%d] global_physicalPortNum[%d] not have valid 'MAC port number' ",
            devNum,global_physicalPortNum);
    }

    rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
        global_macPortNum/*global MAC port*/,
        &portGroupId,/*pipe index*/
        &local_macPortNum/* local port */);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    physicalPortInfoPtr->portGroupId = portGroupId;
    physicalPortInfoPtr->local_macPortNum = local_macPortNum;

    return GT_OK;
}

/*******************************************************************************
* prvCpssSip5GlobalPhysicalPortToLocalMacConvert
*
* DESCRIPTION:
*       sip5 : convert 'global' physical port number to 'local' MAC port number.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon;
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       global_physicalPortNum - the global physical port number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the 'local' MAC port number
*       NOTE: value 0xFFFFFFFF means error
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*local_macPortNum*/ prvCpssSip5GlobalPhysicalPortToLocalMacConvert(
    IN GT_U8 devNum ,
    IN GT_U32 global_physicalPortNum
)
{
    PHYSICAL_PORT_INFO_STC physicalPortInfo;

    sip5GlobalPhysicalPortInfoGet(devNum,global_physicalPortNum,&physicalPortInfo);

    return physicalPortInfo.local_macPortNum;
}
/*******************************************************************************
* prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert
*
* DESCRIPTION:
*       sip5 : convert 'global' physical port number to port group number.
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon;
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* INPUTS:
*       devNum          - the device number.
*       global_physicalPortNum - the global physical port number.
* OUTPUTS:
*       None
*
* RETURNS:
*       the portGroupId
*       NOTE: value 0xFFFFFFFF means error
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 /*portGroupId*/ prvCpssSip5GlobalPhysicalPortToPortGroupIdConvert(
    IN GT_U8 devNum ,
    IN GT_U32 global_physicalPortNum
)
{
    PHYSICAL_PORT_INFO_STC physicalPortInfo;

    sip5GlobalPhysicalPortInfoGet(devNum,global_physicalPortNum,&physicalPortInfo);

    return physicalPortInfo.portGroupId;
}



