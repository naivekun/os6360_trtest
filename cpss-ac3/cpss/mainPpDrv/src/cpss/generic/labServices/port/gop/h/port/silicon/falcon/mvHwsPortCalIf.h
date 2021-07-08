/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCalIf.h
*
* @brief This file contains API for port d2d calendars and credits configuartion
*
* @version   0
********************************************************************************
*/

#ifndef __mvHwsPortCalIf_H
#define __mvHwsPortCalIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

/**
* @enum HWS_D2D_BANDWIDTH_ENT
 *
 * @brief Defines the different d2d bandwidths.
*/
typedef enum {

    /** bandwidth 400G . */
    HWS_D2D_BANDWIDTH_400G_E = 400,

    /** bandwidth 200G . */
    HWS_D2D_BANDWIDTH_200G_E = 200,

    /** bandwidth 100G . */
    HWS_D2D_BANDWIDTH_100G_E = 100,

    /** bandwidth 50G . */
    HWS_D2D_BANDWIDTH_50G_E  = 50,

    /** bandwidth 25G . */
    HWS_D2D_BANDWIDTH_25G_E  = 25

}HWS_D2D_BANDWIDTH_ENT;


/**
* @internal mvHwsD2dConfigChannel function
* @endinternal
*
* @brief  Enable Port , D2D TX and D2D RX.
*         The configuration performs enable of Tx and Rx on for
*         specific channel for PCS calendar , MAC TDM RX
*         calendar, and credits. this function should be called
*         twice, from eagle side and from raven side.
*  
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
* @param[in] serdesSpeed              - serdes speed 
* @param[in] numOfActLanes            - active lanes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dConfigChannel
(
    GT_U8               devNum,
    GT_U32              d2dNum,
    GT_U32              channel,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8               numOfActLanes
);

/**
* @internal mvHwsD2dDisableChannel function
* @endinternal
*
* @brief  Disable Port , D2D TX and D2D RX.The configuration
*         performs Disable of Tx and Rx on for specific channel
*         for PCS calendar , MAC TDM RX calendar, credits reset.
*         this function should be called twice, from eagle side
*         and from raven side.
* 
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index
*  
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dDisableChannel
(
    GT_U8           devNum,
    GT_U32          d2dNum,
    GT_U32          channel
);

/**
* @internal mvHwsD2dDisableAll50GChannel function
* @endinternal
*
* @brief  Disable all 50G bandwidth channels 0 ..7 for specific
*         d2d. this function should be called twise, from eagle
*         side and from raven side. 
* 
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dDisableAll50GChannel
(
    GT_U8   devNum,
    GT_U32  d2dNum
);

/**
* @internal mvHwsD2dPcsCalMacCalAndMacCreditsDump function
* @endinternal
*
* @brief  Print rx and tx pcs and mac calendars slices and mac 
*         credits. for both eagle and raven. the function print
*         all d2d registers values or spesific d2d registers
*         values.
*  
* @param[in] devNum                   - system device number
* @param[in] d2dIndex                 - d2d index
*  
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dPcsCalMacCalAndMacCreditsDump
(
    GT_U8                       devNum,
    GT_U32                      d2dNum
);

/**
* @internal mvHwsD2dPcsCalDump function
* @endinternal
*
* @brief  print rx and tx pcs calendar slices. the function is
*         for eagle or raven side
*  
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address 
* @param[in] portsBandwidthArr        -array to count the ports 
*       slices
* 
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dPcsCalDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_U32              *portsBandwidthArr
);

/**
* @internal mvHwsD2dMacCalDump function
* @endinternal
*
* @brief  print rx tdm mac calendar slices. the function is for
*         eagle or raven side
*  
* @param[in] devNum                   - system device number
* @param[in] baseAddr                 - d2d base address
* 
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCalDump
(
    GT_U8               devNum,
    GT_U32              baseAddr
);

/**
* @internal mvHwsD2dMacCreditsDump function
* @endinternal
*
* @brief  print tx and rx mac credits . The configuration 
*         performs print of tx channels credits, tx segments, rx
*         channels credits and rx segment. the function is for
*         eagle or raven side
*  
* @param[in] devNum                   - system device number 
* @param[in] baseAddr                 - d2d base address
* @param[in] isEagle                  - 1: is eagle
*                                       0: is raven
* 
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCreditsDump
(
    GT_U8               devNum,
    GT_U32              baseAddr,
    GT_BOOL             isEagle
);


/**
* @internal mvHwsD2dGetChannelsConfigurations function
* @endinternal
*
* @brief  count the slices each channel holds in pcs or mac 
*         calendars.
* 
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* 
* @param[out] channelSlicesArr           - pointer to matrix of 
*       counters. the first row contains mac calendar channels
*       slices counters . the second row contains pcs calendars
*       channels slices counters .in each row the channel is the
*       cell index. the cell holds the number of the channel
*       slices
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dGetChannelsConfigurations
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channelSlicesArr[HWS_D2D_PORTS_NUM_CNS+1][2]
);


/**
* @internal mvHwsD2dMacCreditsGet function
* @endinternal
*
* @brief  Get Tx and Rx mac credits for specific channel index 
*         and D2D number
* 
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
* @param[in] channel                  - d2d local channel index 
*  
* @param[out] txChannelCreditsArr     - array of tx credits 
*       parameters
* @param[out] rxChannelCreditsArr     - array of rx credits 
*       parameters
* 
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsD2dMacCreditsGet
(
    GT_U8                  devNum,
    GT_U32                 d2dNum,
    GT_U32                 channel,
    GT_U32                 *txChannelCreditsArr,
    GT_U32                 *rxChannelCreditsArr
);


#ifdef __cplusplus
}
#endif

#endif /* __mvHwsPortCalIf_H */

