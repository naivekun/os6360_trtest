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
* @file prvTgfIpMtuExceedExceptionCheck.h
*
* @brief Basic MTU exceed exception command
*
* @version   1
********************************************************************************
*/
#include <common/tgfIpGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfIpv4UcCaptureToCpuConfigurationSet function
* @endinternal
*
* @brief   start capture to the CPU configuration set
*/
GT_VOID prvTgfIpv4UcCaptureToCpuConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4UcExceptionConfigurationSet function
* @endinternal
*
* @brief   save default values of exception type and exception command.
*         set new values: exception type= MTU_EXCEEDED_FOR_NON_DF_E
*         exception command= CMD_DROP_HARD
* @param[in] exceptionType            - exception type
* @param[in] exceptionCmd             - exception command
*                                       None
*/
GT_VOID prvTgfIpv4UcExceptionConfigurationSet
(
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT    exceptionType,
    IN PRV_TGF_IP_EXC_CMD_ENT           exceptionCmd
);

/**
* @internal prvTgfIpv4UcMtuConfigurationSet function
* @endinternal
*
* @brief   save default mtu
*         & set a new mtu value
* @param[in] mtu                      - the maximum transmission unit
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuConfigurationSet
(
    IN GT_U32                           mtu
);
/**
* @internal prvTgfIpv4UcMtuConfigurationRestore function
* @endinternal
*
* @brief   restore mtu,exception type and exception command values
*
* @param[in] exceptionType            - exception type
* @param[in] protocolStack            - protocol stack
* @param[in] packetType               - packet type
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuConfigurationRestore
(
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType
);

/**
* @internal prvTgfIpv4UcMtuRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] exceptionCmd             - exception command
*                                       None
*/
GT_VOID prvTgfIpv4UcMtuRoutingTrafficGenerate
(
    IN GT_U32                            sendPortNum,
    IN GT_U32                            nextHopPortNum,
    IN PRV_TGF_IP_EXC_CMD_ENT           exceptionCmd
);


#ifdef __cplusplus
}
#endif /* __cplusplus */




