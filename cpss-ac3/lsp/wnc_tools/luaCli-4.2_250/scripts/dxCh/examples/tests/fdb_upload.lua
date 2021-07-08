--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fdb_upload.lua
--*
--* DESCRIPTION:
--*       stress test by FDB upload
--*       
--*       purpose of example:
--*       1. Run test to load simulation with FDB upload in parallel task, 
--*          and limited traffic on port
--*       2. Wait for task completion
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- this test is relevant to ALL tested devices
--##################################
--##################################
SUPPORTED_FEATURE_DECLARE(devNum, "NOT_BC2_GM")

-- function to implement the test
local function doTest()
    new_instance_shell_cmd_check("mainUtFdbUploadStress")
end

local configFileName = "fdb_upload"

-- run pre-test config
local function preTestConfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName)) 
end

-- run pre-test config
preTestConfig()
-- run the test
doTest()

