--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* const.lua
--*
--* DESCRIPTION:
--*       General constants
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

-- Version info.
CLI_VERSION         = "1"
CLI_VERSION_DATE    = "27-Jul-2011"
CLI_VERSION_TIME    = "22:44:19"
CLI_BASELINE        = "LUA_CLI_01_283"

-- Bit constants
BIT_11  = 2048

-- Device families
LION_DEVFAMILY = 14

-- Trunk metrics.
-- NOTE: may be changed by function reloadTrunkIdDictionary()
MAXIMUM_TRUNK_ID = 127

-- (MRU) Received packet metrics
DEFAULT_RECOMENDED_MAXIMUM_VLAN_PACKET_SIZE         = 10240
DEFAULT_NORMAL_MAXIMUM_RECEIVED_PACKET_SIZE         = 1522 -- the default by appDemo
DEFAULT_JUMBO_FRAME_MAXIMUM_RECEIVED_PACKET_SIZE    = 9000

-- Mirror analyser interfaces
MIRROR_ANALYSER_RX_INTERFACE = 0
MIRROR_ANALYSER_TX_INTERFACE = 1

