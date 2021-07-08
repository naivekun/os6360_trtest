--********************************************************************************
--*              (c), Copyright 2016, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* eom.lua
--*
--* DESCRIPTION:
--*       The test for testing eom feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
SUPPORTED_SIP_DECLARE(devEnv.dev, "SIP_5_15")
NOT_SUPPORTED_DEV_DECLARE(devEnv.dev, "CPSS_PP_FAMILY_DXCH_BOBCAT3_E" , "CPSS_PP_FAMILY_DXCH_ALDRIN2_E")

--------------------------------------------------------------------------------
--load configuration
printLog("Load configuration ..")
executeLocalConfig("dxCh/examples/configurations/eom.txt")
