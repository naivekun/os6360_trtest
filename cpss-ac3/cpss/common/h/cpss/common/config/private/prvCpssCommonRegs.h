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
* @file prvCpssCommonRegs.h
*
* @brief This file includes the declaration of the structure to hold the
* addresses of Common PP registers.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssCommonRegsh
#define __prvCpssCommonRegsh

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#pragma pack(push, 4)
typedef struct{

   struct /*DFXServerUnitsDeviceSpecificRegs*/{

       GT_U32    deviceResetCtrl;
       /* SkipInitializationMatrixes registers related to legacy, SIP5 and SIP6 devices           */
       /* For SIP6 devices DFX units are multiple and present in each Tile (2 Processing Cores    */
       /* container) and in each GOP.                                                             */
       /* SIP6 SkipInitializationMatrixes registers of Tile and of GOP are similar, but there are */
       /* some different registers with the same offsets.                                         */
       /* The registers DB contains all SkipInitializationMatrixes registers both common, both of */
       /* registers of Tile only, both registers of GOP only.                                     */
       /* addresses in DB adjusted to Tile DFX address range.                                     */
       /* For access to GOP DFX these Registers Addresses should be recalculated using register   */
       /* offset inside DFX unit from this DB only and ajusting it to GOP address range.          */
       /*                                                                                         */
       /* SIP5 devices and SIP6 Processing Core (Eagle) and SIP6 GOP SkipInitializationMatrixes */
       GT_U32    configSkipInitializationMatrix;
       GT_U32    RAMInitSkipInitializationMatrix;
       GT_U32    trafficManagerSkipInitializationMatrix;
       GT_U32    tableSkipInitializationMatrix;
       GT_U32    SERDESSkipInitializationMatrix;
       GT_U32    EEPROMSkipInitializationMatrix;
       GT_U32    PCIeSkipInitializationMatrix;
       GT_U32    DDRSkipInitializationMatrix;
       /* SIP6 devices Processing Core (Eagle) and GOP SkipInitializationMatrixes */
       GT_U32    SRRSkipInitializationMatrix;
       GT_U32    BISTSkipInitializationMatrix;
       GT_U32    SoftRepairSkipInitializationMatrix;
       GT_U32    RegxGenxSkipInitializationMatrix;
       GT_U32    MultiActionSkipInitializationMatrix;
       GT_U32    DeviceEEPROMSkipInitializationMatrix;
       GT_U32    D2DLinkSkipInitializationMatrix;
       GT_U32    DFXRegistersSkipInitializationMatrix;
       GT_U32    DFXPipeSkipInitializationMatrix;
       /* SIP6 devices Processing Core only (Eagle) SkipInitializationMatrixes */
       GT_U32    Core_TileSkipInitializationMatrix;
       GT_U32    Core_MngSkipInitializationMatrix;
       GT_U32    Core_D2DSkipInitializationMatrix;
       GT_U32    Core_RavenSkipInitializationMatrix;
       /* SIP6 devices Processing GOP only (Raven) SkipInitializationMatrixes */
       GT_U32    GOP_D2DSkipInitializationMatrix;
       GT_U32    GOP_MainSkipInitializationMatrix;
       GT_U32    GOP_CNMSkipInitializationMatrix;
       GT_U32    GOP_GWSkipInitializationMatrix;

       GT_U32    deviceSAR1;
       GT_U32    deviceSAR2;
       GT_U32    deviceSAR1Override;
       GT_U32    deviceSAR2Override;
       GT_U32    deviceCtrl0;
       GT_U32    deviceCtrl1;
       GT_U32    deviceCtrl4;
       GT_U32    deviceCtrl5;
       GT_U32    deviceCtrl8;
       GT_U32    deviceCtrl10;
       GT_U32    deviceCtrl12;
       GT_U32    deviceCtrl16;
       GT_U32    deviceCtrl21;
       GT_U32    deviceCtrl20;
       GT_U32    deviceCtrl19;
       GT_U32    deviceCtrl18;
       GT_U32    deviceCtrl15;
       GT_U32    deviceCtrl14;
       GT_U32    deviceCtrl33;
       GT_U32    deviceStatus0;
       GT_U32    deviceStatus1;
       GT_U32    initializationStatusDone;
       GT_U32    pllMiscConfig;
       GT_U32    pllMiscParameters;
       GT_U32    deviceCtrl6;
       GT_U32    deviceCtrl7;
       GT_U32    deviceCtrl9;
       GT_U32    DFXSkipInitializationMatrix;
       GT_U32    CPUSkipInitializationMatrix;

       GT_U32    deviceCtrl28;
       GT_U32    deviceCtrl29;
   }DFXServerUnitsDeviceSpecificRegs;

   struct /*DFXServerUnits*/{

       struct /*DFXServerRegs*/{

           GT_U32    AVSDisabledCtrl2[1]/*AVS*/;
           GT_U32    AVSDisabledCtrl1[1]/*AVS*/;
           GT_U32    temperatureSensor28nmCtrlLSB;
           GT_U32    serverXBARTargetPortConfig[4]/*Target Port*/;
           GT_U32    serverStatus;
           GT_U32    serverInterruptSummaryCause;
           GT_U32    serverInterruptSummaryMask;
           GT_U32    serverInterruptMask;
           GT_U32    serverInterruptCause;
           GT_U32    serverAddrSpace;
           GT_U32    pipeSelect;
           GT_U32    temperatureSensor28nmCtrlMSB;
           GT_U32    temperatureSensorStatus;
           GT_U32    AVSStatus[1]/*AVS*/;
           GT_U32    AVSMinStatus[1]/*AVS*/;
           GT_U32    AVSMaxStatus[1]/*AVS*/;
           GT_U32    AVSEnabledCtrl[1]/*AVS*/;
           GT_U32    snoopBusStatus;
           GT_U32    deviceIDStatus;
           GT_U32    JTAGDeviceIDStatus;
           GT_U32    PLLCoreParameters;
           GT_U32    PLLCoreConfig;
       }DFXServerRegs;

   }DFXServerUnits;

   /* bookmark to skip this struct during registers printing */
   GT_U32    dfxClientBookmark;     /* PRV_CPSS_SW_PTR_BOOKMARK_CNS */
   GT_U32    dfxClientBookmarkType; /* PRV_CPSS_SW_TYPE_WRONLY_CNS */
   GT_U32    dfxClientBookmarkSize;

    struct /*DFXClientUnits*/{
       GT_U32    clientControl;
       GT_U32    clientDataControl;
       GT_U32    clientRamBISTInfo[4];
       GT_U32    BISTControl/*Target Port*/;
       GT_U32    BISTOpCode;
       GT_U32    BISTMaxAddress;
       GT_U32    dummyWrite;
       GT_U32    clientRamMC_Group[4];
       GT_U32    clientStatus;

   }DFXClientUnits;

    /* bookmark to skip this struct during registers printing */
   GT_U32    dfxRamBookmark;     /* PRV_CPSS_SW_PTR_BOOKMARK_CNS */
   GT_U32    dfxRamBookmarkType; /* PRV_CPSS_SW_TYPE_WRONLY_CNS */
   GT_U32    dfxRamBookmarkSize;

   struct /*DFXRam*/{
       GT_U32   memoryControl;
   }DFXRam;

}PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC;
/* restore alignment setting */
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonRegsh */

