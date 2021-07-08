--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* isolate_lookup_bits.lua
--*
--* DESCRIPTION:
--*       Port isolation lookup bits
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--[[ Sets lookup bits values, for example :
      <num bits for portNum > = 6
      <num bits for hwDevNum > = 5
      <num bits for trunkId > = 7
]]
local function lookup_bits_func(params)
    local result
    local values

    if(nil == params.numberOfDeviceBits) then
        params.numberOfDeviceBits = 5
    end
    if(nil == params.numberOfPortBits) then
        params.numberOfPortBits = 6
    end
    if(nil == params.numberOfTrunkBits) then
        params.numberOfTrunkBits = 7
    end


    --[=====[
    local index
    local ifPortChannel = getGlobal("ifPortChannel")
    local ifType = getGlobal("ifType")


    if ifPortChannel ~= nil then
        --[[ If using trunkId:
             Index =
             <num bits for trunkId > from trunkId << (<num bits for portNum > + <num bits for hwDevNum >)
         ]]
      
        local trunkId = ifPortChannel

        print ("using trunkId calculation, trunkId: "..trunkId)

        local trunk_mask = bit_shl(1,params.numberOfTrunkBits) - 1
        local trunk_bits = bit_and(trunkId, trunk_mask) 
        index = bit_shl(trunk_bits, params.numberOfPortBits + params.numberOfDeviceBits);

    elseif ifType == "ethernet" then
        --[[ If using {dev,port}
            Index =
            (<num bits for portNum > from portNum ) << 0 |
            (<num bits for hwDevNum > from hwDevNum) << (<num bits for portNum >)
        ]]

        local ifRange = getGlobal("ifRange")
        local numports = 0
        local dev, ports
        for dev,ports in pairs(ifRange) do
            res0, devNum = device_to_hardware_format_convert(dev)

            local portNum = ports[1]
            print ("using ethernet calculation, devNum: "..devNum..", portNum: "..portNum)

            local port_mask = bit_shl(1,params.numberOfPortBits) - 1
            local port_bits = bit_and(portNum, port_mask) 

            local dev_mask = bit_shl(1,params.numberOfDeviceBits) - 1
            local dev_bits = bit_and(devNum, dev_mask) 

            index = bit_or(port_bits, bit_shl(dev_bits, params.numberOfPortBits))

            numports = numports + #ports
        end

        if numports ~= 1 then
            print("Only one port should be specified in the interface.")
            return false
        end
    else
        print("ifType: "..ifType)

    end

    --[[ =================
         The index MUST not exceed (2K+128 entries):
         ================= ]] 

    local max_index = 2048+128
    if index > max_index then
        print("Error: wrong parameters, index > max index ")
        print("Index: "..index..", max index: "..max_index)
        return false
    end

    print("Index: "..index..", max index: "..max_index)
    --]=====]

    local devices
    local devNum
    local j
    
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    if is_sip_5(devNum) then
        for j=1,#devices do
            devNum = devices[j]

            --devNum = device_to_hardware_format_convert(devNum)

            result, values =
            myGenWrapper("cpssDxChNstPortIsolationLookupBitsSet", {
                { "IN",    "GT_U8",  "devNum",             devNum },
                { "IN",    "GT_U32", "numberOfPortBits",   params.numberOfPortBits   },
                { "IN",    "GT_U32", "numberOfDeviceBits", params.numberOfDeviceBits },
                { "IN",    "GT_U32", "numberOfTrunkBits",  params.numberOfTrunkBits  }});
            if result ~= 0 then
                print("Error while setting Port Isolation Lookup Bits information. "..returnCodes[result])
                return false
            end

            explicit_set_PortIsolationLookupBits = 1
        end
    else
        print("The command is applicable for sip5 devices only "..returnCodes[result])
        return false
    end

    return true
end

--[[ Restores default lookup bits values :
      <num bits for portNum > = 6
      <num bits for hwDevNum > = 5
      <num bits for trunkId > = 7
]]
local function no_lookup_bits_func(params)
    local result
    local values
    
    local devices
    local devNum
    local j
    
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    if is_sip_5(devNum) then
        for j=1,#devices do
            devNum = devices[j]
            result, values =
            myGenWrapper("cpssDxChNstPortIsolationLookupBitsSet", {
                { "IN",    "GT_U8",    "devNum",          devNum },
                { "IN",    "GT_U32",   "numberOfPortBits",   6   },    -- as default
                { "IN",    "GT_U32",   "numberOfDeviceBits", 5     },  -- as default
                { "IN",    "GT_U32",   "numberOfTrunkBits",  7    }}); -- as default
            if result ~= 0 then
                print("Error while setting Port Isolation Lookup Bits information. "..returnCodes[result])
                return false
            end
            explicit_set_PortIsolationLookupBits = 0
        end
    else
        print("The command is applicable for sip5 devices only "..returnCodes[result])
    end
end

-- Showslookup bits values
local function show_lookup_bits_func(params)
    local result
    local values
    
    local devices
    local devNum
    local j
    
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    if is_sip_5(devNum) then
        for j=1,#devices do
            devNum = devices[j]
            result, values =
            myGenWrapper("cpssDxChNstPortIsolationLookupBitsGet", {
                { "IN",  "GT_U8",  "devNum",   devNum },
                { "OUT", "GT_U32", "numberOfPortBitsPtr"  },
                { "OUT", "GT_U32", "numberOfDeviceBitsPtr"},
                { "OUT", "GT_U32", "numberOfTrunkBitsPtr" }});

            if result ~= 0 then
                print("Error while getting Port Isolation Lookup Bits information. "..returnCodes[result])
                return false
            end
        end
         print("dev: "..devNum) 
         print("numberOfPortBits: "..values["numberOfPortBitsPtr"])
         print("numberOfDeviceBits: "..values["numberOfDeviceBitsPtr"])
         print("numberOfTrunkBits: "..values["numberOfTrunkBitsPtr"])

    else
        print("The command is applicable for sip5 devices only "..returnCodes[result])
    end
end


CLI_addHelp("config", "isolate lookup bits", "isolate lookup bits commands") 

--------------------------------------------------------
-- command registration: isolate lookup bits
--------------------------------------------------------
CLI_addCommand("config", "isolate lookup bits", {
  func   = lookup_bits_func,
  help   = "Configure Port Isolation lookup bits",
  params = {
            { type = "named",
             { format="device %devID_all", name="devID" ,help="The device number"},
             { format="portBits %lookupBits", name="numberOfPortBits", help="The number of port bits (optional, default 6)" },
             { format="devBits %lookupBits", name="numberOfDeviceBits", help="The number of device bits (optional, default 5)" },
             { format="trunkBits %lookupBits", name="numberOfTrunkBits", help="The number of trunk bits (optional, default 7)" },
             mandatory = {"devID"}
            }
           }
})


--------------------------------------------------------
-- command registration: no isolate lookup bits
--------------------------------------------------------
CLI_addCommand("config", "no isolate lookup bits", {
  func   = no_lookup_bits_func,
  help   = "Disable Port Isolation lookup bits",
  params = {
            { type = "named",
             { format="device %devID_all", name="devID" ,help="The device number"},
            }
           }
})

--------------------------------------------------------
-- command registration: show isolate lookup bits
--------------------------------------------------------
CLI_addCommand("config", "show isolate lookup bits", {
  func   = show_lookup_bits_func,
  help   = "Shows Port Isolation lookup bits",
  params = {
            { type = "named",
             { format="device %devID_all", name="devID" ,help="The device number"},
            }
           }
})



CLI_type_dict["lookupBits"] = {
    checker = CLI_check_param_number,
    min=0,
    max=15,
    complete = CLI_complete_param_number,
    help="The lookup bits value 0..15"
} 

