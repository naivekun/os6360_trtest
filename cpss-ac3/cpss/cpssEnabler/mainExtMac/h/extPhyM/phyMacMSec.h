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
* @file phyMacMSec.h
*
* @brief
*
* @version   4
********************************************************************************
*/
#ifndef __phyMacMSech
#define __phyMacMSech

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PHYMAC_SIDE_SET_ACTION_ENT
 *
 * @brief Generic Serial Management Interface numbering
*/
typedef enum{

    PHYMAC_SWITCH_SIDE_SET_E,

    PHYMAC_PHY_SIDE_SET_E,

    PHYMAC_BOTH_SIDE_SET_E

} PHYMAC_SIDE_SET_ACTION_ENT;


/**************************************************************
*  EEE definitions
***************************************************************/

/**
* @enum MACPHY_EEE_LPI_MODE_ENT
 *
 * @brief EEE mode numbering
*/
typedef enum{

    /** EEE disable mode */
    MACPHY_EEE_LPI_DISABLE_MODE_E        = 0,

    /** EEE enable master mode */
    MACPHY_EEE_LPI_MASTER_MODE_E,

    /** EEE enable slave mode */
    MACPHY_EEE_LPI_SLAVE_MODE_E,

    /** EEE enable force mode */
    MACPHY_EEE_LPI_FORCE_MODE_E

} MACPHY_EEE_LPI_MODE_ENT;

/**
* @enum MACPHY_EEE_LPI_TIMER_ENT
 *
 * @brief EEE mode numbering
*/
typedef enum{

    /** LPI exit timer when port speed is 1000 Mbps */
    MACPHY_EEE_LPI_FAST_EXIT_TIME_E = 0,

    /** @brief LPI exit timer when port speed is 10/100 Mbps
     *  MACPHY_EEE_LPI_FAST_ENTER_TIME_E LPI enter timer when port speed is 1000 Mbps
     *  MACPHY_EEE_LPI_SLOW_ENTER_TIME_E - LPI enter timer when port speed is 10/100 Mbps
     */
    MACPHY_EEE_LPI_SLOW_EXIT_TIME_E,

    MACPHY_EEE_LPI_FAST_ENTER_TIMEOUT_E,

    MACPHY_EEE_LPI_SLOW_ENTER_TIMEOUT_E

} MACPHY_EEE_LPI_TIMER_ENT;


/**
* @internal phyMacMSecSetSpeed function
* @endinternal
*
* @brief   This function sets speed to PHY SIDE MAC and SWITCH SIDE MAC of
*         1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssSpeed                  - speed value to set in Switch Side MAC
*                                      psStage          - speed value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetSpeed
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_SPEED_ENT  ssSpeed,
    IN CPSS_PORT_SPEED_ENT  psSpeed,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/*******************************************************************************
* phyMacMSecGetSpeed (2)
*
*       This function returns speed values from PHY SIDE MAC and SWITCH SIDE MAC of 
*       1540M PHY.
*
* INPUTS:
*
*    devNum           - device number    
*    portNum          - port number
*    sideAction       - side actions (SS or PS or BOTH) 
*
* OUTPUTS:
*    *ssSpeed          - speed ptr to get the value from Switch Side MAC
*    *psStage          - speed ptr to get the value from Phy Side MAC   
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on wrong parameters and set failed
*       GT_BAD_PARAM             - wrong parameter
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS phyMacMSecGetSpeed
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_PORT_SPEED_ENT  *ssSpeed,
    IN CPSS_PORT_SPEED_ENT  *psSpeed,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideMacActions
);

/**
* @internal phyMacMSecSetANDuplex function
* @endinternal
*
* @brief   This function sets duplex Auto Negotiation state to MacSec
*         Switch Side & Phy Side MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssState                  - duplex state to set the value from Switch Side MAC
* @param[in] psState                  - duplex state to set the value from Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetANDuplex
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssState,
    IN GT_BOOL psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetANDuplex function
* @endinternal
*
* @brief   This function returns AN duplex values from MacSec SS & PS MACs.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - SS MAC duplex state ptr
* @param[out] psState                  - PS MAC duplex state ptr
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecGetANDuplex
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssState,
    OUT GT_BOOL *psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);


/**
* @internal phyMacMSecSetANFlowControl function
* @endinternal
*
* @brief   This function sets flow control parameters to Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssState                  - flow control state to set the value to Switch Side MAC
* @param[in] ssPauseAdvertise         - pause advertise (symmetric/assimetric) to set
*                                      the value to Switch Side MAC
* @param[in] psState                  - flow control state to set the value to Phy Side MAC
* @param[in] psPauseAdvertise         - pause advertise (symmetric/assimetric) to set
*                                      the value to Switch Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval MAD_OK                   - on success
* @retval MAD_FAIL                 - on error
*/
GT_STATUS phyMacMSecSetANFlowControl
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssState,
    IN GT_BOOL ssPauseAdvertise,
    IN GT_BOOL psState,
    IN GT_BOOL psPauseAdvertise,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetANFlowControl function
* @endinternal
*
* @brief   This function returns flow control parameters from Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - flow control state to set the value to Switch Side MAC
* @param[out] ssPauseAdvertise         - pause advertise (symmetric/assimetric) to set
*                                      the value to Switch Side MAC
* @param[out] psState                  - flow control state to set the value to Phy Side MAC
* @param[out] psPauseAdvertise         - pause advertise (symmetric/assimetric) to set
*
* @retval MAD_OK                   - on success
* @retval MAD_FAIL                 - on error
*/
GT_STATUS phyMacMSecGetANFlowControl
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssState,
    OUT GT_BOOL *ssPauseAdvertise,
    OUT GT_BOOL *psState,
    OUT GT_BOOL *psPauseAdvertise,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecSetSpeedANEnable function
* @endinternal
*
* @brief   This function enables/disables speed AN on Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssState                  - flow control state to set the value to Switch Side MAC
* @param[in] psState                  - flow control state to set the value to Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecSetSpeedANEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssState,
    IN GT_BOOL psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecGetSpeedANEnable function
* @endinternal
*
* @brief   This function returns speed AN values from Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - speed enable value from Switch Side MAC
* @param[out] psState                  - speed enable value from Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecGetSpeedANEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssState,
    OUT GT_BOOL *psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecSetFlowControlEnable function
* @endinternal
*
* @brief   This function sets flow control mode for Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssState                  - flow control mode for Switch Side MAC
* @param[in] psState                  - flow control mode for Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecSetFlowControlEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_PORT_FLOW_CONTROL_ENT ssState,
    IN CPSS_PORT_FLOW_CONTROL_ENT psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetFlowControlEnable function
* @endinternal
*
* @brief   This function returns flow control mode for Switch Side &
*         Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - flow control mode for Switch Side MAC
* @param[out] psState                  - flow control mode for Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecGetFlowControlEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT *ssState,
    OUT CPSS_PORT_FLOW_CONTROL_ENT *psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecSetPeriodicFlowControlEnable function
* @endinternal
*
* @brief   This function sets Periodic Xon Flow Control enable/disable for PHY SIDE MAC
*         and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssPrdFlowControl         - periodic flow control value to set in Switch Side MAC
* @param[in] psPrdFlowControl         - periodic flow control value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetPeriodicFlowControlEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssPrdFlowControl,
    IN GT_BOOL psPrdFlowControl,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecGetPeriodicFlowControlEnable function
* @endinternal
*
* @brief   This function gets status of periodic 802.3x flow control
*         for Switch Side & Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - periodic flow control state for Switch Side MAC
* @param[out] psState                  - periodic flow control state for Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecGetPeriodicFlowControlEnable
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssState,
    OUT GT_BOOL *psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecSetBackPressureEnable function
* @endinternal
*
* @brief   This function Enable/Disable back pressure on port in half duplex
*         condition for Switch Side & Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssState                  - back pressure mode for Switch Side MAC
* @param[in] psState                  - back pressure mode for Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecSetBackPressureEnable
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL   ssState,
    IN GT_BOOL   psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecGetBackPressureEnable function
* @endinternal
*
* @brief   This function gets status of backpressure on port
*         for Switch Side & Phy Side MACSEC MACs
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssState                  - back pressure state for Switch Side MAC
* @param[out] psState                  - back pressure state for Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacMSecGetBackPressureEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *ssState,
    OUT GT_BOOL *psState,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/*******************************************************************************
* phyMacMSecGetLinkStatus (15)
*
*       This function returns link status values from PHY SIDE MAC and SWITCH SIDE MAC of 
*       1540M PHY.
*
* INPUTS:
*
*    devNum           - device number    
*    portNum          - port number
*    sideAction       - side actions (SS or PS or BOTH) 
*
* OUTPUTS:
*    *ssSpeed          - speed ptr to get the value from Switch Side MAC
*    *psStage          - speed ptr to get the value from Phy Side MAC   
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on wrong parameters and set failed
*       GT_BAD_PARAM             - wrong parameter
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS phyMacMSecGetLinkStatus
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssLinkStatus,
    OUT GT_BOOL *psLinkStatus,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction

);

/**
* @internal phyMacMSecSetDuplexMode function
* @endinternal
*
* @brief   This function sets duplex mode for PHY SIDE MAC
*         and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssDuplexMode             - duplex mode value to set in Switch Side MAC
* @param[in] psDuplexMode             - duplex mode value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetDuplexMode
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssDuplexMode,
    IN GT_BOOL psDuplexMode,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);


/**
* @internal phyMacMSecGetDuplexMode function
* @endinternal
*
* @brief   This function returns duplex mode enable/disable
*         status for PHY SIDE MAC and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecGetDuplexMode 
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT *ssDuplexModePtr,
    OUT CPSS_PORT_DUPLEX_ENT *psDuplexModePtr,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecSetPortEnable function
* @endinternal
*
* @brief   This function sets port enable/disable mode for PHY SIDE MAC
*         and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssPortEnable             - port enable/disable value to set in Switch Side MAC
* @param[in] psPortEnable             - port enable/disable value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetPortEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssPortEnable,
    IN GT_BOOL psPortEnable,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetPortEnable function
* @endinternal
*
* @brief   This function returns port enable/disable
*         status for PHY SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssPortEnablePtr          - ptr to port enable value from Switch Side MAC
* @param[out] psPortEnablePtr          - ptr to port enable value from Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecGetPortEnable 
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssPortEnablePtr,
    OUT GT_BOOL *psPortEnablePtr,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecSetPaddingEnable function
* @endinternal
*
* @brief   This function sets port padding enable mode
*         for PHY SIDE MAC and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssPaddingEnable          - Padding enable/disable value to set in Switch Side MAC
* @param[in] psPaddingEnable          - Padding enable/disable value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetPaddingEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssPaddingEnable,
    IN GT_BOOL psPaddingEnable,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetPaddingEnable function
* @endinternal
*
* @brief   This function returns port padding enable mode
*         for PHY SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssPaddingEnable          - Padding enable/disable value to get from Switch Side MAC
* @param[out] psPaddingEnable          - Padding enable/disable value to get from Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecGetPaddingEnable
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssPaddingEnable,
    OUT GT_BOOL *psPaddingEnable,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);


/**
* @internal phyMacMSecSetCRCCheckEnable function
* @endinternal
*
* @brief   This function enable/disable CRC check mode
*         for PHY SIDE MAC and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssCRCCheckEnable         - CRC check enable/disable value to set in Switch Side MAC
* @param[in] psCRCCheckEnable         - CRC check enable/disable value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetCRCCheckEnable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL ssCRCCheckEnable,
    IN GT_BOOL psCRCCheckEnable,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);
/**
* @internal phyMacMSecGetCRCCheckEnable function
* @endinternal
*
* @brief   This function returns port padding enable mode
*         for PHY SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssCRCCheckEnable         - CRC Check enable/disable value to get from Switch Side MAC
* @param[out] psCRCCheckEnable         - CRC Check enable/disable value to get from Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecGetCRCCheckEnable
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL *ssCRCCheckEnable,
    OUT GT_BOOL *psCRCCheckEnable,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecSetMRU function
* @endinternal
*
* @brief   This function sets MRU size
*         for PHY SIDE MAC and SWITCH SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ssMruSize                - MruSize value to set in Switch Side MAC
* @param[in] psMruSize                - MruSize value to set in Phy Side MAC
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecSetMRU
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_U32  ssMruSize,
    IN GT_U32  psMruSize,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/**
* @internal phyMacMSecGetMRU function
* @endinternal
*
* @brief   This function returns port padding enable mode
*         for PHY SIDE MAC of 1540M PHY.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] sideAction               - side actions (SS or PS or BOTH)
*
* @param[out] ssMruSize                - Mru Size value to get from Switch Side MAC
* @param[out] psMruSize                - Mru Size value to get from Phy Side MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacMSecGetMRU
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32 *ssMruSize,
    OUT GT_U32 *psMruSize,
    IN PHYMAC_SIDE_SET_ACTION_ENT sideAction
);

/******************************
*  EEE functions 
*******************************/


/**
* @internal phyMacEEEModeSet function
* @endinternal
*
* @brief   This function sets EEE mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - defines one of:
*                                      master mode
*                                      slave mode
*                                      force mode
*                                      EEE disable mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacEEEModeSet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN MACPHY_EEE_LPI_MODE_ENT mode 
);

/**
* @internal phyMacEEEModeGet function
* @endinternal
*
* @brief   This function returns EEE mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] modePtr                  - defines one of:
*                                      master mode
*                                      slave mode
*                                      force mode
*                                      EEE disable mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on wrong parameters and set failed
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS phyMacEEEModeGet
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT MACPHY_EEE_LPI_MODE_ENT *modePtr    
);

/**
* @internal phyMacEEEInit function
* @endinternal
*
* @brief   This function:
*         - enable EEE auto-neg advertisment
*         - soft reset
*         - run EEE workaround for 1540
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS phyMacEEEInit
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __phyMacMSech */





