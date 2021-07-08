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
* @file prvCpssDxChTxqPsi.c
*
* @brief CPSS SIP6 TXQ Psi low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.6}*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/prvCpssFalconTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxq.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>



/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesSet function
* @endinternal
*
* @brief   Map port to pdq queues
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] pNodeIndex             -        -index of Pnode representing the local port
* @param[in] firstQueueInDp           - the first pdq queue of DP
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 pNodeIndex,
    IN  GT_U32 firstQueueInDp
)
{


    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);

    if((firstQueueInDp>=CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)||(pNodeIndex>=CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


  
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PSI_PORT_QUEUE_OFFSET_E+psiNum,
                                    pNodeIndex,
                                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                    TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_OFFSET,
                                    TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_SIZE,
                                    firstQueueInDp);


    return rc;

}

/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesGet function
* @endinternal
*
* @brief   Read sdq to pdq queues mapping
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   -  SDQ that being mapped number (APPLICABLE RANGES:0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 psiNum,
    IN   GT_U32 sdqNum,
    IN   GT_U32 localPortNum,
    OUT  GT_U32 * pdqLowQueueNumPtr
)

{

    GT_U32 pNodeIndex;
    GT_STATUS rc;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(localPortNum);


    pNodeIndex = localPortNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC+sdqNum;


    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_PSI_PORT_QUEUE_OFFSET_E+psiNum,
                                        pNodeIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_OFFSET,
                                        TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_SIZE,
                                        pdqLowQueueNumPtr);


    return rc;

}

/**
* @internal prvCpssFalconPsiCreditValueSet function
* @endinternal
*
* @brief   Set credit value that is being given to SDQ from PDQ
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] psiNum                   -   current PSI id(APPLICABLE RANGES:0..3).
* @param[in] creditValue              -  credit value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/

GT_STATUS  prvCpssFalconPsiCreditValueSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 creditValue
)
{
     GT_U32 regAddr,tileOffset ;
     GT_U32 regValue = 0;

     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Credit_Value;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,psiNum/*tileId*/);
     regAddr+=tileOffset;


     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET,
                            TXQ_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE,
                            creditValue);


     rc = prvCpssHwPpWriteRegister(devNum,regAddr,regValue);


     return rc;


}



