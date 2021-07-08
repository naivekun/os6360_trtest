/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief prvCpssDxChTxqSchedShadow  implementation for cpss
 *
* @file cpss_tm_ctl.c
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCtlInternal.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>


GT_U32 prvSchedLibOpenGetSchedGlobalAddress
(
    GT_U8   devNum,
    GT_U32 tileNum
)
{

    GT_U32  baseAddr,tileOffset;
    static PRV_CPSS_DXCH_UNIT_ENT unitId = PRV_CPSS_DXCH_UNIT_TXQ_PSI_E;

    baseAddr   = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId ,NULL);
     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);

    return baseAddr + tileOffset;

}


/**
 */
int prvSchedLibOpenExt(GT_U8 devNum, struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC *tree_struct_ptr, struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC *init_params_PTR, PRV_CPSS_SCHED_HANDLE * pHndl)
{

    int rc = 0;


        struct cpssSchedEnv * hEnv=NULL;

    if (!pHndl) return  -EINVAL;



        hEnv = (struct cpssSchedEnv * )prvCpssCreateNewEnvironmentHandle(devNum);


    prvCpssSchedSetGlobalAddress((void *)hEnv,init_params_PTR->schedLibPdqNum,prvSchedLibOpenGetSchedGlobalAddress(devNum,init_params_PTR->schedLibPdqNum));


        /* copy TM tree structure parameters */
        schedMemCpy(&(hEnv->tree_structure),tree_struct_ptr,sizeof(struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC));


    /* temporary   : burst mode is suppressed */
    hEnv->burstMode = 0;



        rc = prvSchedLibSwInit((void *)hEnv, pHndl, init_params_PTR);
        if  (rc) goto out;

        rc = prvSchedLibHwInit(*pHndl);

out:
    if (rc)
        {
                if (*pHndl)
                {
                        prvSchedLibCloseSw(*pHndl);
                        *pHndl = NULL;
                }
                if (hEnv) schedFree(hEnv);
        }
    return rc;
}





