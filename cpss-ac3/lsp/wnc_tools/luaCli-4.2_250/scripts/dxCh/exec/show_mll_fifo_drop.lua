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


local function fifo_show(params)
    local device=params["device"]
    local portgroups
    local clearOnRead=false
    local ret, val
    
    if nil==params["portgroup"] or "all"==params["portgroup"] then
        portgroups=0xFFFFFFFF
    else
        portgroups=bit_shl(1, params["portgroup"])
    end

    if nil~=params["clear_on_read"] then
        clearOnRead=true
    end

    ret,val = myGenWrapper("cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet",{	--get counter
    {"IN","GT_U8","devNum",device},
    {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",portgroups},
    {"OUT","GT_U32","dropPktsPtr"}
    })

    if 0==ret then
        print()	--seperation line
        print("Multi target queue full drop packet counter: "..val["dropPktsPtr"])
        if clearOnRead then
            --reset counter
            ret = myGenWrapper("cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet",{	--set counter
                    {"IN","GT_U8","devNum",device},
                    {"IN","GT_PORT_GROUPS_BMP","portGroupsBmp",portgroups},
                    {"IN","GT_U32","dropPkts",0}
                    })

            if 0~=ret then
            --couldn't reset
                print("Error at clearing multi target queue full drop packet counter: "..returnCodes[ret].."\n")
            end
        end
    else
        print("Error at getting multi target queue full drop packet counter: "..returnCodes[ret].."\n")
    end
    print()	--seperation line
end

--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show counters mll-fifo-drop ", {
  func   = fifo_show,
  help   = 'The number of packets dropped due to MLL FIFO full',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          {format="portgroup %portGroup", name="portgroup", help="Port group"},
                           {format="clear-on-read", name="clear_on_read", help="Clear the counter after reading"},
                          mandatory = {"device"}}
  }
})