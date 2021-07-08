--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_fdb_hash_mode.lua
--*
--* DESCRIPTION:
--*       set of FDB Hash mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  bridge_fdb_hash_mode
--        @description  Set FDB Hash mode of the device
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function bridge_fdb_hash_mode(params)
    local status, err
    local result, values
    local dev, mode

    status = true;
    dev = params["devID"]
    mode = params["mac_hash_func"]
    if (mode == "nil") then
        mode = 0; -- CRC mode default
    end

    result, values = cpssPerDeviceParamSet(
            "cpssDxChBrgFdbHashModeSet",
            dev, mode, "CPSS_MAC_HASH_FUNC_MODE_ENT")
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


--------------------------------------------
-- type registration: mac_hash_func
--------------------------------------------
CLI_type_dict["mac_hash_func"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "",
    enum = {
        ["xor"] = { value=0, -- "CPSS_MAC_HASH_FUNC_XOR_E"
            help="XOR based hash function mode" },
        ["crc"] = { value=1, -- "CPSS_MAC_HASH_FUNC_CRC_E"
            help="CRC based hash function mode" },
        ["crc-multi"] = { value=2, -- "CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E"
            help="Multiple CRC hash functions per FDB bank" },
    }
}

--------------------------------------------
-- command registration: bridge aging-time
--------------------------------------------
CLI_addCommand("config", "bridge fdb-hash-mode", {
    func=bridge_fdb_hash_mode,
    help="Set FDB Hash mode of the device",
    params={
        {   type="values", "%mac_hash_func" },
        {   type="named",
            { format="device %devID", name="devID", help = "Device ID"},
            mandatory={"devID"}
        }
    }
})
