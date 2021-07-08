--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_bridge_interface.lua
--*
--* DESCRIPTION:
--*       clearing of all entries on a device
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--
-- CPSS DxCh Vlan functions
--
cmdLuaCLI_registerCfunction("wraplCpssDxChBrgFdbFlush")

local function clear_bridge_internal_cfg_get(dev)

    local result, values, cfg

    cfg = {};

    result, values = myGenWrapper(
            "cpssDxChBrgFdbStaticDelEnableGet", {
                { "IN", "GT_U8", "dev", dev },
                { "OUT", "CPSS_DXCH_BRG_FDB_DEL_MODE_ENT", "del_mode"}
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbStaticDelEnableGet call error");
        return false, nil;
    end
    cfg.del_mode = values.del_mode;

    result, values = myGenWrapper(
            "cpssDxChBrgFdbActionActiveDevGet", {
                { "IN", "GT_U8", "dev", dev },
                { "OUT", "GT_U32", "actDev"},
                { "OUT", "GT_U32", "actDevMask"}
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveDevGet call error");
        return false, nil;
    end
    cfg.actDev = values.actDev;
    cfg.actDevMask = values.actDevMask;

    result, values = myGenWrapper(
            "cpssDxChBrgFdbActionActiveVlanGet", {
                { "IN", "GT_U8", "dev", dev },
                { "OUT", "GT_U32", "vlanId" },
                { "OUT", "GT_U32", "vlanIdMask" }
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveVlanGet call error");
        return false, nil;
    end
    cfg.vlanId = values.vlanId;
    cfg.vlanIdMask = values.vlanIdMask;

    if is_device_eArch_enbled(dev) then
        result, values = myGenWrapper(
                "cpssDxChBrgFdbActionActiveUserDefinedGet", {
                    { "IN", "GT_U8", "dev", dev },
                    { "OUT", "GT_U32", "actUserDefined" },
                    { "OUT", "GT_U32", "actUserDefinedMask" }
                }
            )
        if result ~= 0 then
            print("cpssDxChBrgFdbActionActiveUserDefinedGet call error");
            return false, nil;
        end
        cfg.actUserDefined = values.actUserDefined;
        cfg.actUserDefinedMask = values.actUserDefinedMask;
    end

    result, values = myGenWrapper(
            "cpssDxChBrgFdbTrunkAgingModeGet", {
                { "IN", "GT_U8", "dev", dev },
                { "OUT", "GT_U32", "trunkAgingMode"}
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbTrunkAgingModeGet call error");
        return false, nil;
    end
    cfg.trunkAgingMode = values.trunkAgingMode;

    result, values = myGenWrapper(
            "cpssDxChBrgFdbActionActiveInterfaceGet", {
                { "IN", "GT_U8", "dev", dev },
                { "OUT", "GT_U32", "actIsTrunk" },
                { "OUT", "GT_U32", "actIsTrunkMask" },
                { "OUT", "GT_U32", "actTrunkPort" },
                { "OUT", "GT_U32", "actTrunkPortMask" }
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveInterfaceGet call error");
        return false, nil;
    end
    cfg.actIsTrunk = values.actIsTrunk;
    cfg.actIsTrunkMask = values.actIsTrunkMask;
    cfg.actTrunkPort = values.actTrunkPort;
    cfg.actTrunkPortMask = values.actTrunkPortMask;

    return true, cfg;
end

local function clear_bridge_internal_cfg_set(
    dev, del_mode,
    actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask,
    actDev, actDevMask, vlanId, vlanIdMask,
    actUserDefined, actUserDefinedMask, trunkAgingMode)

	local result, values;

    result, values = myGenWrapper(
            "cpssDxChBrgFdbStaticDelEnable", {
                { "IN", "GT_U8", "dev", dev },
                { "IN", "CPSS_DXCH_BRG_FDB_DEL_MODE_ENT", "del_mode", del_mode}
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbStaticDelEnable call error");
        return false;
    end

	result, values = myGenWrapper(
			"cpssDxChBrgFdbActionActiveDevSet", {
				{ "IN", "GT_U8", "dev", dev },
				{ "IN", "GT_U32", "actDev", actDev},
				{ "IN", "GT_U32", "actDevMask", actDevMask}
			}
		)
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveDevSet call error");
        return false;
    end

    result, values = myGenWrapper(
            "cpssDxChBrgFdbActionActiveVlanSet", {
                { "IN", "GT_U8", "dev", dev },
                { "IN", "GT_U32", "vlanId", vlanId},
                { "IN", "GT_U32", "vlanIdMask", vlanIdMask}
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveVlanSet call error");
        return false;
    end

    if is_device_eArch_enbled(dev) then
        result, values = myGenWrapper(
                "cpssDxChBrgFdbActionActiveUserDefinedSet", {
                    { "IN", "GT_U8", "dev", dev },
                    { "IN", "GT_U32", "actUserDefined", actUserDefined},
                    { "IN", "GT_U32", "actUserDefinedMask", actUserDefinedMask}
                }
            )
        if result ~= 0 then
            print("cpssDxChBrgFdbActionActiveUserDefinedSet call error");
            return false;
        end
    end

    result, values = myGenWrapper(
			"cpssDxChBrgFdbTrunkAgingModeSet", {
				{ "IN", "GT_U8", "dev", dev },
				{ "IN", "GT_U32", "trunkAgingMode", trunkAgingMode}
			}
		)
    if result ~= 0 then
        print("cpssDxChBrgFdbTrunkAgingModeSet call error");
        return false;
    end

    result, values = myGenWrapper(
            "cpssDxChBrgFdbActionActiveInterfaceSet", {
                { "IN", "GT_U8", "dev", dev },
                { "IN", "GT_U32", "actIsTrunk", actIsTrunk},
                { "IN", "GT_U32", "actIsTrunkMask", actIsTrunkMask},
                { "IN", "GT_U32", "actTrunkPort", actTrunkPort},
                { "IN", "GT_U32", "actTrunkPortMask", actTrunkPortMask},
            }
        )
    if result ~= 0 then
        print("cpssDxChBrgFdbActionActiveInterfaceSet call error");
        return false;
    end

	return true
end

local function clear_bridge_internal_util(
    dev, del_mode,
    actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask,
    actDev, actDevMask, vlanId, vlanIdMask,
    actUserDefined, actUserDefinedMask, trunkAgingMode)

	local result, values, status, cfg;
	local err
	status = true

    result, cfg = clear_bridge_internal_cfg_get(dev);
    if result ~= true then
        return false;
    end

    result, err = clear_bridge_internal_cfg_set(
        dev, del_mode,
        actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask,
        actDev, actDevMask, vlanId, vlanIdMask,
        actUserDefined, actUserDefinedMask, trunkAgingMode);
    if result ~= true then
        return false;
    end

    result, values = myGenWrapper(
			"cpssDxChBrgFdbTrigActionStart", {
				{ "IN", "GT_U8", "dev", dev },
				{ "IN", TYPE["ENUM"], "mode", 2} --CPSS_FDB_ACTION_DELETING_E
			}
		)
    if result ~= 0 then
        print("cpssDxChBrgFdbTrigActionStart call error");
        status = false
    end

    -- restore HW value
    result, err = clear_bridge_internal_cfg_set(
        dev, cfg.del_mode,
        cfg.actIsTrunk, cfg.actIsTrunkMask, cfg.actTrunkPort, cfg.actTrunkPortMask,
        cfg.actDev, cfg.actDevMask, cfg.vlanId, cfg.vlanIdMask,
        cfg.actUserDefined, cfg.actUserDefinedMask, cfg.trunkAgingMode);

	return status
end

-- ************************************************************************
---
--  clear_bridge_internal
--        @description  Does the actual job of clearing entries from
--                      the FDB
--
--        @param dev            - The device ID on which to work
--
--        @return       true, if successful otherwise the error code and
--                      error message
--
local function clear_bridge_internal(
    dev, del_mode,
    actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask)

    clear_bridge_internal_util(
        dev, del_mode,
        actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask,
        0 --[[actDev--]],
        0 --[[actDevMask--]],
        0 --[[vlanId--]],
        0 --[[vlanIdMask--]],
        0 --[[actUserDefined--]],
        0 --[[actUserDefinedMask--]],
        0 --[[trunkAgingMode--]]); --CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E
end

-- ************************************************************************
---
--  clear_bridge_interface
--        @description  Clears an entries from the FDB
--
--        @param cmdtype        - command type
--        @param params         - The hash table with the parameters
--
--        @return       true, if successful otherwise the error message
--
local function clear_bridge_interface(cmdtype, params)
    local status, dev
    local del_mode, actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask
    local call_wraplCpssDxChBrgFdbFlush = false-- indication to call wraplCpssDxChBrgFdbFlush
    local result

    dev = params["devID"]
    status = true

    del_mode=0
    if (params["static_dynamic_all"] == "static") then
        if (not is_device_eArch_enbled(dev)) then
            print("Clearing of static only MAC addresses is not supported on "..wrlCpssDeviceFamilyGet(devNum).. " device family")
            return
        else
            del_mode = 2
        end
    elseif (params["static_dynamic_all"] == "all") then
        del_mode = 1
    end

    actIsTrunk = 0x00
    actIsTrunkMask = 0x00
    actTrunkPort = 0x00
    actTrunkPortMask = 0x00
    --Select the interfaces to clear
    if params["interface"] == "ethernet" then
    		--Ethernet
    		actIsTrunk = 0x00
    		actIsTrunkMask = 0x00
    		actTrunkPort = 0 --This is the acutal trunk ID
    		actTrunkPortMask = 0x7F
        params.range = params.ethernet
   	elseif params["interface"] == "port-channel" then
    		--LAG
    		actIsTrunk = 0x01
    		actIsTrunkMask = 0x01
    		actTrunkPort = 0 --This is the acutal trunk ID
    		actTrunkPortMask = 0x7F
        params.range = params["port-channel"]
    else
        -- no indication about specific mask
        call_wraplCpssDxChBrgFdbFlush = true
  	end

    if (type(params["range"]) == "nil") then
        params["range"] = {[0] = {-1}}
    else
        call_wraplCpssDxChBrgFdbFlush = false
    end

    if params.dynamic_all == "all" then
        call_wraplCpssDxChBrgFdbFlush = true
    end

    if call_wraplCpssDxChBrgFdbFlush then
        if fdb_use_shadow == true then
            print("-- use FDB shadow -- \n")
        end 
        result = wraplCpssDxChBrgFdbFlush(dev, del_mode)
        if result ~= 0 then
            print("Error while flushing Fdb table.")
        end
    else
        for k, port in pairs(params["range"][dev]) do
            if (port ~= -1) then
                actTrunkPort = port
            end
            if fdb_use_shadow == true then
                print("WARNING : not use FDB shadow , although need to ! \n")
            end 
            status = clear_bridge_internal(dev, del_mode, actIsTrunk, actIsTrunkMask, actTrunkPort, actTrunkPortMask)
        end
    end

    return status
end

-- *exec*
-- clear bridge interface %devID [ ethernet %port-range | port-channel %port-range ] [vlan %vlan] [ all | dynamic ]
CLI_addHelp("exec", "clear", "Reset functions")
CLI_addHelp("exec", "clear bridge", "Clears the FDB table")
CLI_addCommand("exec", "clear bridge interface", {
    func=clear_bridge_interface,
    constFuncArgs={"interface"},
    help="Clears a certain interface from the FDB",
    params={
        { type="values", "%devID" },
        { type="named",
            "#interface",
            { format="vlan %vlan", help="Clear MACs only from a certain VLAN" },
            "#static_dynamic_all"
        }
    }
})
-- *exec*
-- clear bridge type %devID [ all | dynamic ]
CLI_addCommand("exec", "clear bridge type", {
    func=clear_bridge_interface,
    constFuncArgs={"type"},
    help="Clear all entries on a device",
    params={
        { type="values", "%devID" },
        { type="named",
            "#static_dynamic_all"
        }
    }
})
