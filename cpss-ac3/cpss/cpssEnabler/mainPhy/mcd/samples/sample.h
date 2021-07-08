/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#ifndef SAMPLE_H
#define SAMPLE_H
#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/*******************************************************************************
 mcdSampleInitDrv

 Inputs:
   phyId         - logical number of PHY device, 0..15
   pHostContext  - user specific data for host to pass to the low layer
   loadImage     - MCD_TRUE load all firmware images, MCD_FALSE don't load
   firstMDIOPort - PHY address of first port on XSMI bus

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 and EC808 Driver Initialization Routine.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleInitDrv
(
    IN MCD_U32   phyId,
    IN MCD_PVOID pHostContext,   /* optional host context */
    IN MCD_BOOL  loadImage,      /* 1:load image; 0:do not load */
    IN MCD_U16   mdioFirstPort
);

/*******************************************************************************
 mcdSampleInitDrvLoadAllDevices

 Inputs:
   phyIdBmp         - bitmap of logical Ids
                      logical Id range is 0..15
                      device with logical Id == logId has  firstMDIOPort == (logId * 2)
                      pHostContext wil be NULL for each device
   imagesAmount     - what FW images to load:
                      0 - nothing
                      1 - serdes only
                      2 - serdes and master
                      3 - serdes, master and swap
                      4 - (yet not supported) serdes, master, swap and CM3

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 and EC808 Driver Initialization Routine.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleInitDrvLoadAllDevices
(
    IN MCD_U32   phyIdBmp,
    IN MCD_U32   imagesAmount
);

/******************************************************************************
  mcdSampleGetDrvDev

 Inputs:
   phyId         - logical number of PHY device, 0..15

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Unload MCD Device driver. Free allocated memory.

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleUnloadDrv
(
    IN MCD_U32   phyId
);

/*******************************************************************************
 mcdSampleGetDrvDev

 Inputs:
   phyId         - logical number of PHY device, 0..15

 Outputs:
       none

 Returns:
       pointer to device handler

 Description:
   Get pointer to device handler for logical PHY number.
   NULL - in case of error or not initialized

 Notes/Warnings:

*******************************************************************************/
MCD_DEV_PTR mcdSampleGetDrvDev
(
    IN MCD_U32   phyId
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* defined SAMPLE_H*/


