#include <Copyright.h>

/**
********************************************************************************
* @file gtPortCtrl.c
*
* @brief API implementation for switch port common control.
*
* @version   /
********************************************************************************
*/
/********************************************************************************
* gtPortCtrl.c
*
* DESCRIPTION:
*       API implementation for switch port common control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <gtPortCtrl.h>
#include <gtSem.h>
#include <gtHwAccess.h>
#include <msApiInternal.h>
#include <gtDrvSwRegs.h>
#include <gtUtils.h>
#include <gtPhyCtrl.h>

#define GT_GET_RATE_LIMIT_PER_FRAME(_frames, _dec)    \
        ((_frames)?(1000000000 / (16 * (_frames)) + ((1000000000 % (16 * (_frames)) > (16*(_frames)/2))?1:0)):0)

#define GT_GET_RATE_LIMIT_PER_BYTE(_kbps, _dec)    \
        ((_kbps)?((8000000*(_dec)) / (16 * (_kbps)) + ((8000000*(_dec)) % (16 * (_kbps))?1:0)):0)


/* Store a port's link status - bits <Force Link>, <Link Value> - and force it down */
#define STORE_AND_FORCE_LINK_STATUS_DOWN(dev, phyAddr, linkStatusPtr)               \
    if ( (retVal = prvStoreAndForceLinkDown(dev, phyAddr, linkStatusPtr)) != GT_OK) \
    {                                                                               \
        return retVal;                                                              \
    }

/*  Set a port's link status - two bits <Force Link>, <Link Value>  */
#define SET_FORCED_LINK_STATUS(dev, phyAdd, linkStatus)                                                 \
    if ((retVal = prvCpssDrvHwSetAnyRegField((dev), (phyAddr), PRV_CPSS_QD_REG_PHY_CONTROL, 4, 2, (linkStatus))) != GT_OK) \
    {                                                                                                   \
        PRV_CPSS_DBG_INFO(("Set forced link status error\n"));                                                   \
        return retVal;                                                                                  \
    }


static GT_STATUS writeFlowCtrlReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_CPSS_LPORT  port,
    IN  GT_U8          pointer,
    IN  GT_U8          data
);
static GT_STATUS readFlowCtrlReg
(
    IN  GT_CPSS_QD_DEV *dev,
    IN  GT_CPSS_LPORT  port,
    IN  GT_U8          pointer,
    OUT GT_U8          *data
);

/**
* @internal prvStoreAndForceLinkDown function
* @endinternal
*
* @brief   Store a port's link status - bits <Force Link>, <Link Value> - and force
*         it down
* @param[in] phyAddr                  - addressort - the logical port number.
* @param[in] phyAddr                  - SMI device address approrpriate to the port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
static GT_STATUS prvStoreAndForceLinkDown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_U8           phyAddr,
    OUT GT_U16          *linkStatusPtr
)
{
    GT_STATUS retVal;
    /* get the force link status */
    if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 4, 2, linkStatusPtr)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get link status Failed\n"));
        return retVal;
    }

    /* Set state of bits <Forced Link>, <Link Value> */
    SET_FORCED_LINK_STATUS(dev, phyAddr, 1);

    return GT_OK;
}

/**
* @internal prvCpssDrvGvlnSetPortVid function
* @endinternal
*
* @brief   This routine Set the port default vlan id.
*
* @param[in] port                     - logical  number to set.
* @param[in] vid                      - the port vlan id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvlnSetPortVid
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_U16            vid
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvlnSetPortVid Called.\n"));
    phyPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, phyPort);
    if (phyPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvHwSetAnyRegField(dev,phyAddr,PRV_CPSS_QD_REG_PVID,0,12, vid);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvlnGetPortVid function
* @endinternal
*
* @brief   This routine Get the port default vlan id.
*
* @param[in] port                     - logical  number to set.
*
* @param[out] vid                      - the port vlan id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvlnGetPortVid
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_U16          *vid
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           phyAddr;

    phyPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, phyPort);
    if (phyPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PVID, 0, 12, &data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *vid = data;
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvlnSetPortVlanDot1qMode function
* @endinternal
*
* @brief   This routine sets the IEEE 802.1q mode for this port
*
* @param[in] port                     - logical  number to set.
* @param[in] mode                     - 802.1q  for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvlnSetPortVlanDot1qMode
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_CPSS_LPORT         port,
    IN  GT_CPSS_DOT1Q_MODE    mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvlnSetPortVlanDot1qMode Called.\n"));
    phyPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, phyPort);
    if (phyPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvHwSetAnyRegField(dev,phyAddr,PRV_CPSS_QD_REG_PORT_CONTROL2,10,2,(GT_U16)mode );

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGvlnGetPortVlanDot1qMode function
* @endinternal
*
* @brief   This routine gets the IEEE 802.1q mode for this port.
*
* @param[in] port                     - logical  number to get.
*
* @param[out] mode                     - 802.1q  for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGvlnGetPortVlanDot1qMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_DOT1Q_MODE   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGvlnGetPortVlanDot1qMode Called.\n"));

    phyPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, phyPort);
    if (phyPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(mode == NULL)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev,phyAddr,PRV_CPSS_QD_REG_PORT_CONTROL2,10,2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *mode = data;
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetDiscardTagged function
* @endinternal
*
* @brief   When this bit is set to a one, all non-MGMT frames that are processed as
*         Tagged will be discarded as they enter this switch port. Priority only
*         tagged frames (with a VID of 0x000) are considered tagged.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE to discard tagged frame, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDiscardTagged
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDiscardTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set DiscardTagged. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 9, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetDiscardTagged function
* @endinternal
*
* @brief   This routine gets DiscardTagged bit for the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE if DiscardTagged bit is set, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDiscardTagged
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetDiscardTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the DiscardTagged. */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 9, 1, &data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetDiscardUntagged function
* @endinternal
*
* @brief   When this bit is set to a one, all non-MGMT frames that are processed as
*         Untagged will be discarded as they enter this switch port. Priority only
*         tagged frames (with a VID of 0x000) are considered tagged.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE to discard untagged frame, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDiscardUntagged
(
    IN GT_CPSS_QD_DEV    *dev,
    IN GT_CPSS_LPORT     port,
    IN GT_BOOL           mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDiscardUntagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set DiscardUnTagged. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 8, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetDiscardUntagged function
* @endinternal
*
* @brief   This routine gets DiscardUntagged bit for the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE if DiscardUntagged bit is set, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDiscardUntagged
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("gprtGetDiscardUnTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the DiscardUnTagged. */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 8, 1, &data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetForwardUnknown function
* @endinternal
*
* @brief   This routine set Forward Unknown mode of a switch port.
*         When this mode is set to GT_TRUE, normal switch operation occurs.
*         When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*         will not egress out this port.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE for normal switch operation or GT_FALSE to do not egress out the unknown DA unicast frames
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetForwardUnknown
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_BOOL          mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetForwardUnknown Called.\n"));

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the forward unknown mode.            */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL,2,1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetForwardUnknown function
* @endinternal
*
* @brief   This routine gets Forward Unknown mode of a switch port.
*         When this mode is set to GT_TRUE, normal switch operation occurs.
*         When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*         will not egress out this port.
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE for normal switch operation or GT_FALSE to do not egress out the unknown DA unicast frames
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetForwardUnknown
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetForwardUnknown Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the forward unknown Mode.            */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL, 2, 1, &data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetDefaultForward function
* @endinternal
*
* @brief   When this bit is set to a one, normal switch operation will occurs and
*         multicast frames with unknown DA addresses are allowed to egress out this
*         port (assuming the VLAN settings allow the frame to egress this port too).
*         When this bit is cleared to a zero, multicast frames with unknown DA
*         addresses will not egress out this port.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE for normal switch operation or GT_FALSE to do not egress out the unknown DA multicast frames
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDefaultForward
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDefaultForward Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set DefaultForward. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL, 3, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetDefaultForward function
* @endinternal
*
* @brief   This routine gets DefaultForward bit for the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE for normal switch operation or GT_FALSE to do not egress
*                                      out the unknown DA multicast frames
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDefaultForward
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;
    GT_U16          data;           /* to keep the read valve       */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetDefaultForward Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the DefaultForward. */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL, 3, 1, &data);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetDiscardBCastMode function
* @endinternal
*
* @brief   This routine sets the Discard Broadcast mode.
*         If the mode is enabled, all the broadcast frames to the given port will
*         be discarded.
* @param[in] port                     - logical  number
* @param[in] en                       - GT_TRUE, to enable the mode,
*                                      GT_FALSE, otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDiscardBCastMode
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           en
)
{
    GT_U16          data;           /* Used to poll the data */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDiscardBCastMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(en, data);

    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, 0x19, 13, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtGetDiscardBCastMode function
* @endinternal
*
* @brief   This routine gets the Discard Broadcast Mode. If the mode is enabled,
*         all the broadcast frames to the given port will be discarded.
* @param[in] port                     - logical  number
*
* @param[out] en                       - GT_TRUE, if enabled,
*                                      GT_FALSE, otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDiscardBCastMode
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    OUT GT_BOOL           *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetDiscardBCastMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = 0;

    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, 0x19, 13, 1, &data);

    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *en);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetJumboMode function
* @endinternal
*
* @brief   This routine Set the max frame size allowed to be received and transmitted
*         from or to a given port.
* @param[in] port                     - the logical  number
* @param[in] mode                     - GT_CPSS_JUMBO_MODE (1522, 2048, or 10240)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetJumboMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    IN  GT_CPSS_JUMBO_MODE   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("gsysSetJumboMode Called.\n"));

    if (mode > GT_CPSS_JUMBO_MODE_10240)
    {
        PRV_CPSS_DBG_INFO(("Bad Parameter\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the Jumbo Fram Size bit.               */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 12, 2, (GT_U16)mode);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtGetJumboMode function
* @endinternal
*
* @brief   This routine gets the max frame size allowed to be received and transmitted
*         from or to a given port.
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_CPSS_JUMBO_MODE (1522, 2048, or 10240)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetJumboMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_JUMBO_MODE   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;
    GT_U16          data;           /* to keep the read valve       */

    PRV_CPSS_DBG_INFO(("gsysGetJumboMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get Jumbo Frame Mode.            */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 12, 2, &data );
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *mode = (GT_CPSS_JUMBO_MODE)data;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetLearnDisable function
* @endinternal
*
* @brief   This routine enables/disables automatic learning of new source MAC
*         addresses on the given port ingress
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE for disable or GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetLearnDisable
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;
    GT_U16          pav;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetLearnDisable Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    /*
    PRV_CPSS_BOOL_2_BIT(mode, data);
    */

    /* Set the port's PAV to all zeros */
    if(mode)
        pav = 0;
    else
        pav = (GT_U16)(1 << port);

    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PAV, 0, 11, pav);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetLearnDisable function
* @endinternal
*
* @brief   This routine gets LearnDisable setup
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE: Learning disabled on the given port ingress frames,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetLearnDisable
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    OUT GT_BOOL           *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;
    GT_U16          data;           /* to keep the read valve       */
    GT_U16          pav;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetLearnDisable Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PAV, 0, 11, &pav);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    if (pav)
        data = 0;
    else
        data = 1;

    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetTrunkPort function
* @endinternal
*
* @brief   This function enables/disables and sets the trunk ID.
*
* @param[in] port                     - the logical  number.
* @param[in] en                       - GT_TRUE to make the port be a member of a trunk with the giv trunkId.
*                                      GT_FALSE, otherwise.
* @param[in] trunkId                  - valid ID is 0 ~ 31
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetTrunkPort
(
    IN GT_CPSS_QD_DEV    *dev,
    IN GT_CPSS_LPORT     port,
    IN GT_BOOL           en,
    IN GT_U32            trunkId
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetTrunkPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(en, data);

    if(en == GT_TRUE)
    {
        /* need to enable trunk. so check the trunkId */
        if (!PRV_CPSS_IS_TRUNK_ID_VALID(dev, trunkId))
        {
            PRV_CPSS_DBG_INFO(("GT_BAD_PARAM(trunkId)\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set TrunkId. */
        retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL1, 8, 5, (GT_U16)trunkId);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            return retVal;
        }
    }

    /* Set TrunkPort bit. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL1, 14, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetTrunkPort function
* @endinternal
*
* @brief   This function returns trunk state of the port.
*         When trunk is disabled, trunkId field won't have valid value.
* @param[in] port                     - the logical  number.
*
* @param[out] en                       - GT_TRUE, if the port is a member of a trunk,
*                                      GT_FALSE, otherwise.
* @param[out] trunkId                  - 0 ~ 31, valid only if en is GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetTrunkPort
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    OUT GT_BOOL           *en,
    OUT GT_U32            *trunkId
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetTrunkPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data = 0;
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL1, 14, 1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *en);

    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL1, 8, 5, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *trunkId = (GT_U32)data;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetFlowCtrl function
* @endinternal
*
* @brief   This routine enable/disable port flow control and set flow control mode
*         mode - CPSS_PORT_FC_TX_RX_ENABLED,
*         CPSS_PORT_RX_ONLY,
*         CPSS_PORT_TX_ONLY,
*         CPSS_PORT_PFC_ENABLED
* @param[in] port                     - the logical  number.
* @param[in] en                       - enable/disable the flow control
* @param[in] mode                     - flow control mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*
* @note ForcedFC is not affected, FCValue = enable/disable, FCMode
*
*/
GT_STATUS prvCpssDrvGprtSetFlowCtrl
(
    IN  GT_CPSS_QD_DEV        *dev,
    IN  GT_CPSS_LPORT         port,
    IN  GT_BOOL               en,
    IN  GT_CPSS_PORT_FC_MODE  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           tmpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetFlowCtrl Called.\n"));

    retVal = readFlowCtrlReg(dev, port, 0x10, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("read Flow Control Failed.\n"));
        return retVal;
    }

    tmpData &= ~0x07;
    tmpData = (GT_U8)(tmpData |((en == GT_TRUE? 1 : 0) << 2) | mode);

    retVal = writeFlowCtrlReg(dev, port, 0x10, tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("write Flow Control Failed.\n"));
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetFlowCtrl function
* @endinternal
*
* @brief   This routine get switch port flow control enable/disable status and return
*         flow control mode
* @param[in] port                     - the logical  number.
*
* @param[out] en                       - enable/disable the flow control
* @param[out] mode                     - flow control mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetFlowCtrl
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_BOOL              *en,
    OUT GT_CPSS_PORT_FC_MODE *mode
)
{
    GT_U8           tmpData;
    GT_STATUS       retVal;

    retVal = readFlowCtrlReg(dev, port, 0x10, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("read Flow Control Failed.\n"));
        return retVal;
    }

    *en = (tmpData >> 2) & 0x1;
    *mode = (GT_CPSS_PORT_FC_MODE)(tmpData & 0x3);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvGprtSetSpeed function
* @endinternal
*
* @brief   This routine forces switch MAC speed.
*
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g,or No Speed Force)
* @param[in] force                    - modify (GT_TRUE) or not (GT_FALSE) forceSpd bit.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
static GT_STATUS prvGprtSetSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    IN  GT_CPSS_PORT_FORCED_SPEED_MODE  mode,
    IN  GT_BOOL                         force
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          data1;
    GT_U16          data2;
    GT_U16          linkStatus;

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the force link status and then force link down */
    STORE_AND_FORCE_LINK_STATUS_DOWN(dev, phyAddr, &linkStatus);

    /* Set the ForceSpd bit.  */
    if (force == GT_TRUE)
    {
        retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 13, 1,
                                  (mode == CPSS_PORT_DO_NOT_FORCE_SPEED ? 0 : 1));
        if (retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Set Force Speed bit error\n"));
            return retVal;
        }
    }
    else if (mode == CPSS_PORT_DO_NOT_FORCE_SPEED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data1 = ((GT_U16)mode >> 12) & 0x1;
    data2 = (GT_U16)mode & 0x3;

    if ((retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 12, 1, data1)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set Alternate Speed bit error\n"));
        return retVal;
    }
    if ((retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 0, 2, data2)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set Speed bit error\n"));
        return retVal;
    }

    /* Set back state of bits <Forced Link>, <Link Value> */
    SET_FORCED_LINK_STATUS(dev, phyAddr, linkStatus);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetForceSpeed function
* @endinternal
*
* @brief   This routine forces switch MAC speed.
*
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g,
*                                      or No Speed Force)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetForceSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    IN  GT_CPSS_PORT_FORCED_SPEED_MODE  mode
)
{
    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetForceSpeed Called.\n"));

    return prvGprtSetSpeed(dev, port, mode, GT_TRUE);
};

/**
* @internal prvCpssDrvGprtSetSpeed function
* @endinternal
*
* @brief   This routine sets MAC speed. forcedSpd bit (speed auto-negotiation) is
*         not affected by this routine.
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g)
*                                      CPSS_PORT_DO_NOT_FORCE_SPEED value is not supported.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    IN  GT_CPSS_PORT_FORCED_SPEED_MODE  mode
)
{
    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetSpeed Called.\n"));
    return prvGprtSetSpeed(dev, port, mode, GT_FALSE);
}

/**
* @internal prvCpssDrvGprtGetForceSpeed function
* @endinternal
*
* @brief   This routine retrieves switch MAC Force Speed value
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g,or No Speed Force)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetForceSpeed
(
    IN  GT_CPSS_QD_DEV                   *dev,
    IN  GT_CPSS_LPORT                    port,
    OUT GT_CPSS_PORT_FORCED_SPEED_MODE   *mode
)
{
    GT_U16          data1;
    GT_U16          data2;
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetForceSpeed Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the ForceSpeed bit.  */
    if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 13, 1, &data1)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get force Speed bit error\n"));
        return retVal;
    }

    if(data1 == 0)
    {
        *mode = CPSS_PORT_DO_NOT_FORCE_SPEED;
    }
    else
    {
        if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 12, 1, &data1)) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Get Alternate Speed bit error\n"));
            return retVal;
        }
        if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 0, 2, &data2)) != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Get Speed bit error\n"));
            return retVal;
        }
        *mode = (GT_CPSS_PORT_FORCED_SPEED_MODE)((data1<<12) | data2);
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetEgressMonitorSource function
* @endinternal
*
* @brief   When this bit is cleared to a zero, normal network switching occurs.
*         When this bit is set to a one, any frame that egresses out this port will
*         also be sent to the EgressMonitorDest Port
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetEgressMonitorSource
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetEgressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set EgressMonitorSource. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 5, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetEgressMonitorSource function
* @endinternal
*
* @brief   This routine gets EgressMonitorSource bit for the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE if EgressMonitorSource bit is set, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetEgressMonitorSource
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetEgressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the EgressMonitorSource. */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 5, 1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetIngressMonitorSource function
* @endinternal
*
* @brief   When this be is cleared to a zero, normal network switching occurs.
*         When this bit is set to a one, any frame that ingresses in this port will
*         also be sent to the IngressMonitorDest Port
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE to set IngressMonitorSource, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetIngressMonitorSource
(
    IN GT_CPSS_QD_DEV    *dev,
    IN GT_CPSS_LPORT     port,
    IN GT_BOOL           mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetIngressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set IngressMonitorSource. */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 4, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetIngressMonitorSource function
* @endinternal
*
* @brief   This routine gets IngressMonitorSource bit for the given port
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE if IngressMonitorSource bit is set, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetIngressMonitorSource
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetIngressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the IngressMonitorSource. */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 4, 1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGrcSetEgressRate function
* @endinternal
*
* @brief   This routine sets the port's egress data limit.
*
* @param[in] port                     - logical  number.
* @param[in] rateType                 - egress data rate limit (GT_CPSS_ERATE_TYPE union type).
*                                      union type is used to support multiple devices with the
*                                      different formats of egress rate.
*                                      GT_CPSS_ERATE_TYPE has the following fields:
*                                      kbRate - rate in kbps that should used with the GT_CPSS_ELIMIT_MODE of
*                                      GT_CPSS_ELIMIT_LAYER1,
*                                      GT_CPSS_ELIMIT_LAYER2, or
*                                      GT_CPSS_ELIMIT_LAYER3 (see prvCpssDrvGrcSetELimitMode)
*                                      64kbps ~ 1Mbps    : increments of 64kbps,
*                                      1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                                      100Mbps ~ 1000Mbps: increments of 10Mbps
*                                      1Gbps ~ 5Gbps: increments of 100Mbps
*                                      Therefore, the valid values are:
*                                      64, 128, 192, 256, 320, 384,..., 960,
*                                      1000, 2000, 3000, 4000, ..., 100000,
*                                      110000, 120000, 130000, ..., 1000000
*                                      1100000, 1200000, 1300000, ..., 5000000.
*                                      fRate - frame per second that should used with GT_CPSS_ELIMIT_MODE of
*                                      GT_CPSS_PIRL_ELIMIT_FRAME
*                                      Valid values are between 3815 and 14880000
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGrcSetEgressRate
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    IN  GT_CPSS_ERATE_TYPE   *rateType
)
{
    GT_STATUS             retVal;         /* Functions return value.      */
    GT_U16                data;
    GT_U32                rate, eDec;
    GT_CPSS_ELIMIT_MODE   mode;
    GT_U8                 hwPort,phyAddr; /* Physical port.               */

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((retVal = prvCpssDrvGrcGetELimitMode(dev,port,&mode)) != GT_OK)
    {
        return retVal;
    }

    if (mode == GT_CPSS_ELIMIT_FRAME)
    {
        /* Count Per Frame */
        rate = rateType->fRate;

        if (rate == 0) /* disable egress rate limit */
        {
            eDec = 0;
            data = 0;
        }
        else if((rate < 3815)  || (rate > 14880000))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else
        {
            eDec = 1;
            data = (GT_U16)GT_GET_RATE_LIMIT_PER_FRAME(rate,eDec);
        }
    }
    else
    {
        /* Count Per Byte */
        rate = rateType->kbRate;

        if(rate == 0)
        {
            eDec = 0;
        }
        else if(rate < 1000)    /* less than 1Mbps */
        {
            /* it should be divided by 64 */
            if(rate % 64)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            eDec = rate/64;
        }
        else if(rate <= 100000)    /* less than or equal to 100Mbps */
        {
            /* it should be divided by 1000 */
            if(rate % 1000)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            eDec = rate/1000;
        }
        else if(rate <= 1000000)    /* less than or equal to 1000Mbps */
        {
            /* it should be divided by 10000 */
            if(rate % 10000)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            eDec = rate/10000;
        }
        else if(rate <= 5000000)    /* less than or equal to 10Gbps */
        {
            /* it should be divided by 100000 */
            if(rate % 100000)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            eDec = rate/100000;
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        if(rate == 0)
        {
            data = 0;
        }
        else
        {
            data = (GT_U16)GT_GET_RATE_LIMIT_PER_BYTE(rate,eDec);
        }
    }

    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_EGRESS_RATE_CTRL, 0, 7, (GT_U16)eDec);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_EGRESS_RATE_CTRL2, 0, 14, (GT_U16)data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGrcGetEgressRate function
* @endinternal
*
* @brief   This routine gets the port's egress data limit.
*
* @param[in] port                     - logical  number.
*
* @param[out] rateType                 - egress data rate limit (GT_CPSS_ERATE_TYPE union type).
*                                      union type is used to support multiple devices with the
*                                      different formats of egress rate.
*                                      GT_CPSS_ERATE_TYPE has the following fields:
*                                      kbRate - rate in kbps that should used with the GT_CPSS_ELIMIT_MODE of
*                                      GT_CPSS_ELIMIT_LAYER1,
*                                      GT_CPSS_ELIMIT_LAYER2, or
*                                      GT_CPSS_ELIMIT_LAYER3 (see prvCpssDrvGrcSetELimitMode)
*                                      64kbps ~ 1Mbps    : increments of 64kbps,
*                                      1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                                      100Mbps ~ 1000Mbps: increments of 10Mbps
*                                      1Gbps ~ 5Gbps: increments of 100Mbps
*                                      Therefore, the valid values are:
*                                      64, 128, 192, 256, 320, 384,..., 960,
*                                      1000, 2000, 3000, 4000, ..., 100000,
*                                      110000, 120000, 130000, ..., 1000000
*                                      1100000, 1200000, 1300000, ..., 5000000.
*                                      fRate - frame per second that should used with GT_CPSS_ELIMIT_MODE of
*                                      GT_CPSS_PIRL_ELIMIT_FRAME
*                                      Valid values are between 3815 and 14880000
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGrcGetEgressRate
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_ERATE_TYPE   *rateType
)
{
    GT_STATUS             retVal;         /* Functions return value.      */
    GT_U16                rate, eDec;
    GT_CPSS_ELIMIT_MODE   mode;
    GT_U8                 hwPort,phyAddr; /* Physical port.               */

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((retVal = prvCpssDrvGrcGetELimitMode(dev,port,&mode)) != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev,phyAddr,PRV_CPSS_QD_REG_EGRESS_RATE_CTRL,0,7,&eDec);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    retVal = prvCpssDrvHwGetAnyRegField(dev,phyAddr,PRV_CPSS_QD_REG_EGRESS_RATE_CTRL2,0,14,&rate );
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    if (mode == GT_CPSS_ELIMIT_FRAME)
    {
        rateType->fRate = GT_GET_RATE_LIMIT_PER_FRAME(rate,eDec);
    }
    else
    {
        /* Count Per Byte */
        rateType->kbRate = GT_GET_RATE_LIMIT_PER_BYTE(rate,eDec);
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}


/**
* @internal prvCpssDrvGrcSetELimitMode function
* @endinternal
*
* @brief   This routine sets Egress Rate Limit counting mode.
*         The supported modes are as follows:
*         GT_CPSS_ELIMIT_FRAME -
*         Count the number of frames
*         GT_CPSS_ELIMIT_LAYER1 -
*         Count all Layer 1 bytes:
*         Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*         GT_CPSS_ELIMIT_LAYER2 -
*         Count all Layer 2 bytes: Frame's DA to CRC
*         GT_CPSS_ELIMIT_LAYER3 -
*         Count all Layer 1 bytes:
*         Frame's DA to CRC - 18 - 4 (if frame is tagged)
* @param[in] port                     - logical  number
* @param[in] mode                     - GT_CPSS_ELIMIT_MODE enum type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGrcSetELimitMode
(
    IN  GT_CPSS_QD_DEV           *dev,
    IN  GT_CPSS_LPORT            port,
    IN  GT_CPSS_ELIMIT_MODE      mode
)
{
    GT_U16            data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGrcSetELimitMode Called.\n"));

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    data = (GT_U16)mode & 0x3;

    /* Set the Elimit mode.            */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_EGRESS_RATE_CTRL2, 14, 2, data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}


/**
* @internal prvCpssDrvGrcGetELimitMode function
* @endinternal
*
* @brief   This routine gets Egress Rate Limit counting mode.
*         The supported modes are as follows:
*         GT_CPSS_ELIMIT_FRAME -
*         Count the number of frames
*         GT_CPSS_ELIMIT_LAYER1 -
*         Count all Layer 1 bytes:
*         Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*         GT_CPSS_ELIMIT_LAYER2 -
*         Count all Layer 2 bytes: Frame's DA to CRC
*         GT_CPSS_ELIMIT_LAYER3 -
*         Count all Layer 1 bytes:
*         Frame's DA to CRC - 18 - 4 (if frame is tagged)
* @param[in] port                     - logical  number
*
* @param[out] mode                     - GT_CPSS_ELIMIT_MODE enum type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGrcGetELimitMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_ELIMIT_MODE  *mode
)
{
    GT_U16            data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGrcGetELimitMode Called.\n"));

    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);

    /* Get the Elimit mode.            */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_EGRESS_RATE_CTRL2, 14, 2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *mode = data;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtGetDuplex function
* @endinternal
*
* @brief   This routine retrives the port duplex mode.
*
* @param[in] port                     - the logical  number.
*
* @param[out] mode                     - GT_TRUE for Full-duplex  or GT_FALSE for Half-duplex
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetDuplex
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetDuplex Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);

    /* Get the Duplex bit.  */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_STATUS, 10, 1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *mode);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    /* return */
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetDuplex function
* @endinternal
*
* @brief   This routine sets the port duplex mode.
*
* @param[in] port                     - the logical  number.
* @param[in] mode                     - GT_TRUE for Full-duplex
*                                      GT_FALSE for Half-duplex
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetDuplex
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDuplex Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(mode, data);

    /* Set the DpxValue bit.  */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 3, 1, data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));

    return GT_OK;
}


/**
* @internal prvCpssDrvGprtGetLinkState function
* @endinternal
*
* @brief   This routine retrives the link state.
*
* @param[in] port                     - the logical  number.
*
* @param[out] state                    - GT_TRUE for Up  or GT_FALSE for Down
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetLinkState
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetLinkState Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);

    /* Get the force flow control bit.  */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_STATUS, 11, 1, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *state);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    /* return */
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtGetSpeed function
* @endinternal
*
* @brief   This routine retrives the port speed.
*
* @param[in] port                     - the logical  number.
*
* @param[out] speed                    - GT_CPSS_PORT_SPEED type.
*                                      (CPSS_PORT_SPEED_10_MBPS,CPSS_PORT_SPEED_100_MBPS,
*                                      CPSS_PORT_SPEED_1000_MBPS, etc.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetSpeed
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_PORT_SPEED   *speed
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;


    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetSpeed Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);


    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_STATUS, 8, 2, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *speed = (GT_CPSS_PORT_SPEED)data;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    /* return */
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtGetPortAttributes function
* @endinternal
*
* @brief   This routine retrieves the link / speed / duplex state.
*
* @param[in] dev                      - Remote device driver data.
* @param[in] port                     - the logical  number within the remote device.
*
* @param[out] statePtr                 - GT_TRUE for Up  or GT_FALSE for Down
* @param[out] speedPtr                 - GT_CPSS_PORT_SPEED type.
*                                      (CPSS_PORT_SPEED_10_MBPS,CPSS_PORT_SPEED_100_MBPS,
*                                      CPSS_PORT_SPEED_1000_MBPS, etc.)
* @param[out] duplexPtr                - GT_TRUE for Full-duplex  or GT_FALSE for Half-duplex
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortAttributes
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    OUT GT_BOOL             *statePtr,
    OUT GT_CPSS_PORT_SPEED  *speedPtr,
    OUT GT_BOOL             *duplexPtr
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetLinkState Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);

    /* Get the force flow control bit.  */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_STATUS, 8, 4, &data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* translate binary to BOOL or speed */
    *speedPtr = (GT_CPSS_PORT_SPEED)((data >> 0) & 0x03);
    PRV_CPSS_BIT_2_BOOL(((data >> 2) & 0x01), *duplexPtr);
    PRV_CPSS_BIT_2_BOOL(((data >> 3) & 0x01), *statePtr);


    PRV_CPSS_DBG_INFO(("OK.\n"));
    /* return */
    return GT_OK;
}

/****************************************************************************/
/* Internal functions.                                                      */
/****************************************************************************/
static GT_STATUS writeFlowCtrlReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U8           pointer,
    IN  GT_U8           data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        hwPort;         /* the physical port number     */
    GT_U8        phyAddr;
    GT_U16       count, tmpData;
    PRV_CPSS_DBG_INFO(("writeFlowCtrlReg Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    count = 5;
    tmpData = 1;
    while(tmpData ==1)
    {
        retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL, 15, 1, &tmpData);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if ((count--) == 0)
        {
            prvCpssDrvGtSemGive(dev, dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    tmpData =  (GT_U16)((1 << 15) | ((pointer&0x7F) << 8) | (GT_U16)data);

    retVal = prvCpssDrvHwSetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL, tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;

}
static GT_STATUS readFlowCtrlReg
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U8           pointer,
    OUT GT_U8           *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U8        hwPort;         /* the physical port number     */
    GT_U8        phyAddr;
    GT_U16       tmpData;
    int count=0x10;
    PRV_CPSS_DBG_INFO(("readFlowCtrlReg Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    tmpData =  (GT_U16)(((pointer&0x7F) << 8) | 0);
    retVal = prvCpssDrvHwSetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL, tmpData);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
       return retVal;
    }

    do {
        retVal = prvCpssDrvHwGetAnyReg(dev, phyAddr, PRV_CPSS_QD_REG_LIMIT_PAUSE_CONTROL, &tmpData);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
        if((count--)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    } while (tmpData&0x8000);

    *data = (GT_U8)(tmpData & 0xff);
    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    return retVal;
}

/**
* @internal prvCpssDrvGstpSetPortState function
* @endinternal
*
* @brief   This routine set the port state.
*
* @param[in] port                     - the logical  number.
* @param[in] state                    - the port  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*
*/
GT_STATUS prvCpssDrvGstpSetPortState
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_PORT_STP_STATE  state
)
{
    GT_U8           hwPort;         /* Physical port                */
    GT_U16          data;           /* Data to write to register.   */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstpSetPortState Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if (hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    data    = state;

    /* Set the port state bits.             */
    retVal= prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL,0,2,data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGstpGetPortState function
* @endinternal
*
* @brief   This routine returns the port state.
*
* @param[in] port                     - the logical  number.
*
* @param[out] state                    - the current port state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 
*       GalTis:
*
*/
GT_STATUS prvCpssDrvGstpGetPortState
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    OUT GT_CPSS_PORT_STP_STATE  *state
)
{
    GT_U8           hwPort;         /* Physical port                */
    GT_U16          data;           /* Data read from register.     */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGstpGetPortState Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the port state bits.             */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL,0,2,&data);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *state = data & 0x3;
    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetPortForceLinkDown function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @param[in] port                     - the logical  number.
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvGprtSetPortForceLinkDown
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           state
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortForceLinkDown Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the <ForceLink>=state, <LinkValue>=0.  */
    SET_FORCED_LINK_STATUS(dev, phyAddr, (state == GT_TRUE? 1 : 0));

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPortForceLinkDown function
* @endinternal
*
* @brief   Get Force Link Down state on specified port on specified device.
*
* @param[in] port                     - the logical  number.
*
* @param[out] statePtr                 - (pointer to) force link down state.
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortForceLinkDown
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *statePtr
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortForceLinkDown Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the ForceLink bit.  */
    if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 4, 2, &data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get Force Link bit error\n"));
        return retVal;
    }

    if (data == 1)
    {
        *statePtr = GT_TRUE;
    }
    else
    {
        *statePtr = GT_FALSE;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

static GT_STATUS writeEnergyManagementReg
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_U8             pointer,
    IN  GT_U32            data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U16       count, tmpData;
    PRV_CPSS_DBG_INFO(("writeEnergyManagementReg Called.\n"));

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    count = 5;
    tmpData = 1;
    while(tmpData ==1)
    {
        if ((count--) == 0)
        {
            prvCpssDrvGtSemGive(dev, dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_ENERGY_MNT, 15, 1, &tmpData);
        if(retVal != GT_OK)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    }

    tmpData =  (GT_U16)((1 << 15) | ((pointer&0x7F) << 8) | (data & 0xFF));

    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_ENERGY_MNT, tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
        return retVal;
    }

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

static GT_STATUS readEnergyManagementReg
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_U8            pointer,
    OUT GT_U32           *data
)
{
    GT_STATUS    retVal;         /* Functions return value.      */
    GT_U16       count, tmpData;
    PRV_CPSS_DBG_INFO(("readEnergyManagementReg Called.\n"));

    prvCpssDrvGtSemTake(dev,dev->tblRegsSem,PRV_CPSS_OS_WAIT_FOREVER);

    tmpData =  (GT_U16)(((pointer&0x7F) << 8) | 0);
    retVal = prvCpssDrvHwSetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_ENERGY_MNT, tmpData);
    if(retVal != GT_OK)
    {
       PRV_CPSS_DBG_INFO(("Failed.\n"));
       prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
       return retVal;
    }

    count = 5;
    do {
        if((count--)==0)
        {
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDrvHwGetAnyReg(dev, PRV_CPSS_GLOBAL2_DEV_ADDR, PRV_CPSS_QD_REG_ENERGY_MNT, &tmpData);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            prvCpssDrvGtSemGive(dev,dev->tblRegsSem);
            return retVal;
        }
    } while (tmpData&0x8000);

    prvCpssDrvGtSemGive(dev,dev->tblRegsSem);

    *data = (tmpData & 0xff);
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetPortEEE function
* @endinternal
*
* @brief   Enable/disable Force EEE state on specified port on specified device and
*         set relevant timers.
* @param[in] port                     - the logical  number.
* @param[in] state                    - When manual: GT_TRUE for force EEE enable, GT_FALSE otherwise
* @param[in] manual                   - GT_TRUE for force EEE enable or disable, GT_FALSE to use AN result.
* @param[in] twLimit                  - Wake timer in usec.
* @param[in] liLimit                  - Assertion timer in usec.
* @param[in] tsLimit                  - TxIdle timer in usec.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvGprtSetPortEEE
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_BOOL          state,
    IN  GT_BOOL          manual,
    IN  GT_U32           twLimit,
    IN  GT_U32           liLimit,
    IN  GT_U32           tsLimit
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortEEE Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the Force EEE state and value.  */
    data = 0;
    if (manual)
    {
        /* Forced */
        data |= 1;
    }
    if (state)
    {
        /* Forced enabled */
        data |= 2;
    }
    if ((retVal = prvCpssDrvHwSetAnyRegField((dev), (phyAddr), PRV_CPSS_QD_REG_PHY_CONTROL, 8, 2, data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set EEE status error\n"));
        return retVal;
    }

    /* Write timers */
    /* Index: 0x0A - Assertion timer */
    retVal = writeEnergyManagementReg(dev, 0x0A, liLimit);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set EEE Assertion timer error\n"));
        return retVal;
    }
    /* Index: 0x0B - Wake timer Fast Ethernet ports */
    retVal = writeEnergyManagementReg(dev, 0x0B, twLimit);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set EEE Wake timer Fast Ethernet error\n"));
        return retVal;
    }
    /* Index: 0x0C - Wake timer Giga Ethernet ports */
    retVal = writeEnergyManagementReg(dev, 0x0C, twLimit);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set EEE Wake timer Giga Ethernet error\n"));
        return retVal;
    }
    /* Index: 0x0F - TxIdle timer */
    retVal = writeEnergyManagementReg(dev, 0x0F, tsLimit);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set EEE TxIdle timer error\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPortEEE function
* @endinternal
*
* @brief   Get Enable/disable Force EEE state on specified port on specified device
*         and relevant timers.
* @param[in] port                     - the logical  number.
*                                      state   - GT_TRUE for force link down, GT_FALSE otherwise
*
* @param[out] statePtr                 - When manual: GT_TRUE for force EEE enable, GT_FALSE otherwise
* @param[out] manualPtr                - GT_TRUE for force EEE enable or disable, GT_FALSE to use AN result.
* @param[out] twLimitPtr               - Wake timer in usec.
* @param[out] liLimitPtr               - Assertion timer in usec.
* @param[out] tsLimitPtr               - TxIdle timer in usec.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDrvGprtGetPortEEE
(
    IN GT_CPSS_QD_DEV    *dev,
    IN GT_CPSS_LPORT     port,
    IN GT_BOOL           *statePtr,
    IN GT_BOOL           *manualPtr,
    IN GT_U32            *twLimitPtr,
    IN GT_U32            *liLimitPtr,
    IN GT_U32            *tsLimitPtr
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortEEE Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *statePtr = GT_FALSE;
    *manualPtr = GT_FALSE;
    *twLimitPtr = 0;
    *liLimitPtr = 0;
    *tsLimitPtr = 0;

    /* Set the <ForceLink>=state, <LinkValue>=0.  */
    data = 0;
    if ((retVal = prvCpssDrvHwGetAnyRegField((dev), (phyAddr), PRV_CPSS_QD_REG_PHY_CONTROL, 8, 2, &data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set forced EEE status error\n"));
        return retVal;
    }
    if (data & 0x01)
    {
        /* Forced */
        *manualPtr = GT_TRUE;
    }
    if (data & 0x02)
    {
        /* Forced enabled */
        *statePtr = GT_TRUE;
    }


    /* Read timers */
    /* Index: 0x0A - Assertion timer */
    retVal = readEnergyManagementReg(dev, 0x0A, liLimitPtr);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set forced EEE Assertion timer error\n"));
        return retVal;
    }
    /* Index: 0x0C - Wake timer Giga Ethernet ports */
    retVal = readEnergyManagementReg(dev, 0x0C, twLimitPtr);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set forced EEE Wake timer Giga Ethernet error\n"));
        return retVal;
    }
    /* Index: 0x0F - TxIdle timer */
    retVal = readEnergyManagementReg(dev, 0x0F, tsLimitPtr);
    if (retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set forced EEE TxIdle timer error\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetPortForceLinkPassEnable function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @param[in] port                     - the logical  number.
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtSetPortForceLinkPassEnable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortForceLinkPassEnable Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the ForceLink Pass bit.  */
    SET_FORCED_LINK_STATUS(dev, phyAddr, (state==GT_TRUE? 3 : 0));

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPortForceLinkPassEnable function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @param[in] port                     - the logical  number.
*
* @param[out] statePtr                 - (pointer to) current force link pass state.
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortForceLinkPassEnable
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *statePtr
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          data;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortForceLinkDown Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the ForceLink bit.  */
    if ((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 4, 2, &data)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get Force Link Pass bit error\n"));
        return retVal;
    }

    if (data == 3)
    {
        *statePtr = GT_TRUE;
    }
    else
    {
        *statePtr = GT_FALSE;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}


/**
* @internal prvCpssDrvGprtSetSpeedAutoNegEn function
* @endinternal
*
* @brief   This routine enables/disables speed auto-negotiation
*
* @param[in] port                     - the logical  number.
* @param[in] enable                   - GT_TRUE -  auto-negotiation,
*                                      GT_FALSE - disable auto-negotiation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtSetSpeedAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         enable
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          linkStatus;
    GT_U16          forcedSpeed;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetSpeedAutoNegEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the force link status and then force link down */
    STORE_AND_FORCE_LINK_STATUS_DOWN(dev, phyAddr, &linkStatus);

    forcedSpeed = (enable == GT_TRUE) ? 0 : 1;
    if((retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 13, 1, forcedSpeed)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set Force Speed bit error\n"));
        return retVal;
    }

    /* Set back state of bits <Forced Link>, <Link Value> */
    SET_FORCED_LINK_STATUS(dev, phyAddr, linkStatus);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}


/**
* @internal prvCpssDrvGprtGetSpeedAutoNegEn function
* @endinternal
*
* @brief   Return speed auto-negotiation status
*
* @param[in] port                     - the logical  number.
*
* @param[out] enablePtr                - GT_TRUE -  if auto-negotiation is enable,
*                                      GT_FALSE - if auto-negotiation is disabled,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtGetSpeedAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          forcedSpeed;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetSpeedAutoNegEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 13, 1, &forcedSpeed)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get Force Speed bit error\n"));
        return retVal;
    }

    *enablePtr = forcedSpeed ? GT_FALSE : GT_TRUE;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetDuplexAutoNegEn function
* @endinternal
*
* @brief   This routine enables/disables duplex auto-negotiation
*
* @param[in] port                     - the logical  number.
* @param[in] enable                   - GT_TRUE  -  auto-negotiation,
*                                      GT_FALSE - disable auto-negotiation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtSetDuplexAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         enable
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          linkStatus;
    GT_U16          forcedDuplex;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetDuplexAutoNegEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the force link status and then force link down */
    STORE_AND_FORCE_LINK_STATUS_DOWN(dev, phyAddr, &linkStatus);

    forcedDuplex = (enable == GT_TRUE) ? 0 : 1;
    if((retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 2, 1, forcedDuplex)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Set Force Duplex bit error\n"));
        return retVal;
    }

    /* Set back state of bits <Forced Link>, <Link Value> */
    SET_FORCED_LINK_STATUS(dev, phyAddr, linkStatus);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetDuplexAutoNegEn function
* @endinternal
*
* @brief   Return duplex auto-negotiation status
*
* @param[in] port                     - the logical  number.
*
* @param[out] enablePtr                - GT_TRUE -  if auto-negotiation is enable,
*                                      GT_FALSE - if auto-negotiation is disabled,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtGetDuplexAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS       retVal;
    GT_U8           hwPort;
    GT_U8           phyAddr;
    GT_U16          forcedDuplex;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetDuplexAutoNegEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PHY_CONTROL, 2, 1, &forcedDuplex)) != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Get Force Duplex bit error\n"));
        return retVal;
    }
    *enablePtr = forcedDuplex ? GT_FALSE : GT_TRUE;

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetPortEType function
* @endinternal
*
* @brief   This routine sets the port's special Ether Type. This Ether Type is used
*         for Policy (see gprtSetPolicy API) and FrameMode (see prvCpssDrvGprtSetFrameMode API).
* @param[in] port                     - the logical  number
* @param[in] etype                    - port's special ether type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtSetPortEType
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_CPSS_ETYPE   etype
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPortEType Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    data = (GT_U16)etype;

    /* Set the EtherType.            */
    retVal = prvCpssDrvHwSetAnyReg(dev,hwPort, PRV_CPSS_QD_REG_PORT_ETH_TYPE,data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetFrameMode function
* @endinternal
*
* @brief   Frame Mode is used to define the expected Ingress and the generated Egress
*         tagging frame format for this port as follows:
*         GT_CPSS_FRAME_MODE_NORMAL -
*         Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*         Untagged frames. Tagged frames use an Ether Type of 0x8100.
*         GT_CPSS_FRAME_MODE_DSA -
*         DSA mode uses a Marvell defined tagged frame format for
*         Chip-to-Chip and Chip-to-CPU connections.
*         GT_CPSS_FRAME_MODE_PROVIDER -
*         Provider mode uses user definable Ether Types per port
*         (see prvCpssDrvGprtSetPortEType/gprtGetPortEType API).
*         GT_CPSS_FRAME_MODE_ETHER_TYPE_DSA -
*         Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*         flowing a user definable Ether Type. This mode allows the mixture
*         of Normal Network frames with DSA Tagged frames and is useful to
*         be used on ports that connect to a CPU.
* @param[in] port                     - the logical  number
* @param[in] mode                     - GT_CPSS_FRAME_MODE type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if mode is unknown
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtSetFrameMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_FRAME_MODE  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetFrameMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    switch (mode)
    {
        case GT_CPSS_FRAME_MODE_NORMAL:
        case GT_CPSS_FRAME_MODE_DSA:
        case GT_CPSS_FRAME_MODE_PROVIDER:
        case GT_CPSS_FRAME_MODE_ETHER_TYPE_DSA:
            break;
        default:
            PRV_CPSS_DBG_INFO(("Bad Parameter\n"));
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set Frame Mode.  */
    retVal = prvCpssDrvHwSetAnyRegField(dev,hwPort, PRV_CPSS_QD_REG_PORT_CONTROL, 8, 2, (GT_U16)mode);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal gprtGetTxPaused function
* @endinternal
*
* @brief   This routine retrives Transmit Pause state.
*
* @param[in] port                     - the logical  number.
*
* @param[out] state                    - GT_TRUE if Rx MAC receives a PAUSE frame with none-zero Puase Time
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*
* @note GalTis:
*
*/
GT_STATUS gprtGetTxPaused
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    PRV_CPSS_DBG_INFO(("gprtGetTxPaused Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    /* Get the TxPaused bit.  */
    retVal = prvCpssDrvHwGetAnyRegField(dev,hwPort, PRV_CPSS_QD_REG_PORT_STATUS,5,1,&data);

    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }

    /* return */
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPortMacStatus function
* @endinternal
*
* @brief   This routine retrives the port MAC info.
*
* @param[in] port                     - the logical  number.
*
* @param[out] macStatus                - (pointer to) info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetPortMacStatus
(
    IN  GT_CPSS_QD_DEV           *dev,
    IN  GT_CPSS_LPORT            port,
    OUT GT_CPSS_PORT_MAC_STATUS  *macStatus
)
{
    GT_STATUS             retVal;     /* Functions return value.      */
    GT_BOOL               status;
    GT_BOOL               duplexMode;
    GT_CPSS_PORT_FC_MODE  mode;

    PRV_CPSS_DBG_INFO(("gprtGetPortStatus Called.\n"));

    /* isPortTxPause */
    retVal = gprtGetTxPaused(dev, port, &status);
    macStatus->isPortTxPause = status;
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* isPortRxPause and isPortBackPres */
    retVal = prvCpssDrvGprtGetDuplex(dev, port, &duplexMode);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* full duplex */
    if(duplexMode == GT_TRUE)
    {
        macStatus->isPortBackPres = GT_FALSE;
        retVal = prvCpssDrvGprtGetFlowCtrl(dev, port, &status, &mode);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            return retVal;
        }
        macStatus->isPortRxPause = status;
    }
    /* half duplex */
    else
    {
        macStatus->isPortRxPause = GT_FALSE;
        retVal = prvCpssDrvGprtGetFlowCtrl(dev, port, &status, &mode);
        if(retVal != GT_OK)
        {
            PRV_CPSS_DBG_INFO(("Failed.\n"));
            return retVal;
        }
        macStatus->isPortBackPres = status;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));

    return GT_OK;
}

/**
* @internal prvCpssDrvGqosSetDefFPri function
* @endinternal
*
* @brief   This routine sets the default frame priority (0 ~ 7).
*         This priority is used as the default frame priority (FPri) to use when
*         no other priority information is available.
* @param[in] port                     - the logical  number
* @param[in] pri                      - default frame priority
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if pri > 7
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGqosSetDefFPri
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U8           pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGqosSetDefFPri Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);

    if (pri > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the DefFPri.  */
    retVal = prvCpssDrvHwSetAnyRegField(dev,hwPort, PRV_CPSS_QD_REG_PVID, 13, 3, (GT_U16)pri);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }

    return retVal;
}


/**
* @internal prvCpssDrvGprtSetFlowCtrlAutoNegEn function
* @endinternal
*
* @brief   This routine enables/disables flow control auto-negotiation
*
* @param[in] port                     - the logical  number.
* @param[in] state                    - GT_TRUE  - enable flow control auto-negotiation,
*                                      GT_FALSE - disable flow control auto-negotiation
* @param[in] pauseAdv                 -  advertise symmetric flow control support in
*                                      Auto-Negotiation.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtSetFlowCtrlAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state,
    IN  GT_BOOL         pauseAdv
)
{

    GT_STATUS       retVal;
    GT_U8           tmpData;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetFlowCtrlAutoNegEn Called.\n"));

    /* read Flow Control Mode register (0x10) */
    retVal = readFlowCtrlReg(dev, port, 0x10, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("read Flow Control Failed.\n"));
        return retVal;
    }

    /* udate ForcedFC bit */
    if (state == GT_TRUE)
    {
        tmpData &= ~(1 << 3);
    }
    else
    {
        tmpData |= (1 << 3);
    }

    /* write Flow Control Mode register (0x10) */
    retVal = writeFlowCtrlReg(dev, port, 0x10, tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("write Flow Control Failed.\n"));
        return retVal;
    }

    /* set the pause bit in the Phy Autonegotiation Advertisement Register */
    retVal = prvCpssDrvGphySetPause(dev, port, state && pauseAdv);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("write Pause bit into Phy register is failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}


/**
* @internal prvCpssDrvGprtGetFlowCtrlAutoNegEn function
* @endinternal
*
* @brief   This routine get switch port flow control enable/disable status and return
*         flow control mode
* @param[in] port                     - the logical  number.
*
* @param[out] statePtr                 - GT_TRUE  - flow control auto-negotiation is enabled
*                                      GT_FALSE - flow  control  auto-negotiation is disabled
* @param[out] pauseAdvPtr              - if advertising of symmetric flow control in
*                                      Auto-Negotiation is enabled.
*                                      GT_TRUE  - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if invalid parameter is given
*/
GT_STATUS prvCpssDrvGprtGetFlowCtrlAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *statePtr,
    OUT GT_BOOL         *pauseAdvPtr
)
{
    GT_U8           tmpData;
    GT_STATUS       retVal;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetFlowCtrlAutoNegEn Called.\n"));

    /* read Flow Control Mode register (0x10) */
    retVal = readFlowCtrlReg(dev, port, 0x10, &tmpData);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("read Flow Control Failed.\n"));
        return retVal;
    }
    *statePtr  = ((tmpData >> 3) & 0x1) ? GT_FALSE : GT_TRUE;

    /* get the pause bit from the Phy Autonegotiation Advertisement Register */
    retVal = prvCpssDrvGphyGetPause(dev, port, pauseAdvPtr);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("read Pause bit from Phy register is failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return retVal;
}

/**
* @internal prvCpssDrvGprtSetAllowedBad function
* @endinternal
*
* @brief   Allow receiving frames on this port with a bad FCS..
*
* @param[in] port                     - the logical  number.
* @param[in] enable                   - GT_TRUE, frames receviced on this port with a CRC error are
*                                      not discarded.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDrvGprtSetAllowedBad
(
    IN  GT_CPSS_QD_DEV    *dev,
    IN  GT_CPSS_LPORT     port,
    IN  GT_BOOL           enable
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetAllowedBad Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* translate BOOL to binary */
    PRV_CPSS_BOOL_2_BIT(enable, data);

    /* Set Allowed bad FCS..            */
    retVal = prvCpssDrvHwSetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 14, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetAllowedBad function
* @endinternal
*
* @brief   This routine gets Ignore FCS setup
*         DESCRIPTION:
*         Allow receiving frames on this port with a bad FCS..
* @param[in] port                     - the logical  number.
*
* @param[out] enablePtr                - GT_TRUE, frames receviced on this port with a CRC error are not
*                                      discarded.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtGetAllowedBad
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U8           phyAddr;
    GT_U16          data;           /* to keep the read valve       */

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetAllowedBad Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_CPSS_LPORT_2_PORT(port);
    phyAddr = GT_CPSS_CALC_SMI_DEV_ADDR(dev, hwPort);
    if(hwPort == GT_CPSS_INVALID_PORT)
    {
        PRV_CPSS_DBG_INFO(("Failed (Bad Port).\n"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get Allowed bad FCS..            */
    retVal = prvCpssDrvHwGetAnyRegField(dev, phyAddr, PRV_CPSS_QD_REG_PORT_CONTROL2, 14, 1, &data );
    /* translate binary to BOOL  */
    PRV_CPSS_BIT_2_BOOL(data, *enablePtr);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }

    return retVal;
}


/**
* @internal prvCpssDrvGsysSetDiscardExcessive function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
* @param[in] enable                   - GT_TRUE - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 
*       GalTis:
*
*/
GT_STATUS prvCpssDrvGsysSetDiscardExcessive
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         enable
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysSetDiscardExcessive Called.\n"));
    PRV_CPSS_BOOL_2_BIT(enable,data);

    /* Set the Discard Exissive bit.                */
    retVal = prvCpssDrvHwSetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR, PRV_CPSS_QD_REG_GLOBAL_CONTROL, 13, 1, data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}


/**
* @internal prvCpssDrvGsysGetDiscardExcessive function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
* @param[in] enablePtr                - GT_TRUE - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 
*       GalTis:
*
*/
GT_STATUS prvCpssDrvGsysGetDiscardExcessive
(
    IN  GT_CPSS_QD_DEV   *dev,
    OUT GT_BOOL          *enablePtr
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    PRV_CPSS_DBG_INFO(("prvCpssDrvGsysGetDiscardExcessive Called.\n"));

    /* Set the Discard Exissive bit.                */
    retVal = prvCpssDrvHwGetAnyRegField(dev, PRV_CPSS_GLOBAL1_DEV_ADDR, PRV_CPSS_QD_REG_GLOBAL_CONTROL, 13, 1, &data);

    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
        return retVal;
    }
    PRV_CPSS_BIT_2_BOOL(data, *enablePtr);

    PRV_CPSS_DBG_INFO(("OK.\n"));
    return GT_OK;
}

/**
* @internal prvCpssDrvGprtSetPauseLimitOut function
* @endinternal
*
* @brief   Limit the number of continuous Pause refresh frames that can be transmitted
*         from this port. When full duplex Flow Control is enabled on this port,
*         these bits are used to limit the number of Pause refresh frames that can
*         be generated from this port to keep this port's link partner from sending
*         any data.
*         Setting this value to 0 will allow continuous Pause frame refreshes to
*         egress this port as long as this port remains congested.
*         Setting this value to 1 will allow 1 Pause frame to egress from this port
*         for each congestion situation.
*         Setting this value to 2 will allow 2 Pause frames to egress from this port
*         for each congestion situation, etc.
* @param[in] port                     - the logical  number
* @param[in] limit                    - the max number of Pause refresh frames for each congestion situation
*                                      ( 0 ~ 0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - if limit > 0xFF
*/
GT_STATUS prvCpssDrvGprtSetPauseLimitOut
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_U8            limit
)
{
    GT_STATUS retVal;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtSetPauseLimitOut Called.\n"));

    /* Set Pause Limit.  */
    retVal = writeFlowCtrlReg(dev, port, 0x01, limit);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    /* Reset pointer to 0x10.  */
    readFlowCtrlReg(dev, port, 0x10, &limit);
    return retVal;
}

/**
* @internal prvCpssDrvGprtGetPauseLimitOut function
* @endinternal
*
* @brief   Limit the number of continuous Pause refresh frames that can be transmitted
*         from this port. When full duplex Flow Control is enabled on this port,
*         these bits are used to limit the number of Pause refresh frames that can
*         be generated from this port to keep this port's link partner from sending
*         any data.
*         Setting this value to 0 will allow continuous Pause frame refreshes to
*         egress this port as long as this port remains congested.
*         Setting this value to 1 will allow 1 Pause frame to egress from this port
*         for each congestion situation.
*         Setting this value to 2 will allow 2 Pause frames to egress from this port
*         for each congestion situation, etc.
* @param[in] port                     - the logical  number
*
* @param[out] limitPtr                 - the max number of Pause refresh frames for each congestion
*                                      situation (0 ~ 0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - if current device does not support this feature.
*/
GT_STATUS prvCpssDrvGprtGetPauseLimitOut
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    OUT GT_U8            *limitPtr
)
{
    GT_STATUS   retVal;
    GT_U8       limit;

    PRV_CPSS_DBG_INFO(("prvCpssDrvGprtGetPauseLimitOut Called.\n"));

    /* Get Pause Limit.  */
    retVal = readFlowCtrlReg(dev, port, 0x1, limitPtr);
    if(retVal != GT_OK)
    {
        PRV_CPSS_DBG_INFO(("Failed.\n"));
    }
    else
    {
        PRV_CPSS_DBG_INFO(("OK.\n"));
    }
    /* Reset pointer to 0x10.  */
    readFlowCtrlReg(dev, port, 0x10, &limit);
    return retVal;
}

