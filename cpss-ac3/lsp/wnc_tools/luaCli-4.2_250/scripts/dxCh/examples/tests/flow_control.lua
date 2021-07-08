--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* flow_contorl.lua
--*
--* DESCRIPTION:
--*       The test for testing flow-control feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------------------------------------------
-- this test is not relevant for LION devices 
NOT_SUPPORTED_DEV_DECLARE(devNum,"CPSS_PP_FAMILY_DXCH_LION_E")

--load configuration
printLog("Load configuration ..")
executeLocalConfig("dxCh/examples/configurations/flow_control.txt")

--restore configuration
printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/flow_control_deconfig.txt")


