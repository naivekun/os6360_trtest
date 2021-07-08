--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_nat_drop_counter.lua
--*
--* DESCRIPTION:  show nat drop counter
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
---
--  nat_drop_show
--        @description  show nat drop counter
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--
function nat_drop_show(params)

    local device=params["device"]

    ret,val = myGenWrapper("cpssDxChIpNatDroppedPacketsCntGet",{    --get counter
    {"IN","GT_U8","devNum",device},
    {"OUT","GT_U32","natDropPktsPtr"}
    })
    -- the register is ROC; no need to do reset
    if 0==ret then
        print("\nNAT:")
        print("Drop Counter: "..to_string(val.natDropPktsPtr))
        print() --seperation line
        return to_string(val.natDropPktsPtr)
    else
        -- NAT is only supported for Bobcat2 B0 and above 30="GT_NOT_APPLICABLE_DEVICE"
        print("cpssDxChIpNatDroppedPacketsCntGet() failed: "..to_string(ret))
        print() --seperation line
        return false
    end
end

--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show counters nat-drop", {
  func   = nat_drop_show,
  help   = 'Show the NAT drop counter',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})