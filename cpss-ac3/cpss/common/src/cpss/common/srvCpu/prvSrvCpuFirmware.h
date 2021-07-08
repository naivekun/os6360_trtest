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
* @file prvSrvCpuFirmware.h
*
* @brief Firmware management APIs
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssGenericSrvCpuFirmware_h__
#define __prvCpssGenericSrvCpuFirmware_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

typedef struct PRV_CPSS_FIRMWARE_IMAGE_FILE_STCT {
    GT_U32      fwLoadAddr; /* MSYS/CM3 address */
    GT_U32      fwSize;     /* actual (uncompressed) size */
    GT_U32      ipcShmAddr; /* MSYS address */
    GT_U32      ipcShmSize; /* Size of shm */
} PRV_CPSS_FIRMWARE_IMAGE_FILE_STC;

PRV_CPSS_FIRMWARE_IMAGE_FILE_STC* prvCpssSrvCpuFirmareOpen(
    IN  const char *fwName
);
GT_U32 prvCpssSrvCpuFirmareRead(
    IN  PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *f,
    OUT void                             *bufPtr,
    IN  GT_U32                            bufLen
);
GT_STATUS prvCpssSrvCpuFirmareClose(
    IN  PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *f
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenericSrvCpuFirmware_h__ */
