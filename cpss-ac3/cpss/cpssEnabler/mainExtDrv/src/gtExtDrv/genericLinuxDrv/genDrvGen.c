/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtExtDrv/drivers/gtGenDrv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "kerneldrv/include/presteraGlob.h"


/*************** Globals ******************************************************/

/* file descriptor returnd by openning the PP *nix device driver */
extern GT_32 gtPpFd;


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
*/
GT_STATUS extDrvBoardIdGet
(
    OUT GT_U32  *boardIdPtr
)
{
    mv_board_id_t id;

    if (ioctl (gtPpFd, PRESTERA_IOC_GETBOARDID, &id))
    {
        fprintf(stderr, "ioctl(PRESTERA_IOC_GETBOARDID) connect failed: errno(%s)\n",
                        strerror(errno));
        return GT_FAIL;
    }
    *boardIdPtr = (GT_U32)id;
    return GT_OK;
}


