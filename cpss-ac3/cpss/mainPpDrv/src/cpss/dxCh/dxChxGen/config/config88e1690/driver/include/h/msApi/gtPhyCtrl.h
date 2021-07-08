#include <Copyright.h>
/**
********************************************************************************
* @file gtPhyCtrl.h
*
* @brief API/Structure definitions for Marvell PhyCtrl functionality.
*
* @version   /
********************************************************************************
*/
/*******************************************************************************
* gtPhyCtrl.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell PhyCtrl functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtPhyCtrl_h
#define __prvCpssDrvGtPhyCtrl_h

#include <msApiTypes.h>
#include <gtSysConfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Exported Phy Control Types                                                 */
/******************************************************************************/

/**
* @enum GT_CPSS_PHY_AUTO_MODE
 *
 * @brief Enumeration of Autonegotiation mode.
 * Auto for both speed and duplex.
 * Auto for speed only and Full duplex.
 * Auto for speed only and Half duplex. (1000Mbps is not supported)
 * Auto for duplex only and speed 1000Mbps.
 * Auto for duplex only and speed 100Mbps.
 * Auto for duplex only and speed 10Mbps.
 * 1000Mbps Full duplex.
 * 100Mbps Full duplex.
 * 100Mbps Half duplex.
 * 10Mbps Full duplex.
 * 10Mbps Half duplex.
*/
typedef enum{

    CPSS_SPEED_AUTO_DUPLEX_AUTO,

    CPSS_SPEED_1000_DUPLEX_AUTO,

    CPSS_SPEED_100_DUPLEX_AUTO,

    CPSS_SPEED_10_DUPLEX_AUTO,

    CPSS_SPEED_AUTO_DUPLEX_FULL,

    CPSS_SPEED_AUTO_DUPLEX_HALF,

    CPSS_SPEED_1000_DUPLEX_FULL,

    CPSS_SPEED_1000_DUPLEX_HALF,

    CPSS_SPEED_100_DUPLEX_FULL,

    CPSS_SPEED_100_DUPLEX_HALF,

    CPSS_SPEED_10_DUPLEX_FULL,

    CPSS_SPEED_10_DUPLEX_HALF

} GT_CPSS_PHY_AUTO_MODE;

/**
* @enum GT_CPSS_PHY_SPEED
 *
 * @brief Enumeration of Phy Speed
*/
typedef enum{

    /** 10Mbps */
    CPSS_PHY_SPEED_10_MBPS,

    /** 100Mbps */
    CPSS_PHY_SPEED_100_MBPS,

    /** 1000Mbps */
    CPSS_PHY_SPEED_1000_MBPS

} GT_CPSS_PHY_SPEED;

/******************************************************************************/
/* Exported Phy Control Functions                                             */
/******************************************************************************/

/**
* @internal prvCpssDrvGphyReset function
* @endinternal
*
* @brief   This routine preforms PHY reset.
*         After reset, phy will be in Autonegotiation mode.
* @param[in] port                     - The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.15 - Reset
*       data sheet register 0.13 - Speed(LSB)
*       data sheet register 0.12 - Autonegotiation
*       data sheet register 0.8 - Duplex Mode
*       data sheet register 0.6 - Speed(MSB)
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to restart AN.
*       When port recover from link down, AN will restart automatically.
*
*/
GT_STATUS prvCpssDrvGphyReset
(
    IN GT_CPSS_QD_DEV   *dev,
    IN GT_CPSS_LPORT    port
);

/**
* @internal prvCpssDrvGphySetPortLoopback function
* @endinternal
*
* @brief   Enable/Disable Internal Port Loopback.
*         For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*         If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and
*         forces speed to be 10Mbps.
*         If Auto-Negotiation is disabled, the forced speed is used.
*         Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*         it is recommended to call gprtSetPortAutoMode for PHY configuration after
*         Loopback test.
*         For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*         If Auto-Negotiation is enabled and Link is active, the current speed is used.
*         If Auto-Negotiation is disabled, the forced speed is used.
*         All other cases, default MAC Interface speed is used. Please refer to the data
*         sheet for the information of the default MAC Interface speed.
* @param[in] port                     - The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] enable                   - If GT_TRUE,  loopback mode
*                                      If GT_FALSE, disable loopback mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.14 - Loop_back
*
*/
GT_STATUS prvCpssDrvGphySetPortLoopback
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         enable
);

/**
* @internal prvCpssDrvGphySetPortSpeed function
* @endinternal
*
* @brief   Sets speed for a specific logical port. This function will keep the duplex
*         mode and loopback mode to the previous value, but disable others, such as
*         Autonegotiation.
* @param[in] port                     -  The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] speed                    - port speed.
*                                      CPSS_PHY_SPEED_10_MBPS for 10Mbps
*                                      CPSS_PHY_SPEED_100_MBPS for 100Mbps
*                                      CPSS_PHY_SPEED_1000_MBPS for 1000Mbps
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.13 - Speed Selection (LSB)
*       data sheet register 0.6 - Speed Selection (MSB)
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to force take effect .
*       When port recover from link down, configure will take effect automatically.
*
*/
GT_STATUS prvCpssDrvGphySetPortSpeed
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_PHY_SPEED   speed
);

/**
* @internal prvCpssDrvGphyPortPowerDown function
* @endinternal
*
* @brief   Enable/disable (power down) on specific logical port.
*         Phy configuration remains unchanged after Power down.
* @param[in] port                     -  The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] state                    - GT_TRUE: power down
*                                      GT_FALSE: normal operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.11 - Power Down
*
*/
GT_STATUS prvCpssDrvGphyPortPowerDown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state
);

/**
* @internal prvCpssDrvGphySetPortDuplexMode function
* @endinternal
*
* @brief   Sets duplex mode for a specific logical port. This function will keep
*         the speed and loopback mode to the previous value, but disable others,
*         such as Autonegotiation.
* @param[in] port                     - The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] dMode                    - dulpex mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.8 - Duplex Mode
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to force take effect .
*       When port recover from link down, configure will take effect
*       automatically.
*
*/
GT_STATUS prvCpssDrvGphySetPortDuplexMode
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         dMode
);

/**
* @internal prvCpssDrvGphySetPortAutoMode function
* @endinternal
*
* @brief   This routine sets up the port with given Auto Mode.
*         Supported mode is as follows:
*         - Auto for both speed and duplex.
*         - Auto for speed only and Full duplex.
*         - Auto for speed only and Half duplex.
*         - Auto for duplex only and speed 1000Mbps.
*         - Auto for duplex only and speed 100Mbps.
*         - Auto for duplex only and speed 10Mbps.
*         - Speed 1000Mbps and Full duplex.
*         - Speed 1000Mbps and Half duplex.
*         - Speed 100Mbps and Full duplex.
*         - Speed 100Mbps and Half duplex.
*         - Speed 10Mbps and Full duplex.
*         - Speed 10Mbps and Half duplex.
* @param[in] port                     - The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] mode                     - Auto Mode to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note data sheet register 4.6, 4.5 Autonegotiation Advertisement for 10BT
*       data sheet register 4.8, 4.7 Autonegotiation Advertisement for 100BT
*       data sheet register 9.9, 9.8 Autonegotiation Advertisement for 1000BT
*       If DUT is in power down or loopback mode, port will link down,
*       in this case, no need to do software reset to restart AN.
*       When port recover from link down, AN will restart automatically.
*
*/
GT_STATUS prvCpssDrvGphySetPortAutoMode
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_PHY_AUTO_MODE   mode
);

/**
* @internal prvCpssDrvGphyGetPortLoopback function
* @endinternal
*
* @brief   Get Internal Port Loopback state.
*
* @param[in] port                     - The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
*
* @param[out] enablePtr                - (pointer to) loopback mode state.
*                                      GT_TRUE for loopback enable, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note data sheet register 0.14 - Loop_back
*
*/
GT_STATUS prvCpssDrvGphyGetPortLoopback
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal prvCpssDrvGphySetPause function
* @endinternal
*
* @brief   This routine will set the pause bit in Autonegotiation Advertisement
*         Register. And restart the autonegotiation.
* @param[in] port                     - The logical  number
* @param[in] state                    - GT_TRUE  - support pause
*                                      GT_FALSE - no pause
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGphySetPause
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state
);


/**
* @internal prvCpssDrvGphyGetPause function
* @endinternal
*
* @brief   This routine will get the pause bit in Autonegotiation Advertisement
*         Register.
* @param[in] port                     - The logical  number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGphyGetPause
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *statePtr
);

#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtPhyCtrl_h */

