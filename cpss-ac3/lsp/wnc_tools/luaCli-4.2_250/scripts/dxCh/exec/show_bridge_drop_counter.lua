--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_drop_counter.lua
--*
--* DESCRIPTION:  show bridge drop counter
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
---
--  bridge_drop_show
--        @description  show bridge drop counter
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--
local function bridge_drop_show(params)
    --print(to_string(params))
    local device=params["device"]
    local ret, val

    ret,val = myGenWrapper("cpssDxChBrgCntDropCntrGet",{    --get counter
    {"IN","GT_U8","devNum",device},
    {"OUT","GT_U32","dropCntPtr"}
    })

    if 0==ret then
        print() --seperation line
        print("Bridge Ingress Drop Counter: "..val["dropCntPtr"])
        --reset counter
        ret = myGenWrapper("cpssDxChBrgCntDropCntrSet",{    --set counter
                {"IN","GT_U8","devNum",device},
                {"IN","GT_U32","dropCnt",0}
                })
        if 0~=ret then
        --couldn't reset
            print("Error at reseting bridge drop counter: "..returnCodes[ret].."\n")
        end
    else
        print("Error at getting bridge drop counter: "..returnCodes[ret].."\n")
    end
    print() --seperation line
end


--********************************************************************************

--includes

--constants
---
--  bridge_drop_cause
--        @description  show the cause of the bridge-drop
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--


local function bridge_drop_cause(params)
    local device=params["device"]
    local ret, val

    ret = myGenWrapper("cpssDxChBrgCntDropCntrModeSet",{  --set mode
        {"IN","GT_U8","devNum",device},
        {"IN","CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT","dropMode",
        "CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E"}
        })


    ret = myGenWrapper("cpssDxChBrgCntDropCntrSet",{    --set counter
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U32","dropCnt",0}
        })

    delay(25)   --delay system to wait for packets

    ret,val = myGenWrapper("cpssDxChBrgCntDropCntrGet",{    --get counter
        {"IN","GT_U8","devNum",device},
        {"OUT","GT_U32","dropCntPtr"}
        })

    if 0==val["dropCntPtr"] then
        print("No Bridge Ingress Drop")
    else
        print("Bridge Ingress Drop Found")
        print("Detected causes:")
		for modeStr, mode in pairs(CLI_type_dict.bridge_drop_counter_mode.enum) do
			if(modeStr ~= "COUNT_ALL") then
                ret = myGenWrapper("cpssDxChBrgCntDropCntrModeSet",{  --set mode
                    {"IN","GT_U8","devNum",device},
                    {"IN","CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT","dropMode",mode["value"]}
                    })

                ret = myGenWrapper("cpssDxChBrgCntDropCntrSet",{    --set counter
                    {"IN","GT_U8","devNum",device},
                    {"IN","GT_U32","dropCnt",0}
                    })

                delay(25)   --delay system to wait for packets

                ret,val = myGenWrapper("cpssDxChBrgCntDropCntrGet",{    --get counter
                    {"IN","GT_U8","devNum",device},
                    {"OUT","GT_U32","dropCntPtr"}
                    })

                if 0~= val["dropCntPtr"] then   --if packets dropped
                    print() --seperation line
                    print(mode["help"] .. " has dropped " .. val["dropCntPtr"]
                    .. " packets")
                end
            end
         end

    --reset counter
    ret = myGenWrapper("cpssDxChBrgCntDropCntrSet",{    --set counter
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U32","dropCnt",0}
        })
    print() --seperation line
    end
end




--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show counters bridge-drop", {
  func   = bridge_drop_show,
  help   = 'Show the counter configured by "counters bridge-drop"',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})

CLI_addCommand("exec", "show counters bridge-drop-cause", {
  func   = bridge_drop_cause,
  help   = 'Show the cause of the bridge-drop',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})


