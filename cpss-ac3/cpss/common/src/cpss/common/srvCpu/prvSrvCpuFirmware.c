/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file prvSrvCpuFirmware.c
*
* @brief Firmware management APIs
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include "prvSrvCpuFirmware.h"

/*TODO: reimplement */

/* rename to avoid duplicates */
#define mvHwsServiceCpuBc2Fw _aprv_mvHwsServiceCpuBc2Fw
#define mvHwsServiceCpuBobkFw _aprv_mvHwsServiceCpuBobkFw
#define mvHwsServiceCpuCm3BobKFw _aprv_mvHwsServiceCpuCm3BobKFw
#define mvHwsServiceCpuCm3Fw _aprv_mvHwsServiceCpuCm3Fw
#define mvHwsServiceCpuCm3AldrinFw _aprv_mvHwsServiceCpuCm3AldrinFw
#define mvHwsServiceCpuCm3Bc3Fw _aprv_mvHwsServiceCpuCm3Bc3Fw
#define mvHwsServiceCpuCm3PipeFw _aprv_mvHwsServiceCpuCm3PipeFw
#define mvHwsServiceCpuCm3Aldrin2Fw _aprv_mvHwsServiceCpuCm3Aldrin2Fw
#include "mvHwsServiceCpuBc2Fw.h"
#include "mvHwsServiceCpuBobkFw.h"
#include "mvHwsServiceCpuCm3BobKFw.h"
#include "mvHwsServiceCpuCm3AldrinFw.h"
#include "mvHwsServiceCpuCm3Bc3Fw.h"
#include "mvHwsServiceCpuCm3PipeFw.h"
#include "mvHwsServiceCpuCm3Aldrin2Fw.h"
#define bc3DbaFw _aptv_bc3DbaFw
#include "bc3DbaFw.h"
#include "bc3TamFw.h"


typedef struct PRV_CPSS_FIRMWARE_IMAGE_STCT {
    const char *fwName;
    GT_U32      fwLoadAddr; /* MSYS/CM3 address */
    GT_U8      *fwData;
    GT_U32      fwSize;     /* actual (uncompressed) size */
    GT_U32      fwArrSize;  /* size of buffer */
    GT_U32      compress;   /* 0 - no compression, 2 - simple compression */
    GT_U32      ipcShmAddr; /* MSYS address */
    GT_U32      ipcShmSize; /* Size of shm */
} PRV_CPSS_FIRMWARE_IMAGE_STC;
static PRV_CPSS_FIRMWARE_IMAGE_STC firmwares[] = {
    { "AP_Bobcat2",     mvHwsServiceCpuBc2Fw_INFO },
    { "AP_AC3",         mvHwsServiceCpuBc2Fw_INFO },
    { "AP_BobK_MSYS",   mvHwsServiceCpuBobkFw_INFO },
    { "AP_BobK_CM3",    mvHwsServiceCpuCm3BobKFw_INFO },
    { "AP_Aldrin",      mvHwsServiceCpuCm3AldrinFw_INFO },
    { "AP_Bobcat3",     mvHwsServiceCpuCm3Bc3Fw_INFO },
    { "AP_PIPE",        mvHwsServiceCpuCm3PipeFw_INFO },
    { "AP_Aldrin2",     mvHwsServiceCpuCm3Aldrin2Fw_INFO },

    { "DBA_Bobcat3",    bc3DbaFw_INFO },
    { "TAM_Bobcat3",    bc3TamFw_INFO },

    /* end */
    { NULL, 0, NULL, 0, 0, 0, 0, 0 }
};

typedef struct PRV_FW_IMPL_EMBED_STCT {
    PRV_CPSS_FIRMWARE_IMAGE_FILE_STC    common;
    GT_U8      *fwData;
    GT_U32      fwArrSize;
    GT_U32      compress;
    GT_U32      pos;
    GT_U32      state;
    GT_U32      cnt;
} PRV_FW_IMPL_EMBED_STC;

/* TODO: implement load from file */
PRV_CPSS_FIRMWARE_IMAGE_FILE_STC* prvCpssSrvCpuFirmareOpen(
    IN  const char *fwName
)
{
    GT_U32 i;
    PRV_FW_IMPL_EMBED_STC  *f;

    for (i = 0; firmwares[i].fwName; i++)
    {
        if (cpssOsStrCmp(firmwares[i].fwName, fwName) != 0)
            continue;
        break;
    }
    if (firmwares[i].fwName == NULL)
        return NULL;

    f = (PRV_FW_IMPL_EMBED_STC*)cpssOsMalloc(sizeof(*f));
    if (!f)
        return NULL;
    cpssOsMemSet(f, 0, sizeof(*f));
    f->common.fwLoadAddr = firmwares[i].fwLoadAddr;
    f->common.fwSize     = firmwares[i].fwSize;
    f->common.ipcShmAddr = firmwares[i].ipcShmAddr;
    f->common.ipcShmSize = firmwares[i].ipcShmSize;
    f->fwData            = firmwares[i].fwData;
    f->fwArrSize         = firmwares[i].fwArrSize;
    f->compress          = firmwares[i].compress;
    f->pos               = 0;
    f->state             = 0;
    f->cnt               = 0;

    return &(f->common);
}

GT_STATUS prvCpssSrvCpuFirmareClose(
    IN  PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *f
)
{
    if (f)
        cpssOsFree(f);
    return GT_OK;
}

static GT_U32 prvCpssSrvCpuFirmareReadRaw(
    IN  PRV_FW_IMPL_EMBED_STC            *f,
    OUT GT_U8                            *bufPtr,
    IN  GT_U32                            bufLen
)
{
    GT_U32 c;
    if (f->pos >= f->fwArrSize)
        return 0xffffffff;
    for (c = 0; c < bufLen && f->pos < f->fwArrSize; c++, f->pos++)
        bufPtr[c] = f->fwData[f->pos];
    return c;
}

static GT_U32 prvCpssSrvCpuFirmareReadSimpleCompression(
    IN  PRV_FW_IMPL_EMBED_STC            *f,
    OUT GT_U8                            *bufPtr,
    IN  GT_U32                            bufLen
)
{
    GT_U32 c;
    if (f->pos >= f->fwArrSize && f->state == 0)
        return 0xffffffff;
    for (c = 0; c < bufLen; )
    {
        if (f->pos >= f->fwArrSize && f->state == 0)
            break;
        switch (f->state)
        {
            case 0:
                f->cnt = (f->fwData[f->pos] & 0x7f) + 1;
                if ((f->fwData[f->pos] & 0x80) == 0)
                    f->state = 1; /* bytes */
                else
                    f->state = 2; /* zeroes */
                f->pos++;
                break;
            case 1: /* bytes */
                bufPtr[c++] = f->fwData[f->pos++];
                f->cnt--;
                if (f->cnt == 0)
                    f->state = 0;
                break;
            case 2: /* zeroes */
                bufPtr[c++] = 0;
                f->cnt--;
                if (f->cnt == 0)
                    f->state = 0;
                break;
        }
    }
    return c;
}

GT_U32 prvCpssSrvCpuFirmareRead(
    IN  PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *fp,
    OUT void                             *bufPtr,
    IN  GT_U32                            bufLen
)
{
    PRV_FW_IMPL_EMBED_STC  *f = (PRV_FW_IMPL_EMBED_STC*)fp;

    if (!f)
        return 0xffffffff;
    if (f->compress == 0)
        return prvCpssSrvCpuFirmareReadRaw(f, (GT_U8*)bufPtr, bufLen);
    if (f->compress == 2)
        return prvCpssSrvCpuFirmareReadSimpleCompression(f, (GT_U8*)bufPtr, bufLen);
    return 0xffffffff;
}
