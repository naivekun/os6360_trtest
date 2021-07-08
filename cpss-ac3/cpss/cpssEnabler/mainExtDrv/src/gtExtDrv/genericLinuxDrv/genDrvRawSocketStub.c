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
* @file genDrvRawSocketStub.c
*
* @brief This file includes stubs for raw socket support on
*
* @version   1
********************************************************************************
*/
#include <cpss/common/cpssTypes.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtOs/gtOs.h>


/**
* @internal prvExtDrvSendRawSocket function
* @endinternal
*
* @brief   This function transmits a packet through the linux raw socket
*
* @param[in] segmentList[]            - A list of pointers to the packets segments.
* @param[in] segmentLen[]             - A list of segement length.
* @param[in] numOfSegments            - The number of segment in segment list.
* @param[in] txQueue                  - The TX queue.
*
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvExtDrvSendRawSocket
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           txQueue
)
{
    return GT_NOT_SUPPORTED;
}


/**
* @internal extDrvEthMuxSet function
* @endinternal
*
* @brief   Sets the mux mode to one of cpss, raw, linux
*/
GT_STATUS extDrvEthMuxSet
(
 IN unsigned long portNum,
 IN extDrvEthNetPortType_ENT portType
)
{
    return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvEthMuxGet function
* @endinternal
*
* @brief   Get the mux mosde of the port
*/
GT_STATUS extDrvEthMuxGet
(
 IN unsigned long portNum,
 OUT extDrvEthNetPortType_ENT *portTypeP
)
{
    return GT_NOT_SUPPORTED;
}

/**
* @internal prvExtDrvRawSocketEnable function
* @endinternal
*
* @brief   Enable raw socket interface
*/
GT_STATUS prvExtDrvRawSocketEnable(GT_VOID)
{
    return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvLinuxModeSet function
* @endinternal
*
* @brief   Set port <portNum> to Linux Mode (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
* @param[in] ip1                      ip2, ip3, ip4 - The ip address to assign to the port, 4 numbers
*
* @retval GT_OK                    - always,
*/
GT_STATUS extDrvLinuxModeSet
(
 IN GT_U32 portNum,
 IN GT_U32 ip1,
 IN GT_U32 ip2,
 IN GT_U32 ip3,
 IN GT_U32 ip4
)
{
    return GT_NOT_SUPPORTED;
}


/**
* @internal prvExtDrvRawSocketInit function
* @endinternal
*
* @brief   Initialize raw socket operations
*
* @retval GT_OK                    - if success
*/
GT_STATUS prvExtDrvRawSocketInit(GT_VOID)
{
    return GT_NOT_SUPPORTED;
}


