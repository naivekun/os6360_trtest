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
* @file osWin32Ether.c
*
* @brief Operating System wrapper for Win32
*
* @version   2
********************************************************************************
*/
#include <gtOs/gtGenTypes.h>
#ifndef APPLICATION_SIDE_ONLY
    #include <asicsimulation/SKernel/skernel.h>
#else
    #define SIM_NIC_NOT_IMPLEMENTED
#endif /*!APPLICATION_SIDE_ONLY*/

#ifndef SIM_NIC_NOT_IMPLEMENTED
static SKERNEL_NIC_RX_CB_FUN  userRxCallback = NULL;
#endif/* !SIM_NIC_NOT_IMPLEMENTED*/
/**
* @internal osEtherInit function
* @endinternal
*
* @brief   This function inits the internal data structure of the Ethernet port
*/
GT_STATUS osEtherInit
(
    void
)
{
    return GT_OK;
}
/**
* @internal osInputCallback function
* @endinternal
*
* @brief   This function is the internal Rx callback
*
* @param[in] segmentList[]            - array of transmit data segments.
* @param[in] segmentLen[]             - array of transmit data segments length.
* @param[in] numOfSegments            - number of transmit data segments.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS osInputCallback
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments
)
{

    /* Call the user callback routine */
#ifndef SIM_NIC_NOT_IMPLEMENTED
    if (NULL == userRxCallback)
    {
        return GT_BAD_PTR;
    }

    userRxCallback(segmentList, segmentLen, numOfSegments);
#endif/* !SIM_NIC_NOT_IMPLEMENTED*/

    return GT_OK;
}

/**
* @internal osEtherInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callback function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS osEtherInputHookAdd
(
#ifndef SIM_NIC_NOT_IMPLEMENTED
    IN SKERNEL_NIC_RX_CB_FUN userRxFunc
#else
    IN void* userRxFunc
#endif
)
{

#ifndef SIM_NIC_NOT_IMPLEMENTED
    skernelNicRxBind(osInputCallback);

    userRxCallback = userRxFunc;
#endif/* !SIM_NIC_NOT_IMPLEMENTED*/

    return GT_OK;
}

/**
* @internal osEtherOutput function
* @endinternal
*
* @brief   This function transmits an Ethernet packet to the Packet processor
*
* @param[in] segmentList[]            - array of transmit data segments.
* @param[in] segmentLen[]             - array of transmit data segments length.
* @param[in] numOfSegments            - number of transmit data segments.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS osEtherOutput
(
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments
)
{

    GT_STATUS status=GT_FAIL;

#ifndef SIM_NIC_NOT_IMPLEMENTED
    status = skernelNicOutput(segmentList, segmentLen, numOfSegments);
#endif/* !SIM_NIC_NOT_IMPLEMENTED*/


    return (GT_OK == status) ? GT_OK : GT_FAIL;
}

/**
* @internal osEtherInputHookDelete function
* @endinternal
*
* @brief   This unbinds the user Rx callback
*/
GT_STATUS osEtherInputHookDelete
(
    void
)
{

#ifndef SIM_NIC_NOT_IMPLEMENTED
    skernelNicRxUnBind(osInputCallback);
#endif/* !SIM_NIC_NOT_IMPLEMENTED*/

    return GT_OK;
}



