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
* @file tgfTrafficEngine.h
*
* @brief -- packet generator , parser.
* -- with trace capabilities
* -- each packet can be built from :
* -- L2 - mac da,sa
* -- vlan tag (optional) -- single/double tag(Q-in-Q)
* -- Ethernet encapsulation (optional) -- LLC/SNAP/etherII/Novell
* -- ether type
* -- L3 (optional) - ipv4/6 / mpls / arp / ..
* -- L4 (optional) - tcp / udp / icmp / IGMPv2,3,6 / MLD / RIPv1,2 ..
* -- payload
* -- CRC32
* -- support tunneling (tunnel + passenger)
* -- each passenger is like a new packet
* -- each packet has - total length
* -- other specific info:
* -- table index , num packets , waitTime
* -- Tx parameters (TGF_DSA_TX_PARAMS_STC):
* - DSA tag info , txSynch , invokeTxBufferQueueEvent , txQueue ...
* -- for Rx traffic : CPU code (for 'to_cpu')
* -- the engine need CB function to get the CPU code of this frame.
*
* -- special CPU codes:
* list of CPU codes that can be attached to CB for advanced purposes:
* distinguish between the 'rx in cpu'(any CPU code) and the 'Tx port capture' (tx_analyzer cpu code)
*
*
* -- basic init test capabilities:
* -- set port in loopback mode
* -- set port with force link up
* -- emulate SMB port counters:
* -- read/clear port counter : all/rx/tx all/packets/bytes
* -- 'capture' - set port as 'Egress mirror port' / use STC Sampling to cpu
* - set the CPU as the 'Tx analyzer port'
* - CPU will get those frames with specific CPU code.
*
* -- emulate SMB send to port:
* -- fill table of frames to 'Ingress the device' from the port
* (port should be in: loopback mode and in link up)
* -- send all / specific table index
* -- done in specific task.
* -- this also allow use to break the sending if we want from terminal
* -- set FLAG 'Break Sending' from terminal so task will stop sending
*
*
*
* -- support for multi devices types (Dx/ExMxPm/ExMx):
* -- the engine of build/parse should not care about it.
* -- the send/receive of packet (a 'table')in CPU will call CB with specific
* Tx parameters (TGF_DSA_TX_PARAMS_STC) /
* Rx parameter (TGF_DSA_RX_PARAMS_STC)
* and attach those parameter to the frame -->
* each entry in table point to ' cookie + CB' that hold the specific Rx/Tx info
*
*
* -- SMP port emulation:
* -- read/clear counters - attach CB for read counters (per device , per port)
* -- 'capture' - attach CB for set CPU as analyzer , CB for set port as tx mirrored port
* CB for set Tx port with Sampling to CPU
*
*
*
*
* @note 'test' can distinguish between : rx and ' capture of tx' by the
* mechanism of : 'Special CPU codes'
*
* -SMB trigger emulation - TBD
* -Expected results - TBD
* -Predefined traffic flows - TBD
*
*
* @version   33
********************************************************************************
*/

#include <extUtils/trafficEngine/tgfTrafficEngine.h>

