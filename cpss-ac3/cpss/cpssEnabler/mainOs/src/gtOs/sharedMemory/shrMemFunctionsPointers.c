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
* @file shrMemFunctionsPointers.c
*
* @brief This file contains bind routines for BM architectures to avoid
* pointer issue. Work-around uses dlsym system call to avoid
* private addresses to functions.
*
* @version   13
********************************************************************************
*/

/************* Includes *******************************************************/
#include <stdio.h>
#include <dlfcn.h>
#include <gtOs/gtGenTypes.h>
#include <cpss/extServices/cpssExtServices.h>
#include <gtOs/gtOsMem.h>

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <gtExtDrv/drivers/gtHwIfDrv.h>

/*******************************************************************************
* getCpssSharedLibraryHandle
*
* DESCRIPTION:
*       Gets handler of library object (DSO)
*
* INPUTS:
*       lib_name - library name to be opened
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       NULL - on error
*       Otherwise - handler to shared object
*
* COMMENTS:
*       Used for dlsym system call.
*
*******************************************************************************/
static GT_VOID *getCpssSharedLibraryHandle(const char *lib_name)
{
    GT_VOID *handle = dlopen(lib_name, RTLD_LAZY);
    if (!handle)
    {
        fprintf (stderr, "%s\n", dlerror());
        return NULL;
    }

    dlerror();    /* Clear any existing error */
    return handle;
}

/*******************************************************************************
* getFunctionThruDynamicLoader
*
* DESCRIPTION:
*       Gets address of specified function into dynamic symbol 
*       (described by handle) thru dlsym system call.
*
* INPUTS:
*       handle - handle of dynamic object to be searched into
*       func_name - name of symbol to be searched for
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       NULL - on error
*       Otherwise - pointer to function
*
* COMMENTS:
*       Uses dlsym system call to overcome private-pointer-to-func issue
*       observed for ARM GnuEABI.
*
*******************************************************************************/
static GT_VOID *getFunctionThruDynamicLoader(GT_VOID *handle, const char *func_name)
{
    GT_VOID *ptr=dlsym(handle, func_name);
    if (!ptr)
    {
        fprintf(stderr, "getFunctionThruDynamicLoader, function %s: %s\n", func_name, dlerror());
        return NULL;
    }
    return ptr;
}


/**
* @internal closeLibraryHandle function
* @endinternal
*
* @brief   Close library handle opened by getCpssSharedLibraryHandle
*
* @note Cleanup hanle opened by dlopen.
*
*/
static GT_VOID closeLibraryHandle(GT_VOID *handle)
{
    dlclose(handle);
}

/**
* @internal shrMemGetDefaultOsBindFuncsThruDynamicLoader function
* @endinternal
*
* @brief   Receives default cpss bind from OS
*
* @param[out] osFuncBindPtr            - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Uses dlsym system call to overcome private-pointer-to-func issue
*       observed for ARM GnuEABI.
*
*/
GT_STATUS shrMemGetDefaultOsBindFuncsThruDynamicLoader(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
)
{
    GT_VOID *handle=getCpssSharedLibraryHandle("libhelper.so");
    GT_STATUS retcode = GT_OK;
    if(handle==NULL) return GT_FAIL;

    osMemSet(osFuncBindPtr,0,sizeof(*osFuncBindPtr));

    if (!handle)
    {
        fprintf (stderr, "%s\n", dlerror());
        return GT_FAIL;
    }

    dlerror();    /* Clear any existing error */
#define ASSIGN_FUNCPTR(_member,_funcname) if((osFuncBindPtr->_member=getFunctionThruDynamicLoader(handle, _funcname)) == NULL) retcode = GT_FAIL

    /* bind the OS functions to the CPSS */
    ASSIGN_FUNCPTR(osMemBindInfo.osMemBzeroFunc,"osBzero");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemSetFunc,"osMemSet");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemCpyFunc,"osMemCpy");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemMoveFunc,"osMemMove");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemCmpFunc,"osMemCmp");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemStaticMallocFunc,"osStaticMalloc");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemMallocFunc,"osMalloc");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemReallocFunc,"osRealloc");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemFreeFunc,"CPSS_osFree");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemCacheDmaMallocFunc,"osCacheDmaMalloc");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemCacheDmaFreeFunc,"osCacheDmaFree");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemPhyToVirtFunc,"osPhy2Virt");
    ASSIGN_FUNCPTR(osMemBindInfo.osMemVirtToPhyFunc,"osVirt2Phy");

    ASSIGN_FUNCPTR(osStrBindInfo.osStrlenFunc,"osStrlen");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrCpyFunc,"osStrCpy");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrNCpyFunc,"osStrNCpy");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrChrFunc,"osStrChr");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrStrFunc,"osStrStr");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrRevChrFunc,"osStrrChr");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrCmpFunc,"osStrCmp");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrNCmpFunc,"osStrNCmp");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrCatFunc,"osStrCat");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrStrNCatFunc,"osStrNCat");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrChrToUpperFunc,"osToUpper");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrTo32Func,"osStrTo32");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrToU32Func,"osStrToU32");
    ASSIGN_FUNCPTR(osStrBindInfo.osStrTolFunc,"osStrTol");

    ASSIGN_FUNCPTR(osSemBindInfo.osMutexCreateFunc,"CPSS_osMutexCreate");
    ASSIGN_FUNCPTR(osSemBindInfo.osMutexDeleteFunc,"CPSS_osMutexDelete");
 #if defined  CPSS_USE_MUTEX_PROFILER    
    ASSIGN_FUNCPTR(osSemBindInfo.osMutexSetGlAttributes,"osMutexSetGlAttributes");
    ASSIGN_FUNCPTR(osSemBindInfo.osCheckUnprotectedPerDeviceDbAccess,"osCheckUnprotectedPerDeviceDbAccess");
 #endif   
    ASSIGN_FUNCPTR(osSemBindInfo.osMutexLockFunc,"osMutexLock");
    ASSIGN_FUNCPTR(osSemBindInfo.osMutexUnlockFunc,"osMutexUnlock");

    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemBinCreateFunc,"osSemBinCreate");
    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemMCreateFunc,"osSemMCreate");
    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemCCreateFunc,"osSemCCreate");
    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemDeleteFunc,"osSemDelete");
    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemWaitFunc,"osSemWait");
    ASSIGN_FUNCPTR(osSemBindInfo.osSigSemSignalFunc,"osSemSignal");

    ASSIGN_FUNCPTR(osIoBindInfo.osIoBindStdOutFunc,"osBindStdOut");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoPrintfFunc,"osPrintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoVprintfFunc,"osVprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoSprintfFunc,"osSprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoVsprintfFunc,"osVsprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoSnprintfFunc,"osSnprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoVsnprintfFunc,"osVsnprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoPrintSynchFunc,"osPrintSync");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoGetsFunc,"osGets");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFlastErrorStrFunc,"osFlastErrorStr");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFopenFunc,"osFopen");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFcloseFunc,"osFclose");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoRewindFunc,"osRewind");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFprintfFunc,"osFprintf");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFgets,"osFgets");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFwriteFunc,"osFwrite");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFreadFunc,"osFread");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFgetLengthFunc,"osFgetLength");
    ASSIGN_FUNCPTR(osIoBindInfo.osIoFatalErrorFunc,"osFatalError");

    ASSIGN_FUNCPTR(osInetBindInfo.osInetNtohlFunc,"osNtohl");
    ASSIGN_FUNCPTR(osInetBindInfo.osInetHtonlFunc,"osHtonl");
    ASSIGN_FUNCPTR(osInetBindInfo.osInetNtohsFunc,"osNtohs");
    ASSIGN_FUNCPTR(osInetBindInfo.osInetHtonsFunc,"osHtons");
    ASSIGN_FUNCPTR(osInetBindInfo.osInetNtoaFunc,"osInetNtoa");

    ASSIGN_FUNCPTR(osTimeBindInfo.osTimeWkAfterFunc,"osTimerWkAfter");
    ASSIGN_FUNCPTR(osTimeBindInfo.osTimeTickGetFunc,"CPSS_osTickGet");
    ASSIGN_FUNCPTR(osTimeBindInfo.osTimeGetFunc,"osTime");
    ASSIGN_FUNCPTR(osTimeBindInfo.osTimeRTFunc,"osTimeRT");
    ASSIGN_FUNCPTR(osTimeBindInfo.osGetSysClockRateFunc,"osGetSysClockRate");
    ASSIGN_FUNCPTR(osTimeBindInfo.osDelayFunc,"osDelay");
    ASSIGN_FUNCPTR(osTimeBindInfo.osStrftimeFunc,"osStrftime");

#if !defined(_linux) && !defined(_FreeBSD)
    ASSIGN_FUNCPTR(osIntBindInfo.osIntEnableFunc,"osIntEnable");
    ASSIGN_FUNCPTR(osIntBindInfo.osIntDisableFunc,"osIntDisable");
    ASSIGN_FUNCPTR(osIntBindInfo.osIntModeSetFunc,"osSetIntLockUnlock");
    ASSIGN_FUNCPTR(osIntBindInfo.osIntConnectFunc,"osInterruptConnect");
#endif
    ASSIGN_FUNCPTR(osRandBindInfo.osRandFunc,"osRand");
    ASSIGN_FUNCPTR(osRandBindInfo.osSrandFunc,"osSrand");

    ASSIGN_FUNCPTR(osTaskBindInfo.osTaskCreateFunc,"osTaskCreate");
    ASSIGN_FUNCPTR(osTaskBindInfo.osTaskDeleteFunc,"osTaskDelete");
    ASSIGN_FUNCPTR(osTaskBindInfo.osTaskGetSelfFunc,"osTaskGetSelf");
    ASSIGN_FUNCPTR(osTaskBindInfo.osTaskLockFunc,"CPSS_osTaskLock");
    ASSIGN_FUNCPTR(osTaskBindInfo.osTaskUnLockFunc,"CPSS_osTaskUnLock");

    ASSIGN_FUNCPTR(osStdLibBindInfo.osQsortFunc,"osQsort");
    ASSIGN_FUNCPTR(osStdLibBindInfo.osBsearchFunc,"osBsearch");

    ASSIGN_FUNCPTR(osMsgQBindInfo.osMsgQCreateFunc,"CPSS_osMsgQCreate");
    ASSIGN_FUNCPTR(osMsgQBindInfo.osMsgQDeleteFunc,"CPSS_osMsgQDelete");
    ASSIGN_FUNCPTR(osMsgQBindInfo.osMsgQSendFunc,"CPSS_osMsgQSend");
    ASSIGN_FUNCPTR(osMsgQBindInfo.osMsgQRecvFunc,"CPSS_osMsgQRecv");
    ASSIGN_FUNCPTR(osMsgQBindInfo.osMsgQNumMsgsFunc,"CPSS_osMsgQNumMsgs");

    ASSIGN_FUNCPTR(osSocketInfo.osSocketLastErrorFunc,"osSocketLastError");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketLastErrorStrFunc,"osSocketLastErrorStr");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketTcpCreateFunc,"osSocketTcpCreate");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketUdpCreateFunc,"osSocketUdpCreate");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketTcpDestroyFunc,"osSocketTcpDestroy");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketUdpDestroyFunc,"osSocketUdpDestroy");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketCreateAddrFunc,"osSocketCreateAddr");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketDestroyAddrFunc,"osSocketDestroyAddr");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketBindFunc,"osSocketBind");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketListenFunc,"osSocketListen");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketAcceptFunc,"osSocketAccept");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketConnectFunc,"osSocketConnect");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketSetNonBlockFunc,"osSocketSetNonBlock");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketSetBlockFunc,"osSocketSetBlock");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketSendFunc,"osSocketSend");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketSendToFunc,"osSocketSendTo");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketRecvFunc,"osSocketRecv");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketRecvFromFunc,"osSocketRecvFrom");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketSetSocketNoLingerFunc,"osSocketSetSocketNoLinger");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc,"osSocketExtractIpAddrFromSocketAddr");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketGetSocketAddrSizeFunc,"osSocketGetSocketAddrSize");
    ASSIGN_FUNCPTR(osSocketInfo.osSocketShutDownFunc,"osSocketShutDown");


    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectCreateSetFunc,"osSelectCreateSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectEraseSetFunc,"osSelectEraseSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectZeroSetFunc,"osSelectZeroSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectAddFdToSetFunc,"osSelectAddFdToSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectClearFdFromSetFunc,"osSelectClearFdFromSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectIsFdSetFunc,"osSelectIsFdSet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectCopySetFunc,"osSelectCopySet");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSelectFunc,"osSelect");
    ASSIGN_FUNCPTR(osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc,"osSocketGetSocketFdSetSize");
#undef ASSIGN_FUNCPTR

    closeLibraryHandle(handle);

    return retcode;
}

/**
* @internal shrMemGetDefaultExtDrvFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from extDrv
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*/
GT_STATUS shrMemGetDefaultExtDrvFuncs
(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
)
{
    GT_VOID *handle=getCpssSharedLibraryHandle("libcpss.so");
    GT_STATUS retcode = GT_OK;
    if(handle==NULL) return GT_FAIL;

    osMemSet(extDrvFuncBindInfoPtr,0,sizeof(*extDrvFuncBindInfoPtr));

#define ASSIGN_FUNCPTR(_member,_funcname) if((extDrvFuncBindInfoPtr->_member=getFunctionThruDynamicLoader(handle, _funcname)) == NULL) retcode = GT_FAIL
    /* bind the external drivers functions to the CPSS */
    ASSIGN_FUNCPTR(extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush, "extDrvMgmtCacheFlush");
    ASSIGN_FUNCPTR(extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush, "extDrvMgmtCacheFlush");
    ASSIGN_FUNCPTR(extDrvMgmtCacheBindInfo.extDrvMgmtCacheInvalidate, "extDrvMgmtCacheInvalidate");

    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiInitDriverFunc, "hwIfSmiInitDriver");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiWriteRegFunc, "hwIfSmiWriteReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiReadRegFunc, "hwIfSmiReadReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamReadFunc, "hwIfSmiTskRegRamRead");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamWriteFunc, "hwIfSmiTskRegRamWrite");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecReadFunc, "hwIfSmiTskRegVecRead");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecWriteFunc, "hwIfSmiTskRegVecWrite");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteRegFunc, "hwIfSmiTaskWriteReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskReadRegFunc, "hwIfSmiTaskReadReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntReadRegFunc, "hwIfSmiInterruptReadReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntWriteRegFunc, "hwIfSmiInterruptWriteReg");
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiDevVendorIdGetFunc, "extDrvSmiDevVendorIdGet");

/*  used only in linux -- will need to be under some kind of COMPILATION FLAG
    ASSIGN_FUNCPTR(extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteFieldFunc, "hwIfSmiTaskWriteRegField");
*/

#ifdef GT_I2C
    ASSIGN_FUNCPTR(extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiInitDriverFunc, "hwIfTwsiInitDriver");
    ASSIGN_FUNCPTR(extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiWriteRegFunc, "hwIfTwsiWriteReg");
    ASSIGN_FUNCPTR(extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiReadRegFunc, "hwIfTwsiReadReg");
#endif /* GT_I2C */

/*  XBAR related services */
#if defined(IMPL_FA) || defined(IMPL_XBAR)
    ASSIGN_FUNCPTR(extDrvMgmtHwIfBindInfo.extDrvI2cMgmtMasterInitFunc, "gtI2cMgmtMasterInit");
    ASSIGN_FUNCPTR(extDrvMgmtHwIfBindInfo.extDrvMgmtReadRegisterFunc, "extDrvMgmtReadRegister");
    ASSIGN_FUNCPTR(extDrvMgmtHwIfBindInfo.extDrvMgmtWriteRegisterFunc, "extDrvMgmtWriteRegister");
    ASSIGN_FUNCPTR(extDrvMgmtHwIfBindInfo.extDrvMgmtIsrReadRegisterFunc, "extDrvMgmtIsrReadRegister");
    ASSIGN_FUNCPTR(extDrvMgmtHwIfBindInfo.extDrvMgmtIsrWriteRegisterFunc, "extDrvMgmtIsrWriteRegister");
#endif

    ASSIGN_FUNCPTR(extDrvDmaBindInfo.extDrvDmaWriteDriverFunc, "extDrvDmaWrite");
    ASSIGN_FUNCPTR(extDrvDmaBindInfo.extDrvDmaReadFunc, "extDrvDmaRead");

    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortRxInitFunc, "extDrvEthPortRxInit");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthRawSocketModeSetFunc, "extDrvEthRawSocketModeSet");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthRawSocketModeGetFunc, "extDrvEthRawSocketModeGet");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvLinuxModeSetFunc, "extDrvLinuxModeSet");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvLinuxModeGetFunc, "extDrvLinuxModeGet");
    ASSIGN_FUNCPTR(extDrvHsuDrvBindInfo.extDrvHsuMemBaseAddrGetFunc, "extDrvHsuMemBaseAddrGet");
    ASSIGN_FUNCPTR(extDrvHsuDrvBindInfo.extDrvHsuWarmRestartFunc, "extDrvHsuWarmRestart");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortTxInitFunc, "extDrvEthPortTxInit");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortEnableFunc, "extDrvEthPortEnable");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortDisableFunc, "extDrvEthPortDisable");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortTxFunc, "extDrvEthPortTx");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortInputHookAddFunc, "extDrvEthInputHookAdd");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthRawSocketRxHookAddFunc, "extDrvEthRawSocketRxHookAdd");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortTxCompleteHookAddFunc, "extDrvEthTxCompleteHookAdd");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortRxPacketFreeFunc, "extDrvEthRxPacketFree");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPortTxModeSetFunc, "extDrvEthPortTxModeSet");
#ifdef XCAT_DRV
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthCpuCodeToQueueFunc, "extDrvEthCpuCodeToQueue");
    ASSIGN_FUNCPTR(extDrvEthPortBindInfo.extDrvEthPrePendTwoBytesHeaderSetFunc, "extDrvEthPrePendTwoBytesHeaderSet");
#endif

    ASSIGN_FUNCPTR(extDrvIntBindInfo.extDrvIntConnectFunc, "extDrvIntConnect");
    ASSIGN_FUNCPTR(extDrvIntBindInfo.extDrvIntDisconnectFunc, "extDrvIntDisconnect");
    ASSIGN_FUNCPTR(extDrvIntBindInfo.extDrvIntEnableFunc, "extDrvIntEnable");
    ASSIGN_FUNCPTR(extDrvIntBindInfo.extDrvIntDisableFunc, "extDrvIntDisable");
    ASSIGN_FUNCPTR(extDrvIntBindInfo.extDrvIntLockModeSetFunc, "extDrvSetIntLockUnlock");

    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciConfigWriteRegFunc, "extDrvPciConfigWriteReg");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciConfigReadRegFunc, "extDrvPciConfigReadReg");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciDevFindFunc, "extDrvPciFindDev");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciIntVecFunc, "extDrvGetPciIntVec");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciIntMaskFunc, "extDrvGetIntMask");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciCombinedAccessEnableFunc, "extDrvEnableCombinedPciAccess");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciDoubleWriteFunc, "extDrvPciDoubleWrite");
    ASSIGN_FUNCPTR(extDrvPciInfo.extDrvPciDoubleReadFunc, "extDrvPciDoubleRead");

    ASSIGN_FUNCPTR(extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaEnableFunc, "extDrvHsuInboundSdmaEnable");
    ASSIGN_FUNCPTR(extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaDisableFunc, "extDrvHsuInboundSdmaDisable");
    ASSIGN_FUNCPTR(extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaStateGetFunc, "extDrvHsuInboundSdmaStateGet");
#ifdef DRAGONITE_TYPE_A1
    ASSIGN_FUNCPTR(extDrvDragoniteInfo.extDrvDragoniteShMemBaseAddrGetFunc, "extDrvDragoniteShMemBaseAddrGet");
#endif 
#undef ASSIGN_FUNCPTR
    closeLibraryHandle(handle);

    return retcode;
}


