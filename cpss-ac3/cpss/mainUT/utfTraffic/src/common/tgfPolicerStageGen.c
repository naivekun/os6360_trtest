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
* @file tgfPolicerStageGen.c
*
* @brief Generic API for Policing Engine per stage API.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


#include <common/tgfCommon.h>
#include <common/tgfPolicerStageGen.h>

/* DB to store Default Policer Counting modes */
PRV_TGF_POLICER_COUNTING_MODE_ENT prvTgfPolicerCntrModeDefaultArr[PRV_TGF_POLICER_STAGE_NUM] = 
    {PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E, 
     PRV_TGF_POLICER_COUNTING_DISABLE_E, /* ingress stage 1 has disabled state */
     PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E};

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

/**
* @internal prvTgfPolicerVlanCountingStageDefValuesSet function
* @endinternal
*
* @brief   Set Policer VLAN Default values.
*         1. Counting mode set to VLAN.
*         2. Vlan counting mode set to Packet.
*         3. Enable VLAN counting triggering only for
*         CPSS_PACKET_CMD_FORWARD_E packet cmd.
*         4. Flush counters.
*         5. Reset VLAN counters.
* @param[in] devNum                   - device number
*                                      counterSetIndex - policing counters set index
*                                      countersPtr     - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerVlanCountingStageDefValuesSet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U16                              defVid
)
{    
    GT_STATUS   rc;

    /* set counting mode to VLAN */
    rc = prvTgfPolicerCountingModeSet (devNum, stage, 
                                         PRV_TGF_POLICER_COUNTING_VLAN_E);
    if (GT_OK != rc)
    {
        return rc;
    }
    /* The Trigger commands may be changed only for Ingress stages */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E) 
    {
        /* Enable Vlan Counting only for FORWARD cmd */
        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_DROP_SOFT_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }
        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_TRAP_TO_CPU_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }
        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /* Flush counters */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(stage);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Reset default Vlan counter */
    rc = prvTgfPolicerVlanCntrSet(devNum, stage, defVid, 0);
    if (GT_OK != rc)
    {
        return rc;
    }

#ifdef CHX_FAMILY
{
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT      cpssStage;

    /* convert UTF stage to CPSS convention */
    switch (stage) 
    {
        case PRV_TGF_POLICER_STAGE_INGRESS_0_E:
            cpssStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case PRV_TGF_POLICER_STAGE_INGRESS_1_E:
            cpssStage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case PRV_TGF_POLICER_STAGE_EGRESS_E:
            cpssStage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* Set VLAN counting mode to Packets */
    rc = cpssDxChPolicerVlanCountingModeSet(devNum, cpssStage, 
                                            CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChPolicerVlanCountingModeSet FAILED, rc = [%d]", rc);
    
        return rc;
    }
}
    return rc;
#else /* CHX_FAMILY */
    return GT_OK;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfPolicerVlanCountingStageDefValuesRestore function
* @endinternal
*
* @brief   Set Policer VLAN Default values.
*         1. Counting mode set to Billing.
*         2. Vlan counting mode set to Packet.
*         3. Enable VLAN counting for all commands.
*         4. Flush counters.
*         5. Reset VLAN counters.
* @param[in] devNum                   - device number
*                                      counterSetIndex - policing counters set index
*                                      countersPtr     - (pointer to) counters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or counterSetIndex
* @retval GT_BAD_PTR               - on one of the parameters is NULL pointer
*/
GT_STATUS prvTgfPolicerVlanCountingStageDefValuesRestore
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U16                              defVid
)
{

    GT_STATUS   rc;

    /* Flush counters */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(stage);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Reset default Vlan counter */
    rc = prvTgfPolicerVlanCntrSet(devNum, stage, defVid, 0);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* restore counting mode */
    rc = prvTgfPolicerCountingModeSet (devNum, stage,
                                       prvTgfPolicerCntrModeDefaultArr[stage]);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* The Trigger commands may be changed only for Ingress stages 
       Disable all triggers, except for FORWARD command */
    if (stage < PRV_TGF_POLICER_STAGE_EGRESS_E) 
    {

        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_FORWARD_E, 
                                                            GT_TRUE);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_DROP_SOFT_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_TRAP_TO_CPU_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = prvTgfPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, 
                                                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E, 
                                                            GT_FALSE);
        if (GT_OK != rc)
        {
            return rc;
        }
    }


    return GT_OK;

}

