--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* lldp_send.lua
--*
--* DESCRIPTION:
--*   creating LLDB packet and transfering it to thread function (lldp_task.lua)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

-- externs
cmdLuaCLI_registerCfunction("luaMsgQCreate")
cmdLuaCLI_registerCfunction("luaMsgQAttach")
cmdLuaCLI_registerCfunction("luaMsgQDelete")
cmdLuaCLI_registerCfunction("luaMsgQSend")
cmdLuaCLI_registerCfunction("luaMsgQRecv")
cmdLuaCLI_registerCfunction("luaTaskCreate")

do
lldp_task_id = nil
lldp_cntrl = nil


-- ************************************************************************
---
--  createLLDPPacket
--        @description  Creation of LLDP packet with necessary fields
--
--        @param devNum       - device number
--
--        @return       string on success, otherwise nill
--
local function createLLDPPacket(devNum)
    local
        sourceMac,      -- Destination MAC
        destMac,        -- Source MAC (Hooper's MAC address)
        etherType,      -- EtherType
        frameCheckSeq  -- Frame check sequence
    local           
        chssisIdTlv,                      -- Type 1
        portIdTlv,                        -- Type 2
        timeToLiveTlv,                    -- Type 3
        endofLldpuTlv,                    -- Type 0
        portDescriptionTlv,               -- Type 4
        systemNameTlv,                    -- Type 5
        systemDescriptionTlv,             -- Type 6
        systemCapabilitiesTlv,            -- Type 7
        managementAdressTlv               -- Type 8
    local
        lldpFrame,                        -- LLDP Ethernet frame
        lldpDataUnit                      -- LLDP Data Unit

    local status
    
    status, val = myGenWrapper("cpssDxChIpRouterMacSaBaseGet",
                                 {{ "IN",  "GT_U8",        "devNum", devNum},
                                  { "OUT", "GT_ETHERADDR", "mac"}})

    if status ~= 0 then
        return nil
    end
    
    local mac_address = string.gsub(val.mac, ":", "")

    sourceMac       = "0180c200000e"   -- Destination MAC
    destMac         = mac_address      -- Source MAC (Hooper's MAC address)
    etherType       = "88cc"           -- EtherType
    frameCheckSeq   = ""               -- Frame check sequence

    -- Chassis ID TLV (Type 1)
    chssisIdTlv =
        "0207" ..           -- TLV Type: Chassis id(1), TLV Length: 7
        "04" ..             -- Chassis id Subtype: MAC address (4)
        mac_address--"0000b0010004"      -- Chassis id: ???

    -- Port ID TLV (Type 2)
    portIdTlv =
        "0409" ..           -- TLV Port id(2), TLV Length:9
        "05" ..             -- Port id Subtype: Interface name (5)
        "6769312f312f3133"  -- Port id: gi1/1/13

    -- Time To Live TLV (Type 3)
    timeToLiveTlv =
        "0602" ..           -- TLV Type: TIme to Live (3), TLV Length: 2
        "0078"              -- Seconds: 120

    -- End of LLDPDU TLV (Type 0)
    endofLldpuTlv = "0000"  -- TLV Type: End of LLDPDU (0), TLV Length: 0

    -- Port Description TLV (Type 4)
    portDescriptionTlv = ""

    -- System Name TLV (Type 5)
    systemNameTlv = ""

    -- System Description TLV (Type 6)
    systemDescriptionTlv =
    "0c3f" ..               -- TLV Type: System Description (6), TLV Length: 63
    --"System Description = 24-Port Gigabit with 4-Port 10-Gigabit Stackable Managed Hooper"
    "32342d506f72742047696761626974207769746820342d506f72742031302d4769676162697420537461636b61626c65204d616e6167656420486f6f706572"

    -- System Capabilities TLV (Type 7)
    systemCapabilitiesTlv = ""

    -- Management Address TLV (Type = 8)
    managementAdressTlv =
        "100c" ..              -- TLV Type: Management Address (8), TVL Length: 12
        "05" ..                -- Address String Length: 5
        "01" ..                -- Address Subtype: Ipv4 (1)
        "00000000" ..          -- Management Address: (Should be removed)
        "02" ..                -- Interface Subtype: ifIndex (2)
        "000186a0" ..          -- Interface Number: 100000
        "00"                    -- OID String Length: 0

    lldpDataUnit =
        chssisIdTlv ..
        portIdTlv ..
        timeToLiveTlv ..
        --portDescriptionTlv ..
        --systemNameTlv ..
        systemDescriptionTlv ..
        --systemCapabilitiesTlv ..
        --managementAdressTlv ..
        endofLldpuTlv

    lldpFrame = sourceMac..destMac..etherType..lldpDataUnit..frameCheckSeq

    return lldpFrame
end


-- ************************************************************************
---
--  check_lldp_task
--        @description  Creating lldb sending tread
--
--        @return       true on success, otherwise false
--
local function check_lldp_task()
    if lldp_task_id ~= nil then
        return true
    end
    lldp_cntrl = luaMsgQCreate("lldp_ctrl", "GT_U32", 5)
    lldp_task_id = luaTaskCreate("lldp_send_task", "dxCh/traffic/lldp_task.lua", lldp_cntrl)
    
    if type(lldp_task_id) ~= "number" then
        print("Failed to create lldp send task: "..to_string(lldp_task_id))
        luaMsgQDelete(lldp_cntrl)
        lldp_cntrl = nil
        lldp_task_id = nil
        return false
    end
    return true
end

-- ************************************************************************
---
--  lldp_start_sending
--        @description  Starting sending of lldb packets
--
--        @param params.timeout    - time interval for paket sending
--
--        @return       string on success, otherwise nill
--
local function lldp_start_sending(params)
    if not check_lldp_task() then
        return false
    end
    local devPortTable=luaCLI_getDevInfo()
    local send_info = {}
    local dev, portTable
    for dev, portTable in pairs(devPortTable) do
        send_info[dev] = { portTable = portTable }
        send_info[dev].data = createLLDPPacket(dev)
    end
    luaGlobalStore("n", send_info)
    
    luaMsgQSend(lldp_cntrl, tonumber(params.timeout), 2000)
end

-- ************************************************************************
---
--  check_lldp_task
--        @description  Stop lldb packet sending
--
--        @return       true on success, otherwise false
--
local function lldp_stop()
    if not check_lldp_task() then
        return true
    end
    luaMsgQSend(lldp_cntrl, 0, 2000)
end



-- lldp send %timeout
CLI_addCommand("traffic", "lldp send", {
    func = lldp_start_sending,
    help = "Sending lldp packet at certain time interval",
    params =
    {
        {
            type = "named", 
            {
                format = "%timeout",
                name = "timeout",
                help = "Time inrerval between packet sending" 
            },
            mandatory = {"timeout"}
        }	
    }
})


-- lldp stop
CLI_addCommand("traffic", "lldp stop", {
    func = lldp_stop,
    help = "Stop sending of lldb packets"
})


end
