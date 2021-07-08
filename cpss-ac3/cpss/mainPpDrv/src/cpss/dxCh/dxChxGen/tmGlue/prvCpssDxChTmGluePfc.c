/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
* cpssDxChTmGluePfc.c
*
* DESCRIPTION:
*       Traffic Manager Glue - PFC Private APIs
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/


#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>
#include <cpss/extServices/cpssExtServices.h>

/* variable of 'TM PFC init done' */
#define PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) \
    PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcIsInitialized

/* pointer to 'TM PFC info' */
#define PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum) \
    ((PRV_CPSS_DXCH_TM_GLUE_PFC_DEV_INFO_STC*)(PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcDevInfoPtr))

/* global flag indicates whether c node must apear only one time in tc,
   port to c node mapping */
GT_BOOL prvPfcTmGlueCnodeMappingOnce = GT_TRUE;

/**
* @internal prvCpssDxChTmGluePfcTmDbGetTcPortByCNode function
* @endinternal
*
* @brief   get traffic class and port by cNode from SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] cNodeValue               - C node value (APPLICABLE RANGES: 0...511).
*
* @param[out] portPtr                  - (poinetr to) source physical port number.
* @param[out] tcPtr                    - (pointer to) traffic class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbGetTcPortByCNode
(
    IN GT_U8                devNum,
    IN GT_U32               cNodeValue,
    IN GT_PHYSICAL_PORT_NUM *portPtr,
    IN GT_U32               *tcPtr

)
{
    GT_U32  port;
    GT_U32  tc;
    GT_U32  value;
    GT_BOOL found = GT_FALSE;
    GT_STATUS rc = GT_NOT_FOUND;

    if (PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }

    for (port = 0; port < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS && !found; port++)
    {
        for (tc = 0; tc < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS; tc++)
        {
            if (PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc] == cNodeValue)
            {
                *portPtr = port;
                *tcPtr = tc;
                found = GT_TRUE;
                break;
            }
        }
    }

    if (found)
    {
        /* verify sync with HW*/
        rc = prvCpssDxChReadTableEntryField(devNum,
                     CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                     *portPtr,
                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                     *tcPtr * 9, 9, &value);

        if (rc != GT_OK)
        {
            return rc;
        }

        if (value == cNodeValue)
        {
            rc = GT_OK;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTmGluePfcTmDbGetCNodeByTcPort function
* @endinternal
*
* @brief   get cNode by traffic class and port from SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] port                     - source physical port number.
* @param[in] tc                       - traffic class (APPLICABLE RANGES: 0..7).
*
* @param[out] cNodePtr                 - (pointer to) C node value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICBLE_DEVICE  - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbGetCNodeByTcPort
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM port,
    IN GT_U32               tc,
    IN GT_U32               *cNodePtr
)
{
    GT_U32  value;
    GT_STATUS rc = GT_NOT_FOUND;

    if (PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }


    if (port >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS || tc >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

     /* verify sync with HW*/
     rc = prvCpssDxChReadTableEntryField(devNum,
                     CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                     port,
                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                     tc * 9, 9, &value);

     if (value == PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc])
     {
         *cNodePtr = value;
         rc = GT_OK;
     }

    return rc;
}

/**
* @internal prvCpssDxChTmGluePfcTmDbUpdateTcPortCNode function
* @endinternal
*
* @brief   update cNodes traffic class and port changes to SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*                                      cNodeValue  - C node value (APPLICABLE RANGES: 0...511).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number,
*                                       port or traffic class.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmDbUpdateTcPortCNode
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC *cNode1Ptr,
    IN PRV_CPSS_DXCH_TM_GLUE_PFC_CNODE_INFO_STC *cNode2Ptr
)
{
    PRV_CPSS_DXCH_TM_GLUE_PFC_DEV_INFO_STC *pfcDevInfoPtr;

    if (PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }


    CPSS_NULL_PTR_CHECK_MAC(cNode1Ptr);
    CPSS_NULL_PTR_CHECK_MAC(cNode2Ptr);

    if (cNode1Ptr->port >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS ||
        cNode1Ptr->tc >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS ||
        cNode1Ptr->cNode >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_CNODE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    if (cNode2Ptr->port >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS ||
        cNode2Ptr->tc >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS ||
        cNode2Ptr->cNode >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_CNODE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (cNode1Ptr->cNode == cNode2Ptr->cNode)
    {
        if (cNode1Ptr->port != cNode2Ptr->port || cNode1Ptr->tc != cNode2Ptr->tc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }
    }

    pfcDevInfoPtr = PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum);

    pfcDevInfoPtr->
        TcPort2CnodeSwTable[cNode1Ptr->port][cNode1Ptr->tc] = cNode1Ptr->cNode;

    pfcDevInfoPtr->
        TcPort2CnodeSwTable[cNode2Ptr->port][cNode2Ptr->tc] = cNode2Ptr->cNode;

    return GT_OK;
}

/**
* @internal prvCpssDxChTmGluePfcPortMappingGet function
* @endinternal
*
* @brief   Get physical port to pfc port mapping.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number. (APPLICABLE RANGES: 0..255).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number, or port.
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcPortMappingGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_PHYSICAL_PORT_NUM    *pfcPortNumPtr
)
{
    GT_U32  value;                  /* HW field value */
    GT_STATUS rc = GT_OK;           /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(pfcPortNumPtr);

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* Get Physical Port To PFC port mapping Entry */
        /* the manipulations of 4 ports in entry are managed by the 'tables engine' */
        rc = prvCpssDxChReadTableEntryField(devNum,
                                 CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E,
                                 portNum,
                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                 0,
                                 6,
                                 &value);
    }
    else
    {
        value = portNum;

        if (portNum >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS)
        {
            rc = GT_BAD_PARAM;
        }
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    *pfcPortNumPtr = value;

    return GT_OK;
}


/**
* @internal prvCpssDxChTmGluePfcInit function
* @endinternal
*
* @brief   Init TM GLUE PFC software Data Base.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcInit
(
    IN GT_U8 devNum
)
{
    GT_U32  port, tc;
    GT_U32  cNode = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
          CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E  | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (prvPfcTmGlueCnodeMappingOnce == GT_FALSE)
    {
        return GT_OK;
    }

    if (PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) == GT_FALSE)
    {
        PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcDevInfoPtr =
            cpssOsMalloc(sizeof(PRV_CPSS_DXCH_TM_GLUE_PFC_DEV_INFO_STC));

        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcDevInfoPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
    }

    /* initial TC port to Cnode Indexes shadow table mapping
                          TC
                  ---------------------
                    0     ...   7
            Port
            ----
             0    cNode0  ... cNode7
             .
             .
             .
            64    cNode504 ... cNode511

    */
    for (port = 0; port < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS; port++)
    {
        for (tc = 0; tc < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS; tc++)
        {
            PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc] = (GT_U16)cNode++;
        }
    }

    /* sync HW with SW DB */

    for (port = 0; port < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS; port++)
    {
        for (tc = 0; tc < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS; tc++)
        {
            cNode = PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc];

            rc = prvCpssDxChWriteTableEntryField(devNum,
                         CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                         port,
                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                         tc * 9, 9, cNode);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    PFC_TM_GLUE_PFC_IS_INITIALIZED_MAC(devNum) = GT_TRUE;

    return GT_OK;
}


#define prvCpssDxChTmGluePfc_debug_def

#ifdef prvCpssDxChTmGluePfc_debug_def

/**
* @internal prvCpssDxChTmGluePfcPortMappingDump function
* @endinternal
*
* @brief   Dump tm pfc ports mapping table, using cpssDxChTmGluePfcPortMappingGet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Dump Mapping of TM pfc ports >= PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS (64)
*       to first PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS-1 (63) TM pfc ports
*
*/
GT_STATUS prvCpssDxChTmGluePfcPortMappingDump
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_PHYSICAL_PORT_NUM physicalPort;
    GT_PHYSICAL_PORT_NUM pfcPort;
    GT_PHYSICAL_PORT_NUM maxPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("  pfcPort     physicalPort    \n");
    for (physicalPort = 0; physicalPort < maxPort; physicalPort++)
    {
        rc = cpssDxChTmGluePfcPortMappingGet(devNum, physicalPort, &pfcPort);
        if (rc == GT_OK)
        {
            cpssOsPrintf("     %d             %d\n", physicalPort, pfcPort);
        }
        else
        {
            cpssOsPrintf("failed on physicalPort: %d, rc: %d\n", physicalPort, rc);
            break;
        }
    }

    return rc;
}


/**
* @internal prvCpssDxChTmGluePfcPortMappingDumpTest function
* @endinternal
*
* @brief   Fill/Test tm pfc ports mapping table, using cpssDxChTmGluePfcPortMappingSet.
*         Dump Table using prvCpssDxChTmGluePfcPortMappingDump.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcPortMappingDumpTest
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_PHYSICAL_PORT_NUM physicalPort;
    GT_PHYSICAL_PORT_NUM pfcPort;
    GT_PHYSICAL_PORT_NUM maxPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    for (physicalPort = 0, pfcPort = 63; physicalPort < maxPort; physicalPort += 2, pfcPort--)
    {
        rc = cpssDxChTmGluePfcPortMappingSet(devNum, physicalPort, pfcPort);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChTmGluePfcPortMappingSet: physicalPort: %d, pfcPort: %d, rc: %d\n", physicalPort, pfcPort, rc);
            break;
        }
    }

    prvCpssDxChTmGluePfcPortMappingDump(devNum);

    return rc;
}


/**
* @internal prvCpssDxChTmGluePfcTmTcPort2CNodeDump function
* @endinternal
*
* @brief   Dump tm pfc ports and TC to Cnode mapping table,
*         Compare HW values to SW DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmTcPort2CNodeDump
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;
    GT_U32      port;
    GT_U32      tc;
    GT_U32      cNode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("Port  Tc  CNode\n");
    for (port = 0; port < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS; port++)
    {
        for (tc = 0; tc < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS; tc++)
        {
            rc = prvCpssDxChReadTableEntryField(devNum,
                         CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
                         port,
                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                         tc * 9, 9, &cNode);

            if(rc != GT_OK)
            {
                return rc;
            }

            if ((GT_U16)cNode != PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc])
            {
                cpssOsPrintf(" %d        %d           HW: %d SW: %d !!!\n",
                             port,
                             tc,
                             cNode,
                             PFC_TM_GLUE_PFC_DEV_INFO_PTR(devNum)->TcPort2CnodeSwTable[port][tc]);
            }
            else
            {
                cpssOsPrintf(" %d        %d           %d\n", port, tc, cNode);
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChTmGluePfcTmTcPort2CNodeTest function
* @endinternal
*
* @brief   Test/Update tm pfc ports and TC to Cnode mapping table
*         using cpssDxChTmGluePfcTmTcPort2CNodeSet.
*         Dump table using prvCpssDxChTmGluePfcTmTcPort2CNodeDump.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] startPort                - from tm pfc port.
* @param[in] maxPort                  - to tm pfc port.
* @param[in] startCnode               - from  cnode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChTmGluePfcTmTcPort2CNodeTest
(
    IN GT_U8    devNum,
    IN GT_U8    startPort,
    IN GT_U8    maxPort,
    IN GT_16    startCnode
)
{
    GT_STATUS   rc;
    GT_U8       port;
    GT_U8       tc;
    GT_U32      cNode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    cNode = startCnode;

    for (port = startPort; port <= MIN(PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_PORT_CNS-1, maxPort); port++)
    {
        for (tc = 0; tc < PRV_CPSS_DXCH_TM_GLUE_PFC_MAX_TC_CNS && cNode < 512; tc++, cNode++)
        {
            cpssOsPrintf("Test(%d,%d,%d,%d)\n",devNum, port, tc, cNode);
            rc = cpssDxChTmGluePfcTmTcPort2CNodeSet(devNum,
                                                    port,
                                                    tc,
                                                    cNode);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = prvCpssDxChTmGluePfcTmTcPort2CNodeDump(devNum);

    return rc;
}

#endif

