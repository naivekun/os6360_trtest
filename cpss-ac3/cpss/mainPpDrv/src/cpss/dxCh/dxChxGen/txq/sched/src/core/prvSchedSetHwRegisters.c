/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */

/**
 * @brief  functions for set/get  common for all platforms registers
 *
* @file set_hw_registers.c
*
* $Revision: 2.0 $
*/

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>



#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmStatus.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrors.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>


#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC 511
#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_B_NODE_MAC 127
#define PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC 2047

#define PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR           cpssDeviceRunCheck_onEmulator






#ifdef DEBUG_WIDTH
extern int width_mismatch_known;
#endif

/**
 */
int prvSchedLowLevelSetDwrrLimit(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc = 0;
    TM_REGISTER_VAR(TM_Sched_PBytePerBurstLimit_Addr)

    TM_CTL(ctl, hndl)

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.PBytePerBurstLimit_Addr,TM_Sched_PBytePerBurstLimit_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_PBytePerBurstLimit_Addr, limit , ctl->dwrr_bytes_burst_limit)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.PBytePerBurstLimit_Addr,TM_Sched_PBytePerBurstLimit_Addr)

out:

    return rc;
}


int prvSchedLowLevelSetHwGenConf(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc = 0;
    TM_REGISTER_VAR(TM_Sched_PortExtBPEn_Addr)

    TM_CTL(ctl, hndl)

    /* reset register reserved fields  - sip6 has different reset value*/
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.PortExtBPEn_Addr, TM_Sched_PortExtBPEn_Addr);
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_PortExtBPEn_Addr , En , ctl->port_ext_bp_en);
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.PortExtBPEn_Addr, TM_Sched_PortExtBPEn_Addr)
out:
    return rc;
}






/**
 */
int prvSchedLowLevelSetHwPeriodicScheme(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;
#if 0
    TM_REGISTER_VAR(TM_Sched_ScrubSlots_Addr)
    TM_REGISTER_VAR(TM_Sched_PPerRateShap_Addr)
    TM_REGISTER_VAR(TM_Sched_CPerRateShap_Addr)
    TM_REGISTER_VAR(TM_Sched_BPerRateShap_Addr)
 #endif
 
    TM_REGISTER_VAR(TM_Sched_APerRateShap_Addr)
    TM_REGISTER_VAR(TM_Sched_QPerRateShap_Addr)
    

    TM_CTL(ctl, hndl)


#if 0

    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,P_LEVEL);
    if (rc)
        goto out;

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.PPerRateShap_Addr, TM_Sched_PPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_PPerRateShap_Addr , L , ctl->level_data[P_LEVEL].L)
    TXQ_SCHED_REGISTER_SET(TM_Sched_PPerRateShap_Addr , K , ctl->level_data[P_LEVEL].K)
    TXQ_SCHED_REGISTER_SET(TM_Sched_PPerRateShap_Addr , N , ctl->level_data[P_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.PPerRateShap_Addr, TM_Sched_PPerRateShap_Addr)
    if (rc) goto out;

    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,C_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.CPerRateShap_Addr, TM_Sched_CPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_CPerRateShap_Addr , L , ctl->level_data[C_LEVEL].L)
    TXQ_SCHED_REGISTER_SET(TM_Sched_CPerRateShap_Addr , K , ctl->level_data[C_LEVEL].K)
    TXQ_SCHED_REGISTER_SET(TM_Sched_CPerRateShap_Addr , N , ctl->level_data[C_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.CPerRateShap_Addr, TM_Sched_CPerRateShap_Addr)
    if (rc) goto out;

    rc = prvSchedLowLevelSetPerLevelShapingParameters(hndl,B_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.BPerRateShap_Addr, TM_Sched_BPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_BPerRateShap_Addr , L , ctl->level_data[B_LEVEL].L)
    TXQ_SCHED_REGISTER_SET(TM_Sched_BPerRateShap_Addr , K , ctl->level_data[B_LEVEL].K)
    TXQ_SCHED_REGISTER_SET(TM_Sched_BPerRateShap_Addr , N , ctl->level_data[B_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.BPerRateShap_Addr, TM_Sched_BPerRateShap_Addr)
    if (rc) goto out;
#endif
    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,A_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.AlvlPerRateShpPrms, TM_Sched_APerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_APerRateShap_Addr , L , ctl->level_data[A_LEVEL].L)
    TXQ_SCHED_REGISTER_SET(TM_Sched_APerRateShap_Addr , K , ctl->level_data[A_LEVEL].K)
    TXQ_SCHED_REGISTER_SET(TM_Sched_APerRateShap_Addr , N , ctl->level_data[A_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.APerRateShap_Addr, TM_Sched_APerRateShap_Addr)
    if (rc) goto out;

    rc=prvSchedLowLevelSetPerLevelShapingParameters(hndl,Q_LEVEL);
    if (rc) goto out;

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.QPerRateShap_Addr, TM_Sched_QPerRateShap_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_QPerRateShap_Addr , L , ctl->level_data[Q_LEVEL].L)
    TXQ_SCHED_REGISTER_SET(TM_Sched_QPerRateShap_Addr , K , ctl->level_data[Q_LEVEL].K)
    TXQ_SCHED_REGISTER_SET(TM_Sched_QPerRateShap_Addr , N , ctl->level_data[Q_LEVEL].N)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.QPerRateShap_Addr, TM_Sched_QPerRateShap_Addr)
    if (rc) goto out;
    
#if 0

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.ScrubSlots_Addr, TM_Sched_ScrubSlots_Addr)
    if (rc) goto out;
    /* assign register fields */
    /* write register */
    TXQ_SCHED_REGISTER_SET(TM_Sched_ScrubSlots_Addr , PortSlots , ctl->level_data[P_LEVEL].scrub_slots)
    TXQ_SCHED_REGISTER_SET(TM_Sched_ScrubSlots_Addr , ClvlSlots , ctl->level_data[C_LEVEL].scrub_slots)
    TXQ_SCHED_REGISTER_SET(TM_Sched_ScrubSlots_Addr , BlvlSlots , ctl->level_data[B_LEVEL].scrub_slots)
    TXQ_SCHED_REGISTER_SET(TM_Sched_ScrubSlots_Addr , AlvlSlots , ctl->level_data[A_LEVEL].scrub_slots)
    TXQ_SCHED_REGISTER_SET(TM_Sched_ScrubSlots_Addr , QueueSlots ,ctl->level_data[Q_LEVEL].scrub_slots)
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.ScrubSlots_Addr, TM_Sched_ScrubSlots_Addr)
    if (rc) goto out;
#endif

out:

    return rc;
}


/**
 */
int prvSchedLowLevelSetHwNodeMapping(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel lvl, uint32_t index)
{
    int rc =  -EFAULT;


    TM_REGISTER_VAR(TM_Sched_PortRangeMap)

    TM_REGISTER_VAR(TM_Sched_ClvltoPortAndBlvlRangeMap)

    TM_REGISTER_VAR(TM_Sched_BLvltoClvlAndAlvlRangeMap)

    TM_REGISTER_VAR(TM_Sched_ALvltoBlvlAndQueueRangeMap)

    TM_REGISTER_VAR(TM_Sched_QueueAMap)


    TM_CTL(ctl, hndl)


    switch (lvl)
    {
    case P_LEVEL:

            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortRangeMap , index , TM_Sched_PortRangeMap)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap , Lo , ctl->tm_port_array[index].mapping.childLo)
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap, Hi , ctl->tm_port_array[index].mapping.childHi)


            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortRangeMap , index , TM_Sched_PortRangeMap)

        break;
    case C_LEVEL:

            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap , index , TM_Sched_ClvltoPortAndBlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */

            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , Port , ctl->tm_c_node_array[index].mapping.nodeParent)

            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlLo , ctl->tm_c_node_array[index].mapping.childLo)
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlHi , ctl->tm_c_node_array[index].mapping.childHi)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap , index , TM_Sched_ClvltoPortAndBlvlRangeMap)

        break;
    case B_LEVEL:

        /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap , index , TM_Sched_BLvltoClvlAndAlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */

            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , Clvl , ctl->tm_b_node_array[index].mapping.nodeParent)

            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlLo , ctl->tm_b_node_array[index].mapping.childLo)
            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlHi , ctl->tm_b_node_array[index].mapping.childHi)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap , index , TM_Sched_BLvltoClvlAndAlvlRangeMap)

        break;
    case A_LEVEL:
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap , index , TM_Sched_ALvltoBlvlAndQueueRangeMap)
            if (rc) goto out;

            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap , Blvl , ctl->tm_a_node_array[index].mapping.nodeParent)

            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap, QueueLo , ctl->tm_a_node_array[index].mapping.childLo)
            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueHi , ctl->tm_a_node_array[index].mapping.childHi)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap , index , TM_Sched_ALvltoBlvlAndQueueRangeMap)

        break;
    case Q_LEVEL:

            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueAMap , index , TM_Sched_QueueAMap)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_QueueAMap , Alvl , ctl->tm_queue_array[index].mapping.nodeParent)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueAMap , index , TM_Sched_QueueAMap)

        break;
    }
out:

    return rc;
}




int prvSchedLowLevelSetQueueQuantum(PRV_CPSS_SCHED_HANDLE hndl, uint32_t queue_ind)
{
    int rc =  -EFAULT;

    struct tm_queue *queue = NULL;

   TM_REGISTER_VAR(TM_Sched_QueueQuantum)

    TM_CTL(ctl, hndl)                                                                                                                    
      
    if (queue_ind < ctl->tm_total_queues)
    {
        queue = &(ctl->tm_queue_array[queue_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueQuantum , queue_ind , TM_Sched_QueueQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueQuantum , Quantum , queue->dwrr_quantum)
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueQuantum , queue_ind , TM_Sched_QueueQuantum)
        /* write register */
        if (rc) goto out;


    }
out:

    return rc;
}


int prvSchedLowLeveGetQueueParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct queue_hw_data_t *queue_hw_data)
{
    int rc = -EFAULT;



    TM_REGISTER_VAR(TM_Sched_QueueAMap)
    TM_REGISTER_VAR(TM_Sched_QueueQuantum)
    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)




    if (index < ctl->tm_total_queues)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueAMap, index, TM_Sched_QueueAMap);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueAMap, Alvl, queue_hw_data->parent_a_node, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr, index, TM_Sched_QueueEligPrioFuncPtr);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFuncPtr, Ptr, queue_hw_data->elig_prio_func_ptr, (uint8_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueQuantum, index, TM_Sched_QueueQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_QueueQuantum, Quantum, queue_hw_data->dwrr_quantum, (uint16_t));

    }

    return rc;
}





int prvSchedLowLevelSet_a_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;

    TM_REGISTER_VAR(TM_Sched_AlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

      if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)         
      {                                      
        return 0;        
     }

    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum , node_ind , TM_Sched_AlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum , node_ind , TM_Sched_AlvlQuantum)
        if (rc) goto out;



            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlDWRRPrioEn , node_ind , TM_Sched_AlvlDWRRPrioEn)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlDWRRPrioEn , node_ind , TM_Sched_AlvlDWRRPrioEn)
            /* write register */
            if (rc) goto out;
         }


out:

    return rc;
}
int prvSchedLowLevelGetANodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct a_node_hw_data_t *a_node_hw_data)
{
    int rc = -EFAULT;

    TM_REGISTER_VAR(TM_Sched_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(TM_Sched_AlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)


    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)



    if (index < ctl->tm_total_a_nodes)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap, index, TM_Sched_ALvltoBlvlAndQueueRangeMap);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap, Blvl, a_node_hw_data->parent_b_node, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap, QueueHi, a_node_hw_data->last_child_queue, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap, QueueLo, a_node_hw_data->first_child_queue, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr, index, TM_Sched_AlvlEligPrioFuncPtr);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFuncPtr, Ptr, a_node_hw_data->elig_prio_func_ptr, (uint8_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum, index, TM_Sched_AlvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlQuantum, Quantum, a_node_hw_data->dwrr_quantum, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlDWRRPrioEn, index, TM_Sched_AlvlDWRRPrioEn);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlDWRRPrioEn, En, a_node_hw_data->dwrr_priority, (uint8_t));



    }
    return rc;
}





/**
 */
int prvSchedLowLevelSet_b_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;
    struct tm_b_node *node = NULL;

    TM_REGISTER_VAR(TM_Sched_BlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_BlvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

     if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)         
      {                          
              
        return 0;        
     }

    if (node_ind < ctl->tm_total_b_nodes)
    {
        node = &(ctl->tm_b_node_array[node_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlQuantum , node_ind , TM_Sched_BlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        /* write register */
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlQuantum , node_ind , TM_Sched_BlvlQuantum)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlDWRRPrioEn , node_ind , TM_Sched_BlvlDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlDWRRPrioEn , node_ind , TM_Sched_BlvlDWRRPrioEn)
        if (rc) goto out;


    }
out:

    return rc;
}

int prvSchedLowLevelGetBNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct b_node_hw_data_t *b_node_hw_data)
{
    int rc = -EFAULT;



    TM_REGISTER_VAR(TM_Sched_BLvltoClvlAndAlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_BlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_BlvlDWRRPrioEn)



    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)




    if (index < ctl->tm_total_b_nodes)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap, index, TM_Sched_BLvltoClvlAndAlvlRangeMap);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap, Clvl, b_node_hw_data->parent_c_node, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap, AlvlHi, b_node_hw_data->last_child_a_node, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap, AlvlLo, b_node_hw_data->first_child_a_node, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr, index, TM_Sched_BlvlEligPrioFuncPtr);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFuncPtr, Ptr, b_node_hw_data->elig_prio_func_ptr, (uint8_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlQuantum, index, TM_Sched_BlvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlQuantum, Quantum, b_node_hw_data->dwrr_quantum, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlDWRRPrioEn, index, TM_Sched_BlvlDWRRPrioEn);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlDWRRPrioEn, En, b_node_hw_data->dwrr_priority, (uint8_t));



    }
    return rc;
}





/**
 */
int prvSchedLowLevelSet_c_node_Params(PRV_CPSS_SCHED_HANDLE hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    struct tm_c_node *node = NULL;

    TM_REGISTER_VAR(TM_Sched_ClvlQuantum)
    TM_REGISTER_VAR(TM_Sched_ClvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

    if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)         
      {                          
              
        return 0;        
     }

    if (node_ind < ctl->tm_total_c_nodes)
    {
        node = &(ctl->tm_c_node_array[node_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlQuantum , node_ind , TM_Sched_ClvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlQuantum , node_ind , TM_Sched_ClvlQuantum)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlDWRRPrioEn , node_ind , TM_Sched_ClvlDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlDWRRPrioEn , En , node->dwrr_enable_per_priority_mask)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlDWRRPrioEn , node_ind , TM_Sched_ClvlDWRRPrioEn)
        if (rc) goto out;


    }
out:

    return rc;
}

int prvSchedLowLevelGetCNodeParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct c_node_hw_data_t *c_node_hw_data)
{
    int rc = -EFAULT;


    TM_REGISTER_VAR(TM_Sched_ClvltoPortAndBlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_ClvlQuantum)
    TM_REGISTER_VAR(TM_Sched_ClvlDWRRPrioEn)

    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)


    if (index < ctl->tm_total_c_nodes)
    {

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap, index, TM_Sched_ClvltoPortAndBlvlRangeMap);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap, Port, c_node_hw_data->parent_port, (uint8_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap, BlvlHi, c_node_hw_data->last_child_b_node, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap, BlvlLo, c_node_hw_data->first_child_b_node, (uint16_t));



        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr, index, TM_Sched_ClvlEligPrioFuncPtr);
        if (rc)
          return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFuncPtr, Ptr, c_node_hw_data->elig_prio_func_ptr, (uint8_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlQuantum, index, TM_Sched_ClvlQuantum);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlQuantum, Quantum, c_node_hw_data->dwrr_quantum, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlDWRRPrioEn, index, TM_Sched_ClvlDWRRPrioEn);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlDWRRPrioEn, En, c_node_hw_data->dwrr_priority, (uint8_t));


    }
    return rc;

}


/**
 */

/**
 */
int prvSchedLowLevelSetPortScheduling(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind,GT_BOOL quantumOnly)
{
    int rc =  -EFAULT;
    struct tm_port *port = NULL;

    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosHi)
    TM_REGISTER_VAR(TM_Sched_PortDWRRPrioEn)

    TM_CTL(ctl, hndl);



    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);

        /* DWRR for Port */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosLo , port_ind , TM_Sched_PortQuantumsPriosLo)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosLo , Quantum0 , port->dwrr_quantum_per_level[0].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosLo , Quantum1 , port->dwrr_quantum_per_level[1].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosLo , Quantum2 , port->dwrr_quantum_per_level[2].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosLo , Quantum3 , port->dwrr_quantum_per_level[3].quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosLo , port_ind , TM_Sched_PortQuantumsPriosLo)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosHi , port_ind , TM_Sched_PortQuantumsPriosHi)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum4 , port->dwrr_quantum_per_level[4].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum5 , port->dwrr_quantum_per_level[5].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum6 , port->dwrr_quantum_per_level[6].quantum)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortQuantumsPriosHi , Quantum7 , port->dwrr_quantum_per_level[7].quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosHi , port_ind , TM_Sched_PortQuantumsPriosHi)
        if (rc) goto out;

        if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_FALSE)         
        {
      
            if(quantumOnly == GT_FALSE)
             {  
                
                /* DWRR for C-nodes in Port's range */
                /* reset register reserved fields */
                TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDWRRPrioEn , port_ind , TM_Sched_PortDWRRPrioEn)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_REGISTER_SET(TM_Sched_PortDWRRPrioEn ,En , port->dwrr_enable_per_priority_mask)
                /* write register */
                TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDWRRPrioEn , port_ind , TM_Sched_PortDWRRPrioEn)
                if (rc) goto out;
             }

        }
    }
out:

    return rc;
}






/**
 */
int prvSchedLowLevelSetPortAllParameters(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    TM_CTL(ctl, hndl);
    if (port_ind < ctl->tm_total_ports)
    {
        rc = prvSchedLowLevelSetHwNodeMapping(hndl, P_LEVEL, port_ind);
        if (rc < 0)
            goto out;

        rc = prvSchedLowLevelSetPortShapingParametersToHw(hndl, port_ind);
        if (rc < 0)
            goto out;

        rc = prvSchedLowLevelSetPortScheduling(hndl, port_ind,GT_FALSE);
        if (rc < 0)
            goto out;


        rc = prvSchedLowLevelSetNodeEligPriorityFunc(hndl, P_LEVEL, port_ind);
    }
out:
    /*   reset_hw_connection(ctl); already done  */
    return rc;
}

int prvSchedLowLevelGetPortParams(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index, struct port_hw_data_t *port_hw_data)
{
    int rc = -EFAULT;

    TM_REGISTER_VAR(TM_Sched_PortRangeMap)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosHi)
    TM_REGISTER_VAR(TM_Sched_PortDWRRPrioEn)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    /* No Drop Profile Ptr for Port */

    DECLARE_TM_CTL_PTR(ctl, hndl)
    CHECK_TM_CTL_PTR(ctl)

    if (index < ctl->tm_total_ports)
    {
        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortRangeMap, index, TM_Sched_PortRangeMap);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortRangeMap, Hi, port_hw_data->last_child_c_node, (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortRangeMap, Lo, port_hw_data->first_child_c_node, (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr, index, TM_Sched_PortEligPrioFuncPtr);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFuncPtr, Ptr, port_hw_data->elig_prio_func_ptr, (uint8_t));

        /* DWRR for Port */
        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosLo, index, TM_Sched_PortQuantumsPriosLo);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosLo, Quantum0, port_hw_data->dwrr_quantum[0], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosLo, Quantum1, port_hw_data->dwrr_quantum[1], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosLo, Quantum2, port_hw_data->dwrr_quantum[2], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosLo, Quantum3, port_hw_data->dwrr_quantum[3], (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortQuantumsPriosHi, index, TM_Sched_PortQuantumsPriosHi);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum4, port_hw_data->dwrr_quantum[4], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum5, port_hw_data->dwrr_quantum[5], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum6, port_hw_data->dwrr_quantum[6], (uint16_t));
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortQuantumsPriosHi, Quantum7, port_hw_data->dwrr_quantum[7], (uint16_t));

        TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDWRRPrioEn, index, TM_Sched_PortDWRRPrioEn);
        if (rc)
            return rc;
        TXQ_SCHED_REGISTER_GET(TM_Sched_PortDWRRPrioEn, En, port_hw_data->dwrr_priority, (uint8_t));

        /* No Drop Profile Ptr for Port */
    }
    return rc;
}



/**
 */
int prvSchedLowLevelSetTreeDequeueStatusToHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_TreeDeqEn_Addr)

    TM_CTL(ctl, hndl)

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.TreeDeqEn_Addr, TM_Sched_TreeDeqEn_Addr)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_TreeDeqEn_Addr , En , ctl->tree_deq_status)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.TreeDeqEn_Addr, TM_Sched_TreeDeqEn_Addr )
    if (rc) goto out;
out:

    return rc;
}

int prvSchedLowLevelGetTreeDequeueStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_TreeDeqEn_Addr)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_REGISTER(addressSpace.Sched.TreeDeqEn_Addr, TM_Sched_TreeDeqEn_Addr)
    if (rc)  goto out;

    TXQ_SCHED_REGISTER_GET(TM_Sched_TreeDeqEn_Addr, En , ctl->tree_deq_status, (uint8_t) ) /* no casting */
out:

    return rc;
}

/**
 */
int prvSchedLowLevelSetTreeDwrrPriorityStatusToHw(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_PDWRREnReg)

    TM_CTL(ctl, hndl)

    /* reset register reserved fields */
    TXQ_SCHED_RESET_REGISTER(addressSpace.Sched.PDWRREnReg, TM_Sched_PDWRREnReg)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_PDWRREnReg , PrioEn , ctl->tree_dwrr_enable_per_priority_mask)
    /* write register */
    TXQ_SCHED_WRITE_REGISTER(addressSpace.Sched.PDWRREnReg, TM_Sched_PDWRREnReg)
    if (rc) goto out;
out:

    return rc;
}

int prvSchedLowLevelGetTreeDwrrPriorityStatusFromHw(PRV_CPSS_SCHED_HANDLE hndl)

{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_PDWRREnReg)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_REGISTER(addressSpace.Sched.PDWRREnReg, TM_Sched_PDWRREnReg)
    if (rc) goto out;

    TXQ_SCHED_REGISTER_GET(TM_Sched_PDWRREnReg , PrioEn , ctl->tree_dwrr_enable_per_priority_mask , (uint8_t) )
 out:

    return rc;
}


/**
 */








/*
    function  prvSchedLowLevelSetNodeShapingParametersToHw(hndl,level,node_ind, profile); is platform dependent - in file ...platform.c
*/

#define PROFILE_VALIDATION(nodes,nodes_count)   \
        NODE_VALIDATION(nodes_count)\
        if (rc==0)\
        {\
            sh_profile_index=ctl->nodes[node_ind].shaping_profile_ref;\
            if (prvSchedRmShapingProfileStatus(ctl->rm,sh_profile_index) == 1)\
                profile = &(ctl->tm_shaping_profiles[sh_profile_index]);\
            else\
                rc= -EFAULT;\
        }

int prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind)
{
    int rc = 0;
    uint32_t    sh_profile_index;
    struct prvCpssDxChTxqSchedShapingProfile *profile = NULL;

    TM_CTL(ctl, hndl)

    /* get shaper parameters from attached shaping profile */
    switch (level)
    {
    case Q_LEVEL:
        PROFILE_VALIDATION(tm_queue_array,ctl->tm_total_queues)
        if (rc) goto out;
        break;
    case A_LEVEL:
        PROFILE_VALIDATION(tm_a_node_array,ctl->tm_total_a_nodes)
        if (rc) goto out;
        break;
    case B_LEVEL:
        PROFILE_VALIDATION(tm_b_node_array,ctl->tm_total_b_nodes)
        if (rc) goto out;
        break;
    case C_LEVEL:
        PROFILE_VALIDATION(tm_c_node_array,ctl->tm_total_c_nodes)
        if (rc) goto out;
        break;
    default:
        rc =  -EFAULT;
        goto out;
        break;
    }
    rc = prvSchedLowLevelSetNodeShapingParametersToHw(hndl,level,node_ind, profile);
out:

    return rc;
}

/**
 */
int prvSchedLowLevelSetNodeEligPriorityFuncEx(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index,uint8_t elig_prio_func_ptr)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
    case P_LEVEL:
        if (node_index < ctl->tm_total_ports)
        {
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr , node_index , TM_Sched_PortEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr , node_index , TM_Sched_PortEligPrioFuncPtr)
        }
        break;
    case C_LEVEL:
        if (node_index < ctl->tm_total_c_nodes)
        {
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr , node_index , TM_Sched_ClvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr , node_index , TM_Sched_ClvlEligPrioFuncPtr)
        }
        break;
    case B_LEVEL:
        if (node_index < ctl->tm_total_b_nodes)
        {
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr , node_index , TM_Sched_BlvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr , node_index , TM_Sched_BlvlEligPrioFuncPtr)
        }
        break;
    case A_LEVEL:
        if (node_index < ctl->tm_total_a_nodes)
        {
        /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr , node_index , TM_Sched_AlvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr , node_index , TM_Sched_AlvlEligPrioFuncPtr)
        }
        break;
    case Q_LEVEL:
        if (node_index < ctl->tm_total_queues)
        {
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr , node_index , TM_Sched_QueueEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFuncPtr , Ptr , elig_prio_func_ptr)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr , node_index , TM_Sched_QueueEligPrioFuncPtr)
        }
        break;
    }

    return rc;
}

/**
 */
int prvSchedLowLevelSetNodeEligPriorityFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index)
{
    int rc =  -EFAULT;

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
        case P_LEVEL:
        {
            ctl->tm_port_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_port_array[node_index].elig_prio_func);
        }
        case C_LEVEL:
        {
            ctl->tm_c_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_c_node_array[node_index].elig_prio_func);
        }
        case B_LEVEL:
        {
            ctl->tm_b_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_b_node_array[node_index].elig_prio_func);
        }
        case A_LEVEL:
        {
            ctl->tm_a_node_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_a_node_array[node_index].elig_prio_func);
        }
        case Q_LEVEL:
        {
            ctl->tm_queue_array[node_index].node_temporary_disabled = 0;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, ctl->tm_queue_array[node_index].elig_prio_func);
        }
        default: break;
    }

    return rc;
}

int prvSchedLowLevelDisableNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel node_level, uint32_t node_index)
{
    int rc =  -EFAULT;

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
        case P_LEVEL:
        {
            ctl->tm_port_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case C_LEVEL:
        {
             ctl->tm_c_node_array[node_index].node_temporary_disabled = 1;
             return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case B_LEVEL:
        {
            ctl->tm_b_node_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case A_LEVEL:
        {
            ctl->tm_a_node_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        case Q_LEVEL:
        {
            ctl->tm_queue_array[node_index].node_temporary_disabled = 1;
            return prvSchedLowLevelSetNodeEligPriorityFuncEx(hndl, node_level, node_index, TM_NODE_DISABLED_FUN);
        }
        default: break;
    }

    return rc;
}

/**
 */
int prvSchedLowLevelDisablePortEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    /* Disable Ports */
    for (i = 0; i < (int)total_ports; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr , i , TM_Sched_PortEligPrioFuncPtr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFuncPtr , Ptr , TM_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFuncPtr , i , TM_Sched_PortEligPrioFuncPtr)
        if (rc) goto out;
    }
out:

    return rc;
}

int prvSchedLowLevelSetDefaultPortMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_ports)
{   
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_PortRangeMap)

    TM_CTL(ctl, hndl)
    
    /* Default map p[i] to c[i]*/
    for (i = 0; i < (int)total_ports; i++)
    {
        /* reset register reserved fields */
         TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortRangeMap , i , TM_Sched_PortRangeMap)
         if (rc) goto out;
        
           /* assign register fields */        
           TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap , Lo , i)
           TXQ_SCHED_REGISTER_SET(TM_Sched_PortRangeMap, Hi , i)              
        
            /* write register */
          TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortRangeMap , i , TM_Sched_PortRangeMap)           

    }
out:
      
    return rc;
}


int prvSchedLowLevelSetDefaultCnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes)
{   
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_ClvltoPortAndBlvlRangeMap)

    TM_CTL(ctl, hndl)
    
    /* Default map c[i] to b[i] , parent p[i]*/
    for (i = 0; i < (int)total_c_nodes; i++)
    {
             /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap , i , TM_Sched_ClvltoPortAndBlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */
         
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , Port , i)
   
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlLo , i)
            TXQ_SCHED_REGISTER_SET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlHi , i)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvltoPortAndBlvlRangeMap , i , TM_Sched_ClvltoPortAndBlvlRangeMap)

    }
out:
      
    return rc;
}


int prvSchedLowLevelSetDefaultBnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes)
{   
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_BLvltoClvlAndAlvlRangeMap)

    TM_CTL(ctl, hndl)
    
    /* Default map b[i] to b[i] , parent p[i]*/
    for (i = 0; i < (int)total_b_nodes; i++)
    {
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap , index , TM_Sched_BLvltoClvlAndAlvlRangeMap)
            if (rc) goto out;
            /* assign register fields */  
            
            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , Clvl , i)

            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlLo , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
            TXQ_SCHED_REGISTER_SET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlHi , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
             
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BLvltoClvlAndAlvlRangeMap , i , TM_Sched_BLvltoClvlAndAlvlRangeMap)
        

    }
out:
      
    return rc;
}


int prvSchedLowLevelSetDefaultAnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_a_nodes)
{   
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_ALvltoBlvlAndQueueRangeMap)

    TM_CTL(ctl, hndl)
    
    /* Default map b[i] to b[i] , parent p[i]*/
    for (i = 0; i < (int)total_a_nodes; i++)
    {
             /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap , i , TM_Sched_ALvltoBlvlAndQueueRangeMap)
            if (rc) goto out;
 
            /* assign register fields */        
            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap , Blvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_B_NODE_MAC)

            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap, QueueLo ,PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)
            TXQ_SCHED_REGISTER_SET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueHi , PRV_CPSS_DXCH_TXQ_SCHED_NULL_Q_NODE_MAC)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ALvltoBlvlAndQueueRangeMap , i , TM_Sched_ALvltoBlvlAndQueueRangeMap)
        

    }
out:
      
    return rc;
}


int prvSchedLowLevelSetDefaultQnodeMapping(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_q_nodes)
{   
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_QueueAMap)

    TM_CTL(ctl, hndl)
    
 
    for (i = 0; i < (int)total_q_nodes; i++)
    {
                
            /* reset register reserved fields */
            TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueAMap , i , TM_Sched_QueueAMap)
            if (rc) goto out;
            /* assign register fields */        
            TXQ_SCHED_REGISTER_SET(TM_Sched_QueueAMap , Alvl , PRV_CPSS_DXCH_TXQ_SCHED_NULL_A_NODE_MAC)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueAMap , i , TM_Sched_QueueAMap)
        

    }
out:
      
    return rc;
}



/**
 *  Configure user Q level Eligible Priority Function
 */
int prvSchedLowLevelSetQLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    struct tm_elig_prio_func_queue *params;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFunc)

    TM_CTL(ctl, hndl)


    params = &(ctl->tm_elig_prio_q_lvl_tbl[func_offset]);
    /* reset register reserved fields */
    TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFunc, func_offset, TM_Sched_QueueEligPrioFunc)
    if (rc) goto out;
    /* assign register fields */
    TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFunc , FuncOut0 ,  params->tbl_entry.func_out[0])
    TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFunc , FuncOut1 ,  params->tbl_entry.func_out[1])
    TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFunc , FuncOut2 ,  params->tbl_entry.func_out[2])
    TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFunc , FuncOut3 ,  params->tbl_entry.func_out[3])
    /* write register */
    TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFunc, func_offset, TM_Sched_QueueEligPrioFunc)
    if (rc)
        goto out;

out:

    return rc;
}

/**
 *  Configure user Node level Eligible Priority Function
 */
int prvSchedLowLevelSetALevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)



    for (i = 0; i < 8; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFunc_Entry, func_offset + i*64, TM_Sched_AlvlEligPrioFunc_Entry)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_a_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_AlvlEligPrioFunc_Entry)
        /* write register */
        if (rc) goto out;
    }
out:

    return rc;
}

int prvSchedLowLevelSetBLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

     if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)
     {
            return 0;
     }

    for (i = 0; i < 8; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_BlvlEligPrioFunc_Entry);
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_b_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_BlvlEligPrioFunc_Entry)
        if (rc) goto out;
    }
out:

    return rc;
}

int prvSchedLowLevelSetCLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

     if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)
     {
            return 0;
     }

    for (i = 0; i < 8; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFunc_Entry, func_offset + i*64,TM_Sched_ClvlEligPrioFunc_Entry);
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[0]);
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[1]);
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[2]);
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_c_lvl_tbl[func_offset].tbl_entry[i].func_out[3]);
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_ClvlEligPrioFunc_Entry);
        if (rc) goto out;
    }
out:

    return rc;
}

int prvSchedLowLevelSetPLevelEligPrioFuncEntry(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_PortEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

     if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)
     {
            return 0;
     }

    for (i = 0; i < 8; i++)
    {   /* Entry ID */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_PortEligPrioFunc_Entry)
        if (rc) goto out;
        /* assign register fields */
        /* write register */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFunc_Entry , FuncOut0 , ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[0])
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFunc_Entry , FuncOut1 , ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[1])
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFunc_Entry , FuncOut2 , ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[2])
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortEligPrioFunc_Entry , FuncOut3 , ctl->tm_elig_prio_p_lvl_tbl[func_offset].tbl_entry[i].func_out[3])
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFunc_Entry , func_offset + i*64 ,TM_Sched_PortEligPrioFunc_Entry);
        if (rc) goto out;
    }
out:

    return rc;
}



int prvSchedLowLevelSetQLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    uint16_t j;
    int rc = 0;

    for (j = 0; j < TM_ELIG_Q_SHP /*TM_ELIG_FUNC_TABLE_SIZE - only 16 functions are required*/; j++)
    {   
        
        rc = prvSchedLowLevelSetQLevelEligPrioFuncEntry(hndl, j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int prvSchedLowLevelSetALevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;



    TM_CTL(ctl, hndl)

#if 0
    if(is_sip_6 == 0)
    {
        for (j=0; j< TM_ELIG_FUNC_TABLE_SIZE; j++)
        {
            rc = prvSchedLowLevelSetALevelEligPrioFuncEntry(hndl,j);
            if (rc) break;
        }
    }
#endif

/*Two functions -with and w/o shaping*/


  rc = prvSchedLowLevelSetALevelEligPrioFuncEntry(hndl,TM_ELIG_N_PRIO1);
  if (rc) goto out;

   rc = prvSchedLowLevelSetALevelEligPrioFuncEntry(hndl,TM_ELIG_N_MIN_SHP_FP1);
  if (rc) goto out;

out:

    return rc;
}


/**
 */
int prvSchedLowLevelSetBLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;

    TM_CTL(ctl, hndl)

#if 0
    if(is_sip_6 == 0)
    {
        for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
        {
            rc = prvSchedLowLevelSetBLevelEligPrioFuncEntry(hndl,j);
            if (rc) break;
        }
    }
#endif

    rc = prvSchedLowLevelSetBLevelEligPrioFuncEntry(hndl,TM_ELIG_N_PRIO1);

    return rc;
}


/**
 */
int prvSchedLowLevelSetCLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;


    TM_CTL(ctl, hndl)

#if 0

    if(is_sip_6 == 0)
    {

        for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
        {
            rc = prvSchedLowLevelSetCLevelEligPrioFuncEntry(hndl,j);
            if (rc) break;
        }
    }
#endif

    rc = prvSchedLowLevelSetCLevelEligPrioFuncEntry(hndl,TM_ELIG_N_PRIO1);


    return rc;
}


/**
 */
int prvSchedLowLevelSetPLevelEligPrioFuncAllTable(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc =  -EFAULT;



    TM_CTL(ctl, hndl)

#if 0

    if(is_sip_6 == 0)
    {
        for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
        {
            rc = prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl,j);
            if (rc) break;
        }
    }
 #endif

    rc = prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl,TM_ELIG_N_PRIO1);

    return rc;
}


/**
 */
int prvSchedLowLevelClearPLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint8_t index)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_PortDefPrio0)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio1)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio2)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio3)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio4)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio5)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio6)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio7)

    TM_CTL(ctl, hndl)

    if (index < ctl->tm_total_ports)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio0, index, TM_Sched_PortDefPrio0)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio0 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio0, index, TM_Sched_PortDefPrio0)
        if (rc) goto out;

         /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio1, index, TM_Sched_PortDefPrio1)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio1 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio1, index, TM_Sched_PortDefPrio1)
        if (rc) goto out;


        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio2, index, TM_Sched_PortDefPrio2)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio2 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio2, index, TM_Sched_PortDefPrio2)
        if (rc) goto out;

         /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio3, index, TM_Sched_PortDefPrio3)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio3 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio3, index, TM_Sched_PortDefPrio3)
        if (rc) goto out;


        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio4, index, TM_Sched_PortDefPrio4)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio4 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio4, index, TM_Sched_PortDefPrio4)
        if (rc) goto out;

         /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio5, index, TM_Sched_PortDefPrio5)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio5 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio5, index, TM_Sched_PortDefPrio5)
        if (rc) goto out;


        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio6, index, TM_Sched_PortDefPrio6)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio2 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio6, index, TM_Sched_PortDefPrio6)
        if (rc) goto out;

         /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.PortDefPrio3, index, TM_Sched_PortDefPrio7)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortDefPrio7 , Deficit , 0x1)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.PortDefPrio7, index, TM_Sched_PortDefPrio7)
        if (rc) goto out;
        
    }

out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearCLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_CLvlDef)

    TM_CTL(ctl, hndl)

    if (index < ctl->tm_total_c_nodes)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.CLvlDef, index, TM_Sched_CLvlDef)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_CLvlDef , Deficit , 0x1)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.CLvlDef, index, TM_Sched_CLvlDef)
        if (rc) goto out;
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearBLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_BlvlDef)

    TM_CTL(ctl, hndl)

    if (index < ctl->tm_total_b_nodes)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlDef, index, TM_Sched_BlvlDef)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlDef , Deficit , 0x1)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlDef, index, TM_Sched_BlvlDef)
        if (rc)  goto out;
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearALevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_AlvlDef)

    TM_CTL(ctl, hndl)
    if (index < ctl->tm_total_a_nodes)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlDef, index, TM_Sched_AlvlDef)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlDef , Deficit , 0x1)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlDef, index, TM_Sched_AlvlDef)
        if (rc)
          goto out;
    }
out:

    return rc;
}


/**
 */
int prvSchedLowLevelClearQLevelDeficit(PRV_CPSS_SCHED_HANDLE hndl, uint32_t index)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_QueueDef)

    TM_CTL(ctl, hndl)

    if (index < ctl->tm_total_queues)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueDef, index, TM_Sched_QueueDef)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueDef , Deficit , 0x1)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueDef, index, TM_Sched_QueueDef)
        if (rc)
          goto out;
    }
out:

    return rc;
}










int prvSchedLowLevelGetGeneralHwParams(PRV_CPSS_SCHED_HANDLE hndl)
{


    TM_CTL(ctl, hndl)

    /*fixed to 0,no package in Falcon*/
    ctl->min_pkg_size = 0;

    return 0;

}


/**
 */
int prvSchedLowLevelGetPortStatus(PRV_CPSS_SCHED_HANDLE hndl,
                     uint8_t index,
                     struct tm_port_status *tm_status)
{


    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_PortShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio0)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio1)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio2)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio3)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio4)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio5)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio6)
    TM_REGISTER_VAR(TM_Sched_PortDefPrio7)


    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortShpBucketLvls, index , TM_Sched_PortShpBucketLvls)
    if (rc) return rc;

    TXQ_SCHED_REGISTER_GET(TM_Sched_PortShpBucketLvls, MinLvl , tm_status->min_bucket_level , (uint32_t) ) /* casting to uint32_t */
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortShpBucketLvls, MaxLvl , tm_status->max_bucket_level , (uint32_t) )  /* casting to uint32_t */
    
    /*Prio 0*/
    
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio0, index , TM_Sched_PortDefPrio0)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio0, Deficit , tm_status->deficit[0]  ,  (uint32_t) )  /* casting to uint32_t */

    /*Prio 1*/

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio1, index , TM_Sched_PortDefPrio1)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio1, Deficit , tm_status->deficit[1]  ,  (uint32_t) )  /* casting to uint32_t */

     /*Prio 2*/
    
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio2, index , TM_Sched_PortDefPrio2)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio2, Deficit , tm_status->deficit[2]  ,  (uint32_t) )  /* casting to uint32_t */

    /*Prio 3*/

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio3, index , TM_Sched_PortDefPrio3)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio3, Deficit , tm_status->deficit[3]  ,  (uint32_t) )  /* casting to uint32_t */

    /*Prio 4*/
    
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio4, index , TM_Sched_PortDefPrio4)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio4, Deficit , tm_status->deficit[4]  ,  (uint32_t) )  /* casting to uint32_t */

    /*Prio 5*/

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio5, index , TM_Sched_PortDefPrio5)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio5, Deficit , tm_status->deficit[5]  ,  (uint32_t) )  /* casting to uint32_t */

     /*Prio 6*/
    
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio6, index , TM_Sched_PortDefPrio6)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio6, Deficit , tm_status->deficit[6]  ,  (uint32_t) )  /* casting to uint32_t */

    /*Prio 7*/

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortDefPrio7, index , TM_Sched_PortDefPrio7)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_PortDefPrio7, Deficit , tm_status->deficit[7]  ,  (uint32_t) )  /* casting to uint32_t */
    
    return rc;


}


/**
 */
int prvSchedLowLevelGetCNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_ClvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_CLvlDef)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlShpBucketLvls, index , TM_Sched_ClvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.CLvlDef, index , TM_Sched_CLvlDef)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_CLvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    return rc;
}


/**
 */
int prvSchedLowLevelGetBNodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_BlvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_BlvlDef)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlShpBucketLvls, index , TM_Sched_BlvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlDef, index , TM_Sched_BlvlDef)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */
    return rc;
}


/**
 */
int prvSchedLowLevelGetANodeStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;

    uint32_t lineNum = index/32;
    uint32_t bitOffset = index%32;

    TM_REGISTER_VAR(TM_Sched_AlvlShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_AlvlDef)
    TM_REGISTER_VAR(TM_Sched_ALevelShaperBucketNeg)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlShpBucketLvls, index , TM_Sched_AlvlShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlDef, index , TM_Sched_AlvlDef)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ALevelShaperBucketNeg, lineNum , TM_Sched_ALevelShaperBucketNeg)
    if (rc)
        return rc;

    TXQ_SCHED_REGISTER_GET(TM_Sched_ALevelShaperBucketNeg, MinTBNeg, tm_status->min_bucket_sign ,  (uint32_t) )  /* casting to uint32_t */

    tm_status->min_bucket_sign>>=bitOffset;
    tm_status->min_bucket_sign&=0x1;
    return rc;
}


/**
 */
int prvSchedLowLevelGetQueueStatus(PRV_CPSS_SCHED_HANDLE hndl,
                       uint32_t index,
                       struct schedNodeStatus *tm_status)
{
    int rc =  -EFAULT;

    uint32_t lineNum = index/32;
    uint32_t bitOffset = index%32;

    TM_REGISTER_VAR(TM_Sched_QueueShpBucketLvls)
    TM_REGISTER_VAR(TM_Sched_QueueDef)
    TM_REGISTER_VAR(TM_Sched_QueueShaperBucketNeg)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueShpBucketLvls, index , TM_Sched_QueueShpBucketLvls)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueShpBucketLvls, MinLvl , tm_status->min_bucket_level ,  (uint32_t) )  /* casting to uint32_t */
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueShpBucketLvls, MaxLvl , tm_status->max_bucket_level ,  (uint32_t) )  /* casting to uint32_t */

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueDef, index , TM_Sched_QueueDef)
    if (rc)
        return rc;
    
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueDef, Deficit, tm_status->deficit ,  (uint32_t) )  /* casting to uint32_t */

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueShaperBucketNeg, lineNum , TM_Sched_QueueShaperBucketNeg)
    if (rc)
        return rc;

    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueShaperBucketNeg, MinTBNeg, tm_status->min_bucket_sign ,  (uint32_t) )  /* casting to uint32_t */

    tm_status->min_bucket_sign>>=bitOffset;
    tm_status->min_bucket_sign&=0x1;
    
    return rc;
}






/**
 */





/**
 */
int prvSchedLowLevelGetSchedErrors(PRV_CPSS_SCHED_HANDLE hndl, struct tm_error_info *info)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_ErrCnt_Addr)
    TM_REGISTER_VAR(TM_Sched_ExcpCnt_Addr)

    TM_CTL(ctl, hndl)

    TXQ_SCHED_READ_REGISTER(addressSpace.Sched.ErrCnt_Addr , TM_Sched_ErrCnt_Addr)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_ErrCnt_Addr , Cnt , info->error_counter ,(uint16_t) )  /* casting to uint16_t */

    TXQ_SCHED_READ_REGISTER(addressSpace.Sched.ExcpCnt_Addr , TM_Sched_ExcpCnt_Addr)
    if (rc)
        return rc;
    TXQ_SCHED_REGISTER_GET(TM_Sched_ExcpCnt_Addr, Cnt , info->exception_counter ,(uint16_t) )  /* casting to uint16_t */

    return rc;
}




/**
 */
int prvSchedLowLevelGetSchedStatus
(
    PRV_CPSS_SCHED_HANDLE hndl, 
    uint32_t * sched_status_ptr
)
{
    int rc =  -EFAULT;
    uint32_t data[2];

    TM_CTL(ctl, hndl)

    rc = prvSchedRegisterRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(addressSpace.Sched.ErrorStatus_Addr), data);
    if (rc)
        return rc;

   *sched_status_ptr = data[0];

    return rc;
}









/**
 * get A to P level Eligible Priority Function
 */
int prvSchedLowLevelGetNodeEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint16_t func_offset, struct tm_elig_prio_func_node *params)
{
    int rc = -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFunc_Entry)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFunc_Entry)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFunc_Entry)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    switch (level) {
        case Q_LEVEL:
            rc = -EFAULT;
            break;
        case A_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_AlvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut0 , params->tbl_entry[i].func_out[0], (uint16_t))
                TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut1 , params->tbl_entry[i].func_out[1], (uint16_t))
                TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut2 , params->tbl_entry[i].func_out[2], (uint16_t))
                TXQ_SCHED_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut3 , params->tbl_entry[i].func_out[3], (uint16_t))
            }
            break;
        case B_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_BlvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
            }
            break;
        case C_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_ClvlEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
            }
            break;
        case P_LEVEL:
            for (i = 0; i < 8; i++) {
                TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.PortEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_PortEligPrioFunc_Entry)
                if (rc) goto out;
                TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry, FuncOut0, params->tbl_entry[i].func_out[0], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry, FuncOut1, params->tbl_entry[i].func_out[1], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry, FuncOut2, params->tbl_entry[i].func_out[2], (uint16_t));
                TXQ_SCHED_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry, FuncOut3, params->tbl_entry[i].func_out[3], (uint16_t));
            }
            break;
        default:
            goto out;
        }
out:
    return rc;
}

/**
 * get q level Eligible Priority Function
 */
int prvSchedLowLevelGetQueueEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset, struct tm_elig_prio_func_queue *params)
{
    int rc = -EFAULT;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFunc)

    TM_CTL(ctl, hndl);

    TXQ_SCHED_READ_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFunc, func_offset  ,TM_Sched_QueueEligPrioFunc)
    if (rc) goto out;
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFunc, FuncOut0, params->tbl_entry.func_out[0], (uint16_t));
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFunc, FuncOut1, params->tbl_entry.func_out[1], (uint16_t));
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFunc, FuncOut2, params->tbl_entry.func_out[2], (uint16_t));
    TXQ_SCHED_REGISTER_GET(TM_Sched_QueueEligPrioFunc, FuncOut3, params->tbl_entry.func_out[3], (uint16_t));

out:
    return rc;
}






int prvSchedLowLevelSetANodeQuantum
(
    PRV_CPSS_SCHED_HANDLE hndl, 
    uint32_t node_ind
)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;

    TM_REGISTER_VAR(TM_Sched_AlvlQuantum)

    TM_CTL(ctl, hndl)

      if(PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR() == GT_TRUE)         
      {                          
              
        return 0;        
     }

    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum , node_ind , TM_Sched_AlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlQuantum , Quantum , node->dwrr_quantum)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlQuantum , node_ind , TM_Sched_AlvlQuantum)
        if (rc) goto out;

    }

out:

    return rc;
}


int prvSchedLowLevelDisableAlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_a_nodes)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    /* Disable Ports */
    for (i = 0; i < (int)total_a_nodes; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr , i , TM_Sched_AlvlEligPrioFuncPtr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , TM_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.AlvlEligPrioFuncPtr , i , TM_Sched_AlvlEligPrioFuncPtr)
        if (rc) goto out;
    }
out:

    return rc;
}


int prvSchedLowLevelDisableBlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_b_nodes)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    /* Disable Ports */
    for (i = 0; i < (int)total_b_nodes; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr , i , TM_Sched_BlvlEligPrioFuncPtr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , TM_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.BlvlEligPrioFuncPtr , i , TM_Sched_BlvlEligPrioFuncPtr)
        if (rc) goto out;
    }
out:

    return rc;
}


int prvSchedLowLevelDisableQlevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_q_nodes)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    /* Disable Ports */
    for (i = 0; i < (int)total_q_nodes; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr , i , TM_Sched_QueueEligPrioFuncPtr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_QueueEligPrioFuncPtr , Ptr , TM_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.QueueEligPrioFuncPtr , i , TM_Sched_QueueEligPrioFuncPtr)
        if (rc) goto out;
    }
out:

    return rc;
}


int prvSchedLowLevelDisableClevelEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl, uint32_t total_c_nodes)
{
    int rc =  -EFAULT;
    int i;

    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    /* Disable Ports */
    for (i = 0; i < (int)total_c_nodes; i++)
    {
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr , i , TM_Sched_ClvlEligPrioFuncPtr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , TM_ELIG_DEQ_DISABLE)  /* DeQ disable function ID */
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpace.Sched.ClvlEligPrioFuncPtr , i , TM_Sched_ClvlEligPrioFuncPtr)
        if (rc) goto out;
    }
out:

    return rc;
}






























