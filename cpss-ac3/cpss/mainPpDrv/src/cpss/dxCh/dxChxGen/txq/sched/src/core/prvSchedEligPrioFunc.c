/*
 * (c), Copyright 2011-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief tm eligible functions API implementation 
 *
* @file tm_elig_prio_func.c
*
* $Revision: 2.0 $
 */

#include <stdlib.h>
#include <errno.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedEligPrioFunc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>



/** Configure the Eligible Priority Function according 
*   to the User Application parameters
*
*	The new Eligible Priority Function is written to the 
*	proper Queue or Node level Eligible Table. 
*	A pointer in the API is provided to the location of the new Eligible Priority Function in the table.
*	
*	Note: 	The use of Eligible Function Pointer 63 on both Queue and Node level is forbidden. 
*			Eligible Function 63 is reserved to indicate that Queue/Node is not eligible.
*
*   @param[in]	hndl					TM lib handle
*	@param[in]	elig_prio_func_ptr		The new configured eligible function pointer
*	@param[in]	level					A level to configure the Eligible function with	
*	@param[in]	func_out_arr			The Eligible Priority Function structure array pointer
*	
*   @retval zero on success. 
*   @retval -EINVAL if hndl is NULL.
*   @retval -EBADF if hndl is an invalid handle.
*	@retval -EDOM  if (elig_prio_func_ptr > 63)
*   @retval -EADDRNOTAVAIL if level is invalid
*	@retval TM_HW_ELIG_PRIO_FUNC_FAILED when the configuration to the HW failed
*/

static int prvSchedEligPrioFuncConfig(PRV_CPSS_SCHED_HANDLE hndl,
							 uint16_t elig_prio_func_ptr,
							 enum schedLevel level,
							 union tm_elig_prio_func *func_out_arr);


int prvSchedConfigEligPrioFuncTable(PRV_CPSS_SCHED_HANDLE hndl, int updateHW)
{
    int rc = 0;

    TM_CTL(ctl, hndl);

	/* Update SW image */

    prvSchedInitDefaultQueueEligPrioFuncTable(ctl->tm_elig_prio_q_lvl_tbl);

	prvSchedInitDefaultNodeEligPrioFuncTable(ctl->tm_elig_prio_a_lvl_tbl);
	prvSchedInitDefaultNodeEligPrioFuncTable(ctl->tm_elig_prio_b_lvl_tbl);
	prvSchedInitDefaultNodeEligPrioFuncTable(ctl->tm_elig_prio_c_lvl_tbl);
	prvSchedInitDefaultNodeEligPrioFuncTable(ctl->tm_elig_prio_p_lvl_tbl);

	if (updateHW)
	{
		/* update HW */
		rc = prvSchedLowLevelSetQLevelEligPrioFuncAllTable(hndl);
		if (rc) return rc;
		rc = prvSchedLowLevelSetALevelEligPrioFuncAllTable(hndl);
		if (rc) return rc;
		rc = prvSchedLowLevelSetBLevelEligPrioFuncAllTable(hndl);
		if (rc) return rc;
		rc = prvSchedLowLevelSetCLevelEligPrioFuncAllTable(hndl);
		if (rc) return rc;
		rc = prvSchedLowLevelSetPLevelEligPrioFuncAllTable(hndl);
		if (rc) return rc;
	}
    if (rc)
        rc = -TM_HW_ELIG_PRIO_FUNC_FAILED;
	return rc;
}



	
/**
 */
static int prvSchedEligPrioFuncConfig(PRV_CPSS_SCHED_HANDLE hndl,
							uint16_t elig_prio_func_index,
							enum schedLevel level,
							union tm_elig_prio_func *func_value_arr)
{
    int rc = 0;

    TM_CTL(ctl, hndl);

	if (elig_prio_func_index >= TM_ELIG_FUNC_TABLE_SIZE) return  -EDOM;
	/* TM_ELIG_Q_DEQ_DIS function udate is not allowed */
	if (elig_prio_func_index == TM_ELIG_DEQ_DISABLE) return  -EDOM;
	/* TM_NODE_DISABLED_FUN  slot is for internal use */
	if (elig_prio_func_index == TM_NODE_DISABLED_FUN) return  -EDOM;
	
	switch (level)
	{		
		case Q_LEVEL:
		{
			/* Update SW image for queue level */
			prvSchedSetSwQEligPrioFunc(ctl->tm_elig_prio_q_lvl_tbl, elig_prio_func_index, func_value_arr);
			/* update HW */
			rc = prvSchedLowLevelSetQLevelEligPrioFuncEntry(hndl, elig_prio_func_index);		
			break;
		}
		case A_LEVEL:
		{
			/* Update SW image for A node level */
			prvSchedSetSwNodeEligPrioFunc(ctl->tm_elig_prio_a_lvl_tbl, elig_prio_func_index, func_value_arr);
			/* update HW */
			rc=prvSchedLowLevelSetALevelEligPrioFuncEntry(hndl, elig_prio_func_index);
			break;
		}
		case B_LEVEL:
		{
			/* Update SW image for B node level */
			prvSchedSetSwNodeEligPrioFunc(ctl->tm_elig_prio_b_lvl_tbl, elig_prio_func_index, func_value_arr);
			/* update HW */
			rc=prvSchedLowLevelSetBLevelEligPrioFuncEntry(hndl, elig_prio_func_index);
			break;
		}
		case C_LEVEL:
		{
			/* Update SW image for C node level */
			prvSchedSetSwNodeEligPrioFunc(ctl->tm_elig_prio_c_lvl_tbl, elig_prio_func_index, func_value_arr);
			/* update HW */
			rc=prvSchedLowLevelSetCLevelEligPrioFuncEntry(hndl, elig_prio_func_index);
			break;
		}
		case P_LEVEL:
		{
			/* Update SW image for Port level */
			prvSchedSetSwNodeEligPrioFunc(ctl->tm_elig_prio_p_lvl_tbl, elig_prio_func_index, func_value_arr);
			/* update HW */
			rc=prvSchedLowLevelSetPLevelEligPrioFuncEntry(hndl, elig_prio_func_index);
			break;
		}
		default: return -EADDRNOTAVAIL;
	}
	if (rc)
		rc = -TM_HW_ELIG_PRIO_FUNC_FAILED;
	return rc;
}


/**
 */
int prvSchedConfigAllLevelsEligPrioFunc(PRV_CPSS_SCHED_HANDLE hndl,
									uint16_t elig_prio_func_ptr,
									union tm_elig_prio_func *func_value_arr)
{

	enum schedLevel lvl;
	int rc = 0;

    TM_CTL(ctl, hndl);

	if (elig_prio_func_ptr >= TM_ELIG_FUNC_TABLE_SIZE)
	{
		rc = -EDOM;
		goto out;
	}

	for (lvl = A_LEVEL; lvl <= P_LEVEL; lvl++)
	{
		rc = prvSchedEligPrioFuncConfig(ctl, elig_prio_func_ptr, lvl, func_value_arr);
		if (rc)
		{
            rc = -TM_HW_ELIG_PRIO_FUNC_FAILED;
			goto out;
        }
	}
out:
	return rc;
}




/**
 * Show A to P level Eligible Priority Function
 */
static int prvSchedDebugShowHwEligFunc(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint16_t func_offset)
{
	int rc = -EFAULT;
	struct tm_elig_prio_func_node tm_elig_prio_lvl_tbl;
	struct tm_elig_prio_func_node *params = &tm_elig_prio_lvl_tbl;
	int i;
	uint16_t elig_func_val_0;
	uint16_t elig_func_val_1;
	uint16_t elig_func_val_2;
	uint16_t elig_func_val_3;

    TM_CTL(ctl, hndl);

	rc = prvSchedLowLevelGetNodeEligPrioFunc(hndl, level, func_offset, params);

	if (rc)
		goto out;

	schedPrintf("Function: %d\n", func_offset);

	for (i = 0; i < 8; i++) {
		elig_func_val_0 = params->tbl_entry[i].func_out[0];
		elig_func_val_1 = params->tbl_entry[i].func_out[1];
		elig_func_val_2 = params->tbl_entry[i].func_out[2];
		elig_func_val_3 = params->tbl_entry[i].func_out[3];

		schedPrintf("%02d: 0x%03x  %02d: 0x%03x  %02d: 0x%03x  %02d: 0x%03x\n",
				(i*4), elig_func_val_0,
				(i*4) + 1, elig_func_val_1,
				(i*4) + 2, elig_func_val_2,
				(i*4) + 3, elig_func_val_3);
	}

out:
	return rc;
}

/**
 * Show deatailed A to P level Eligible Priority Function header
 */
static void prvSchedDebugPrintEligFuncHeader(void)
{
	/* in - MinTBNeg, MaxTBNeg, PropPrio */
	/* out - Elig, SchdPrio, PropPrio, MinTBUsed, MaxTBUsed */
	schedPrintf("      Input            OutPut          Value\n");
	schedPrintf("    ---------     -----------------    -----\n");
	schedPrintf("    M   M   P     E   S   P   M   M\n");
	schedPrintf("    i   a   r     l   c   r   i   a\n");
	schedPrintf("    n   x   o     i   h   o   n   x\n");
	schedPrintf("    T   T   p     g   d   p   T   T\n");
	schedPrintf("    B   B   P         P   P   B   B\n");
	schedPrintf("    N   N   r         r   r   U   U\n");
	schedPrintf("    e   e   i         i   i   s   s\n");
	schedPrintf("    g   g   o         o   o   e   e\n");
	schedPrintf("    -   -   -     -   -   -   -   -\n");
}

/**
 * Show deatailed A to P level Eligible Priority Function
 */
static int prvSchedDebugShowEligFuncDetailed(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint16_t func_offset)
{
	int rc = -EFAULT;
	struct tm_elig_prio_func_out elig_func_out[4];
	struct tm_elig_prio_func_node tm_elig_prio_lvl_tbl;
	struct tm_elig_prio_func_node *params = &tm_elig_prio_lvl_tbl;	
	int i,j;
	int entry_index;
	uint16_t elig_val[4];
	int prop_prio;
	
	rc = prvSchedLowLevelGetNodeEligPrioFunc(hndl, level, func_offset, params);

	if (rc)
		goto out;

	schedPrintf("Function: %d\n", func_offset);
	prvSchedDebugPrintEligFuncHeader();

	for (i = 0; i < 8; i++) {
		prvSchedConvertValueToEligFunc(params->tbl_entry[i].func_out[0], &elig_func_out[0]);
		prvSchedConvertValueToEligFunc(params->tbl_entry[i].func_out[1], &elig_func_out[1]);
		prvSchedConvertValueToEligFunc(params->tbl_entry[i].func_out[2], &elig_func_out[2]);
		prvSchedConvertValueToEligFunc(params->tbl_entry[i].func_out[3], &elig_func_out[3]);

		elig_val[0] = params->tbl_entry[i].func_out[0];
		elig_val[1] = params->tbl_entry[i].func_out[1];
		elig_val[2] = params->tbl_entry[i].func_out[2];
		elig_val[3] = params->tbl_entry[i].func_out[3];

		for (j = 0; j < 4; j++) {
			entry_index = i*4 + j;	
			prop_prio = entry_index & 7;
			schedPrintf("%02d: %d   %d   %d     %d   %d   %d   %d   %d   0x%03x\n",
					entry_index,
					entry_index / 16,
					(entry_index / 8) %2,
					prop_prio,
					elig_func_out[j].elig,
					elig_func_out[j].sched_prio,
					elig_func_out[j].prop_prio,
					elig_func_out[j].min_tb,
					elig_func_out[j].max_tb,
					elig_val[j]);
		}
	}

out:
	return rc;
}


/**
 * Show Q level Eligible Priority Function
 */
static int prvSchedDebugShowHwEligPrioQFunc(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
	int rc = -EFAULT;
	struct tm_elig_prio_func_queue tm_elig_prio_q_tbl;
	struct tm_elig_prio_func_queue *params = &tm_elig_prio_q_tbl;
	uint16_t elig_func_val_0;
	uint16_t elig_func_val_1;
	uint16_t elig_func_val_2;
	uint16_t elig_func_val_3;

	rc = prvSchedLowLevelGetQueueEligPrioFunc(hndl, func_offset, params);

	if (rc)
		goto out;

	elig_func_val_0 = params->tbl_entry.func_out[0];
	elig_func_val_1 = params->tbl_entry.func_out[1];
	elig_func_val_2 = params->tbl_entry.func_out[2];
	elig_func_val_3 = params->tbl_entry.func_out[3];

	schedPrintf("%02d: 0x%03x  %02d: 0x%03x  %02d: 0x%03x  %02d: 0x%03x\n",
			0, elig_func_val_0,
			1, elig_func_val_1,
			2, elig_func_val_2,
			3, elig_func_val_3);

out:
	return rc;
}

/**
 * Show deatailed q level Eligible Priority Function header
 */
static void prvSchedDebugPrintEligFuncQHeader(void)
{
	/* in - MinTBNeg, MaxTBNeg, PropPrio */
	/* out - Elig, SchdPrio, PropPrio, MinTBUsed, MaxTBUsed */
	schedPrintf("    Input       OutPut            Value\n");
	schedPrintf("    -----    -----------------    -----\n");
	schedPrintf("    M   M    E   S   P   M   M\n");
	schedPrintf("    i   a    l   c   r   i   a\n");
	schedPrintf("    n   x    i   h   o   n   x\n");
	schedPrintf("    T   T    g   d   p   T   T\n");
	schedPrintf("    B   B        P   P   B   B\n");
	schedPrintf("    N   N        r   r   U   U\n");
	schedPrintf("    e   e        i   i   s   s\n");
	schedPrintf("    g   g        o   o   e   e\n");
	schedPrintf("    -   -    -   -   -   -   -\n");
}

/**
 * Show deatailed q level Eligible Priority Function
 */
static int prvSchedDebugShowEligFuncQDetailed(PRV_CPSS_SCHED_HANDLE hndl, uint16_t func_offset)
{
	int rc = -EFAULT;
	struct tm_elig_prio_func_out elig_func_out[4];
	struct tm_elig_prio_func_queue tm_elig_prio_q_tbl;
	struct tm_elig_prio_func_queue *params = &tm_elig_prio_q_tbl;
	int j;
	int entry_index;
	uint16_t elig_val[4];
	
	rc = prvSchedLowLevelGetQueueEligPrioFunc(hndl, func_offset, params);
	if (rc)
		goto out;

	schedPrintf("Function: %d\n", func_offset);
	prvSchedDebugPrintEligFuncQHeader();

	prvSchedConvertValueToEligFunc(params->tbl_entry.func_out[0], &elig_func_out[0]);
	prvSchedConvertValueToEligFunc(params->tbl_entry.func_out[1], &elig_func_out[1]);
	prvSchedConvertValueToEligFunc(params->tbl_entry.func_out[2], &elig_func_out[2]);
	prvSchedConvertValueToEligFunc(params->tbl_entry.func_out[3], &elig_func_out[3]);

	elig_val[0] = params->tbl_entry.func_out[0];
	elig_val[1] = params->tbl_entry.func_out[1];
	elig_val[2] = params->tbl_entry.func_out[2];
	elig_val[3] = params->tbl_entry.func_out[3];

	for (j = 0; j < 4; j++) {
		entry_index =  j;	
		schedPrintf("%02d: %d   %d    %d   %d   %d   %d   %d     0x%03x\n",
			entry_index,
			entry_index / 2,
			entry_index  %2,
			elig_func_out[j].elig,
			elig_func_out[j].sched_prio,
			elig_func_out[j].prop_prio,
			elig_func_out[j].min_tb,
			elig_func_out[j].max_tb,
			elig_val[j]);
	}

out:
	return rc;
} 

int prvSchedDebugDumpEligFunc(PRV_CPSS_SCHED_HANDLE hndl, int level, uint16_t func_index, int format)
{
	int rc = 0;

	if (level < Q_LEVEL || level > P_LEVEL)
	{
		schedPrintf("Error: wrong level param\n");
		return 1;
	}

	if (func_index > 63)
	{
		schedPrintf("Error: wrong func_index param\n");
		return 1;
	}

    if (level == Q_LEVEL)
        if (format == 1)
            rc = prvSchedDebugShowEligFuncQDetailed(hndl, func_index);
        else
            rc = prvSchedDebugShowHwEligPrioQFunc(hndl,  func_index);
    else
    {
        if (format == 1)
            rc = prvSchedDebugShowEligFuncDetailed(hndl, level, func_index);
        else
            rc = prvSchedDebugShowHwEligFunc(hndl, level, func_index);
    }

    return rc;
}
