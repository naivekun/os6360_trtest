#include <Copyright.h>
/*******************************************************************************

* gtPortCtrl.h
*
* DESCRIPTION:
*       API/Structure definitions for Marvell PortCtrl functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*******************************************************************************/

#ifndef __prvCpssDrvGtPortCtrl_h
#define __prvCpssDrvGtPortCtrl_h

#include <msApiTypes.h>
#include <gtSysConfig.h>
#include <gtBrgStu.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Exported Port Control Types                                                */
/******************************************************************************/

/*  typedef: enum GT_CPSS_DOT1Q_MODE */
typedef enum
{
    GT_CPSS_DISABLE = 0,
    GT_CPSS_FALLBACK,
    GT_CPSS_CHECK,
    GT_CPSS_SECURE
} GT_CPSS_DOT1Q_MODE;

/* Definition for the Ingree/Egress Frame Mode */
typedef enum
{
    GT_CPSS_FRAME_MODE_NORMAL = 0,    /* Normal Network */
    GT_CPSS_FRAME_MODE_DSA,            /* Distributed Switch Architecture */
    GT_CPSS_FRAME_MODE_PROVIDER,        /* Provider */
    GT_CPSS_FRAME_MODE_ETHER_TYPE_DSA    /* Ether Type DSA */
} GT_CPSS_FRAME_MODE;


/*
 * Typedef: enum GT_CPSS_JUMBO_MODE
 *
 * Description: Defines Jumbo Frame Size allowed to be tx and rx
 *
 * Fields:
 *      GT_CPSS_JUMBO_MODE_1522 - Rx and Tx frames with max byte of 1522.
 *      GT_CPSS_JUMBO_MODE_2048 - Rx and Tx frames with max byte of 2048.
 *      GT_CPSS_JUMBO_MODE_10240 - Rx and Tx frames with max byte of 10240.
 *
 */
typedef enum
{
    GT_CPSS_JUMBO_MODE_1522 = 0,
    GT_CPSS_JUMBO_MODE_2048,
    GT_CPSS_JUMBO_MODE_10240
} GT_CPSS_JUMBO_MODE;

/* Definition for full duplex flow control mode */
typedef enum
{
        CPSS_PORT_FC_TX_RX_ENABLED,
        CPSS_PORT_RX_ONLY,
        CPSS_PORT_TX_ONLY,
        CPSS_PORT_PFC_ENABLED
} GT_CPSS_PORT_FC_MODE;

/* Definition for the forced Port Speed */
typedef enum
{
    CPSS_PORT_FORCE_SPEED_10_MBPS   = 0x0,
    CPSS_PORT_FORCE_SPEED_100_MBPS  = 0x1,
    CPSS_PORT_FORCE_SPEED_200_MBPS  = (1<<12) | 0x2,
    CPSS_PORT_FORCE_SPEED_1000_MBPS = 0x2,
    CPSS_PORT_FORCE_SPEED_2_5_GBPS  = (1<<12) | 0x3,
    CPSS_PORT_FORCE_SPEED_10_GBPS   = 0x3,
    CPSS_PORT_DO_NOT_FORCE_SPEED    = 0xf
} GT_CPSS_PORT_FORCED_SPEED_MODE;

/*
 *  typedef: enum GT_CPSS_ELIMIT_MODE
 *
 *  Description: Enumeration of the port egress rate limit counting mode.
 *
 *  Enumerations:
 *      GT_CPSS_ELIMIT_FRAME -
 *                Count the number of frames
 *      GT_CPSS_ELIMIT_LAYER1 -
 *                Count all Layer 1 bytes:
 *                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      GT_CPSS_ELIMIT_LAYER2 -
 *                Count all Layer 2 bytes: Frame's DA to CRC
 *      GT_CPSS_ELIMIT_LAYER3 -
 *                Count all Layer 3 bytes:
 *                Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum
{
    GT_CPSS_ELIMIT_FRAME = 0,
    GT_CPSS_ELIMIT_LAYER1,
    GT_CPSS_ELIMIT_LAYER2,
    GT_CPSS_ELIMIT_LAYER3
} GT_CPSS_ELIMIT_MODE;

/*
 * Typedef: union GT_CPSS_ERATE_TYPE
 *
 * Description: Egress Rate
 *
 * Fields:
 *      kbRate      - rate in kbps that should used with the GT_CPSS_ELIMIT_MODE of
 *                                GT_CPSS_ELIMIT_LAYER1,
 *                                GT_CPSS_ELIMIT_LAYER2, or
 *                                GT_CPSS_ELIMIT_LAYER3 (see prvCpssDrvGrcSetELimitMode)
 *                            64kbps ~ 1Mbps    : increments of 64kbps,
 *                            1Mbps ~ 100Mbps   : increments of 1Mbps, and
 *                            100Mbps ~ 1000Mbps: increments of 10Mbps
 *                            1Gbps ~ 5Gbps: increments of 100Mbps
 *                            Therefore, the valid values are:
 *                                64, 128, 192, 256, 320, 384,..., 960,
 *                                1000, 2000, 3000, 4000, ..., 100000,
 *                                110000, 120000, 130000, ..., 1000000
 *                                1100000, 1200000, 1300000, ..., 5000000.
 *      fRate       - frame per second that should used with GT_CPSS_ELIMIT_MODE of
 *                                GT_CPSS_PIRL_ELIMIT_FRAME
 *                            Valid values are between 3815 and 14880000
 */
typedef union
{
    GT_U32            kbRate;
    GT_U32            fRate;
} GT_CPSS_ERATE_TYPE;

/* Definition for the Port Speed */
typedef enum
{
    CPSS_PORT_SPEED_10_MBPS = 0,
    CPSS_PORT_SPEED_100_MBPS = 1,
    CPSS_PORT_SPEED_1000_MBPS = 2,
    CPSS_PORT_SPEED_10_GBPS = 3,
    CPSS_PORT_SPEED_UNKNOWN = 4
} GT_CPSS_PORT_SPEED;

/*
 * typedef: struct GT_CPSS_PORT_MAC_STATUS_STC
 *
 * Description: Structure for configuring Protocol based classification
 *
 * Fields:
 *      isPortRxPause     - whether port receives pause.
 *      isPortTxPause     - whether port transmit pause.
 *      isPortBackPres    - whether is in Back pressure.
 */
typedef struct
{
    GT_BOOL isPortRxPause;
    GT_BOOL isPortTxPause;
    GT_BOOL isPortBackPres;
}GT_CPSS_PORT_MAC_STATUS;

/******************************************************************************/
/* Exported Port Control Functions                                            */
/******************************************************************************/

/*******************************************************************************
* prvCpssDrvGvlnSetPortVid
*
* DESCRIPTION:
*       This routine Set the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       vid  - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGvlnSetPortVid
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U16          vid
);

/*******************************************************************************
* prvCpssDrvGvlnGetPortVid
*
* DESCRIPTION:
*       This routine Get the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       vid  - the port vlan id.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGvlnGetPortVid
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_U16          *vid
);

/********************************************************************
* prvCpssDrvGvlnSetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine sets the IEEE 802.1q mode for this port
*
* INPUTS:
*       port    - logical port number to set.
*       mode     - 802.1q mode for this port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGvlnSetPortVlanDot1qMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_DOT1Q_MODE  mode
);

/*******************************************************************************
* prvCpssDrvGvlnGetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine gets the IEEE 802.1q mode for this port.
*
* INPUTS:
*       port     - logical port number to get.
*
* OUTPUTS:
*       mode     - 802.1q mode for this port
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*                None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGvlnGetPortVlanDot1qMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    OUT GT_CPSS_DOT1Q_MODE  *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetDiscardTagged
*
* DESCRIPTION:
*       When this bit is set to a one, all non-MGMT frames that are processed as
*       Tagged will be discarded as they enter this switch port. Priority only
*       tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE to discard tagged frame, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDiscardTagged
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetDiscardTagged
*
* DESCRIPTION:
*       This routine gets DiscardTagged bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - GT_TRUE if DiscardTagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*                None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDiscardTagged
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetDiscardUntagged
*
* DESCRIPTION:
*       When this bit is set to a one, all non-MGMT frames that are processed as
*       Untagged will be discarded as they enter this switch port. Priority only
*       tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE to discard untagged frame, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDiscardUntagged
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetDiscardUntagged
*
* DESCRIPTION:
*       This routine gets DiscardUntagged bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - GT_TRUE if DiscardUntagged bit is set, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDiscardUntagged
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetForwardUnknown
*
* DESCRIPTION:
*       This routine set Forward Unknown mode of a switch port.
*       When this mode is set to GT_TRUE, normal switch operation occurs.
*       When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*       will not egress out this port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for normal switch operation or GT_FALSE to do not egress out the unknown DA unicast frames
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetForwardUnknown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetForwardUnknown
*
* DESCRIPTION:
*       This routine gets Forward Unknown mode of a switch port.
*       When this mode is set to GT_TRUE, normal switch operation occurs.
*       When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*       will not egress out this port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for normal switch operation or GT_FALSE to do not egress
*              out the unknown DA unicast frames
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetForwardUnknown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetDefaultForward
*
* DESCRIPTION:
*       When this bit is set to a one, normal switch operation will occurs and
*       multicast frames with unknown DA addresses are allowed to egress out this
*       port (assuming the VLAN settings allow the frame to egress this port too).
*       When this bit is cleared to a zero, multicast frames with unknown DA
*       addresses will not egress out this port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for normal switch operation or GT_FALSE to do not egress
*              out the unknown DA multicast frames
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*                None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDefaultForward
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetDefaultForward
*
* DESCRIPTION:
*       This routine gets DefaultForward bit for the given port
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for normal switch operation or GT_FALSE to do not egress
*              out the unknown DA multicast frames
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*                None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDefaultForward
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetDiscardBCastMode
*
* DESCRIPTION:
*       This routine sets the Discard Broadcast mode.
*       If the mode is enabled, all the broadcast frames to the given port will
*       be discarded.
*
* INPUTS:
*       port - logical port number
*       en   - GT_TRUE, to enable the mode,
*              GT_FALSE, otherwise.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDiscardBCastMode
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         en
);

/*******************************************************************************
* prvCpssDrvGprtGetDiscardBCastMode
*
* DESCRIPTION:
*       This routine gets the Discard Broadcast Mode. If the mode is enabled,
*       all the broadcast frames to the given port will be discarded.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*       en   - GT_TRUE, if enabled,
*              GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK        - on success
*       GT_FAIL      - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDiscardBCastMode
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *en
);

/*******************************************************************************
* prvCpssDrvGprtSetJumboMode
*
* DESCRIPTION:
*       This routine Set the max frame size allowed to be received and transmitted
*       from or to a given port.
*
* INPUTS:
*       port - the logical port number
*       mode - GT_CPSS_JUMBO_MODE (1522, 2048, or 10240)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetJumboMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_JUMBO_MODE  mode
);

/*******************************************************************************
* prvCpssDrvGprtGetJumboMode
*
* DESCRIPTION:
*       This routine gets the max frame size allowed to be received and transmitted
*       from or to a given port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_CPSS_JUMBO_MODE (1522, 2048, or 10240)
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetJumboMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_JUMBO_MODE   *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetLearnDisable
*
* DESCRIPTION:
*       This routine enables/disables automatic learning of new source MAC
*       addresses on the given port ingress
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for disable or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetLearnDisable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetLearnDisable
*
* DESCRIPTION:
*       This routine gets LearnDisable setup
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: Learning disabled on the given port ingress frames,
*              GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetLearnDisable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetTrunkPort
*
* DESCRIPTION:
*        This function enables/disables and sets the trunk ID.
*
* INPUTS:
*        port - the logical port number.
*        en - GT_TRUE to make the port be a member of a trunk with the given trunkId.
*             GT_FALSE, otherwise.
*        trunkId - valid ID is 0 ~ 31
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_BAD_PARAM - if port is invalid nor INVALID_TRUNK_ID
*
* COMMENTS:
*        None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetTrunkPort
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         en,
    IN  GT_U32          trunkId
);

/*******************************************************************************
* prvCpssDrvGprtGetTrunkPort
*
* DESCRIPTION:
*       This function returns trunk state of the port.
*       When trunk is disabled, trunkId field won't have valid value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en - GT_TRUE, if the port is a member of a trunk,
*            GT_FALSE, otherwise.
*       trunkId - 0 ~ 31, valid only if en is GT_TRUE
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetTrunkPort
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *en,
    OUT GT_U32          *trunkId
);

/*******************************************************************************
* prvCpssDrvGprtSetFlowCtrl
*
* DESCRIPTION:
*       This routine enable/disable port flow control and set flow control mode
*          mode - CPSS_PORT_FC_TX_RX_ENABLED,
*                 CPSS_PORT_RX_ONLY,
*                 CPSS_PORT_TX_ONLY,
*                 CPSS_PORT_PFC_ENABLED
*
* INPUTS:
*       port - the logical port number.
*       en - enable/disable the flow control
*       mode - flow control mode
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       Set ForcedFC=1, FCValue = enable/disable, FCMode
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetFlowCtrl
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_BOOL                 en,
    IN  GT_CPSS_PORT_FC_MODE    mode
);
/*******************************************************************************
* prvCpssDrvGprtGetFlowCtrl
*
* DESCRIPTION:
*       This routine get switch port flow control enable/disable status and return
*       flow control mode
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en - enable/disable the flow control
*       mode - flow control mode
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetFlowCtrl
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    OUT GT_BOOL                 *en,
    OUT GT_CPSS_PORT_FC_MODE    *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetForceSpeed
*
* DESCRIPTION:
*       This routine forces switch MAC speed.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g
*              or No Speed Force)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetForceSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    IN  GT_CPSS_PORT_FORCED_SPEED_MODE  mode
);

/*******************************************************************************
* prvCpssDrvGprtGetForceSpeed
*
* DESCRIPTION:
*       This routine retrieves switch MAC Force Speed value
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g
*              or No Speed Force)
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetForceSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    OUT GT_CPSS_PORT_FORCED_SPEED_MODE  *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetEgressMonitorSource
*
* DESCRIPTION:
*       When this bit is cleared to a zero, normal network switching occurs.
*       When this bit is set to a one, any frame that egresses out this port will
*       also be sent to the EgressMonitorDest Port
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetEgressMonitorSource
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetEgressMonitorSource
*
* DESCRIPTION:
*       This routine gets EgressMonitorSource bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - GT_TRUE if EgressMonitorSource bit is set, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetEgressMonitorSource
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetIngressMonitorSource
*
* DESCRIPTION:
*       When this be is cleared to a zero, normal network switching occurs.
*       When this bit is set to a one, any frame that ingresses in this port will
*       also be sent to the IngressMonitorDest Port
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE to set IngressMonitorSource, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetIngressMonitorSource
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetIngressMonitorSource
*
* DESCRIPTION:
*       This routine gets IngressMonitorSource bit for the given port
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode  - GT_TRUE if IngressMonitorSource bit is set, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetIngressMonitorSource
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGrcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*
*
* INPUTS:
*       port      - logical port number.
*       rateType  - egress data rate limit (GT_CPSS_ERATE_TYPE union type).
*                    union type is used to support multiple devices with the
*                    different formats of egress rate.
*                    GT_CPSS_ERATE_TYPE has the following fields:
*                        kbRate - rate in kbps that should used with the GT_CPSS_ELIMIT_MODE of
*                                GT_CPSS_ELIMIT_LAYER1,
*                                GT_CPSS_ELIMIT_LAYER2, or
*                                GT_CPSS_ELIMIT_LAYER3 (see prvCpssDrvGrcSetELimitMode)
*                            64kbps ~ 1Mbps    : increments of 64kbps,
*                            1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                            100Mbps ~ 1000Mbps: increments of 10Mbps
*                            1Gbps ~ 5Gbps: increments of 100Mbps
*                            Therefore, the valid values are:
*                                64, 128, 192, 256, 320, 384,..., 960,
*                                1000, 2000, 3000, 4000, ..., 100000,
*                                110000, 120000, 130000, ..., 1000000
*                                1100000, 1200000, 1300000, ..., 5000000.
*                        fRate - frame per second that should used with GT_CPSS_ELIMIT_MODE of
*                                GT_CPSS_PIRL_ELIMIT_FRAME
*                            Valid values are between 3815 and 14880000
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGrcSetEgressRate
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_ERATE_TYPE  *rateType
);

/*******************************************************************************
* prvCpssDrvGrcGetEgressRate
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port    - logical port number.
*
* OUTPUTS:
*       rateType  - egress data rate limit (GT_CPSS_ERATE_TYPE union type).
*                    union type is used to support multiple devices with the
*                    different formats of egress rate.
*                    GT_CPSS_ERATE_TYPE has the following fields:
*                        kbRate - rate in kbps that should used with the GT_CPSS_ELIMIT_MODE of
*                                GT_CPSS_ELIMIT_LAYER1,
*                                GT_CPSS_ELIMIT_LAYER2, or
*                                GT_CPSS_ELIMIT_LAYER3 (see prvCpssDrvGrcSetELimitMode)
*                            64kbps ~ 1Mbps    : increments of 64kbps,
*                            1Mbps ~ 100Mbps   : increments of 1Mbps, and
*                            100Mbps ~ 1000Mbps: increments of 10Mbps
*                            1Gbps ~ 5Gbps: increments of 100Mbps
*                            Therefore, the valid values are:
*                                64, 128, 192, 256, 320, 384,..., 960,
*                                1000, 2000, 3000, 4000, ..., 100000,
*                                110000, 120000, 130000, ..., 1000000
*                                1100000, 1200000, 1300000, ..., 5000000.
*                        fRate - frame per second that should used with GT_CPSS_ELIMIT_MODE of
*                                GT_CPSS_PIRL_ELIMIT_FRAME
*                            Valid values are between 3815 and 14880000
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGrcGetEgressRate
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_ERATE_TYPE   *rateType
);

/*******************************************************************************
* prvCpssDrvGrcSetELimitMode
*
* DESCRIPTION:
*       This routine sets Egress Rate Limit counting mode.
*       The supported modes are as follows:
*            GT_CPSS_ELIMIT_FRAME -
*                Count the number of frames
*            GT_CPSS_ELIMIT_LAYER1 -
*                Count all Layer 1 bytes:
*                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*            GT_CPSS_ELIMIT_LAYER2 -
*                Count all Layer 2 bytes: Frame's DA to CRC
*            GT_CPSS_ELIMIT_LAYER3 -
*                Count all Layer 1 bytes:
*                Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*       mode - GT_CPSS_ELIMIT_MODE enum type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGrcSetELimitMode
(
    IN  GT_CPSS_QD_DEV           *dev,
    IN  GT_CPSS_LPORT            port,
    IN  GT_CPSS_ELIMIT_MODE      mode
);

/*******************************************************************************
* prvCpssDrvGrcGetELimitMode
*
* DESCRIPTION:
*       This routine gets Egress Rate Limit counting mode.
*       The supported modes are as follows:
*            GT_CPSS_ELIMIT_FRAME -
*                Count the number of frames
*            GT_CPSS_ELIMIT_LAYER1 -
*                Count all Layer 1 bytes:
*                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*            GT_CPSS_ELIMIT_LAYER2 -
*                Count all Layer 2 bytes: Frame's DA to CRC
*            GT_CPSS_ELIMIT_LAYER3 -
*                Count all Layer 1 bytes:
*                Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*       mode - GT_CPSS_ELIMIT_MODE enum type
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGrcGetELimitMode
(
    IN  GT_CPSS_QD_DEV       *dev,
    IN  GT_CPSS_LPORT        port,
    OUT GT_CPSS_ELIMIT_MODE  *mode
);

/*******************************************************************************
* prvCpssDrvGprtGetDuplex
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for Full-duplex  or GT_FALSE for Half-duplex
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDuplex
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *mode
);

/*******************************************************************************
* prvCpssDrvGprtSetDuplex
*
* DESCRIPTION:
*       This routine sets the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for Full-duplex
*              GT_FALSE for Half-duplex
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDuplex
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         mode
);

/*******************************************************************************
* prvCpssDrvGprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Up  or GT_FALSE for Down
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetLinkState
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *state
);

/*******************************************************************************
* prvCpssDrvGprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port MAC speed.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       speed - GT_CPSS_PORT_SPEED type.
*                (CPSS_PORT_SPEED_10_MBPS,CPSS_PORT_SPEED_100_MBPS, CPSS_PORT_SPEED_1000_MBPS,
*                etc.)
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetSpeed
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    OUT GT_CPSS_PORT_SPEED  *speed
);

/*******************************************************************************
* prvCpssDrvGprtGetPortAttributes
*
* DESCRIPTION:
*       This routine retrieves the link / speed / duplex state.
*
* INPUTS:
*       dev - Remote device driver data.
*       port - the logical port number within the remote device.
*
* OUTPUTS:
*       statePtr - GT_TRUE for Up  or GT_FALSE for Down
*       speedPtr - GT_CPSS_PORT_SPEED type.
*                (CPSS_PORT_SPEED_10_MBPS,CPSS_PORT_SPEED_100_MBPS, CPSS_PORT_SPEED_1000_MBPS,
*                etc.)
*       duplexPtr - GT_TRUE for Full-duplex  or GT_FALSE for Half-duplex
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPortAttributes
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    OUT GT_BOOL             *statePtr,
    OUT GT_CPSS_PORT_SPEED  *speedPtr,
    OUT GT_BOOL             *duplexPtr
);

/*******************************************************************************
* prvCpssDrvGstpSetPortState
*
* DESCRIPTION:
*       This routine set the port state.
*
* INPUTS:
*       port  - the logical port number.
*       state - the port state to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGstpSetPortState
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    IN  GT_CPSS_PORT_STP_STATE  state
);

/*******************************************************************************
* prvCpssDrvGstpGetPortState
*
* DESCRIPTION:
*       This routine returns the port state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       state - the current port state.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGstpGetPortState
(
    IN  GT_CPSS_QD_DEV          *dev,
    IN  GT_CPSS_LPORT           port,
    OUT GT_CPSS_PORT_STP_STATE  *state
);

/*******************************************************************************
* prvCpssDrvGprtSetPortForceLinkDown
*
* DESCRIPTION:
*       Enable/disable Force Link Down on specified port on specified device.
*
* INPUTS:
*       port    - the logical port number.
*       state   - GT_TRUE for force link down, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetPortForceLinkDown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state
);

/*******************************************************************************
* prvCpssDrvGprtGetPortForceLinkDown
*
* DESCRIPTION:
*       Get Force Link Down state on specified port on specified device.
*
* INPUTS:
*       port    - the logical port number.
*
* OUTPUTS:
*       statePtr - (pointer to) force link down state.
*                  GT_TRUE for force link down, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPortForceLinkDown
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *statePtr
);

/*******************************************************************************
* prvCpssDrvGprtSetPortForceLinkPassEnable
*
* DESCRIPTION:
*       Enable/disable Force Link Pass on specified port on specified device.
*
* INPUTS:
*       port    - the logical port number.
*       state    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetPortForceLinkPassEnable
(
    IN GT_CPSS_QD_DEV   *dev,
    IN GT_CPSS_LPORT    port,
    IN GT_BOOL          state
);

/*******************************************************************************
* prvCpssDrvGprtGetPortForceLinkPassEnable
*
* DESCRIPTION:
*       Get Force Link Pass on specified port on specified device.
*
* INPUTS:
*       port    - the logical port number.
*
* OUTPUTS:
*       statePtr - (pointer to) current force link pass state:
*                  GT_TRUE for force link pass, GT_FALSE otherwise
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPortForceLinkPassEnable
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *statePtr
);

/*******************************************************************************
* prvCpssDrvGprtSetSpeedAutoNegEn
*
* DESCRIPTION:
*       This routine enables/disables speed auto-negotiation
*
* INPUTS:
*       port   - the logical port number.
*       enable - GT_TRUE - enable auto-negotiation,
*                GT_FALSE - disable auto-negotiation
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetSpeedAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         enable
);

/*******************************************************************************
* prvCpssDrvGprtGetSpeedAutoNegEn
*
* DESCRIPTION:
*       Return speed auto-negotiation status
*
* INPUTS:
*       port   - the logical port number.
*
* OUTPUTS:
*       enablePtr  - GT_TRUE -  if auto-negotiation is enable,
*                    GT_FALSE - if auto-negotiation is disabled,
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetSpeedAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* prvCpssDrvGprtSetDuplexAutoNegEn
*
* DESCRIPTION:
*       This routine enables/disables duplex auto-negotiation
*
* INPUTS:
*       port   - the logical port number.
*       enable - GT_TRUE  - enable auto-negotiation,
*                GT_FALSE - disable auto-negotiation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetDuplexAutoNegEn
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_BOOL             enable
);

/*******************************************************************************
* prvCpssDrvGprtGetDuplexAutoNegEn
*
* DESCRIPTION:
*       Return duplex auto-negotiation status
*
* INPUTS:
*       port   - the logical port number.
*
* OUTPUTS:
*       enablePtr  - GT_TRUE -  if auto-negotiation is enable,
*                    GT_FALSE - if auto-negotiation is disabled,
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetDuplexAutoNegEn
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    OUT GT_BOOL             *enablePtr
);

/*******************************************************************************
* prvCpssDrvGprtSetSpeed
*
* DESCRIPTION:
*       This routine sets MAC speed. forcedSpd bit (speed auto-negotiation) is
*       not affected by this routine.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_CPSS_PORT_FORCED_SPEED_MODE (10, 100, 200, 1000, 2.5g, 10g)
*              CPSS_PORT_DO_NOT_FORCE_SPEED value is not supported.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetSpeed
(
    IN  GT_CPSS_QD_DEV                  *dev,
    IN  GT_CPSS_LPORT                   port,
    IN  GT_CPSS_PORT_FORCED_SPEED_MODE  mode
);

/*******************************************************************************
* prvCpssDrvGprtSetPortEType
*
* DESCRIPTION:
*        This routine sets the port's special Ether Type. This Ether Type is used
*        for Policy (see gprtSetPolicy API) and FrameMode (see prvCpssDrvGprtSetFrameMode API).
*
* INPUTS:
*        port  - the logical port number
*        etype - port's special ether type
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetPortEType
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_CPSS_ETYPE   etype
);

/*******************************************************************************
* prvCpssDrvGprtSetFrameMode
*
* DESCRIPTION:
*        Frame Mode is used to define the expected Ingress and the generated Egress
*        tagging frame format for this port as follows:
*            GT_CPSS_FRAME_MODE_NORMAL -
*                Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*                Untagged frames. Tagged frames use an Ether Type of 0x8100.
*            GT_CPSS_FRAME_MODE_DSA -
*                DSA mode uses a Marvell defined tagged frame format for
*                Chip-to-Chip and Chip-to-CPU connections.
*            GT_CPSS_FRAME_MODE_PROVIDER -
*                Provider mode uses user definable Ether Types per port
*                (see prvCpssDrvGprtSetPortEType/gprtGetPortEType API).
*            GT_CPSS_FRAME_MODE_ETHER_TYPE_DSA -
*                Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*                flowing a user definable Ether Type. This mode allows the mixture
*                of Normal Network frames with DSA Tagged frames and is useful to
*                be used on ports that connect to a CPU.
*
* INPUTS:
*        port - the logical port number
*        mode - GT_CPSS_FRAME_MODE type
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_BAD_PARAM - if mode is unknown
*        GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetFrameMode
(
    IN  GT_CPSS_QD_DEV      *dev,
    IN  GT_CPSS_LPORT       port,
    IN  GT_CPSS_FRAME_MODE  mode
);

/*******************************************************************************
* prvCpssDrvGprtGetPortMacStatus
*
* DESCRIPTION:
*       This routine retrives the port MAC info.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       macStatus - (pointer to) info about port MAC
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPortMacStatus
(
    IN  GT_CPSS_QD_DEV              *dev,
    IN  GT_CPSS_LPORT               port,
    OUT GT_CPSS_PORT_MAC_STATUS     *macStatus
);

/*******************************************************************************
* prvCpssDrvGprtSetFlowCtrlAutoNegEn
*
* DESCRIPTION:
*       This routine enables/disables flow control auto-negotiation
*
* INPUTS:
*       port   - the logical port number.
*       state  - GT_TRUE  - enable flow control auto-negotiation,
*                GT_FALSE - disable flow control auto-negotiation
*       pauseAdv -  advertise symmetric flow control support in
*                   Auto-Negotiation.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetFlowCtrlAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state,
    IN  GT_BOOL         pauseAdv
);

/*******************************************************************************
* prvCpssDrvGprtGetFlowCtrlAutoNegEn
*
* DESCRIPTION:
*       This routine get switch port flow control enable/disable status and return
*       flow control mode
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       statePtr    - GT_TRUE  - flow control auto-negotiation is enabled
*                     GT_FALSE - flow  control  auto-negotiation is disabled
*       pauseAdvPtr - if advertising of symmetric flow control in
*                     Auto-Negotiation is enabled.
*                     GT_TRUE  - enabled
*                     GT_FALSE - disabled
*
* RETURNS:
*       GT_OK  - on success
*       GT_FAIL  - on error
*       GT_BAD_PARAM - if invalid parameter is given
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetFlowCtrlAutoNegEn
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *statePtr,
    OUT GT_BOOL         *pauseAdvPtr
);

/*******************************************************************************
* prvCpssDrvGprtSetAllowedBad
*
* DESCRIPTION:
*        Allow receiving frames on this port with a bad FCS..
*
* INPUTS:
*        port - the logical port number.
*        allowed - GT_TRUE, frames receviced on this port with a CRC error are not
*        discarded.
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetAllowedBad
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         allowed
);

/*******************************************************************************
* prvCpssDrvGprtGetAllowedBad
*
* DESCRIPTION:
*        This routine gets Ignore FCS setup
*
* DESCRIPTION:
*        Allow receiving frames on this port with a bad FCS..
*
* INPUTS:
*        port - the logical port number.
*
* OUTPUTS:
*        allowed - GT_TRUE, frames receviced on this port with a CRC error are not
*        discarded.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetAllowedBad
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_BOOL         *allowed
);

/*******************************************************************************
* prvCpssDrvGsysSetDiscardExcessive
*
* DESCRIPTION:
*       Enable/Disable excessive collision packets drop.
*       In half duplex mode if a collision occurs the device tries to transmit
*       the packet again. If the number of collisions on the same packet is 16
*       and  excessive collision packets drop is enabled,
*       the packet is dropped.

*
* INPUTS:
*       enable - GT_TRUE  - if the number of collisions on the same packet
*                    is 16 the packet is dropped.
*                GT_FALSE - A collided packet will be retransmitted by device
*                    until it is transmitted without collisions,
*                    regardless of the number of collisions on the packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGsysSetDiscardExcessive
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_BOOL         enable
);

/*******************************************************************************
* prvCpssDrvGsysGetDiscardExcessive
*
* DESCRIPTION:
*       Gets status of excessive collision packets drop.
*       In half duplex mode if a collision occurs the device tries to transmit
*       the packet again. If the number of collisions on the same packet is 16
*       and  excessive collision packets drop is enabled,
*       the packet is dropped.

*
* INPUTS:
*       enablePtr - GT_TRUE - if the number of collisions on the same packet
*                       is 16 the packet is dropped.
*                   GT_FALSE - A collided packet will be retransmitted by device
*                       until it is transmitted without collisions,
*                       regardless of the number of collisions on the packet.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGsysGetDiscardExcessive
(
    IN  GT_CPSS_QD_DEV  *dev,
    OUT GT_BOOL         *enablePtr
);

/*******************************************************************************
* prvCpssDrvGprtSetPauseLimitOut
*
* DESCRIPTION:
*        Limit the number of continuous Pause refresh frames that can be transmitted
*        from this port. When full duplex Flow Control is enabled on this port,
*        these bits are used to limit the number of Pause refresh frames that can
*        be generated from this port to keep this port's link partner from sending
*        any data.
*        Setting this value to 0 will allow continuous Pause frame refreshes to
*        egress this port as long as this port remains congested.
*        Setting this value to 1 will allow 1 Pause frame to egress from this port
*        for each congestion situation.
*        Setting this value to 2 will allow 2 Pause frames to egress from this port
*        for each congestion situation, etc.
*
* INPUTS:
*        port - the logical port number
*        limit - the max number of Pause refresh frames for each congestion situation
*                ( 0 ~ 0xFF)
*
* OUTPUTS:
*        None.
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_BAD_PARAM - if limit > 0xFF
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetPauseLimitOut
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U8           limit
);

/*******************************************************************************
* prvCpssDrvGprtGetPauseLimitOut
*
* DESCRIPTION:
*        Limit the number of continuous Pause refresh frames that can be transmitted
*        from this port. When full duplex Flow Control is enabled on this port,
*        these bits are used to limit the number of Pause refresh frames that can
*        be generated from this port to keep this port's link partner from sending
*        any data.
*        Setting this value to 0 will allow continuous Pause frame refreshes to
*        egress this port as long as this port remains congested.
*        Setting this value to 1 will allow 1 Pause frame to egress from this port
*        for each congestion situation.
*        Setting this value to 2 will allow 2 Pause frames to egress from this port
*        for each congestion situation, etc.
*
* INPUTS:
*        port - the logical port number
*
* OUTPUTS:
*        limitPtr - the max number of Pause refresh frames for each congestion
*                   situation (0 ~ 0xFF)
*
* RETURNS:
*        GT_OK   - on success
*        GT_FAIL - on error
*        GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPauseLimitOut
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    OUT GT_U8           *limitPtr
);

/*******************************************************************************
* prvCpssDrvGqosSetDefFPri
*
* DESCRIPTION:
*       This routine sets the default frame priority (0 ~ 7).
*        This priority is used as the default frame priority (FPri) to use when
*        no other priority information is available.
*
* INPUTS:
*       port - the logical port number
*       pri  - default frame priority
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*        GT_BAD_PARAM - if pri > 7
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGqosSetDefFPri
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_U8           pri
);

/*******************************************************************************
* gprtSetPortEee
*
* DESCRIPTION:
*       Enable/disable Force EEE state on specified port on specified device and set relevant timers.
*
* INPUTS:
*       port    - the logical port number.
*       state   - When manual: GT_TRUE for force EEE enable, GT_FALSE otherwise
*       manual   - GT_TRUE for force EEE enable or disable, GT_FALSE to use AN result.
*       twLimit   - Wake timer in usec.
*       liLimit   - Assertion timer in usec.
*       tsLimit   - TxIdle timer in usec.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetPortEEE
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         state,
    IN  GT_BOOL         manual,
    IN  GT_U32          twLimit,
    IN  GT_U32          liLimit,
    IN  GT_U32          tsLimit
);

/*******************************************************************************
* gprtGetPortEee
*
* DESCRIPTION:
*       Get Enable/disable Force EEE state on specified port on specified
*       device and relevant timers.
*
* INPUTS:
*       port    - the logical port number.
*       state   - GT_TRUE for force link down, GT_FALSE otherwise
*
* OUTPUTS:
*       statePtr   - When manual: GT_TRUE for force EEE enable, GT_FALSE otherwise
*       manualPtr   - GT_TRUE for force EEE enable or disable, GT_FALSE to use AN result.
*       twLimitPtr   - Wake timer in usec.
*       liLimitPtr   - Assertion timer in usec.
*       tsLimitPtr   - TxIdle timer in usec.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetPortEEE
(
    IN  GT_CPSS_QD_DEV  *dev,
    IN  GT_CPSS_LPORT   port,
    IN  GT_BOOL         *statePtr,
    IN  GT_BOOL         *manualPtr,
    IN  GT_U32          *twLimitPtr,
    IN  GT_U32          *liLimitPtr,
    IN  GT_U32          *tsLimitPtr
);

/*
 * Definition:
 *      GT_CPSS_LED_LINK_ACT_SPEED   - off = no link, on = link, blink = activity,
 *                                     blink speed = link speed
 *      GT_CPSS_LED_LINK_ACT         - off = no link, on = link, blink = activity
 *      GT_CPSS_LED_LINK             - off = no link, on = link
 *      GT_CPSS_LED_10_LINK_ACT      - off = no link, on = 10, blink = activity
 *      GT_CPSS_LED_10_LINK          - off = no link, on = 10
 *      GT_CPSS_LED_100_LINK_ACT     - off = no link, on = 100 link, blink = activity
 *      GT_CPSS_LED_100_LINK         - off = no link, on = 100 link
 *      GT_CPSS_LED_1000_LINK_ACT    - off = no link, on = 1000 link, blink = activity
 *      GT_CPSS_LED_1000_LINK        - off = no link, on = 1000 link
 *      GT_CPSS_LED_10_100_LINK_ACT  - off = no link, on = 10 or 100 link, blink = activity
 *      GT_CPSS_LED_10_100_LINK      - off = no link, on = 10 or 100 link
 *      GT_CPSS_LED_10_1000_LINK_ACT - off = no link, on = 10 or 1000 link, blink = activity
 *      GT_CPSS_LED_10_1000_LINK     - off = no link, on = 10 or 1000 link
 *      GT_CPSS_LED_100_1000_LINK_ACT- off = no link, on = 100 or 1000 link, blink = activity
 *      GT_CPSS_LED_100_1000_LINK    - off = no link, on = 100 or 1000 link
 *      GT_CPSS_LED_SPECIAL          - special leds
 *      GT_CPSS_LED_DUPLEX_COL       - off = half duplex, on = full duplex, blink = collision
 *      GT_CPSS_LED_ACTIVITY         - off = no link, blink on = activity
 *      GT_CPSS_LED_PTP_ACT          - blink on = PTP activity
 *      GT_CPSS_LED_FORCE_BLINK      - force blink
 *      GT_CPSS_LED_FORCE_OFF        - force off
 *      GT_CPSS_LED_FORCE_ON         - force on
*/
#define GT_CPSS_LED_LINK_ACT_SPEED       1
#define GT_CPSS_LED_LINK_ACT             2
#define GT_CPSS_LED_LINK                 3
#define GT_CPSS_LED_10_LINK_ACT          4
#define GT_CPSS_LED_10_LINK              5
#define GT_CPSS_LED_100_LINK_ACT         6
#define GT_CPSS_LED_100_LINK             7
#define GT_CPSS_LED_1000_LINK_ACT        8
#define GT_CPSS_LED_1000_LINK            9
#define GT_CPSS_LED_10_100_LINK_ACT      10
#define GT_CPSS_LED_10_100_LINK          11
#define GT_CPSS_LED_10_1000_LINK_ACT     12
#define GT_CPSS_LED_10_1000_LINK         13
#define GT_CPSS_LED_100_1000_LINK_ACT    14
#define GT_CPSS_LED_100_1000_LINK        15
#define GT_CPSS_LED_SPECIAL              16
#define GT_CPSS_LED_DUPLEX_COL           17
#define GT_CPSS_LED_ACTIVITY             18
#define GT_CPSS_LED_PTP_ACT              19
#define GT_CPSS_LED_FORCE_BLINK          20
#define GT_CPSS_LED_FORCE_OFF            21
#define GT_CPSS_LED_FORCE_ON             22
#define GT_CPSS_LED_RESERVE              23


/*
 * typedef: enum GT_CPSS_LED_CFG
 *
 * Description: Enumeration for LED configuration type
 *
 * Enumerations:
 *        GT_CPSS_LED_CFG_GLOBAL_CONTROL  - read/write global control
 *        GT_CPSS_LED_CFG_LED0            - read/write led0 value (GT_CPSS_LED_xxx definition)
 *        GT_CPSS_LED_CFG_LED1            - read/write led1 value
 *        GT_CPSS_LED_CFG_LED2            - read/write led2 value
 *        GT_CPSS_LED_CFG_LED3            - read/write led3 value
 *        GT_CPSS_LED_CFG_SPECIAL_LED1    - read/write special led1 value
 *        GT_CPSS_LED_CFG_SPECIAL_LED2    - read/write special led2 value
 *        GT_CPSS_LED_CFG_SPECIAL_LED3    - read/write special led3 value
 *        GT_CPSS_LED_CFG_SPECIAL_LED4    - read/write special led4 value
 *        GT_CPSS_LED_CFG_PULSE_STRETCH   - read/write pulse stretch (0 ~ 4)
 *        GT_CPSS_LED_CFG_BLINK_RATE      - read/write blink rate    (0 ~ 5)
 *        GT_CPSS_LED_CFG_SPECIAL_CONTROL - read/write special control (port vector)
 */
typedef enum
{
    GT_CPSS_LED_CFG_GLOBAL_CONTROL,  /* valid for port 0 */
    GT_CPSS_LED_CFG_LED0,
    GT_CPSS_LED_CFG_LED1,
    GT_CPSS_LED_CFG_LED2,
    GT_CPSS_LED_CFG_LED3,
    GT_CPSS_LED_CFG_SPECIAL_LED1,    /* valid for port 0 */
    GT_CPSS_LED_CFG_SPECIAL_LED2,    /* valid for port 0 */
    GT_CPSS_LED_CFG_SPECIAL_LED3,    /* valid for port 0 */
    GT_CPSS_LED_CFG_SPECIAL_LED4,    /* valid for port 0 */
    GT_CPSS_LED_CFG_PULSE_STRETCH,
    GT_CPSS_LED_CFG_BLINK_RATE,
    GT_CPSS_LED_CFG_SPECIAL_CONTROL
} GT_CPSS_LED_CFG;

/*******************************************************************************
* prvCpssDrvGprtSetLED
*
* DESCRIPTION:
*       This API allows to configure 4 LED sections, Pulse stretch, Blink rate,
*       and special controls.
*
* INPUTS:
*       port    - the logical port number
*       cfg     - GT_CPSS_LED_CFG value
*       value   - value to be configured
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtSetLED
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_CPSS_LED_CFG  cfg,
    IN  GT_U32           value
);


/*******************************************************************************
* prvCpssDrvGprtGetLED
*
* DESCRIPTION:
*       This API allows to retrieve 4 LED sections, Pulse stretch, Blink rate,
*       and special controls.
*
* INPUTS:
*       port        - the logical port number
*       cfg         - GT_CPSS_LED_CFG value
*
* OUTPUTS:
*       value       - configured value
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS prvCpssDrvGprtGetLED
(
    IN  GT_CPSS_QD_DEV   *dev,
    IN  GT_CPSS_LPORT    port,
    IN  GT_CPSS_LED_CFG  cfg,
    OUT GT_U32           *value
);


#ifdef __cplusplus
}
#endif

#endif /* __prvCpssDrvGtPortCtrl_h */
