/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvGen.c
*
* @brief Generic extDrv functions
*
* @version   1
********************************************************************************
*/
#include <gtExtDrv/drivers/gtGenDrv.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <stdio.h>
#include "prvNoKmDrv.h"
#include "drivers/mvResources.h"
#include <cpss/common/cpssTypes.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

#if defined(CPU_ARMv7)
void soc_init(void);
#define SOC_INIT() soc_init()
#else
#define SOC_INIT()
#endif

/**
* @internal extDrvSoCInit function
* @endinternal
*
* @brief   Initialize SoC related drivers
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note This API is called by after cpssExtServicesBind()
*
*/
GT_STATUS extDrvSoCInit(void)
{
    SOC_INIT();
    return GT_OK;
}

/**
* @internal extDrvBoardIdGet function
* @endinternal
*
* @brief   Get BoardId value
*
* @param[out] boardIdPtr               - boardId
*                                      None.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*
* @note This API is called by bobkBoardTypeGet only for some boards. Therefore,
*       only the relevant boards are supported here.
*
*/
GT_STATUS extDrvBoardIdGet
(
    OUT GT_U32  *boardIdPtr
)
{
    static GT_U32 boardId = 0;
    GT_U32      regData = 0x68211ab;
    GT_U32      dataInRegister = 0;
    GT_BOOL     RD = GT_FALSE;
    GT_STATUS   rc;
    CPSS_HW_INFO_RESOURCE_MAPPING_STC res;
    CPSS_HW_DRIVER_STC *internalRegs;

    if (boardId != 0)
    {
        *boardIdPtr = boardId;
        return GT_OK;
    }

    switch (prvNoKmDevId & 0xff00) {

    case DEVID_TO_FAMILY(CPSS_98DX4235_CNS): /* Cetus / Lewis */
    case DEVID_TO_FAMILY(CPSS_98DX4203_CNS): /* caelum / Cygnus */
        /*Cetus / Caelum / Lewis / Cygnus - by device ID (0xBE0* / 0xBC0* / 0xBE1* / 0xBC1* respectively)*/
        switch (prvNoKmDevId & 0xfff0){
        case (CPSS_98DX4235_CNS >> 16) & 0xfff0:
            *boardIdPtr = EXT_DRV_BOBK_CETUS_DB_ID;
            break;
        case (CPSS_98DX4203_CNS >> 16) & 0xfff0:
            *boardIdPtr = EXT_DRV_BOBK_CAELUM_DB_ID;
            break;
        case (CPSS_98DX8212_CNS >> 16) & 0xfff0:
            *boardIdPtr = EXT_DRV_BOBK_CETUS_LEWIS_RD_ID;
            break;
        case (CPSS_98DX4210_CNS >> 16) & 0xfff0:
            *boardIdPtr = EXT_DRV_BOBK_CAELUM_CYGNUS_RD_ID;
            break;
        default:
            return GT_FAIL;
        }
        break;
    
    case DEVID_TO_FAMILY(CPSS_98CX8420_CNS): /* BC3 */
        *boardIdPtr =  EXT_DRV_BOBCAT3_DB_ID;
        break;

    case DEVID_TO_FAMILY(CPSS_98DX8308_CNS): /* Aldrin */
    case DEVID_TO_FAMILY(CPSS_98EX5520_CNS): /* Aldrin2 */
        /* Read CPU device ID from RUnit */
        rc = mbus_map_resource(MV_RESOURCE_MBUS_RUNIT, _1M, &res);
        if (rc != GT_OK)
            fprintf(stderr, "failed to get MBUS resources rc=%d. Is mbusDrv Kernel module present?\nAssuming DB\n", rc);
        else
        {
            internalRegs = cpssHwDriverGenMmapCreateDrv(res.start, res.size);
            if (internalRegs == NULL)
                fprintf(stderr, "failed creating RUnit drv. Assuming DB\n", rc);
            else
            {
                if (GT_OK != internalRegs->read(internalRegs, 0, 0x18238, &regData, 1))
                    fprintf(stderr, "failed reading host CPU devId rc=%d\nAssuming DB\n", rc);
                internalRegs->destroy(internalRegs);
            }
        }

        regData = ((regData >> 16) & 0x0000ffff);

        /* Aldrin */
        if ((prvNoKmDevId & 0xff00) == DEVID_TO_FAMILY(CPSS_98DX8308_CNS))
        {
            if (regData == 0x6811)
                RD = GT_TRUE;
            *boardIdPtr = (RD == GT_TRUE) ? EXT_DRV_ALDRIN_RD_ID : EXT_DRV_ALDRIN_DB_ID;
        }

        /* Aldrin2 */
        else
        {
            /* CPU device ID 6820 exists on RD board, but may also be present on DB board on AMC card. To distinguish we use MPP0-7 register */
            if (regData == 0x6820)
            {
                internalRegs->read(internalRegs, 0, 0xf1018100 ,&dataInRegister, 1);
                if (dataInRegister == 0)
                    RD = GT_TRUE;
            }
            *boardIdPtr = (RD == GT_TRUE) ? EXT_DRV_ALDRIN2_RD_ID : EXT_DRV_ALDRIN2_DB_ID;
        }

        break;

    default:
        return GT_FAIL;
    }

    NOKMDRV_IPRINTF(("boardId: 0x%x\n", *boardIdPtr));
    boardId = *boardIdPtr;
    return GT_OK;
}



