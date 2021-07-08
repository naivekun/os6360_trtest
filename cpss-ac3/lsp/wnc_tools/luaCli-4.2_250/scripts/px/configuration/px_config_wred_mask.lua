--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_config_wred_mask.lua
--*
--* DESCRIPTION:
--*       File contain wred-mask command
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--*******************************************************************************
--  printWrtdMaskError
--
--* @description Print error message for setWrtdMask
--
--*******************************************************************************
local function printWrtdMaskError()
    print("Could not set WRED mask")
end



--*******************************************************************************
--  setWrtdMask
--
--  @description Changes WRED mask for tcDp limits
--
--  @param params -
--          params["queue"] - Traffic Class associated with this set of Drop
--                            Parameters.
--          params["dp"]    - Drop Profile PArameters to associate with the
--                            Traffic Class.
--
--  @return true on success, otherwise false
--
-- ******************************************************************************
local function setWrtdMask(params)
    local ret, val
    local maskLsb
    local devNum = tonumber(params["device"])

    ret, val = myGenWrapper("cpssPxPortTxTailDropWrtdMasksGet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT", "CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC",
              "maskLsbPtr" }
        })

    if ((ret ~= LGT_OK) or (val == nil)) then
        printWrtdMaskError()
        return false
    end

    maskLsb = val["maskLsbPtr"]

    maskLsb["tcDp"] = tonumber(params["tc_dp_mask"])

    ret, val = myGenWrapper("cpssPxPortTxTailDropWrtdMasksSet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC",
              "maskLsbPtr", maskLsb }
        })

    if (ret ~= LGT_OK) then
        printWrtdMaskError()
        return false
    end

    return true
end



--*******************************************************************************
-- command registration: wred-mask
--*******************************************************************************
CLI_addCommand("config", "wred-mask", {
    func = setWrtdMask,
    help = "Change WRED mask for specific TC/DP",
    params = {
        {
            type = "named",
            {
                format = "device %devNum",
                name = "device",
                help = "The device ID"
            },
            {
                format = "tc-dp-mask %tail_drop_tc_dp_mask",
                name = "tc-dp-mask",
                help = "The number of LSBs masked for TC/DP limits"
            },
            requirements={
                ["tc-dp-mask"] = { "device" }
            },
            mandatory = { "tc-dp-mask" }
        }
    }
})
