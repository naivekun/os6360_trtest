--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* system_capabilities.lua
--*
--* DESCRIPTION:
--*       common system, device and port functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 23 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssPortsDevConvertDataBack")
cmdLuaCLI_registerCfunction("wrlCpssDxChVlanList")
cmdLuaCLI_registerCfunction("wrlCpssDxChTrunkDevicesAndPortsList")
cmdLuaCLI_registerCfunction("wrlCpssDxChTrunkTableEntrySet")
cmdLuaCLI_registerCfunction("wrlCpssClearBmpPort")
cmdLuaCLI_registerCfunction("wrlCpssIsPortSetInPortsBmp")
cmdLuaCLI_registerCfunction("wrlCpssSetBmpPort")
cmdLuaCLI_registerCfunction("wrlCpssDxChIsPortTaggingCmdSupported")
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChVlanIdGet")
cmdLuaCLI_registerCfunction("wrlIsDevIdValid")
cmdLuaCLI_registerCfunction("wrlCpssDeviceCountGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumMirrorAnalyzerIndex")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextMirroredPortGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextRxMirroredPortGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextTxMirroredPortGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChRouterArpEntryIndexGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextInvalidRouterArpEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpUcRouteEntryWrite")
cmdLuaCLI_registerCfunction("wrlDxChPortIsValidEport")
cmdLuaCLI_registerCfunction("wrlCpssSipVersionCheck")
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")
cmdLuaCLI_registerCfunction("wrlCpssIsGmUsed")
cmdLuaCLI_registerCfunction("wrlCpssIsEmulator")
cmdLuaCLI_registerCfunction("wrlIsVplsModeEnabled")
cmdLuaCLI_registerCfunction("luaCLI_getDevInfo")
cmdLuaCLI_registerCfunction("luaCLI_DevInfo_portType_Define")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbMacEntrySet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbMacEntryDelete")
cmdLuaCLI_registerCfunction("wrlIsTmEnabled")

--[[
    NumberOfEntriesIn_VLAN_table,
    NumberOfEntriesIn_FDB_table,
    NumberOfEntriesIn_PCL_ACTION_table,
    NumberOfEntriesIn_PCL_TCAM_table,
    NumberOfEntriesIn_ROUTER_NEXT_HOP_table,
    NumberOfEntriesIn_ROUTER_LTT_table, --lookup translation
    NumberOfEntriesIn_ROUTER_TCAM_table,
    NumberOfEntriesIn_MLL_PAIR_table,
    NumberOfEntriesIn_POLICER_METERS_table,
    NumberOfEntriesIn_POLICER_BILLING_COUNTERS_table,
    NumberOfEntriesIn_VIDX_table,
    NumberOfEntriesIn_ARP_TUNNEL_START_table,
    NumberOfEntriesIn_STG_table, --spaning tree
    NumberOfEntriesIn_QOS_PROFILE_table,
    NumberOfEntriesIn_MAC_TO_ME_table,
    NumberOfEntriesIn_CNC_table, centalized counters
    NumberOfEntriesIn_CNC_BLOCKS_table,
    NumberOfEntriesIn_TRUNK_table

    HW_device_number

    currently Hard Coded:
    NumberOf_PORTS_IN_TRUNK
    NumberOf_PORTS_IN_DEVICE
    NumberOf_PRIO

]]--



-- global variable that should not be overriden
system_print = print;

local isSimulation = wrlCpssIsAsicSimulation()
local isGm = wrlCpssIsGmUsed()


-- get indication if GM used
function isGmUsed()
    return isGm
end

-- get indication if simulation used
function isSimulationUsed()
    return isSimulation
end

-- get indication if EMULATOR used
function isEmulatorUsed()
    -- must be called only after the LUA loaded , because this is
    -- NOT 'compilation' time flag
    -- so must call the LUA wrapper
    local isEmulator = wrlCpssIsEmulator()

    return isEmulator
end

local representative_devNum = 0

-- check if device support the sip version
function isSipVersion(devNum , version)
    -- all devices support this case ... but CPSS not really aware of it.(only about sip5..)
    if version == "SIP_LEGACY" then return true end

    if devNum == nil then devNum = representative_devNum end

    local full_version_string = "CPSS_GEN_" .. version .. "_E"
    local isSip = wrlCpssSipVersionCheck(devNum , full_version_string) -- can be nil
    if(isSip == true) then
        return true
    else
        return false
    end
end

-- ************************************************************************
---
--  is_sip_5
--        @description  Returns true if device is sip_5
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5(devNum)
    return isSipVersion(devNum , "SIP_5")
end

-- *****************************************************************************
---
--  waitFdbAuMsgProcessed
--        @description  wait until a PP finish an FDB Address Update (AU)
--                      message processing. A maximum number of tries to read
--                      the status is maxTriesCount. Some delay will be added
--                      in the simulation (WM or GM) before reading the status.
--
--        @param devNum - device number
--
--        @return true                 - if AU message processing finished
--                false, error message - if failed or time out
--
local function  waitFdbAuMsgProcessed(devNum)
   local sleepTime         = 0 -- miliseconds to sleep
   local isCompleted       = false
   local isSucceded        = false

   -- the GM not switch tasks for 'active' memories. all done in the same
   -- contexts. So if operation not ended ... it will never end !
   local delay0TriesCount       = isGm and 0 or 100
   local maxTriesCount          = isGm and 2 or 10 * 1000

   repeat
      -- if Simulation then make some delays before check
      if isSimulation then --
         if delay0TriesCount > 0 then
            delay0TriesCount = delay0TriesCount - 1
         else
            sleepTime = 1
         end
         delay(sleepTime)
      end

      if maxTriesCount == 0 then
         return false, "waitFdbAuMsgProcessed() timed out"
      else
         maxTriesCount = maxTriesCount - 1
      end

      local rc, values = myGenWrapper(
         "cpssDxChBrgFdbFromCpuAuMsgStatusGet",
         {
            {"IN",    "GT_U8",   "devNum", devNum},
            {"OUT",   "GT_BOOL", "completedPtr",},
            {"OUT",   "GT_BOOL", "succeededPtr",},
            {"INOUT", "GT_U32",  "entryOffsetPtr", nil}
         }
      )
      if rc ~= 0 then
         return false, returnCodes[rc]
      end
      isCompleted = values.completedPtr
   until isCompleted
   return true
end

function is_device_xCat_or_higher(devFamily)
    if ("CPSS_PP_FAMILY_DXCH_XCAT_E"  == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     or
       is_sip_5(nil)
       then
            return true
       end
       return false
end

function is_device_eArch_enbled(devNum)
    return is_sip_5(devNum)
end

function is_device_has_mapping(devNum)
    return is_sip_5(devNum)
end
--[[
local CPSS_DXCH_CFG_TABLE_VLAN_E                      =  0
local CPSS_DXCH_CFG_TABLE_FDB_E                       =  1
local CPSS_DXCH_CFG_TABLE_PCL_ACTION_E                =  2
local CPSS_DXCH_CFG_TABLE_PCL_TCAM_E                  =  3
local CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E           =  4
local CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E                =  5
local CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E               =  6
local CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E           =  7
local CPSS_DXCH_CFG_TABLE_TTI_TCAM_E                  =  8
local CPSS_DXCH_CFG_TABLE_MLL_PAIR_E                  =  9
local CPSS_DXCH_CFG_TABLE_POLICER_METERS_E            =  0
local CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E  = 11
local CPSS_DXCH_CFG_TABLE_VIDX_E                      = 12
local CPSS_DXCH_CFG_TABLE_ARP_E                       = 13
local CPSS_DXCH_CFG_TABLE_TUNNEL_START_E              = 14
local CPSS_DXCH_CFG_TABLE_STG_E                       = 15
local CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E               = 16
local CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E                 = 17
local CPSS_DXCH_CFG_TABLE_CNC_E                       = 18
local CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E                 = 19
local CPSS_DXCH_CFG_TABLE_TRUNK_E                     = 20
local CPSS_DXCH_CFG_TABLE_LPM_RAM_E                   = 21
local CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E               = 22
local CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E                = 23
local CPSS_DXCH_CFG_TABLE_EPORT_E                     = 24
local CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E             = 25
--]]
local CPSS_DXCH_CFG_TABLE_VLAN_E                      = "CPSS_DXCH_CFG_TABLE_VLAN_E"
local CPSS_DXCH_CFG_TABLE_FDB_E                       = "CPSS_DXCH_CFG_TABLE_FDB_E"
local CPSS_DXCH_CFG_TABLE_PCL_ACTION_E                = "CPSS_DXCH_CFG_TABLE_PCL_ACTION_E"
local CPSS_DXCH_CFG_TABLE_PCL_TCAM_E                  = "CPSS_DXCH_CFG_TABLE_PCL_TCAM_E"
local CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E           = "CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E"
local CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E                = "CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E"
local CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E               = "CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E"
local CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E           = "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E"
local CPSS_DXCH_CFG_TABLE_TTI_TCAM_E                  = "CPSS_DXCH_CFG_TABLE_TTI_TCAM_E"
local CPSS_DXCH_CFG_TABLE_MLL_PAIR_E                  = "CPSS_DXCH_CFG_TABLE_MLL_PAIR_E"
local CPSS_DXCH_CFG_TABLE_POLICER_METERS_E            = "CPSS_DXCH_CFG_TABLE_POLICER_METERS_E"
local CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E  = "CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E"
local CPSS_DXCH_CFG_TABLE_VIDX_E                      = "CPSS_DXCH_CFG_TABLE_VIDX_E"
local CPSS_DXCH_CFG_TABLE_ARP_E                       = "CPSS_DXCH_CFG_TABLE_ARP_E"
local CPSS_DXCH_CFG_TABLE_TUNNEL_START_E              = "CPSS_DXCH_CFG_TABLE_TUNNEL_START_E"
local CPSS_DXCH_CFG_TABLE_STG_E                       = "CPSS_DXCH_CFG_TABLE_STG_E"
local CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E               = "CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E"
local CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E                 = "CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E"
local CPSS_DXCH_CFG_TABLE_CNC_E                       = "CPSS_DXCH_CFG_TABLE_CNC_E"
local CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E                 = "CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E"
local CPSS_DXCH_CFG_TABLE_TRUNK_E                     = "CPSS_DXCH_CFG_TABLE_TRUNK_E"
local CPSS_DXCH_CFG_TABLE_LPM_RAM_E                   = "CPSS_DXCH_CFG_TABLE_LPM_RAM_E"
local CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E               = "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E"
local CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E                = "CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E"
local CPSS_DXCH_CFG_TABLE_EPORT_E                     = "CPSS_DXCH_CFG_TABLE_EPORT_E"
local CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E             = "CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E"
local CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E             = "CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E"

-- return the MAX number of trunks that the initialization requested
local function get_max_trunks(devNum)
    local result, values = myGenWrapper(
    "cpssDxChTrunkDbInitInfoGet", {
        { "IN", "GT_U8" , "dev", devNum}, -- devNum
        { "OUT","GT_U32", "maxNumberOfTrunks"},
        { "OUT", "CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT",  "trunkMembersMode"}
    })
    if result == 0 then
        return values.maxNumberOfTrunks
    end

    return nil
end

-- ************************************************************************
---
--  system_capability_managment
--        @description  This function updates global entries regarding the
--                      system capabilities
--
--        @param params         - A hash table containing the device number
--                                for which to set the global capabilities
--
--        @return       true on success, otherwise false
--
function system_capability_managment(params)
    local status, err
    local result, values
    ExisitgPortsBitmap = {}

    --Initialize the status
    status = true
    err = true
    -- assuming that system is symetric
    -- in "symmetric" system device "0" represents entire sytem capabilities
    if (type(params) == "nil") then
        devNum = 0
    elseif (type(params.devID) == "nil") then
        devNum = 0
    else
        devNum = params.devID
    end

    HW_device_number = 0
    result, values = myGenWrapper(
                        "cpssDxChCfgHwDevNumGet", {
                            { "IN", "GT_U8" , "dev", devNum}, -- devNum
                            { "OUT","GT_U8" , "hwDevNumPtr"}
                        }
    )
    HW_device_number=values.hwDevNumPtr
    status = status and (result == 0)
    err = ((result == 0) and err) or returnCodes[result]


    -- there is no cpss info yet for this parametr, need to add
    NumberOf_PORTS_IN_TRUNK = 8

    -- there is no cpss info yet for this parametr, need to add
    -- need to add probably port_bitmaps to have accurate mapping
    NumberOf_PRIO = 8

    local function entry(name, index)
        _G["NumberOfEntriesIn_"..name.."_table"] = 0
        local result, values = myGenWrapper(
                        "cpssDxChCfgTableNumEntriesGet", {
                            { "IN", "GT_U8" , "dev", devNum}, -- devNum
                            { "IN", "CPSS_DXCH_CFG_TABLES_ENT",  "table",index},  -- CPSS_DXCH_CFG_TABLE_TRUNK_E
                            { "OUT","GT_U32", "numEntries"}
                        })

        if result == 0 and values.numEntries ~= nil then
            if(is_sip_5(devNum) and name == "TRUNK") then
                -- we need to get the 'initialization' value and not the 'max' that the device supports
                _G["NumberOfEntriesIn_TRUNK_table"] = get_max_trunks(devNum)

            else
                _G["NumberOfEntriesIn_"..name.."_table"] = values.numEntries
            end
        end
        status = status and (result == 0)
        err = ((result == 0) and err) or returnCodes[result]
        --print(string.format("%s %q", "system_capability_managment debug-1 "..name, _G["NumberOfEntriesIn_"..name.."_table"]))
    end

    entry("VLAN",                             CPSS_DXCH_CFG_TABLE_VLAN_E                     )
    entry("FDB",                              CPSS_DXCH_CFG_TABLE_FDB_E                      )
    entry("PCL_ACTION",                       CPSS_DXCH_CFG_TABLE_PCL_ACTION_E               )
    entry("PCL_TCAM",                         CPSS_DXCH_CFG_TABLE_PCL_TCAM_E                 )
    entry("ROUTER_NEXT_HOP",                  CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E          )
    entry("ROUTER_LTT",                       CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E               )
    entry("ROUTER_TCAM",                      CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E              )
    entry("ROUTER_ECMP_QOS",                  CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E          )
    entry("TTI_TCAM",                         CPSS_DXCH_CFG_TABLE_TTI_TCAM_E                 )
    entry("MLL_PAIR",                         CPSS_DXCH_CFG_TABLE_MLL_PAIR_E                 )
    entry("POLICER_METERS",                   CPSS_DXCH_CFG_TABLE_POLICER_METERS_E           )
    entry("POLICER_BILLING_COUNTERS",         CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E )
    entry("VIDX",                             CPSS_DXCH_CFG_TABLE_VIDX_E                     )
    entry("ARP_TUNNEL_START",                 CPSS_DXCH_CFG_TABLE_ARP_E                      )
    entry("TUNNEL_START",                     CPSS_DXCH_CFG_TABLE_TUNNEL_START_E             )
    entry("STG",                              CPSS_DXCH_CFG_TABLE_STG_E                      )
    entry("QOS_PROFILE",                      CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E              )
    entry("MAC_TO_ME",                        CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E                )
    entry("CNC",                              CPSS_DXCH_CFG_TABLE_CNC_E                      )
    entry("CNC_BLOCKS",                       CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E                )
    entry("TRUNK",                            CPSS_DXCH_CFG_TABLE_TRUNK_E                    )
    entry("LPM_RAM",                          CPSS_DXCH_CFG_TABLE_LPM_RAM_E                  )
    entry("ROUTER_ECMP",                      CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E              )
    -- in sip 5 devices the "TRUNK_MAX" hold (4K-1) while the "TRUNK" hold 511-BC2/255BobK/1023BC3
    -- in legacy devices that "TRUNK_MAX" and "TRUNK" hold same value
    --"TRUNK" is the value that the cpssDxChTrunkInit(..,maxNumberOfTrunks,.) was initialized with
    entry("TRUNK_MAX",                        CPSS_DXCH_CFG_TABLE_TRUNK_E                    )

	--if the device isn't sip_5 there is no such tables so there is no need to call to function cpssDxChCfgTableNumEntriesGet, preventing an error from CPSS LOG
	if is_sip_5(devNum) then
		entry("L2_MLL_LTT",                       CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E               )
		entry("EPORT",                            CPSS_DXCH_CFG_TABLE_EPORT_E                    )
		entry("DEFAULT_EPORT",                    CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E            )
	else
		_G["NumberOfEntriesIn_L2_MLL_LTT_table"] = 0
		_G["NumberOfEntriesIn_EPORT_table"] = 0
		_G["NumberOfEntriesIn_DEFAULT_EPORT_table"] = 0
	end

	entry("PHYSICAL_PORT",                    CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E            )

    -- PORT, devType , revision; devFamily, numOfVirtPorts, existingPorts;
    NumberOf_PORTS_IN_DEVICE = 0
    result, values = myGenWrapper(
                        "cpssDxChCfgDevInfoGet", {
                            {"IN", "GT_U8" , "dev", devNum}, -- devNum
                            {"OUT","CPSS_DXCH_CFG_DEV_INFO_STC", "entry"}
                        }
    )

    if result == 0 then
        NumberOf_PORTS_IN_DEVICE= values.entry.genDevInfo.maxPortNum
        DeviceExistingPortsBitmap =
          {
                bitmap_0 = values.entry.genDevInfo.existingPorts.ports[0],
                bitmap_1 = values.entry.genDevInfo.existingPorts.ports[1]
           }

        DeviceType = values.entry.genDevInfo.devType
        DeviceRevision = values.entry.genDevInfo.revision
        DeviceFamily = values.entry.genDevInfo.devFamily
        DeviceSubFamily = values.entry.genDevInfo.devSubFamily
        if DeviceSubFamily == CPSS_PP_SUB_FAMILY_NONE_E or
            DeviceSubFamily == CPSS_MAX_SUB_FAMILY or
            DeviceSubFamily == CPSS_BAD_SUB_FAMILY then
            DeviceSubFamily = nil
        end
        -- DeviceSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E
        DeviceNumberOfVirtualPorts = values.entry.genDevInfo.numOfVirtPorts
        DeviceCpuPortMode = values.entry.genDevInfo.cpuPortMode

        representative_devNum = devNum

    end

    status = status and (result == 0)
    err = ((result == 0) and err) or returnCodes[result]
--print(string.format("%s %q", "debug-16 PORT", tostring(NumberOf_PORTS_IN_DEVICE)))
--print(string.format("%s %x", "debug-17 DeviceType", tostring(DeviceType)))
--print(string.format("%s %q", "debug-18 Revision", tostring(DeviceRevision)))
--print(string.format("%s %q", "debug-19 virPORT", tostring(DeviceNumberOfVirtualPorts)))

--print(string.format("%s %x", "debug-21 bitmap", tostring(DeviceExistingPortsBitmap.bitmap_1)))

    global_counter = 1

    return status, err
end

--cpss version 40 or 41
cpssVersionNumber = 42
do
    local result, version = wrlCpssCommonCpssVersionGet()
    if result == 0 then
        if string.match(version, "^CPSS_4.0_") ~= nil then
            cpssVersionNumber = 40
        elseif string.match(version, "^CPSS_4.1_") ~= nil then
            cpssVersionNumber = 41
        elseif string.match(version, "^CPSS_4.2_") ~= nil then
            cpssVersionNumber = 42
        end
    end
end

-- ************************************************************************
---
--  getCpssVer
--        @description  Returns cpss version
--
--        @return       40 or 41 or nil
--
function getCpssVer()
    return cpssVersionNumber
end



-- call this function always on init
system_capability_managment()

-- ************************************************************************
---
--  is_bobcat2_b0_and_above
--        @description  Returns true if device is BOBCAT2 B0 and above
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_bobcat2_b0_and_above(devNum)
    return is_sip_5_10(devNum)
end

-- ************************************************************************
---
--  is_sip_5_sip_5_10
--        @description  Returns true if device is sip_5_10
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5_10(devNum)
    return isSipVersion(devNum , "SIP_5_10")
end

-- ************************************************************************
---
--  is_sip_5_15
--        @description  Returns true if device is sip_5_15
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5_15(devNum)
    return isSipVersion(devNum , "SIP_5_15")
end

-- ************************************************************************
---
--  is_sip_5_16
--        @description  Returns true if device is sip_5_16
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5_16(devNum)
    return isSipVersion(devNum , "SIP_5_16")
end

-- ************************************************************************
---
--  is_sip_5_20
--        @description  Returns true if device is sip_5_20
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5_20(devNum)
    return isSipVersion(devNum , "SIP_5_20")
end

-- ************************************************************************
---
--  is_sip_5_25
--        @description  Returns true if device is sip_5_25
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_5_25(devNum)
    return isSipVersion(devNum , "SIP_5_25")
end

-- ************************************************************************
---
--  is_sip_6
--        @description  Returns true if device is sip_6
--
--        @param dev     - The device ID
--
--        @return       true or false
--
function is_sip_6(devNum)
    return isSipVersion(devNum , "SIP_6")
end

-- ************************************************************************
---
--  is_multi_fdb_instance
--        @description  Returns true if device hold multiple FDB instances
--                      Such devices need synch of multiple FDB instances by
--                      the CPU , so when auto learn is enabled ... still the
--                      NA to CPU should also be enabled .
--                      (The devices are : Lion2 , Falcon)
--        @param dev  - The device ID
--
--        @return       true or false
--
function is_multi_fdb_instance(devNum)
    local status , numOfPortGroups , numOfPipes , numOfTiles = wrLogWrapper("wrlCpssDevicePortGroupNumberGet", "(devNum)", devNum)
    if(status ~= 0--[[GT_OK]]) then
        return false
    end
    if (numOfPortGroups >= 2 or
        numOfTiles      >= 2) then
        return true
    else
        return false
    end
end
-- ************************************************************************
---
--  add_port_to_port_bmp
--        @description  checking of port existence
--
--        @param portBmp        - port bitmap
--        @param port           - port number
--
--        @return       port bmp structure
--
function add_port_to_port_bmp(portBmp, port)
    if port < 32 then
        portBmp["ports"][0] = bit_or(portBmp["ports"][0], bit_shl(1, port))
    elseif port < 64    then
        portBmp["ports"][1] = bit_or(portBmp["ports"][1], bit_shl(1, port - 32))
    elseif port < 96    then
        portBmp["ports"][2] = bit_or(portBmp["ports"][2], bit_shl(1, port - 64))
    elseif port < 128   then
        portBmp["ports"][3] = bit_or(portBmp["ports"][3], bit_shl(1, port - 96))
    end

    return portBmp
end


-- ************************************************************************
---
--  get_port_bmp_on_port
--        @description  checking of port existence
--
--        @param port           - port number
--
--        @return       port bmp structure
--
function get_port_bmp_on_port(port)
    local ports = {["ports"] = {[0] = 0, 0, 0, 0, 0, 0, 0}}

    if port < 32 then
        ports["ports"][0] = bit_or(ports["ports"][0], bit_shl(1, port))
    elseif port < 64    then
        ports["ports"][1] = bit_or(ports["ports"][1], bit_shl(1, port - 32))
    elseif port < 96    then
        ports["ports"][2] = bit_or(ports["ports"][2], bit_shl(1, port - 64))
    elseif port < 128   then
        ports["ports"][3] = bit_or(ports["ports"][3], bit_shl(1, port - 96))
    end

    return ports
end


-- ************************************************************************
---
--  does_port_exist
--        @description  checking of port existence
--
--        @param devID          - device ID
--        @param port           - port number
--
--        @return       true if exists, otherwise false
--
function does_port_exist(devID, port)
    if port == "CPU" then return true end
    if type(port) ~= "number" then return false end
    if is_device_eArch_enbled(devID) then
        local result, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devID)
        if(result ~= 0) then maxPort = 255 end

        maxPort = maxPort + 1

        if port < maxPort then

          result, values = myGenWrapper("cpssDxChPortPhysicalPortMapIsValidGet",
                                        {{ "IN", "GT_U8" , "dev", devID}, -- devNum
                                         { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                                         { "OUT","GT_BOOL" , "isValid"}} )
          if result ~= 0 then
            print("Error while calling cpssDxChPortPhysicalPortMapIsValidGet. Return code:"..to_string(result))
            return false
          end
          return values["isValid"]
        else
          return wrlDxChPortIsValidEport(devID, port)
        end
    end

    local c, ret
    local result, devInfo = cpssPerDeviceParamGet("cpssDxChCfgDevInfoGet",
                                                  devID, "devInfo",
                                                  "CPSS_DXCH_CFG_DEV_INFO_STC")
    if  0 == result then
        -- if port is 0..31
        if port < 32 then
            c = bit_and(devInfo["devInfo"]["genDevInfo"]["existingPorts"]["ports"][0],
                        bit_shl(1, port))
        elseif port < 64    then
        -- else port 32..63
            c = bit_and(devInfo["devInfo"]["genDevInfo"]["existingPorts"]["ports"][1],
                        bit_shl(1, port - 32))
        elseif port < 96    then
        -- else port 64..95
            c = bit_and(devInfo["devInfo"]["genDevInfo"]["existingPorts"]["ports"][2],
                        bit_shl(1, port - 64))
        elseif port < 128   then
        -- else port 96..127
            c = bit_and(devInfo["devInfo"]["genDevInfo"]["existingPorts"]["ports"][3],
                        bit_shl(1, port - 96))
        else
            return false
        end

        if 0 == c    then
            ret = false
        else
            ret = true
        end
        return ret
    else
        return false
    end
end

if cli_C_functions_registered("wrlDoesPortExists") then
function does_port_exist(devID, port)
    if port == "CPU" then return true end
    return wrlDoesPortExists(devID, port)
end
end -- cli_C_functions_registered("wrlDoesPortExists")

-- ************************************************************************
---
--  does_hw_port_exist
--        @description  checking of hardware port existence
--
--        @param hwDevID        - hardware device ID
--        @param hwPortNum      - hardware port number
--
--        @return       true if exists, otherwise false
--
function does_hw_port_exist(hwDevID, hwPortNum)
    local result, values
    local devId, portNum

    result, values, portNum =
        device_port_from_hardware_format_convert(hwDevID, hwPortNum)
    if 0 == result  then
        devId    = values
    else
        return false
    end

    return does_port_exist(devID, portNum)
end


-- ************************************************************************
---
--  cascadePortTypeGet
--        @description  convert's cascade port type string to
--                      CPSS_CSCD_PORT_TYPE_ENT member
--
--        @param cascade_port_type
--                             - The entry interface string
--
--        @return       cascade port type
--
function cascadePortTypeGet(cascade_port_type)
    if     "regular" == cascade_port_type then
        -- CPSS_CSCD_PORT_DSA_MODE_1_WORD_E == CPSS_CSCD_PORT_DSA_MODE_REGULAR_E
        return "CPSS_CSCD_PORT_DSA_MODE_1_WORD_E"
    elseif "extended" == cascade_port_type then
        -- CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E == CPSS_CSCD_PORT_DSA_MODE_EXTEND_E
        return "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    elseif "3_words" ==  cascade_port_type then
        return "CPSS_CSCD_PORT_DSA_MODE_3_WORDS_E"
    elseif "4_words" ==  cascade_port_type then
        return "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    elseif "network" ==  cascade_port_type then
        return "CPSS_CSCD_PORT_NETWORK_E"
    end
end


-- ************************************************************************
---
--  does_trunk_contains_ports
--        @description  checks, that trunk contains ports
--
--        @param devId              - device number
--        @param trunkId            - trunk id
--
--        @return       true, is trunk contains ports
--
function does_trunk_contains_ports(devNum, trunkId)
    local result, values
    local hwDevNum, hwPortNum
    local numMembers

    -- Trunk entry getting.
    result, values =
        myGenWrapper("cpssDxChTrunkTableEntryGet", {
            { "IN",     "GT_U8",                    "devNum",       devNum  },
            { "IN",     "GT_TRUNK_ID",              "trunkId",      trunkId },
            { "OUT",    "GT_U32",                   "numMembers"            },
            { "OUT",    "CPSS_TRUNK_MEMBER_STC",    "membersArray"          }})

    if     0 == result                      then
        numMembers = values["numMembers"]
        hwDevNum    = values["membersArray"]["device"]
        hwPortNum   = values["membersArray"]["port"]
    else
        return false
    end

    if (1 == numMembers)                                    and
       (false == does_hw_port_exist(hwDevNum, hwPortNum))    then
        numMembers  = 0
    end

    return 0 < numMembers
end


-- ************************************************************************
---
--  does_trunk_exists
--        @description  checks trunk existence
--
--        @param devId              - device number
--        @param trunkId            - trunk id
--
--        @return       true, is trunk exists otherwise false
--
function does_trunk_exists(devNum, trunkId)
    return does_trunk_contains_ports(devNum, trunkId)
end


-- ************************************************************************
---
--  get_trunk_device
--        @description  get first device id where current trunk has ports
--
--        @param trunkId        - The trunk Id
--
--        @return       device id if success otherwise nil
--
function get_trunk_device(trunkId)
    local result, values
    local existence = false
    local devNum
    local all_devices = wrlDevList()

    result = 0

    if 0 == result then
        devNum = getGlobal("devID")
        if nil == devNum then
            devNum = 0
        end

        result, values = does_trunk_exists(devNum, trunkId)
    end

    if (0 == result) and (true == values) then
        existence = values
    else
        for devNum = 1, #all_devices do
            result, values = does_trunk_exists(devNum, trunkId)

            if (0 == result) and (true == values) then
                existence = values
                break
            end
        end
    end

    if 0 == result then
        if true == existence then
            return 0, devNum
        else
            return 0, nil
        end
    else
        return 1, "Error at trunk existence checking."
    end
end


-- ************************************************************************
---
--  get_trunk_device_port_list
--        @description  get dev/port list for given trunk
--
--        @param trunkId        - The trunk Id
--        @param all_devices    - List of all devices, if not relevant set
--                                to wrlDevList
--
--        @return       0 and trunk dev/port list on success otherwise 1
--                      and error message
--
function get_trunk_device_port_list(trunkId, all_devices)
    local result, values, error_message
    local index, devNum
    local trunk_device_ports = {}
    if nil == all_devices then
        all_devices = wrlDevList()
    end

    for index, devNum in pairs(all_devices) do
        result, values = wrlCpssDxChTrunkDevicesAndPortsList(devNum, trunkId)
        if 0 == result then
            trunk_device_ports = mergeMapOfLists(trunk_device_ports, values)
        else
            error_message = values
        end
    end

    -- Converting of hardware dev/port list.
    if 0 == result then
        result, values = hardware_device_port_list_convert(trunk_device_ports, true)
        if 0 == result then
            trunk_device_ports = values
        else
            return result, values
        end
    end

    if 0 == result then
        return 0, trunk_device_ports
    else
        return 1, error_message
    end
end


-- ************************************************************************
---
--  get_default_interface_info_stc
--        @description  get CPSS_INTERFACE_INFO_STC lua table with default
--                      fields
--
--        @return       CPSS_INTERFACE_INFO_STC lua table
--
function get_default_interface_info_stc()
    return {['fabricVidx']  = 0,
            ['type']        = "CPSS_INTERFACE_PORT_E",
            ['index']       = 0,
            ['vidx']        = 0,
            ['vlanId']      = 0,
            ['devNum']      = 0,
            ['trunkId']     = 0,
            ['devPort']     ={['portNum']   = 0,
                              ['devNum']    = 0}}
end


-- ************************************************************************
---
--  system_capability_get_table_size
--        @description  get's sytem capabilities for given device
--
--        @param devId              - device number
--        @param name               - parameter name (see entry_map table)
--
--        @return       GT_OK and requested value, if success otherwise error
--                      code
--
function system_capability_get_table_size(devId, name)
    local entry_map = {
        VLAN = "CPSS_DXCH_CFG_TABLE_VLAN_E",
        FDB = "CPSS_DXCH_CFG_TABLE_FDB_E",
        PCL_ACTION = "CPSS_DXCH_CFG_TABLE_PCL_ACTION_E",
        PCL_TCAM = "CPSS_DXCH_CFG_TABLE_PCL_TCAM_E",
        ROUTER_NEXT_HOP = "CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E",
        ROUTER_LTT = "CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E",
        ROUTER_TCAM = "CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E",
        ROUTER_ECMP_QOS = "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E",
        TTI_TCAM = "CPSS_DXCH_CFG_TABLE_TTI_TCAM_E",
        MLL_PAIR = "CPSS_DXCH_CFG_TABLE_MLL_PAIR_E",
        POLICER_METERS = "CPSS_DXCH_CFG_TABLE_POLICER_METERS_E",
        POLICER_BILLING_COUNTERS = "CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E",
        VIDX = "CPSS_DXCH_CFG_TABLE_VIDX_E",
        ARP = "CPSS_DXCH_CFG_TABLE_ARP_E",
        TUNNEL_START = "CPSS_DXCH_CFG_TABLE_TUNNEL_START_E",
        STG = "CPSS_DXCH_CFG_TABLE_STG_E",
        QOS_PROFILE = "CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E",
        MAC_TO_ME = "CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E",
        CNC = "CPSS_DXCH_CFG_TABLE_CNC_E",
        CNC_BLOCKS = "CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E",
        TRUNK = "CPSS_DXCH_CFG_TABLE_TRUNK_E",
        LPM_RAM = "CPSS_DXCH_CFG_TABLE_LPM_RAM_E",
        ROUTER_ECMP = "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E",
        L2_MLL_LTT = "CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E",
        EPORT = "CPSS_DXCH_CFG_TABLE_EPORT_E",
        DEFAULT_EPORT = "CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E",
        PHYSICAL_PORT = "CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E",
        -- in sip 5 devices the "TRUNK_MAX" hold (4K-1) while the "TRUNK" hold 511-BC2/255BobK/1023BC3
        -- in legacy devices that "TRUNK_MAX" and "TRUNK" hold same value
        --"TRUNK" is the value that the cpssDxChTrunkInit(..,maxNumberOfTrunks,.) was initialized with
        TRUNK_MAX = "CPSS_DXCH_CFG_TABLE_TRUNK_E",
    }

    if entry_map[name] == nil then
        return nil
    end

    if(is_sip_5(devNum) and name == "TRUNK") then
        return get_max_trunks(devNum)
    end

    local result, values = myGenWrapper(
                    "cpssDxChCfgTableNumEntriesGet", {
                        { "IN", "GT_U8" , "dev", devId}, -- devNum
                        { "IN", "CPSS_DXCH_CFG_TABLES_ENT",  "table", entry_map[name]},
                        { "OUT","GT_U32", "numEntries"}
                    })

    --print(string.format("%s %s", "system_capability_managment debug-1 "..name, tostring(values.numEntries)))
    if result == 0 then
        return values.numEntries
    end

    return nil
end


-- ************************************************************************
---
--  device_port_from_hardware_format_convert
--        @description  converts dev/port pair from hardware format to
--                      common format
--
--        @param hwDevNum           - hardware device number
--        @param hwPortNum          - hardware port number

--
--        @return       GT_OK, hardware device/port pair
--                      error code and error message
--
function device_port_from_hardware_format_convert(hwDevNum, hwPortNum)
    local result, values
    local devNum, portNum

    -- Device id converting.
    result, values, portNum =
        wrlCpssPortsDevConvertDataBack(hwDevNum, hwPortNum)
    if 0 == result then
        devNum    = values
    else
        return result, values
    end

    return 0, devNum, portNum
end


-- ************************************************************************
---
--  hardware_device_port_list_convert
--        @description  converts hardware dev/port list
--
--        @param list               - dev/port list
--
--        @return       GT_OK, converted device/port list
--                      error code and error message
--
function hardware_device_port_list_convert(list, skipNullPorts)
    local result, values
    local devNum, portNums, portIndex, portNum
    local hwDevNum, hwPortNum
    local ret_list = {}

    for devNum, portNums in pairs(list) do
        for portIndex, portNum in pairs(portNums) do
            result, values, hwPortNum =
                wrlCpssPortsDevConvertDataBack(devNum, portNum)
            if 0 == result  then
                hwDevNum = values
            else
                return result, values
            end
            if hwPortNum ~= 62 --[[NULL_PORT]] and skipNullPorts == true then -- CPSS_NULL_PORT_NUM_CNS
                ret_list[hwDevNum] = tableAppend(ret_list[hwDevNum], hwPortNum)
            end
        end
    end

    return 0, ret_list
end


-- ************************************************************************
---
--  device_to_hardware_format_convert
--        @description  converts device to hardware format
--
--        @param devNum             - device number
--
--        @return       GT_OK, hardware device id
--                      error code and error message
--
function device_to_hardware_format_convert(devNum)
    local result, values
    local hwDevNum

    -- Device id converting.
    result, values = cpssPerDeviceParamGet("cpssDxChCfgHwDevNumGet", devNum,
                                           "hwDevNum", "GT_HW_DEV_NUM")
    if 0 == result then
        hwDevNum    = values["hwDevNum"]
    else
        return result, "Error at HW device number reading on device " .. devNum
    end

    return 0, hwDevNum
end


-- ************************************************************************
---
--  device_port_to_hardware_format_convert
--        @description  converts dev/port pair to hardware format
--
--        @param devNum             - device number
--        @param portNum            - port number
--
--        @return       GT_OK, hardware device/port pair
--                      error code and error message
--
function device_port_to_hardware_format_convert(devNum, portNum)
    local result, values
    local hwDevNum, hwPortNum

    -- Device id converting.
    result, values = cpssPerDeviceParamGet("cpssDxChCfgHwDevNumGet", devNum,
                                           "hwDevNum", "GT_HW_DEV_NUM")
    if 0 == result then
        hwDevNum    = values["hwDevNum"]
    else
        return result, "Error at HW device number reading on device " .. devNum
    end

    -- Port number converting.
    hwPortNum   = portNum

    return 0, hwDevNum, hwPortNum
end


-- ************************************************************************
---
--  device_port_list_to_hardware_format_convert
--        @description  converts dev/port list to hardware format
--
--        @param list               - dev/port list
--
--        @return       GT_OK, hardware device/port list
--                      error code and error message
--
function device_port_list_to_hardware_format_convert(list)
    local result, values
    local devNum, portNums, portIndex, portNum
    local hwDevNum, hwPortNum
    local ret_list = {}

    for devNum, portNums in pairs(list) do
        for portIndex, portNum in pairs(portNums) do
            result, values, hwPortNum =
                device_port_to_hardware_format_convert(devNum, portNum)
            if 0 == result  then
                hwDevNum = values
            else
                return result, values
            end

            ret_list[hwDevNum] = tableAppend(ret_list[hwDevNum], hwPortNum)
        end
    end

    return 0, ret_list
end


-- ************************************************************************
---
--  mac_address_adding_condition_check
--        @description  ckeck's mac-address condition of adding to
--                      Fdb-table
--
--        @param devId              - device number
--        @param mac_address        - mac-address input structure
--
--        @return       GT_OK and mac address adding condition, if check
--                      was successful
--                      GT_OK, mac address adding condition and checking
--                      warning message, if check has failed
--                      error code and error message
--
function mac_address_adding_condition_check(devId, mac_address)
    local checking_condition, checking_warning_message

    checking_condition  = true

    -- if (true == mac_address["key"]["macAddr"]["isMulticast"])           and
    if (true == ((((((mac_address or {})["key"] or {})["key"] or {})["macVlan"] or {})["macAddr"] or {})["isMulticast"]) )  and
       ("CPSS_INTERFACE_VIDX_E" ~= mac_address["dstInterface"]["type"]) and
       ("CPSS_INTERFACE_VID_E"  ~= mac_address["dstInterface"]["type"]) then
        checking_condition          = false
        checking_warning_message    = "For multicast Mac entries the " ..
                                      "interface should be only VIDX or VLAN"
    end

    return 0, checking_condition, checking_warning_message
end

local use_shared_resources_db_for_router_arp_entry = true;

function  use_shared_resources_db_for_router_arp_entry_enable(enable)
    use_shared_resources_db_for_router_arp_entry = (enable or false);
end

-- ************************************************************************
---
--  get_existing_or_create_new_router_arp_entry
--        @description  get's router ARP entry index with same mac address
--                      or creates such new entry
--
--        @param devId              - device number
--        @param index              - starting index for searching
--        @param mac_address        - mac-address
--
--        @return       operation succed and founded or created index,
--                      if exists;
--                      operation succed, if searched index not exists and
--                      new entry is impossible to create;
--                      error code and error message, if errors occures
--
function get_existing_or_create_new_router_arp_entry(devId, index, mac_address)
    local result, values
    local mac_entry
    local returned_index
    local arp_entry_found
    local arp_name

    if use_shared_resources_db_for_router_arp_entry then
        -- when used shared DB cuurently there is no API to check
        -- whether the needed entry found (old) or allocated now (new)
        -- so HW Entry will be rewritten any case
        arp_entry_found = false;
        arp_name = "ARP_MAC " .. mac_address.string;
        returned_index = sharedResourceNamedArpMemoAlloc(
            arp_name, 1 --[[size--]]);
    else
        arp_entry_found = false;
        -- Searching of existing router ARP entry with the same MAC.
        result, values = wrlCpssDxChRouterArpEntryIndexGet(devId, index, mac_address)
        if 0 == result  then
            returned_index = values
            if returned_index then
                arp_entry_found = true;
            end
        else
            return result, values
        end
        -- Searching of empty router ARP entry.
        if not arp_entry_found then
        result, values  = wrlCpssDxChNextInvalidRouterArpEntryGet(devId, index)
            if 0 == result  then
                returned_index = values
            else
                return result, values
            end
        end
    end

    -- Creating of new router ARP entry.
    if not arp_entry_found  then
        -- Adding of new arp index.
        if nil ~= returned_index    then
            result, values =
                myGenWrapper("cpssDxChIpRouterArpAddrWrite",
                             {{ "IN",   "GT_U8",        "devNum",      devId },
                              { "IN",   "GT_U32",       "routerArpIndex",
                                                              returned_index },
                              { "IN",   "GT_ETHERADDR", "arpMacAddr",
                                                                 mac_address }})
        end
        if 0 ~= result  then
            return result, returnCodes[result]
        end
    end

    return result, returned_index, arp_name
end


-- ************************************************************************
---
--  is_vlan_active
--        @description  checks vlan activation
--
--        @param devId              - device number
--        @param vlanId             - vlan id
--
--        @return       true, is vlan is activated otherwise false
--
function is_vlan_active(devNum, vlanId)
    local result, values

    -- Vlan entry writing.
    result, values =
        myGenWrapper("cpssDxChBrgVlanEntryRead", {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "GT_U16",                       "vlanId",   vlanId },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsMembers"     },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsTagging"     },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"         },
            { "OUT",    "GT_BOOL",                      "isValid"          },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                        "portsTaggingCmd"  }})
    if     0 == result                      then
        return true
    else
        return false
    end
end


-- ************************************************************************
---
--  get_max_vlan_id
--        @description  get's maximum vlanId
--
--        @param devId              - device number
--
--        @return       maximum vlanId
--
function get_max_vlan_id(devId)
    return 4094
end


-- ************************************************************************
---
--  vlan_info_from_lua_to_c_convert
--        @description  convert's vlan information form lua scripts to vlan
--                      information in CPSS API format
--
--        @param devNum             - device number
--        @param vlanId             - vlan id -->>> parameter NOT used !!!
--        @param vlanInfo           - vlan info from lua
--
--        @return       GT_OK and vlan ports, its tagging, vlan info and
--                      vlan ports tagging commands
--                      error code and error message
--
function vlan_info_from_lua_to_c_convert(devNum, vlanId, vlanInfo)
    local result, values
    local index, portNum
    local max_port_number, taggingCmdSupported
    local portsMembers, portsTagging, portsTaggingCmd

    -- Port counts getting.
    result, values = wrlCpssDevicePortNumberGet(devNum)
    if     0 == result                      then
        max_port_number = values - 1
    elseif 0 ~= result                      then
        return result, values
    end

    -- Port tagging allowness getting
    result, values = wrlCpssDxChIsPortTaggingCmdSupported(devNum)
    if 0 == result                          then
        taggingCmdSupported = values
    else
        return result, values
    end

    -- Ports members amd its tagging cmd setting.
    portsMembers     = wrlCpssClearBmpPort()
    portsTagging     = wrlCpssClearBmpPort()

--[[
    if vlanInfo.portsTaggingCmd == nil then
        vlanInfo.portsTaggingCmd = {}
    end
]]--

    portsTaggingCmd  = { ["portsCmd"] = {} }
    if "table" == type(vlanInfo) then
      if "table" == type(vlanInfo["portsMembers"])  then
        for index, portNum in pairs(vlanInfo["portsMembers"]) do
          portsMembers = wrlCpssSetBmpPort(portsMembers, portNum)
          if true == isItemInTable(portNum, vlanInfo["portsTagging"]) then
            portsTagging = wrlCpssSetBmpPort(portsTagging, portNum)
          end

          if true == taggingCmdSupported  then

            if true == isItemInTable(portNum, vlanInfo["portsTagging"]) then

              if vlanInfo.portsTaggingCmd ~= nil and
                 vlanInfo.portsTaggingCmd[portNum] ~= nil then

                portsTaggingCmd["portsCmd"][portNum] = vlanInfo.portsTaggingCmd[portNum]
              else
                portsTaggingCmd["portsCmd"][portNum] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E"
              end

             else
               portsTaggingCmd["portsCmd"][portNum] = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"
             end

           end
        end -- for
      end
    end

    return result, portsMembers, portsTagging, vlanInfo, portsTaggingCmd
end


-- ************************************************************************
---
--  vlan_info_from_c_to_lua_convert
--        @description  convert's vlan information form API vlan
--                      information to lua scripts format
--
--        @param portsMembers       - vlan ports
--        @param portsTagging       - vlan tagged ports
--        @param vlanInfo           - vlan info from API
--        @param portsTaggingCmd    - vlan ports tagging commands
--
--        @return       GT_OK and converted vlan information
--                      error code and error message
--
function vlan_info_from_c_to_lua_convert(portsMembers, portsTagging, vlanInfo, portsTaggingCmd)
    local result, values
    local portNum
    local max_port_number, taggingCmdSupported

    -- Port counts getting.
    result, values = wrlCpssDevicePortNumberGet(devNum)
    if     0 == result                      then
        max_port_number = values - 1
    elseif 0 ~= result                      then
        return result, values
    end

    -- Port tagging allowness getting
    result, values = wrlCpssDxChIsPortTaggingCmdSupported(devNum)
    if 0 == result                          then
        taggingCmdSupported = values
    else
        return result, values
    end

    -- Ports members amd its tagging cmd setting.
    vlanInfo["portsMembers"]        = {}
    vlanInfo["portsTagging"]        = {}
    vlanInfo["portsTaggingCmd"]     =
        valueIfConditionTrue(taggingCmdSupported, { }, nil)
    for portNum = 0, max_port_number do
        if true == wrlCpssIsPortSetInPortsBmp(portsMembers, portNum) and does_port_exist(devNum,portNum) then
            vlanInfo["portsMembers"] =
                tableAppend(vlanInfo["portsMembers"], portNum)

            if true == taggingCmdSupported  then
                vlanInfo["portsTaggingCmd"][portNum] = portsTaggingCmd[portNum]

                if nil ~= portsTaggingCmd[portNum] then
                    if ("CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E" ~=
                                                portsTaggingCmd[portNum])   then
                        vlanInfo["portsTagging"] =
                            tableAppend(vlanInfo["portsTagging"], portNum)
                    end
                else
                    if true == wrlCpssIsPortSetInPortsBmp(portsTagging,
                                                          portNum)          then
                        vlanInfo["portsTagging"] =
                            tableAppend(vlanInfo["portsTagging"], portNum)
                    end
                end
            else
                if true == wrlCpssIsPortSetInPortsBmp(portsTagging,
                                                      portNum)              then
                    vlanInfo["portsTagging"] =
                        tableAppend(vlanInfo["portsTagging"], portNum)
                end
            end
        end
    end

    return result, vlanInfo
end


-- ************************************************************************
---
--  does_vlan_exists
--        @description  checks vlan existence
--
--        @param devId              - device number
--        @param vlanId             - vlan id
--
--        @return       true, is vlan exists otherwise false
--
function does_vlan_exists(devNum, vlanId)
    local result, values
    local isValid = false

    -- Vlan entry writing.
    result, values =
        myGenWrapper("cpssDxChBrgVlanEntryRead", {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "GT_U16",                       "vlanId",   vlanId },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsMembers"     },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsTagging"     },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"         },
            { "OUT",    "GT_BOOL",                      "isValid"          },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                        "portsTaggingCmd"  }})
    if     0 == result                      then
        isValid = values["isValid"]
    else
        return false
    end

    return isValid
end


-- ************************************************************************
---
--  vlan_info_get
--        @description  get's vlan related information
--
--        @param devId              - device number
--        @param vlanId             - vlan id
--
--        @return       GT_OK or error code and error message
--
function vlan_info_get(devNum, vlanId)
    local result, values
    local portNum
    local max_port_number, taggingCmdSupported
    local portsMembers, portsTagging, portsTaggingCmd
    local vlanInfo

    -- Vlan entry getting.
    result, values =
        myGenWrapper("cpssDxChBrgVlanEntryRead", {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "GT_U16",                       "vlanId",   vlanId },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsMembers"     },
            { "OUT",    "CPSS_PORTS_BMP_STC",           "portsTagging"     },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"         },
            { "OUT",    "GT_BOOL",                      "isValid"          },
            { "OUT",    "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",
                                                        "portsTaggingCmd"  }})
    if 0 == result                          then
        vlanInfo        = values["vlanInfo"]
        portsMembers    = values["portsMembers"]
        portsTagging    = values["portsTagging"]
        portsTaggingCmd = values["portsTaggingCmd"]["portsCmd"]
    else
        return result, returnCodes[result]
    end

    result, values =
        vlan_info_from_c_to_lua_convert(portsMembers, portsTagging, vlanInfo,
                                        portsTaggingCmd)
    if 0 == result                          then
        vlanInfo    = values
    else
        return result, values
    end

    return result, vlanInfo
end


-- ************************************************************************
---
--  vlan_info_set
--        @description  set's vlan related information
--
--        @param devNum             - device number
--        @param vlanId             - vlan id
--        @param vlanInfo           - vlan info from lua
--
--        @return       GT_OK or error code and error message
--
function vlan_info_set(devNum, vlanId, vlanInfo)
    local result, values
    local index, portNum
    local max_port_number, taggingCmdSupported
    local portsMembers, portsTagging, vlanInfoNew, portsTaggingCmd

    result, values, portsTagging, vlanInfoNew, portsTaggingCmd =
        vlan_info_from_lua_to_c_convert(devNum, vlanId, vlanInfo)
    if     0 == result                      then
        portsMembers    = values
    else
        return result, values
    end
  --print("portsTaggingCmd="..to_string(portsTaggingCmd))

    -- Vlan entry writing.
    result, values =
        myGenWrapper("cpssDxChBrgVlanEntryWrite", {
            { "IN", "GT_U8",                        "devNum",       devNum          },
            { "IN", "GT_U16",                       "vlanId",       vlanId          },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsMembers", portsMembers    },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsTagging", portsTagging    },
            { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"    , vlanInfoNew     },
            { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", portsTaggingCmd  }
            })

    if     0 ~= result                      then
        return result, returnCodes[result]
    end

    return result
end


-- ************************************************************************
---
--  vlan_id_found
--        @description  get's vlan id of vlan that matches the pattern
--
--        @param devId              - device number
--
--        @return       GT_OK, vlan presence poperty and vlanId, if vlan
--                      was found
--                      GT_OK and vlan absence poperty, if vlan was not
--                      found
--                      error code and error message
--
function vlan_id_found(devNum, vlanInfo)
    local result, values
    local vlanId, isVlanFound, index, portNum
    local max_port_number, taggingCmdSupported
    local portsMembers, portsTagging, vlanInfoNew, portsTaggingCmd

    result, values, portsTagging, vlanInfoNew, portsTaggingCmd =
        vlan_info_from_lua_to_c_convert(devNum, 0, vlanInfo)
    if     0 == result                      then
        portsMembers    = values
    else
        return result, values
    end

    -- Searching of vlan, that matches the pattern.
    result, values, portsMembers, portsTagging, vlanInfoNew, portsTaggingCmd =
        wrlCpssDxChVlanIdGet(devNum, get_max_vlan_id(devNum), portsMembers,
                             portsTagging, vlanInfoNew, portsTaggingCmd, true)
    if     0 == result                      then
        vlanId  = values
    else
        return result, values
    end

    isVlanFound = nil ~= vlanId

    if true == isVlanFound then
        result, values =
            vlan_info_from_c_to_lua_convert(portsMembers, portsTagging,
                                            vlanInfoNew, portsTaggingCmd)
        if     0 == result                      then
            vlanInfoNew = values
        else
            return result, values
        end
    end

    return result, isVlanFound, vlanId, vlanInfoNew
end


-- ************************************************************************
---
--  get_highest_unused_vlan
--        @description  get's unused vlan with search strarting from
--
--
--        @return       GT_OK and vlanId, if it exists
--                      GT_OK and nil, if there is no unusable vlans
--                      error code and error message
--
function get_highest_unused_vlan(devId)
    local vlanId = get_max_vlan_id(devId)

    while (false ~= does_vlan_exists(devId, vlanId)) and (0 < vlanId) do
        vlanId  = vlanId - 1
    end

    if 0 < vlanId   then
        return 0, vlanId
    else
        return 0
    end
end


-- ************************************************************************
---
--  next_unused_mru_index_get
--        @description  set's vlan related information
--
--        @param devNum             - device number
--        @param start_mru_index    - mru index for search starting
--
--        @return       GT_OK and next unsed MRU-index, if exists
--                      GT_OK, if unsed MRU-index does not exist
--                      error code and error message
--
function next_unused_mru_index_get(devNum, start_mru_index)
    return wrlCpssDxChNextUnusedMRUIndexGet(devNum, start_mru_index)
end


-- ************************************************************************
---
--  mac_entry_set
--        @description  set's vlan related information
--
--        @param devNum             - device number
--        @param macEntry           - mac entry
--
--        @return       GT_OK or error code and error message
--
function mac_entry_set(devNum, macEntry)
    local result, values

    result, values = wrlCpssDxChBrgFdbMacEntrySet(devNum, macEntry)
    if 0 ~= result   then
            values  = returnCodes[result]
    else -- result == 0
        -- allow the device to process the 'by message' before we continue ...
        local isOk, msg = waitFdbAuMsgProcessed(devNum)
        if not isOk then
          -- don't make function failed. Just print a warning.
          print("warning: ", msg)
        end
    end
    return result, values
end

-- ************************************************************************
---
--  mac_entry_delete
--        @description  delete mac entry related information
--
--        @param devNum             - device number
--        @param macEntryKey        - mac entry key
--
--        @return       GT_OK or error code and error message
--
function mac_entry_delete(devNum, macEntryKey)
    local result, values

    result, values = wrlCpssDxChBrgFdbMacEntryDelete(devNum, macEntryKey)
    if 0 ~= result   then
            values  = returnCodes[result]
    else -- result == 0
        -- allow the device to process the 'by message' before we continue ...
        local isOk, msg = waitFdbAuMsgProcessed(devNum)
        if not isOk then
          -- don't make function failed. Just print a warning.
          print("warning: ", msg)
        end
    end
    return result, values
end

-- ************************************************************************
---
--  mac_entry_write
--        @description  writes FDB related information
--
--        @param devNum             - device number
--        @param mac_entry_index    - mac entry address
--        @param skip               - entry skip control
--        @param macEntry           - mac entry
--
--        @return       GT_OK or error code and error message
--
function mac_entry_write(devNum, mac_entry_index, skip, macEntry)
    local result, values

    result = 0

    -- IPV6 correction.
    if nil ~= macEntry["key"] then
        if "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" ==
                                            macEntry["key"]["entryType"] then
            -- call cpssDxChBrgMcIpv6BytesSelectSet
        end
    end

    -- Mac entry writing.
    if 0 == result then
        result, values = myGenWrapper(
            "cpssDxChBrgFdbMacEntryWrite", {
                { "IN", "GT_U8",                    "devNum",   devNum          },
                { "IN", "GT_U32",                   "index",    mac_entry_index },
                { "IN", "GT_BOOL",                  "skip",     skip            },
                { "IN", "CPSS_MAC_ENTRY_EXT_STC",   "macEntry", macEntry        }})
    end

    return result, values
end

-- ************************************************************************
---
--  mac_entry_invalidate
--        @description  invalidate fdb entry related information
--
--        @param devNum             - device number
--        @param mac_entry_index    - mac entry address
--
--        @return       GT_OK or error code and error message
--
function mac_entry_invalidate(devNum, mac_entry_index)
    if fdb_use_shadow == true then
        -- need to know which entry to remove from the shadow
        local result, values = myGenWrapper(
            "cpssDxChBrgFdbMacEntryRead", {
                { "IN", "GT_U8", "devNum", devNum}, -- devNum
                { "IN", "GT_U32", "index", mac_entry_index },
                { "OUT","GT_BOOL", "valid" }, -- *validPtr,
                { "OUT","GT_BOOL", "skip" }, -- *skipPtr,
                { "OUT","GT_BOOL", "aged" }, -- *agedPtr,
                { "OUT","GT_U8", "associatedDevNum" },   -- *associatedDevNumPtr,
                { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
            })
        if 0 == result and values and values.entry and values.entry.key and
            values.valid == true and values.skip == false
        then
            -- remove the entry from the shadow
            mac_entry_delete(devNum,values.entry.key)
        end
    end

    local result, values =
        cpssPerDeviceParamSet("cpssDxChBrgFdbMacEntryInvalidate",
                              devNum, mac_entry_index)

    return result, values

end

-- ************************************************************************
---
--  next_valid_mac_entry_get
--        @description  get's next fdb valid entry index
--
--        @param devNum             - device number
--        @param mac_entry_index    - max entry address
--        @param mac_aged           - mac-entry aged property
--        @param mac_filter         - mac entry with filled fields used as
--                                    filter
--
--        @return       GT_OK or error code and error message
--
function next_valid_mac_entry_get(devNum, mac_entry_index, mac_aged, mac_filter)
    local result, values, skip_valid_aged, mac_entry
    local mac_entry_type_array, vlan_filter_array, interface_type_filter_array

    if "table" == type(mac_filter)      then
        if "table" == type(mac_filter["key"])   then

            if "table" == type((mac_filter["key"]["key"] or {})["entryType"]) then
                mac_entry_type_array = mac_filter["key"]["key"]["entryType"]
            end

            if "table" == type(((mac_filter["key"]["key"] or {})["macVlan"] or {})["vlanId"]) then
                vlan_filter_array = mac_filter["key"]["key"]["macVlan"]["vlanId"]
            end
        end

        if "table" == type(mac_filter["dstInterface"])  then
            if "table" == type(mac_filter["dstInterface"]["type"])  then
                interface_type_filter_array = mac_filter["dstInterface"]["type"]
            end
        end
    end

    result, values, skip_valid_aged, mac_entry =
        wrlCpssDxChBrgFdbNextValidEntryGet(devNum, mac_entry_index, mac_aged,
                                           mac_filter);
    if 0 == result then
        mac_entry_index = values
    else
        return result, values
    end

    if (nil ~= mac_entry_type_array) or (nil ~= vlan_filter_array)  or
       (nil ~= interface_type_filter_array)         then
        while (nil ~= mac_entry_index) and
              (false == isItemInTable(mac_entry["key"]["key"]["entryType"],
                                      mac_entry_type_array))            and
              (false == isItemInTable( ((mac_entry["key"]["key"] or {})["macVlan"] or {})["vlanId"],
                                      vlan_filter_array))               and
              (false == isItemInTable(mac_entry["dstInterface"]["type"],
                                      interface_type_filter_array))     do
            result, values, skip_valid_aged, mac_entry =
                wrlCpssDxChBrgFdbNextValidEntryGet(devNum, mac_entry_index + 1,
                                                   mac_aged, mac_filter);
            if 0 == result then
                mac_entry_index = values
            else
                return result, values
            end
        end
    end

    return result, values, skip_valid_aged, mac_entry
end


-- ************************************************************************
---
--  does_mac_entry_exist
--        @description  checks existing of mac-entry
--
--        @param devNum             - device number
--        @param mac_aged           - mac-entry aged property
--        @param mac_entry          - serched mac entry
--
--        @return       operation status and mac-entry existence property
--
function does_mac_entry_exist(devNum, mac_aged, mac_entry)
    local result, values, skip_valid_aged, mac_entry

    result, values, skip_valid_aged, mac_entry  =
        next_valid_mac_entry_get(devNum, 0, mac_aged, mac_filter)
    if 0 ~= result  then
        return result, values
    end

    return result, isNotNil(values)
end


-- ************************************************************************
---
--  next_mirrored_port_get
--        @description  get's next rx/tx mirror analyzer port
--
--        @param rx_or_tx           - rx/tx property
--        @param devId              - device number
--        @param portNum            - first port number for next entry
--                                    searching
--        @param index              - searched mirror analyser entry index,
--                                    could be irrelevant
--
--        @return       GT_OK ot error code and error message
--
function next_mirrored_port_get(rx_or_tx, devId, portNum, index)
    if     "rx" == rx_or_tx     then
        return wrlCpssDxChNextRxMirroredPortGet(devId, portNum, index)
    elseif "tx" == rx_or_tx     then
        return wrlCpssDxChNextTxMirroredPortGet(devId, portNum, index)
    else
        return wrlCpssDxChNextMirroredPortGet(devId, portNum, index)
    end
end


-- ************************************************************************
---
--  is_mirrored_port_exist
--        @description  checks if at least rx/tx mirror analyzer port
--                      exist
--
--        @param rx_or_tx           - rx/tx property
--        @param devId              - device number
--        @param portNum            - first port number for next entry
--                                    searching
--        @param index              - searched mirror analyser entry index,
--                                    could be irrelevant
--
--        @return       GT_OK ot error code and error message
--
function is_mirrored_port_exist(rx_or_tx, devId, portNum, index)
    local result, values = next_mirrored_port_get(rx_or_tx, devId, portNum,
                                                  index)
    if 0 == result then
        return result, nil ~= values
    else
        return result, values
    end
end


-- ************************************************************************
---
--  next_mirror_analyzer_index_get
--        @description  get's next mirror analyzer index statisfy validies
--                      and filtering reqierement
--
--        @param devId              - device number
--        @param index              - mirror analyser entry index
--        @param valid              - valid entry property
--        @param filter             - entry mask
--
--        @return       GT_OK ot error code and error message
--
function next_mirror_analyzer_index_get(devId, index, valid, filter)
    local result, values
    local max_index, analyzer_interface, analyzer_device, analyzer_port

    -- Maximum mirror analyser index getting.
    result, max_index = wrlCpssDxChMaximumMirrorAnalyzerIndex()
    if result ~= 0 then
        return result, max_index -- max_index stores an error message if result != 0
    end

    local index_was_found = false
    while (index <= max_index) and (false == index_was_found) do
        -- Mirror analyser entry getting.
        result, values =
            myGenWrapper(
                "cpssDxChMirrorAnalyzerInterfaceGet", {
                    { "IN",   "GT_U8",    "dev",      devId  },
                    { "IN",   "GT_U32",   "index",    index  },
                    { "OUT", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC", "interface" }})
        if result ~= 0 then
            return result, returnCodes[result]
        end

        analyzer_interface  = values["interface"]
        analyzer_device = analyzer_interface["interface"]["devPort"]["devNum"]
        analyzer_port   = analyzer_interface["interface"]["devPort"]["portNum"]

        -- The analyzer device and port data converting.
        result, analyzer_device, analyzer_port =
            device_port_from_hardware_format_convert(analyzer_device, analyzer_port)
        if result ~= 0  then
            return result, analyzer_device  -- analyzer_device stores an error message if result != 0
        end

        -- Mirror analyser entry validity checking.
        local is_desired_index = true
        if analyzer_interface["interface"]["type"] == "CPSS_INTERFACE_PORT_E" then
            if (analyzer_interface["interface"]["devPort"]["devNum"] ~= 0 and
                does_port_exist(analyzer_device, analyzer_port))
            then
                is_desired_index = valid
            else
                is_desired_index = not valid
            end
        end

        -- Mirror analyser entry comparision with etalon.
        if is_desired_index and filter ~= nil then
            is_desired_index = compareWithEtalonTable(filter, analyzer_interface)
        end
--print(index, is_desired_index, to_string(analyzer_interface))
        index           = index + 1
        index_was_found = is_desired_index
    end

    if index_was_found then
        analyzer_interface["interface"]["devPort"]["devNum"] = analyzer_device
        analyzer_interface["interface"]["devPort"]["portNum"] = analyzer_port
        return 0, index - 1, analyzer_interface
    else
        return 0
    end

end

-- ************************************************************************
---
--  given_mirror_analyzer_or_invalid_index_get
--        @description  get's mirror analyzer index that match given entry
--                      mask, if no such entries, than return first invalid
--                      entry index
--
--        @param devId              - device number
--        @param mirror_analyzer_entry_mask
--                                  - mirror analyser entry index
--
--        @return       GT_OK ot error code and error message
--
function given_mirror_analyzer_or_invalid_index_get(devId,
                                                  mirror_analyzer_entry_mask)
    local result, values
    local analyzer_interface, index

    index = 0

    -- Getting of mirror analyzer entry which match given mask.
    result, values, analyzer_interface =
        next_mirror_analyzer_index_get(devId, index, true,
                                       mirror_analyzer_entry_mask)
    if 0 == result then
        index  = values
    else
        return result, returnCodes[result]
    end

    -- Getting of first invalid mirror analyzer entry.
    if nil == index then
        index = 0

        result, values, analyzer_interface =
            next_mirror_analyzer_index_get(devId, index, false)
        if 0 == result then
            index  = values
        else
            return result, returnCodes[result]
        end
    end

    if nil ~= index then
        return 0, index, analyzer_interface
    else
        return 0
    end
end

-- ************************************************************************
---
--  getCNCBlock
--        @description  gets CNC block according to the client
--
--        @param client              - client as string
--
--        @return       return the number of the CNCBlock
--

function getCNCBlock(client)
  local retTbl = {}
  local i

  -- exclude PPs that does not support CNC
  if boardEnv.FamiliesList["CPSS_PP_FAMILY_CHEETAH_E"] then
      return -1
  end
  if boardEnv.FamiliesList["CPSS_PP_FAMILY_CHEETAH2_E"] then
      return -1
  end

  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_XCAT_E"] then
    if (client == "IPCL_0") or (client == "IngressVlanPass") or
       (client == "IngressVlanPassAndDrop") or (client == "EgressQueuePass") or
       (client == "EgressQueuePassAndDrop") or (client == "ArpNatTableAccess") then
      retTbl = {0, 1}
      return retTbl
    end
    return -1
  end

  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_XCAT2_E"] then
    if (client == "IPCL_0") or (client == "IngressVlanPass") or
       (client == "IngressVlanPassAndDrop") or (client == "EgressQueuePass") or
       (client == "EgressQueuePassAndDrop") or (client == "ArpNatTableAccess") then
      retTbl = {0, 1, 3, 4, 5, 6, 7}
      return retTbl
    end
    return -1
  end

  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_XCAT3_E"] then
    if (client == "IPCL_0") or (client == "IngressVlanPass") or
       (client == "IngressVlanPassAndDrop") or (client == "EgressQueuePass") or
       (client == "EgressQueuePassAndDrop") or (client == "ArpNatTableAccess") then
      retTbl = {0, 1}
      return retTbl
    end
    return -1

  end

  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
    if (client == "IPCL_0") or (client == "IngressVlanPass") or
       (client == "IngressVlanPassAndDrop") or (client == "EgressQueuePass") or
       (client == "EgressQueuePassAndDrop") or (client == "ArpNatTableAccess") then
      retTbl = {0, 1, 3, 4, 5, 6, 7}
      return retTbl
    end
    return -1

  end

  if is_sip_5(nil)
  then

    if (client == "IPCL_0") then
      table.insert(retTbl, 0)
      return retTbl
    end

    if ((NumberOfEntriesIn_CNC_table / NumberOfEntriesIn_CNC_BLOCKS_table) < 8)
    then

      if (client == "IngressVlanPass") then
         retTbl = {1}
         return retTbl
      end

      if (client == "IngressVlanPassAndDrop") then
        retTbl = {1, 2}
        return retTbl
      end

      if (client == "EgressQueuePass") then
        retTbl = {1}
        return retTbl
      end

      if (client == "EgressQueuePassAndDrop") then
        retTbl = {1, 2}
        return retTbl
      end

    elseif ((NumberOfEntriesIn_CNC_table / NumberOfEntriesIn_CNC_BLOCKS_table) < 16)
    then

      if (client == "IngressVlanPass") then
         retTbl = {1, 2}
         return retTbl
      end

      if (client == "IngressVlanPassAndDrop") then
        retTbl = {1, 2, 3, 4}
        return retTbl
      end

      if (client == "EgressQueuePass") then
        retTbl = {5}
        return retTbl
      end

      if (client == "EgressQueuePassAndDrop") then
        retTbl = {5,6}
        return retTbl
      end

    else

      if (client == "IngressVlanPass") then
        for i=1, 4 do
          table.insert(retTbl, i)
        end
        return retTbl
      end

      if (client == "IngressVlanPassAndDrop") then
        for i=1, 8 do
          table.insert(retTbl, i)
        end
        return retTbl
      end

      if (client == "EgressQueuePass") then
        for i=9, 12 do
          table.insert(retTbl, i)
        end
        return retTbl
      end

      if (client == "EgressQueuePassAndDrop") then
        for i=9, 16 do
          table.insert(retTbl, i)
        end
        if is_sip_5_20(nil) then
          -- the trg port can be high
            for i=17, 17+7 do
              table.insert(retTbl, i)
            end
        end
        return retTbl
      end

    end

    if (client == "ArpNatTableAccess") then
      table.insert(retTbl, 0)
      return retTbl
    end

  end

  return -1
end

-- the type of physical ports that the system currently uses.
-- nil - means 'default' which is 'MAC' (ports with MAC)
-- "RXDMA"
currentSystemPortType = nil -- default
local function currentSystemPortTypeSet(portType)
    currentSystemPortType = portType
    luaCLI_DevInfo_portType_Define(portType)
end

-- set to 'RXDMA' the port iterators
function currentSystemPortType_RXDMA()
    -- the port iterators that starts to build before the 'func' called
    -- should already build "RXDMA" ports
    currentSystemPortTypeSet("RXDMA")
end

-- set to default the port iterators
function currentSystemPortType_default()
    currentSystemPortTypeSet(nil)
end
-- ************************************************************************
---
--  system_CLI_command_starts
--        @description  function called every time the CLI starts
--                      generic place to set ALL global configurations needed
--                      for possible left-overs from previous command
--
--      NOTE:           This function replaces implementation of CLI_command_starts()
--
local function system_CLI_command_starts()
    -- reset mode of port iterator from luaCLI_getDevInfo(...)
    currentSystemPortType_default(nil)
end
-- state that actual implementation of function CLI_command_starts
-- is the function system_CLI_command_starts()
CLI_command_starts = system_CLI_command_starts

-- ************************************************************************
---
--  isVplsModeEnabled
--        @description  Check VPLS mode enabled
--
function isVplsModeEnabled(devNum)
    return wrlIsVplsModeEnabled(devNum)
end

-- ************************************************************************
---
--  isTmEnabled
--        @description  Check Traffic Manager is enabled
--
function isTmEnabled(devNum)
    return wrlIsTmEnabled(devNum)
end


-- ************************************************************************
--  trafficClassMaxGet
---
--  @description  get traffic class max value for the device
--
--  @param devNum - a device number
--
function trafficClassMaxGet(devNum)
    return (CLI_type_dict and CLI_type_dict["traffic_class_range"].max) or 7
end


-- ************************************************************************
--  loopbackProfileMaxGet
---
--  @description  get a loopback profile max value for the device
--
--  @param devNum - a device number
--
function loopbackProfileMaxGet(devNum)
    return (CLI_type_dict and CLI_type_dict["loopback_profile"].max) or 3
end
