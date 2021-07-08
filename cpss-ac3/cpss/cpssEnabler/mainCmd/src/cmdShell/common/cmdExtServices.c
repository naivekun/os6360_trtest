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
* @file cmdExtServices.c
*
* @brief Common definition and APIs for Commander external services.
*
*
* @version   44
********************************************************************************
*/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>

/* hold main cmd external driver CB */
CPSS_EXT_DRV_FUNC_BIND_STC   cmdExtDrvFuncBindInfo;
/* hold main cmd OS CB */
CPSS_OS_FUNC_BIND_STC        cmdOsFuncBind;
/* hold main cmd extra OS CB */
CMD_OS_FUNC_BIND_EXTRA_STC   cmdOsExtraFuncBind;

/* hold main cmd extra CB */
CMD_FUNC_BIND_EXTRA_STC   cmdExtraFuncBind;

/* hold main cmd Trace CB */
CPSS_TRACE_FUNC_BIND_STC  cmdTraceFuncBind;

#define SET_NON_NULL(dst,src)   if(src != NULL) dst = src

#define STR_NOT_IMPLEMENTED_CNS " extServiceFuncNotImplementedCalled \n"
static GT_STATUS extServiceFuncNotImplementedCalled
(
    void
)
{
    if(cmdOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cmdOsPrintf(STR_NOT_IMPLEMENTED_CNS);
    }

    return GT_NOT_IMPLEMENTED;
}


/************* Prototypes *****************************************************/
CPSS_OS_INET_NTOHL_FUNC  cmdOsNtohl                                                  = (CPSS_OS_INET_NTOHL_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_INET_HTONL_FUNC  cmdOsHtonl                                                  = (CPSS_OS_INET_HTONL_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_INET_NTOHS_FUNC  cmdOsNtohs                                                  = (CPSS_OS_INET_NTOHS_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_INET_HTONS_FUNC  cmdOsHtons                                                  = (CPSS_OS_INET_HTONS_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_INET_NTOA_FUNC   cmdOsInetNtoa                                               = (CPSS_OS_INET_NTOA_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_IO_BIND_STDOUT_FUNC cmdOsBindStdOut                                          = (CPSS_OS_IO_BIND_STDOUT_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_IO_PRINTF_FUNC      cmdOsPrintf                                              = (CPSS_OS_IO_PRINTF_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_IO_SPRINTF_FUNC     cmdOsSprintf                                             = (CPSS_OS_IO_SPRINTF_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_IO_PRINT_SYNC_FUNC  cmdOsPrintSync                                           = (CPSS_OS_IO_PRINT_SYNC_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_IO_GETS_FUNC        cmdOsGets                                                = (CPSS_OS_IO_GETS_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_IO_FOPEN_FUNC       cmdOsFopen                                               = (CPSS_OS_IO_FOPEN_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_IO_FCLOSE_FUNC      cmdOsFclose                                              = (CPSS_OS_IO_FCLOSE_FUNC)extServiceFuncNotImplementedCalled;
CPSS_OS_IO_REWIND_FUNC      cmdOsRewind                                              = (CPSS_OS_IO_REWIND_FUNC)extServiceFuncNotImplementedCalled;
CPSS_OS_IO_FPRINTF_FUNC     cmdOsFprintf                                             = (CPSS_OS_IO_FPRINTF_FUNC)extServiceFuncNotImplementedCalled;
CPSS_OS_IO_FGETS_FUNC       cmdOsFgets                                               = (CPSS_OS_IO_FGETS_FUNC)extServiceFuncNotImplementedCalled;

CPSS_OS_BZERO_FUNC             cmdOsBzero                                            = (CPSS_OS_BZERO_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_MEM_SET_FUNC           cmdOsMemSet                                           = (CPSS_OS_MEM_SET_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_MEM_CPY_FUNC           cmdOsMemCpy                                           = (CPSS_OS_MEM_CPY_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_MEM_CMP_FUNC           cmdOsMemCmp                                           = (CPSS_OS_MEM_CMP_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STATIC_MALLOC_FUNC     cmdOsStaticMalloc                                     = (CPSS_OS_STATIC_MALLOC_FUNC) extServiceFuncNotImplementedCalled;
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    CPSS_OS_MALLOC_FUNC            cmdOsMalloc_MemoryLeakageDbg                      = (CPSS_OS_MALLOC_FUNC) extServiceFuncNotImplementedCalled;
    CPSS_OS_REALLOC_FUNC           cmdOsRealloc_MemoryLeakageDbg                     = (CPSS_OS_REALLOC_FUNC) extServiceFuncNotImplementedCalled;
    CPSS_OS_FREE_FUNC              cmdOsFree_MemoryLeakageDbg                        = (CPSS_OS_FREE_FUNC) extServiceFuncNotImplementedCalled;
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    CPSS_OS_MALLOC_FUNC            cmdOsMalloc                                       = (CPSS_OS_MALLOC_FUNC) extServiceFuncNotImplementedCalled;
    CPSS_OS_REALLOC_FUNC           cmdOsRealloc                                      = (CPSS_OS_REALLOC_FUNC) extServiceFuncNotImplementedCalled;
    CPSS_OS_FREE_FUNC              cmdOsFree                                         = (CPSS_OS_FREE_FUNC) extServiceFuncNotImplementedCalled;
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/

CPSS_OS_CACHE_DMA_MALLOC_FUNC  cmdOsCacheDmaMalloc                                   = (CPSS_OS_CACHE_DMA_MALLOC_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_CACHE_DMA_FREE_FUNC    cmdOsCacheDmaFree                                     = (CPSS_OS_CACHE_DMA_FREE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_PHY_TO_VIRT_FUNC       cmdOsPhy2Virt                                         = (CPSS_OS_PHY_TO_VIRT_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_VIRT_TO_PHY_FUNC       cmdOsVirt2Phy                                         = (CPSS_OS_VIRT_TO_PHY_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_MUTEX_CREATE_FUNC        cmdOsMutexCreate                                    = (CPSS_OS_MUTEX_CREATE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_MUTEX_DELETE_FUNC        cmdOsMutexDelete                                    = (CPSS_OS_MUTEX_DELETE_FUNC) extServiceFuncNotImplementedCalled;
#if defined  CPSS_USE_MUTEX_PROFILER
CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC cmdOsMutexSetGlAttributes                      = (CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS cmdOsCheckUnprotectedPerDeviceDbAccess = (CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS) extServiceFuncNotImplementedCalled;
#endif


CPSS_OS_MUTEX_LOCK_FUNC          cmdOsMutexLock                                      = (CPSS_OS_MUTEX_LOCK_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_MUTEX_UNLOCK_FUNC        cmdOsMutexUnlock                                    = (CPSS_OS_MUTEX_UNLOCK_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_SIG_SEM_BIN_CREATE_FUNC  cmdOsSigSemBinCreate                                = (CPSS_OS_SIG_SEM_BIN_CREATE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_SIG_SEM_DELETE_FUNC      cmdOsSigSemDelete                                   = (CPSS_OS_SIG_SEM_DELETE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_SIG_SEM_WAIT_FUNC        cmdOsSigSemWait                                     = (CPSS_OS_SIG_SEM_WAIT_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_SIG_SEM_SIGNAL_FUNC      cmdOsSigSemSignal                                   = (CPSS_OS_SIG_SEM_SIGNAL_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_STR_LEN_FUNC     cmdOsStrlen                                                 = (CPSS_OS_STR_LEN_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_CPY_FUNC     cmdOsStrCpy                                                 = (CPSS_OS_STR_CPY_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_CHR_FUNC     cmdOsStrChr                                                 = (CPSS_OS_STR_CHR_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_REV_CHR_FUNC cmdOsStrRevChr                                              = (CPSS_OS_STR_REV_CHR_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_CMP_FUNC     cmdOsStrCmp                                                 = (CPSS_OS_STR_CMP_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_CAT_FUNC     cmdOsStrCat                                                 = (CPSS_OS_STR_CAT_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_N_CAT_FUNC   cmdOsStrNCat                                                = (CPSS_OS_STR_N_CAT_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_TO_UPPER_FUNC    cmdOsToUpper                                                = (CPSS_OS_TO_UPPER_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_TO_32_FUNC   cmdOsStrTo32                                                = (CPSS_OS_STR_TO_32_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_TO_U32_FUNC  cmdOsStrToU32                                               = (CPSS_OS_STR_TO_U32_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_STR_TOL_FUNC     cmdOsStrTol                                                 = (CPSS_OS_STR_TOL_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_TIME_WK_AFTER_FUNC  cmdOsTimerWkAfter                                        = (CPSS_OS_TIME_WK_AFTER_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_TIME_TICK_GET_FUNC  cmdOsTickGet                                             = (CPSS_OS_TIME_TICK_GET_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_TIME_GET_FUNC       cmdOsTime                                                = (CPSS_OS_TIME_GET_FUNC) extServiceFuncNotImplementedCalled;

CPSS_OS_TASK_CREATE_FUNC    cmdOsTaskCreate                                          = (CPSS_OS_TASK_CREATE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_OS_TASK_DELETE_FUNC    cmdOsTaskDelete                                          = (CPSS_OS_TASK_DELETE_FUNC) extServiceFuncNotImplementedCalled;

CPSS_TRACE_HW_ACCESS_WRITE_FUNC     cmdTraceHwAccessWrite                            = (CPSS_TRACE_HW_ACCESS_WRITE_FUNC) extServiceFuncNotImplementedCalled;
CPSS_TRACE_HW_ACCESS_READ_FUNC      cmdTraceHwAccessRead                             = (CPSS_TRACE_HW_ACCESS_READ_FUNC) extServiceFuncNotImplementedCalled;
CPSS_TRACE_HW_ACCESS_DELAY_FUNC      cmdTraceHwAccessDelay                           = (CPSS_TRACE_HW_ACCESS_DELAY_FUNC) extServiceFuncNotImplementedCalled;

CMD_OS_TASK_GET_SELF_FUN    cmdOsTaskGetSelf                                         = (CMD_OS_TASK_GET_SELF_FUN) extServiceFuncNotImplementedCalled;
CMD_OS_TASK_SET_TASK_PRIOR_FUNC cmdOsSetTaskPrior                                    = (CMD_OS_TASK_SET_TASK_PRIOR_FUNC) extServiceFuncNotImplementedCalled;


CMD_OS_SOCKET_TCP_CREATE_FUN        cmdOsSocketTcpCreate                                      = (CMD_OS_SOCKET_TCP_CREATE_FUN     ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_UDP_CREATE_FUN        cmdOsSocketUdpCreate                                      = (CMD_OS_SOCKET_UDP_CREATE_FUN     ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_TCP_DESTROY_FUN       cmdOsSocketTcpDestroy                                     = (CMD_OS_SOCKET_TCP_DESTROY_FUN    ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_UDP_DESTROY_FUN       cmdOsSocketUdpDestroy                                     = (CMD_OS_SOCKET_UDP_DESTROY_FUN    ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_CREATE_ADDR_FUN       cmdOsSocketCreateAddr                                     = (CMD_OS_SOCKET_CREATE_ADDR_FUN    ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_DESTROY_ADDR_FUN      cmdOsSocketDestroyAddr                                    = (CMD_OS_SOCKET_DESTROY_ADDR_FUN   ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_BIND_FUN              cmdOsSocketBind                                           = (CMD_OS_SOCKET_BIND_FUN           ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_LISTEN_FUN            cmdOsSocketListen                                         = (CMD_OS_SOCKET_LISTEN_FUN         ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_ACCEPT_FUN            cmdOsSocketAccept                                         = (CMD_OS_SOCKET_ACCEPT_FUN         ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_CONNECT_FUN           cmdOsSocketConnect                                        = (CMD_OS_SOCKET_CONNECT_FUN        ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SET_NON_BLOCK_FUN     cmdOsSocketSetNonBlock                                    = (CMD_OS_SOCKET_SET_NON_BLOCK_FUN  ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SET_BLOCK_FUN         cmdOsSocketSetBlock                                       = (CMD_OS_SOCKET_SET_BLOCK_FUN      ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SEND_FUN              cmdOsSocketSend                                           = (CMD_OS_SOCKET_SEND_FUN           ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SEND_TO_FUN           cmdOsSocketSendTo                                         = (CMD_OS_SOCKET_SEND_TO_FUN        ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_RECV_FUN              cmdOsSocketRecv                                           = (CMD_OS_SOCKET_RECV_FUN           ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_RECV_FROM_FUN         cmdOsSocketRecvFrom                                       = (CMD_OS_SOCKET_RECV_FROM_FUN      ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SET_SOCKET_NO_LINGER_FUN cmdOsSocketSetSocketNoLinger                           = (CMD_OS_SOCKET_SET_SOCKET_NO_LINGER_FUN) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_EXTRACT_IP_ADDR_FROM_SOCKET_ADDR_FUN cmdOsSocketExtractIpAddrFromSocketAddr     = (CMD_OS_SOCKET_EXTRACT_IP_ADDR_FROM_SOCKET_ADDR_FUN) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_GET_SOCKET_ADDR_SIZE_FUN cmdOsSocketGetSocketAddrSize                           = (CMD_OS_SOCKET_GET_SOCKET_ADDR_SIZE_FUN) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_SHUT_DOWN_FUN         cmdOsSocketShutDown                                       = (CMD_OS_SOCKET_SHUT_DOWN_FUN) extServiceFuncNotImplementedCalled;

CMD_OS_SELECT_CREATE_SET_FUN        cmdOsSelectCreateSet                                      = (CMD_OS_SELECT_CREATE_SET_FUN       ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_ERASE_SET_FUN         cmdOsSelectEraseSet                                       = (CMD_OS_SELECT_ERASE_SET_FUN        ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_ZERO_SET_FUN          cmdOsSelectZeroSet                                        = (CMD_OS_SELECT_ZERO_SET_FUN         ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_ADD_FD_TO_SET_FUN     cmdOsSelectAddFdToSet                                     = (CMD_OS_SELECT_ADD_FD_TO_SET_FUN    ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_CLEAR_FD_FROM_SET_FUN cmdOsSelectClearFdFromSet                                 = (CMD_OS_SELECT_CLEAR_FD_FROM_SET_FUN) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_IS_FD_SET_FUN         cmdOsSelectIsFdSet                                        = (CMD_OS_SELECT_IS_FD_SET_FUN        ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_COPY_SET_FUN          cmdOsSelectCopySet                                        = (CMD_OS_SELECT_COPY_SET_FUN         ) extServiceFuncNotImplementedCalled;
CMD_OS_SELECT_FUN                   cmdOsSelect                                               = (CMD_OS_SELECT_FUN                  ) extServiceFuncNotImplementedCalled;
CMD_OS_SOCKET_GET_SOCKET_FD_SET_SIZE_FUN cmdOsSocketGetSocketFdSetSize                        = (CMD_OS_SOCKET_GET_SOCKET_FD_SET_SIZE_FUN) extServiceFuncNotImplementedCalled;


CMD_OS_POOL_CREATE_POOL_FUNC        cmdOsPoolCreatePool                                       = (CMD_OS_POOL_CREATE_POOL_FUNC     ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_CREATE_DMA_POOL_FUNC    cmdOsPoolCreateDmaPool                                    = (CMD_OS_POOL_CREATE_DMA_POOL_FUNC ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_RE_CREATE_POOL_FUNC     cmdOsPoolReCreatePool                                     = (CMD_OS_POOL_RE_CREATE_POOL_FUNC  ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_DELETE_POOL_FUNC        cmdOsPoolDeletePool                                       = (CMD_OS_POOL_DELETE_POOL_FUNC     ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_GET_BUF_FUNC            cmdOsPoolGetBuf                                           = (CMD_OS_POOL_GET_BUF_FUNC         ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_FREE_BUF_FUNC           cmdOsPoolFreeBuf                                          = (CMD_OS_POOL_FREE_BUF_FUNC        ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_GET_BUF_SIZE_FUNC       cmdOsPoolGetBufSize                                       = (CMD_OS_POOL_GET_BUF_SIZE_FUNC    ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_EXPAND_POOL_FUNC        cmdOsPoolExpandPool                                       = (CMD_OS_POOL_EXPAND_POOL_FUNC     ) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_GET_BUF_FREE_CNT_FUNC   cmdOsPoolGetBufFreeCnt                                    = (CMD_OS_POOL_GET_BUF_FREE_CNT_FUNC) extServiceFuncNotImplementedCalled;
CMD_OS_POOL_PRINT_STATS_FUNC        cmdOsPoolPrintStats                                       = (CMD_OS_POOL_PRINT_STATS_FUNC     ) extServiceFuncNotImplementedCalled;

CMD_CPU_ETHERNET_IS_CPU_ETHER_PORT_USED cmdIsCpuEtherPortUsed                                 = (CMD_CPU_ETHERNET_IS_CPU_ETHER_PORT_USED) extServiceFuncNotImplementedCalled;

CMD_CPSS_EVENT_BIND_FUNC            cmdCpssEventBind                                          = (CMD_CPSS_EVENT_BIND_FUNC           ) extServiceFuncNotImplementedCalled;
CMD_CPSS_EVENT_SELECT_FUNC          cmdCpssEventSelect                                        = (CMD_CPSS_EVENT_SELECT_FUNC         ) extServiceFuncNotImplementedCalled;
CMD_CPSS_EVENT_RECV_FUNC            cmdCpssEventRecv                                          = (CMD_CPSS_EVENT_RECV_FUNC           ) extServiceFuncNotImplementedCalled;
CMD_CPSS_EVENT_DEVICE_MASK_SET_FUNC cmdCpssEventDeviceMaskSet                                 = (CMD_CPSS_EVENT_DEVICE_MASK_SET_FUNC) extServiceFuncNotImplementedCalled;


CMD_APP_DB_ENTRY_ADD_FUNC       cmdAppDbEntryAdd                                              = (CMD_APP_DB_ENTRY_ADD_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_DB_ENTRY_GET_FUNC       cmdAppDbEntryGet                                              = (CMD_APP_DB_ENTRY_GET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_DB_DUMP_FUNC            cmdAppDbDump                                                  = (CMD_APP_DB_DUMP_FUNC     ) extServiceFuncNotImplementedCalled;
CMD_APP_PP_CONFIG_GET_FUNC      cmdAppPpConfigGet                                             = (CMD_APP_PP_CONFIG_GET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_INIT_SYSTEM_FUNC        cmdInitSystem                                                 = (CMD_APP_INIT_SYSTEM_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_INIT_SYSTEM_GET_FUNC    cmdInitSystemGet                                              = (CMD_APP_INIT_SYSTEM_GET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_RESET_SYSTEM_FUNC       cmdResetSystem                                                = (CMD_APP_RESET_SYSTEM_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_INIT_CONFI_FUNC         cmdInitConfi                                                 = (CMD_APP_INIT_CONFI_FUNC)   extServiceFuncNotImplementedCalled;
CMD_APP_SHOW_BOARDS_LIST_FUNC   cmdAppShowBoardsList                                          = (CMD_APP_SHOW_BOARDS_LIST_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_IS_SYSTEM_INITIALIZED_FUNC  cmdAppIsSystemInitialized                                 = (CMD_APP_IS_SYSTEM_INITIALIZED_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_PP_CONFIG_PRINT_FUNC  cmdAppPpConfigPrint                                             = (CMD_APP_PP_CONFIG_PRINT_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_ALLOW_PROCESSING_OF_AUQ_MESSAGES_FUNC cmdAppAllowProcessingOfAuqMessages              = (CMD_APP_ALLOW_PROCESSING_OF_AUQ_MESSAGES_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_TRACE_HW_ACCESS_ENABLE_FUNC cmdAppTraceHwAccessEnable                                 = (CMD_APP_TRACE_HW_ACCESS_ENABLE_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_TRACE_HW_ACCESS_OUTPUT_MODE_SET_FUNC cmdAppTraceHwAccessOutputModeSet                 = (CMD_APP_TRACE_HW_ACCESS_OUTPUT_MODE_SET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_START_HEAP_ALLOC_COUNTER_FUNC cmdAppStartHeapAllocCounter                             = (CMD_APP_START_HEAP_ALLOC_COUNTER_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_GET_HEAP_ALLOC_COUNTER_FUNC   cmdAppGetHeapAllocCounter                               = (CMD_APP_GET_HEAP_ALLOC_COUNTER_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_EVANT_FATAL_ERROR_ENABLE_FUNC cmdAppEventFatalErrorEnable                             = (CMD_APP_EVANT_FATAL_ERROR_ENABLE_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_EVANT_DATA_BASE_GET_FUNC cmdAppDemoEventsDataBaseGet                                  = (CMD_APP_EVANT_DATA_BASE_GET_FUNC) extServiceFuncNotImplementedCalled;

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
CMD_APP_PORT_RATE_TABLE_GET_FUNC                                    cmdPortRateTableGet = (CMD_APP_PORT_RATE_TABLE_GET_FUNC) extServiceFuncNotImplementedCalled;
#endif

#ifdef CHX_FAMILY
CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC                               cmdAppLionPortModeSpeedSet = (CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_XCAT2_STACK_PORTS_MODE_SPEED_SET_FUNC                       cmdAppXcat2StackPortsModeSpeedSet = (CMD_APP_LION_PORT_MODE_SPEED_SET_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_BC2_PORTS_CONFIG_FUNC                                       cmdAppBc2PortsConfig = (CMD_APP_BC2_PORTS_CONFIG_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_BOBK_PORTS_CONFIG_FUNC                                      cmdAppBobkPortsConfig = (CMD_APP_BOBK_PORTS_CONFIG_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_XCAT2_SFP_PORT_ENABLE_FUNC                                  cmdAppXcat2SfpPortEnable = (CMD_APP_XCAT2_SFP_PORT_ENABLE_FUNC) extServiceFuncNotImplementedCalled;
CMD_APP_DXCH_EXTERNAL_HSU_OLD_IMAGE_PRE_UPDATE_PREPARATION_FUNC     cmdAppDxChHsuOldImagePreUpdatePreparation        = (CMD_APP_DXCH_EXTERNAL_HSU_OLD_IMAGE_PRE_UPDATE_PREPARATION_FUNC ) extServiceFuncNotImplementedCalled;
CMD_APP_DXCH_EXTERNAL_HSU_NEW_IMAGE_UPDATE_AND_SYNCH_FUNC           cmdAppDxChHsuNewImageUpdateAndSynch              = (CMD_APP_DXCH_EXTERNAL_HSU_NEW_IMAGE_UPDATE_AND_SYNCH_FUNC       ) extServiceFuncNotImplementedCalled;
CMD_APP_TRUNK_CASCADE_TRUNK_PORTS_SET_FUNC                          cmdAppTrunkCascadeTrunkPortsSet = 0;
CMD_APP_TRUNK_MEMBERS_SET_FUNC                                      cmdAppTrunkMembersSet = 0;
CMD_APP_TRUNK_MEMBER_ADD_FUNC                                       cmdAppTrunkMemberAdd = 0;
CMD_APP_TRUNK_MEMBER_DISABLE_FUNC                                   cmdAppTrunkMemberDisable = 0;
CMD_APP_TRUNK_MEMBER_ENABLE_FUNC                                    cmdAppTrunkMemberEnable = 0;
CMD_APP_TRUNK_MEMBER_REMOVE_FUNC                                    cmdAppTrunkMemberRemove = 0;
CMD_APP_TRUNK_UPDATED_PORTS_GET_FUNC                                cmdAppTrunkUpdatedPortsGet = 0;
#endif /*CHX_FAMILY*/

/**
* @internal cmdExtServicesDump function
* @endinternal
*
* @brief   print which one of the external services was not bound
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdExtServicesDump
(
    void
)
{
    GT_U32  ii,jj;
    void*   *currPtr;
    GT_U32  sizes[] = {
                sizeof(cmdExtDrvFuncBindInfo) / sizeof(void*),
                sizeof(cmdOsFuncBind        ) / sizeof(void*),
                sizeof(cmdOsExtraFuncBind   ) / sizeof(void*),
                sizeof(cmdExtraFuncBind     ) / sizeof(void*),
                0};
    void**  ptrArray[]={
                (void*)&cmdExtDrvFuncBindInfo,
                (void*)&cmdOsFuncBind        ,
                (void*)&cmdOsExtraFuncBind   ,
                (void*)&cmdExtraFuncBind     ,
                0};


    jj=0;
    do
    {
        currPtr = ptrArray[jj];

        for(ii = 0 ; ii < sizes[jj] ; ii++ , currPtr++)
        {
            if((*currPtr) == (void*)extServiceFuncNotImplementedCalled)
            {
                cmdOsPrintf("in section [%ld] , function index [%ld] was not bound \n ",
                    jj,ii);
            }
            else if((*currPtr) == NULL)
            {
                cmdOsPrintf("in section [%ld] , function index [%ld] was given NULL \n ",
                    jj,ii);
            }

        }

        jj++;
    }while(sizes[++jj]);


    return GT_OK;

}

/**
* @internal cmdInitExtServices function
* @endinternal
*
* @brief   commander external services initialization
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdInitExtServices
(
    void
)
{
    static GT_BOOL cmdInitExtServices_done = GT_FALSE;
    if (cmdInitExtServices_done == GT_TRUE)
    {
        return GT_OK;
    }
    if(GT_OK != applicationExtServicesBind(&cmdExtDrvFuncBindInfo,&cmdOsFuncBind,&cmdOsExtraFuncBind,&cmdExtraFuncBind, &cmdTraceFuncBind))
    {
        return GT_FAIL;
    }

    SET_NON_NULL(cmdOsNtohl                          , cmdOsFuncBind.osInetBindInfo.osInetNtohlFunc         );
    SET_NON_NULL(cmdOsHtonl                          , cmdOsFuncBind.osInetBindInfo.osInetHtonlFunc         );
    SET_NON_NULL(cmdOsNtohs                          , cmdOsFuncBind.osInetBindInfo.osInetNtohsFunc         );
    SET_NON_NULL(cmdOsHtons                          , cmdOsFuncBind.osInetBindInfo.osInetHtonsFunc         );
    SET_NON_NULL(cmdOsInetNtoa                       , cmdOsFuncBind.osInetBindInfo.osInetNtoaFunc          );

    SET_NON_NULL(cmdOsBindStdOut                     , cmdOsFuncBind.osIoBindInfo.osIoBindStdOutFunc        );
    SET_NON_NULL(cmdOsPrintf                         , cmdOsFuncBind.osIoBindInfo.osIoPrintfFunc            );
    SET_NON_NULL(cmdOsSprintf                        , cmdOsFuncBind.osIoBindInfo.osIoSprintfFunc           );
    SET_NON_NULL(cmdOsPrintSync                      , cmdOsFuncBind.osIoBindInfo.osIoPrintSynchFunc        );
    SET_NON_NULL(cmdOsGets                           , cmdOsFuncBind.osIoBindInfo.osIoGetsFunc              );

    SET_NON_NULL(cmdOsFopen                          , cmdOsFuncBind.osIoBindInfo.osIoFopenFunc             );
    SET_NON_NULL(cmdOsFclose                         , cmdOsFuncBind.osIoBindInfo.osIoFcloseFunc            );
    SET_NON_NULL(cmdOsRewind                         , cmdOsFuncBind.osIoBindInfo.osIoRewindFunc            );
    SET_NON_NULL(cmdOsFprintf                        , cmdOsFuncBind.osIoBindInfo.osIoFprintfFunc           );
    SET_NON_NULL(cmdOsFgets                          , cmdOsFuncBind.osIoBindInfo.osIoFgets                 );

    SET_NON_NULL(cmdOsBzero                          , cmdOsFuncBind.osMemBindInfo.osMemBzeroFunc           );
    SET_NON_NULL(cmdOsMemSet                         , cmdOsFuncBind.osMemBindInfo.osMemSetFunc             );
    SET_NON_NULL(cmdOsMemCpy                         , cmdOsFuncBind.osMemBindInfo.osMemCpyFunc             );
    SET_NON_NULL(cmdOsMemCmp                         , cmdOsFuncBind.osMemBindInfo.osMemCmpFunc             );
    SET_NON_NULL(cmdOsStaticMalloc                   , cmdOsFuncBind.osMemBindInfo.osMemStaticMallocFunc    );
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    SET_NON_NULL(cmdOsMalloc_MemoryLeakageDbg        , cmdOsFuncBind.osMemBindInfo.osMemMallocFunc          );
    SET_NON_NULL(cmdOsRealloc_MemoryLeakageDbg       , cmdOsFuncBind.osMemBindInfo.osMemReallocFunc         );
    SET_NON_NULL(cmdOsFree_MemoryLeakageDbg          , cmdOsFuncBind.osMemBindInfo.osMemFreeFunc            );
#else  /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    SET_NON_NULL(cmdOsMalloc                         , cmdOsFuncBind.osMemBindInfo.osMemMallocFunc          );
    SET_NON_NULL(cmdOsRealloc                        , cmdOsFuncBind.osMemBindInfo.osMemReallocFunc         );
    SET_NON_NULL(cmdOsFree                           , cmdOsFuncBind.osMemBindInfo.osMemFreeFunc            );
#endif /*!OS_MALLOC_MEMORY_LEAKAGE_DBG*/
    SET_NON_NULL(cmdOsCacheDmaMalloc                 , cmdOsFuncBind.osMemBindInfo.osMemCacheDmaMallocFunc  );
    SET_NON_NULL(cmdOsCacheDmaFree                   , cmdOsFuncBind.osMemBindInfo.osMemCacheDmaFreeFunc    );
    SET_NON_NULL(cmdOsPhy2Virt                       , cmdOsFuncBind.osMemBindInfo.osMemPhyToVirtFunc       );
    SET_NON_NULL(cmdOsVirt2Phy                       , cmdOsFuncBind.osMemBindInfo.osMemVirtToPhyFunc       );

    SET_NON_NULL(cmdOsMutexCreate                    , cmdOsFuncBind.osSemBindInfo.osMutexCreateFunc  );
    SET_NON_NULL(cmdOsMutexDelete                    , cmdOsFuncBind.osSemBindInfo.osMutexDeleteFunc     );
 #if defined  CPSS_USE_MUTEX_PROFILER
    SET_NON_NULL(cmdOsMutexSetGlAttributes                    , cmdOsFuncBind.osSemBindInfo.osMutexSetGlAttributesFunc);
    SET_NON_NULL(cmdOsCheckUnprotectedPerDeviceDbAccess                    , cmdOsFuncBind.osSemBindInfo.osCheckUnprotectedPerDeviceDbAccess);
 #endif
    SET_NON_NULL(cmdOsMutexLock                      , cmdOsFuncBind.osSemBindInfo.osMutexLockFunc       );
    SET_NON_NULL(cmdOsMutexUnlock                    , cmdOsFuncBind.osSemBindInfo.osMutexUnlockFunc     );

    SET_NON_NULL(cmdOsSigSemBinCreate                , cmdOsFuncBind.osSemBindInfo.osSigSemBinCreateFunc    );
    SET_NON_NULL(cmdOsSigSemDelete                   , cmdOsFuncBind.osSemBindInfo.osSigSemDeleteFunc       );
    SET_NON_NULL(cmdOsSigSemWait                     , cmdOsFuncBind.osSemBindInfo.osSigSemWaitFunc         );
    SET_NON_NULL(cmdOsSigSemSignal                   , cmdOsFuncBind.osSemBindInfo.osSigSemSignalFunc       );

    SET_NON_NULL(cmdOsStrlen                         , cmdOsFuncBind.osStrBindInfo.osStrlenFunc             );
    SET_NON_NULL(cmdOsStrCpy                         , cmdOsFuncBind.osStrBindInfo.osStrCpyFunc             );
    SET_NON_NULL(cmdOsStrChr                         , cmdOsFuncBind.osStrBindInfo.osStrChrFunc             );
    SET_NON_NULL(cmdOsStrRevChr                      , cmdOsFuncBind.osStrBindInfo.osStrRevChrFunc          );
    SET_NON_NULL(cmdOsStrCmp                         , cmdOsFuncBind.osStrBindInfo.osStrCmpFunc             );
    SET_NON_NULL(cmdOsStrCat                         , cmdOsFuncBind.osStrBindInfo.osStrCatFunc             );
    SET_NON_NULL(cmdOsStrNCat                        , cmdOsFuncBind.osStrBindInfo.osStrStrNCatFunc         );
    SET_NON_NULL(cmdOsToUpper                        , cmdOsFuncBind.osStrBindInfo.osStrChrToUpperFunc      );
    SET_NON_NULL(cmdOsStrTo32                        , cmdOsFuncBind.osStrBindInfo.osStrTo32Func            );
    SET_NON_NULL(cmdOsStrToU32                       , cmdOsFuncBind.osStrBindInfo.osStrToU32Func           );
    SET_NON_NULL(cmdOsStrTol                         , cmdOsFuncBind.osStrBindInfo.osStrTolFunc             );

    SET_NON_NULL(cmdOsTimerWkAfter                   , cmdOsFuncBind.osTimeBindInfo.osTimeWkAfterFunc       );
    SET_NON_NULL(cmdOsTickGet                        , cmdOsFuncBind.osTimeBindInfo.osTimeTickGetFunc       );
    SET_NON_NULL(cmdOsTime                           , cmdOsFuncBind.osTimeBindInfo.osTimeGetFunc           );

    SET_NON_NULL(cmdOsTaskCreate                     , cmdOsFuncBind.osTaskBindInfo.osTaskCreateFunc        );
    SET_NON_NULL(cmdOsTaskDelete                     , cmdOsFuncBind.osTaskBindInfo.osTaskDeleteFunc        );

    SET_NON_NULL(cmdOsTaskGetSelf                    , cmdOsExtraFuncBind.osTasksBindInfo.osTaskGetSelf     );
    SET_NON_NULL(cmdOsSetTaskPrior                   , cmdOsExtraFuncBind.osTasksBindInfo.osSetTaskPrior    );

    SET_NON_NULL(cmdOsSocketTcpCreate                , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketTcpCreate                           );
    SET_NON_NULL(cmdOsSocketUdpCreate                , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketUdpCreate                           );
    SET_NON_NULL(cmdOsSocketTcpDestroy               , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketTcpDestroy                          );
    SET_NON_NULL(cmdOsSocketUdpDestroy               , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketUdpDestroy                          );
    SET_NON_NULL(cmdOsSocketCreateAddr               , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketCreateAddr                          );
    SET_NON_NULL(cmdOsSocketDestroyAddr              , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketDestroyAddr                         );
    SET_NON_NULL(cmdOsSocketBind                     , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketBind                                );
    SET_NON_NULL(cmdOsSocketListen                   , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketListen                              );
    SET_NON_NULL(cmdOsSocketAccept                   , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketAccept                              );
    SET_NON_NULL(cmdOsSocketConnect                  , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketConnect                             );
    SET_NON_NULL(cmdOsSocketSetNonBlock              , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketSetNonBlock                         );
    SET_NON_NULL(cmdOsSocketSetBlock                 , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketSetBlock                            );
    SET_NON_NULL(cmdOsSocketSend                     , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketSend                                );
    SET_NON_NULL(cmdOsSocketSendTo                   , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketSendTo                              );
    SET_NON_NULL(cmdOsSocketRecv                     , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketRecv                                );
    SET_NON_NULL(cmdOsSocketRecvFrom                 , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketRecvFrom                            );
    SET_NON_NULL(cmdOsSocketSetSocketNoLinger        , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketSetSocketNoLinger                   );
    SET_NON_NULL(cmdOsSocketExtractIpAddrFromSocketAddr, cmdOsExtraFuncBind.osSocketsBindInfo.osSocketExtractIpAddrFromSocketAddr       );
    SET_NON_NULL(cmdOsSocketGetSocketAddrSize        , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketGetSocketAddrSize                   );
    SET_NON_NULL(cmdOsSocketShutDown                 , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketShutDown                            );
    SET_NON_NULL(cmdOsSelectCreateSet                , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectCreateSet                           );
    SET_NON_NULL(cmdOsSelectEraseSet                 , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectEraseSet                            );
    SET_NON_NULL(cmdOsSelectZeroSet                  , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectZeroSet                             );
    SET_NON_NULL(cmdOsSelectAddFdToSet               , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectAddFdToSet                          );
    SET_NON_NULL(cmdOsSelectClearFdFromSet           , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectClearFdFromSet                      );
    SET_NON_NULL(cmdOsSelectIsFdSet                  , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectIsFdSet                             );
    SET_NON_NULL(cmdOsSelectCopySet                  , cmdOsExtraFuncBind.osSocketsBindInfo.osSelectCopySet                             );
    SET_NON_NULL(cmdOsSelect                         , cmdOsExtraFuncBind.osSocketsBindInfo.osSelect                                    );
    SET_NON_NULL(cmdOsSocketGetSocketFdSetSize       , cmdOsExtraFuncBind.osSocketsBindInfo.osSocketGetSocketFdSetSize                  );

    SET_NON_NULL(cmdOsPoolCreatePool                 , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolCreatePool                              );
    SET_NON_NULL(cmdOsPoolCreateDmaPool              , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolCreateDmaPool                           );
    SET_NON_NULL(cmdOsPoolReCreatePool               , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolReCreatePool                            );
    SET_NON_NULL(cmdOsPoolDeletePool                 , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolDeletePool                              );
    SET_NON_NULL(cmdOsPoolGetBuf                     , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolGetBuf                                  );
    SET_NON_NULL(cmdOsPoolFreeBuf                    , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolFreeBuf                                 );
    SET_NON_NULL(cmdOsPoolGetBufSize                 , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolGetBufSize                              );
    SET_NON_NULL(cmdOsPoolExpandPool                 , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolExpandPool                              );
    SET_NON_NULL(cmdOsPoolGetBufFreeCnt              , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolGetBufFreeCnt                           );
    SET_NON_NULL(cmdOsPoolPrintStats                 , cmdOsExtraFuncBind.osPoolsBindInfo.gtPoolPrintStats                              );

    SET_NON_NULL(cmdTraceHwAccessWrite                 , cmdTraceFuncBind.traceHwBindInfo.traceHwAccessWriteFunc                        );
    SET_NON_NULL(cmdTraceHwAccessRead                  , cmdTraceFuncBind.traceHwBindInfo.traceHwAccessReadFunc                        );
    SET_NON_NULL(cmdTraceHwAccessDelay                 , cmdTraceFuncBind.traceHwBindInfo.traceHwAccessDelayFunc                        );

    SET_NON_NULL(cmdIsCpuEtherPortUsed               , cmdExtraFuncBind.cpuEthernetPortBindInfo.cmdIsCpuEtherPortUsed               );

    SET_NON_NULL(cmdCpssEventBind                    , cmdExtraFuncBind.eventsBindInfo.cmdCpssEventBind                             );
    SET_NON_NULL(cmdCpssEventSelect                  , cmdExtraFuncBind.eventsBindInfo.cmdCpssEventSelect                           );
    SET_NON_NULL(cmdCpssEventRecv                    , cmdExtraFuncBind.eventsBindInfo.cmdCpssEventRecv                             );
    SET_NON_NULL(cmdCpssEventDeviceMaskSet           , cmdExtraFuncBind.eventsBindInfo.cmdCpssEventDeviceMaskSet                    );

    SET_NON_NULL(cmdAppDbEntryAdd                    , cmdExtraFuncBind.appDbBindInfo.cmdAppDbEntryAdd                              );
    SET_NON_NULL(cmdAppDbEntryGet                    , cmdExtraFuncBind.appDbBindInfo.cmdAppDbEntryGet                              );
    SET_NON_NULL(cmdAppDbDump                        , cmdExtraFuncBind.appDbBindInfo.cmdAppDbDump                                  );
    SET_NON_NULL(cmdAppPpConfigGet                   , cmdExtraFuncBind.appDbBindInfo.cmdAppPpConfigGet                             );
    SET_NON_NULL(cmdInitSystem                       , cmdExtraFuncBind.appDbBindInfo.cmdInitSystem                                 );
    SET_NON_NULL(cmdInitSystemGet                    , cmdExtraFuncBind.appDbBindInfo.cmdInitSystemGet                              );
    SET_NON_NULL(cmdResetSystem                      , cmdExtraFuncBind.appDbBindInfo.cmdResetSystem                                );
    SET_NON_NULL(cmdInitConfi                        , cmdExtraFuncBind.appDbBindInfo.cmdInitConfi                                  );
    SET_NON_NULL(cmdAppShowBoardsList                , cmdExtraFuncBind.appDbBindInfo.cmdAppShowBoardsList                          );
    SET_NON_NULL(cmdAppIsSystemInitialized           , cmdExtraFuncBind.appDbBindInfo.cmdAppIsSystemInitialized                     );
    SET_NON_NULL(cmdAppPpConfigPrint                 , cmdExtraFuncBind.appDbBindInfo.cmdAppPpConfigPrint                           );
    SET_NON_NULL(cmdAppAllowProcessingOfAuqMessages  , cmdExtraFuncBind.appDbBindInfo.cmdAppAllowProcessingOfAuqMessages            );
    SET_NON_NULL(cmdAppTraceHwAccessEnable           , cmdExtraFuncBind.appDbBindInfo.cmdAppTraceHwAccessEnable                     );
    SET_NON_NULL(cmdAppTraceHwAccessOutputModeSet    , cmdExtraFuncBind.appDbBindInfo.cmdAppTraceHwAccessOutputModeSet              );
    SET_NON_NULL(cmdAppStartHeapAllocCounter         , cmdExtraFuncBind.appDbBindInfo.cmdAppStartHeapAllocCounter                   );
    SET_NON_NULL(cmdAppGetHeapAllocCounter           , cmdExtraFuncBind.appDbBindInfo.cmdAppGetHeapAllocCounter                     );
    SET_NON_NULL(cmdAppEventFatalErrorEnable         , cmdExtraFuncBind.appDbBindInfo.cmdAppEventFatalErrorEnable                   );
    SET_NON_NULL(cmdAppDemoEventsDataBaseGet         , cmdExtraFuncBind.appDbBindInfo.cmdAppDemoEventsDataBaseGet                   );

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    SET_NON_NULL(cmdPortRateTableGet                                 , cmdExtraFuncBind.appDbBindInfo.cmdPortRateTableGet                               );
#endif
#ifdef CHX_FAMILY
    SET_NON_NULL(cmdAppLionPortModeSpeedSet                          , cmdExtraFuncBind.appDbBindInfo.cmdAppLionPortModeSpeedSet                        );
    SET_NON_NULL(cmdAppBc2PortsConfig                                , cmdExtraFuncBind.appDbBindInfo.cmdAppBc2PortsConfig                              );
    SET_NON_NULL(cmdAppBobkPortsConfig                               , cmdExtraFuncBind.appDbBindInfo.cmdAppBobkPortsConfig                             );
    SET_NON_NULL(cmdAppXcat2StackPortsModeSpeedSet                   , cmdExtraFuncBind.appDbBindInfo.cmdAppXcat2StackPortsModeSpeedSet                 );
    SET_NON_NULL(cmdAppXcat2SfpPortEnable                            , cmdExtraFuncBind.appDbBindInfo.cmdAppXcat2SfpPortEnable                          );
    SET_NON_NULL(cmdAppDxChHsuOldImagePreUpdatePreparation           , cmdExtraFuncBind.appDbBindInfo.cmdAppDxChHsuOldImagePreUpdatePreparation         );
    SET_NON_NULL(cmdAppDxChHsuNewImageUpdateAndSynch                 , cmdExtraFuncBind.appDbBindInfo.cmdAppDxChHsuNewImageUpdateAndSynch               );

#endif
    cmdInitExtServices_done = GT_TRUE;
    return GT_OK;
}


#ifdef GALTIS_NO_APP_DEMO

extern GT_STATUS cmdInit
(
    IN  GT_U32  devNum
);

/**
* @internal userAppInit function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS userAppInit(GT_VOID)
{
    /* Start the command shell */
    return cmdInit(0);
} /* userAppInit */

/**
* @internal applicationExtServicesBind function
* @endinternal
*
* @brief   the mainCmd calls this function , so the application (that implement
*         this function) will bind the mainCmd (and GaltisWrapper) with OS ,
*         external driver functions and other application functions
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of external driver call back functions
* @param[out] osFuncBindPtr            - (pointer to) set of OS call back functions
* @param[out] osExtraFuncBindPtr       - (pointer to) set of extra OS call back functions (that CPSS not use)
* @param[out] extraFuncBindPtr         - (pointer to) set of extra call back functions (that CPSS not use) (non OS functions)
* @param[out] traceFuncBindPtr         - (pointer to) set of Trace call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This function must be implemented by the Application !!!
*
*/
GT_STATUS   applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
)
{

    /* function is built only to support 'linker' (the ability to build image) */

    /* the application must implement this function by itself , if application
       want to work with the mainCmd and the 'Galtis wrappers'.
       a reference (for function applicationExtServicesBind) can be found in file:
       <appDemo/boardConfig/appDemoBoardConfig.c> */


    return GT_NOT_IMPLEMENTED;
}
#endif /*GALTIS_NO_APP_DEMO*/


