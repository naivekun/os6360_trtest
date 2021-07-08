--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan.lua
--*
--* DESCRIPTION:
--*       creating/removing of a vlan
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


--Register C functions with the CLI
cmdLuaCLI_registerCfunction("wrlCpssDxChVlanEntryInvalidate")


-- ************************************************************************
---
--  vlan
--        @description  Create a vlan on the device 
--
--        @param params         - the parameters
--
--        @return       true if successfule otherwise false
--
local function vlan(params)
    local key, value 
    local dev, rc, vlaninfo
    
    --Since it is in the vlan database context, we get the deviceID from the global pool
    dev = getGlobal("devID")
    vlaninfo = {}
    vlaninfo.portsMembers = {}
    --Create the vlan, erase all member ports if there are any
    for key, value in pairs(params["vlanRange"]) do
        rc = vlan_info_set(dev, value, vlaninfo)
        if rc ~= 0 then
            print("error " .. rc .. " writing vlan entry " .. value .. " device " .. dev)
            return false
        end
    end
    return true
end


-- ************************************************************************
---
--  no_vlan
--        @description  Removes a vlan from the device
--
--        @param params         - params the parameters
--
--        @return       true if successfule otherwise false
--
local function no_vlan(params)
    local key, value 
    local dev, rc
    
    --Since it is in the vlan database context, we get the deviceID from the global pool
    dev = getGlobal("devID")
    --Remove the vlan(s)
    for key, value in pairs(params["vlanRange"]) do
        rc = wrLogWrapper("wrlCpssDxChVlanEntryInvalidate", "(dev, value)", dev, value)
        if rc ~= 0 then
            return false, "error " .. rc .. " writing vlan entry " .. value .. " device " .. dev
        end
    end
    
    return true
end

-- *vlan_configuration*
-- vlan %vlanRange
CLI_addCommand("vlan_configuration", "vlan", {
    func = vlan,
    help = "Enter vlan database mode",
    params = {{ type="values", "%vlanRange" }}
})
-- *vlan_configuration*
-- no vlan %vlanRange
CLI_addCommand("vlan_configuration", "no vlan", {
    func = no_vlan,
    help = "Enter vlan database mode",
    params = {{ type="values", "%vlanRange" }}
})
