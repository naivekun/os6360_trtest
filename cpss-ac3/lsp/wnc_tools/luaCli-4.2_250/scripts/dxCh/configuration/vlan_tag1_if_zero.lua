--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_tag1_if_zero.lua
--*
--* DESCRIPTION:
--*       configure Tag1 removal mode from the egress port if VID is 0.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  vlanTag1IfZero
--        @description  Set Tag1 removal mode from the egress port Tag State if Tag1 VID
--					    is assigned a value of 0.
--
--        @param params             - params["devID"]: device number
--									  params["mode"]: Vlan Tag1 Removal mode when Tag1 VID=0
--
--        @return       true on success, otherwise false
--

local function vlanTag1IfZero(params)
	
	local devNum = params["devID"]
    local mode = params["vlanRemoveTag1"]
	local CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E = 0
	if params["flagNo"] ~= nil then
		mode = CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E
	end
	local ret = myGenWrapper("cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet",{	
						{"IN","GT_U8","devNum",devNum},
						{"IN","CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT","mode",mode}
						})

	if 0~=ret then
		print("can't set Tag1 removal mode: " .. returnCodes[ret])
		return false
	end
	return true
end


-------------------------------------------------------
-- type registration: vlanRemoveTag1Earch
-------------------------------------------------------

CLI_type_dict["vlanRemoveTag1Earch"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Removes tag1 if it assigned the value 0\n",
    enum = {
        ["always"] = { value="2", help="regardless of ingress tagging" },
        ["noTag1"] = { value="3", help="and the frame at ingress was not tagged1" }
   }
}

-------------------------------------------------------
-- type registration: vlanRemoveTag1NotEarch
-------------------------------------------------------

CLI_type_dict["vlanRemoveTag1NotEarch"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Removes tag1 if it assigned the value 0\n",
    enum = {
        ["notDoubleTag"] = { value="1", help="and the frame at ingress was not double tagged" },
        ["always"] = { value="2", help="regardless of ingress tagging" }
   }
}
CLI_addHelp("config", "vlan", "Removes tag1 if it assigned the value 0")
CLI_addHelp("config", "vlan remove", "Removes tag1 if it assigned the value 0")
if (is_sip_5(nil)) then
		--------------------------------------------
		-- command registration: vlan tag1_if_zero. eArch devices
		--------------------------------------------
		CLI_addCommand("config", "vlan remove tag1", {
			func=vlanTag1IfZero,
			help="Removes tag1 if it assigned the value 0.",
			params={
				{   type="values",
					"%devID",
					{ format="%vlanRemoveTag1Earch", name="vlanRemoveTag1" }
				}
			}
		})
else
		--------------------------------------------
		-- command registration: vlan tag1_if_zero. non-eArch devices
		--------------------------------------------
		CLI_addCommand("config", "vlan remove tag1", {
			func=vlanTag1IfZero,
			help="Removes tag1 if it assigned the value 0.",
			params={
				{   type="values",
					"%devID",
					{ format="%vlanRemoveTag1NotEarch", name="vlanRemoveTag1" }
				}
			}
		})
end

--------------------------------------------
-- command registration: no vlan tag1_if_zero
--------------------------------------------
CLI_addHelp("config", "no vlan", "doesn't remove tag1 if it is assigned the value 0")
CLI_addHelp("config", "no vlan remove", "doesn't remove tag1 if it is assigned the value 0")
CLI_addCommand("config", "no vlan remove tag1", {
	func=function(params) 
			params["flagNo"] = true
			return vlanTag1IfZero(params)
		end,
	help="doesn't remove tag1 if it is assigned the value 0.",
	params={
		{   type="values",
			"%devID"
		}
	}
})