#include <Copyright.h>

/**
********************************************************************************
* @file gtPhyCtrl.c
*
* @brief API implementation for switch internal Copper PHY control.
*
* @version   /
********************************************************************************
*/
/********************************************************************************
* gtPhyCtrl.c
*
* DESCRIPTION:
*       API implementation for switch internal Copper PHY control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtPhyCtrl.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>

/*
* This routine set Auto-Negotiation Ad Register for Copper
*/
static
GT_STATUS gigCopperSetAutoMode
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8                   hwPort,
    IN  GT_CPSS_PHY_AUTO_MODE   mode
)
{
    GT_U16             u16Data, u16Data1;

    PRV_CPSS_DBG_INFO(("gigCopperSetAutoMode Called.\n"));

    if (prvCpssDrvHwReadPagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG, &u16Data) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n", hwPort, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Mask out all auto mode related bits. */
    u16Data &= ~PRV_CPSS_QD_PHY_MODE_AUTO_AUTO;

    if (prvCpssDrvHwReadPagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_1000AD_REG, &u16Data1) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n", hwPort, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Mask out all auto mode related bits. */
    u16Data1 &= ~(PRV_CPSS_QD_GIGPHY_1000T_FULL | PRV_CPSS_QD_GIGPHY_1000T_HALF);

    switch (mode)
    {
        case CPSS_SPEED_AUTO_DUPLEX_AUTO:
            u16Data |= PRV_CPSS_QD_PHY_MODE_AUTO_AUTO;
        case CPSS_SPEED_1000_DUPLEX_AUTO:
            u16Data1 |= PRV_CPSS_QD_GIGPHY_1000T_FULL | PRV_CPSS_QD_GIGPHY_1000T_HALF;
            break;
        case CPSS_SPEED_AUTO_DUPLEX_FULL:
            u16Data |= PRV_CPSS_QD_PHY_MODE_AUTO_FULL;
            u16Data1 |= PRV_CPSS_QD_GIGPHY_1000T_FULL;
            break;
        case CPSS_SPEED_1000_DUPLEX_FULL:
            u16Data1 |= PRV_CPSS_QD_GIGPHY_1000T_FULL;
            break;
        case CPSS_SPEED_1000_DUPLEX_HALF:
            u16Data1 |= PRV_CPSS_QD_GIGPHY_1000T_HALF;
            break;
        case CPSS_SPEED_AUTO_DUPLEX_HALF:
            u16Data |= PRV_CPSS_QD_PHY_MODE_AUTO_HALF;
            u16Data1 |= PRV_CPSS_QD_GIGPHY_1000T_HALF;
            break;
        case CPSS_SPEED_100_DUPLEX_AUTO:
            u16Data |= PRV_CPSS_QD_PHY_MODE_100_AUTO;
            break;
        case CPSS_SPEED_10_DUPLEX_AUTO:
            u16Data |= PRV_CPSS_QD_PHY_MODE_10_AUTO;
            break;
        case CPSS_SPEED_100_DUPLEX_FULL:
            u16Data |= PRV_CPSS_QD_PHY_100_FULL;
            break;
        case CPSS_SPEED_100_DUPLEX_HALF:
            u16Data |= PRV_CPSS_QD_PHY_100_HALF;
            break;
        case CPSS_SPEED_10_DUPLEX_FULL:
            u16Data |= PRV_CPSS_QD_PHY_10_FULL;
            break;
        case CPSS_SPEED_10_DUPLEX_HALF:
            u16Data |= PRV_CPSS_QD_PHY_10_HALF;
            break;
        default:
            PRV_CPSS_DBG_INFO(("Unknown Auto Mode (%d)\n", mode));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if (prvCpssDrvHwWritePagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG, u16Data) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n", hwPort, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG, u16Data));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Write to Phy AutoNegotiation 1000B Advertisement Register.  */
    if (prvCpssDrvHwWritePagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_1000AD_REG, u16Data1) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n", hwPort, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*
* This routine sets Auto Mode and Reset the phy
*/
static
GT_STATUS phySetAutoMode
(
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_U8                  hwPort,
    IN  GT_CPSS_PHY_AUTO_MODE  mode
)
{
    GT_U16       u16Data;
    GT_STATUS    status;

    PRV_CPSS_DBG_INFO(("phySetAutoMode Called.\n"));

    if ((status = gigCopperSetAutoMode(dev, hwPort, mode)) != GT_OK)
    {
        return status;
    }

    /* Read to Phy Control Register.  */
    if (prvCpssDrvHwReadPagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_CONTROL_REG, &u16Data) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    u16Data |= PRV_CPSS_QD_PHY_AUTONEGO;

    PRV_CPSS_DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        hwPort, PRV_CPSS_QD_PHY_CONTROL_REG, u16Data));

    /* Write to Phy Control Register.  */
    if (prvCpssDrvHwWritePagedPhyReg(dev, hwPort, 0, PRV_CPSS_QD_PHY_CONTROL_REG, u16Data) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);


    return prvCpssDrvHwPhyReset(dev, hwPort, 0, 0xFF);
}


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
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("gphySetPhyReset Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    /* set Auto Negotiation AD Register */
    retVal = phySetAutoMode(dev, hwPort, CPSS_SPEED_AUTO_DUPLEX_AUTO);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);

    return retVal;
}

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
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_BOOL          enable
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          u16Data;
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPortLoopback Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    PRV_CPSS_BOOL_2_BIT(enable,u16Data);

    /* Write to Phy Control Register.  */
    retVal = prvCpssDrvHwSetPagedPhyRegField(dev,hwPort,0,PRV_CPSS_QD_PHY_CONTROL_REG,14,1,u16Data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return retVal;
}

/**
* @internal prvCpssDrvGphySetPortSpeed function
* @endinternal
*
* @brief   Sets speed for a specific logical port. This function will keep the duplex
*         mode and loopback mode to the previous value, but disable others, such as
*         Autonegotiation.
* @param[in] port                     - The logical  number, unless SERDES device is accessed
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
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          u16Data;
    GT_STATUS       retVal;
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPortSpeed Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if(prvCpssDrvHwReadPagedPhyReg(dev,hwPort,0,PRV_CPSS_QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,PRV_CPSS_QD_PHY_CONTROL_REG));
        prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch(speed)
    {
        case CPSS_PHY_SPEED_10_MBPS:
            u16Data = u16Data & (PRV_CPSS_QD_PHY_LOOPBACK | PRV_CPSS_QD_PHY_DUPLEX);
            break;
        case CPSS_PHY_SPEED_100_MBPS:
            u16Data = (u16Data & (PRV_CPSS_QD_PHY_LOOPBACK | PRV_CPSS_QD_PHY_DUPLEX)) | PRV_CPSS_QD_PHY_SPEED;
            break;
        case CPSS_PHY_SPEED_1000_MBPS:
            u16Data = (u16Data & (PRV_CPSS_QD_PHY_LOOPBACK | PRV_CPSS_QD_PHY_DUPLEX)) | PRV_CPSS_QD_PHY_SPEED_MSB;
            break;
        default:
            prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        hwPort, PRV_CPSS_QD_PHY_CONTROL_REG, u16Data));

    retVal = prvCpssDrvHwPhyReset(dev, hwPort, 0, u16Data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return retVal;
}

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
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          u16Data;
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("gprtPortPowerDown Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    PRV_CPSS_BOOL_2_BIT(state,u16Data);

    if((retVal=prvCpssDrvHwSetPagedPhyRegField(dev,hwPort,0,PRV_CPSS_QD_PHY_CONTROL_REG,11,1,u16Data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return GT_OK;
}

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
*       When port recover from link down, configure will take effect automatically.
*
*/
GT_STATUS prvCpssDrvGphySetPortDuplexMode
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         dMode
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          u16Data;
    GT_STATUS       retVal;
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPortDuplexMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if(prvCpssDrvHwReadPagedPhyReg(dev,hwPort,0,PRV_CPSS_QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,PRV_CPSS_QD_PHY_CONTROL_REG));
        prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(dMode)
    {
        u16Data = (u16Data & (PRV_CPSS_QD_PHY_LOOPBACK | PRV_CPSS_QD_PHY_SPEED | PRV_CPSS_QD_PHY_SPEED_MSB)) | PRV_CPSS_QD_PHY_DUPLEX;
    }
    else
    {
        u16Data = u16Data & (PRV_CPSS_QD_PHY_LOOPBACK | PRV_CPSS_QD_PHY_SPEED | PRV_CPSS_QD_PHY_SPEED_MSB);
    }


    PRV_CPSS_DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,PRV_CPSS_QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = prvCpssDrvHwPhyReset(dev, hwPort, 0, u16Data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }
    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return retVal;
}

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
* @param[in] port                     -  The logical  number, unless SERDES device is accessed
*                                      The physical address, if SERDES device is accessed
* @param[in] mode                     -  Auto Mode to be written
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
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
    IN  GT_CPSS_QD_DEV         *dev,
    IN  GT_CPSS_LPORT          port,
    IN  GT_CPSS_PHY_AUTO_MODE  mode
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPortAutoMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    retVal = phySetAutoMode(dev, hwPort, mode);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return retVal;

}

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
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          u16Data;
    GT_U16          pageReg;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphyGetPortLoopback Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PHY(port);
    if (hwPort == GT_CPSS_INVALID_PHY)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad PHY Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    /* Read from Phy Control Register.  */
    retVal = prvCpssDrvHwGetPagedPhyRegField(dev,hwPort,0,PRV_CPSS_QD_PHY_CONTROL_REG,14,1,&u16Data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(u16Data, *enablePtr);

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->phyRegsSem);
    return retVal;
}

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
)
{
    GT_U8     hwPort;         /* the physical port number     */
    GT_U16    u16Data;
    GT_U16    pageReg;
    GT_STATUS retVal = GT_OK;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPause Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    PRV_CPSS_BOOL_2_BIT(state, u16Data);

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if ((retVal = prvCpssDrvHwSetPagedPhyRegField(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG, 10,1, u16Data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d).\n",hwPort,PRV_CPSS_QD_PHY_AUTONEGO_AD_REG));
        return retVal;
    }


    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    return retVal;
}

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
)
{
    GT_U8     hwPort;         /* the physical port number     */
    GT_U16    u16Data;
    GT_U16    pageReg;
    GT_STATUS retVal = GT_OK;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGphySetPause Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    prvCpssDrvGtSemTake(dev,dev->phyRegsSem,PRV_CPSS_OS_WAIT_FOREVER);
    if ((retVal = prvCpssDrvHwGetSMIPhyRegField(dev, hwPort, 22, 0, 8, &pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if ((retVal = prvCpssDrvHwGetPagedPhyRegField(dev, hwPort, 0, PRV_CPSS_QD_PHY_AUTONEGO_AD_REG, 10,1, &u16Data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,PRV_CPSS_QD_PHY_AUTONEGO_AD_REG));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(u16Data, *statePtr);

    if ((retVal = prvCpssDrvHwSetSMIPhyRegField(dev, hwPort, 22, 0, 8, pageReg)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Not able to Set back Phy Page Register.\n"));
        prvCpssDrvGtSemGive(dev, dev->phyRegsSem);
        return retVal;
    }

    return retVal;
}

