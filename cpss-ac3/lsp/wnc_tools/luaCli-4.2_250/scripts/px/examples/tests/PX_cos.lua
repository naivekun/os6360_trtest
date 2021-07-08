--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_cos.lua
--*
--* DESCRIPTION:
--*       Configuration CoS Attributes examples for PIPE
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- test frame
local COS_EXAMPLE_FRAME_WITHOUT_MAC_DA = "000000000001" ..              -- SRC MAC
                                         "5555" ..                      -- Ethertype
                                         "000102030405060708090A0B" ..  -- Payload
                                         "0C0D0E0F1011121314151617" ..
                                         "18191A1B1C1D1E1F20212223" ..
                                         "2425262728292A2B2C2D2E2F" ..
                                         "303132333435363738393A3B" ..
                                         "3C3D3E3F"

-- receive port number used on tests
local COS_EXAMPLE_RX_PORTNUM = devEnv.port[1]

-- CoS attributes per Format Entry configuration
COS_EXAMPLE_FORMAT_ENTRY = {
    txPortNum = devEnv.port[2], -- transmit port number
    macDa = "00000102030A",     -- Destination MAC address
    tc = 4,                     -- Traffic Class used on test.
    dp = "green"                -- Drop Precedence used on test.
                                -- Applicable values: "green"/"yellow"/"red".
}

-- CoS attributes per Port configuration
COS_EXAMPLE_PORT = {
    txPortNum = devEnv.port[3], -- transmit port number
    macDa = "00000102030B",     -- Destination MAC address
    tc = 3,                     -- Traffic Class used on test.
    dp = "yellow"               -- Drop Precedence used on test.
                                -- Applicable values: "green"/"yellow"/"red".
}

-- CoS attributes per Packet configuration
COS_EXAMPLE_PACKET = {
    txPortNum = devEnv.port[5], -- transmit port number
    macDa = "00000102030C",     -- Destination MAC address
    tc = 2,                     -- Traffic Class used on test.
    dp = "red"                  -- Drop Precedence used on test.
                                -- Applicable values: "green"/"yellow"/"red".
}


--******************************************************************************
-- string2mac
--
-- @description Convert string to GT_ETHERADDR value
--
-- @param str - string value of MAC Address
--
-- @return Converted to GT_ETHERADDR value of MAC Address
--
--*****************************************************************************
local function string2mac(str)
    local i, strNum
    local macAddr = {arEther = {}}

    -- replace ':' to '' on MAC address
    str = str:gsub('%:', '')

    for i = 0, 5 do
        strNum = string.sub(str, 2*i+1, 2*i+2)
        macAddr.arEther[i] = tonumber(strNum, 16)
    end

    return macAddr
end


--******************************************************************************
-- readCncCounter
--
-- @description Read CNC counter
--
-- @param devNum  - device number
-- @param portNum - receive port number
-- @param tc      - Traffic Class.
--                  (APPLICABLE RANGE: 0..7).
-- @param dp      - Drop Precedence.
--                  (APPLICABLE VALUES: "green", "yellow", "red").
--
-- @return LGT_OK and CNC counter value on success. Otherwise error status and
--         error message.
--
--*****************************************************************************
local function readCncCounter(devNum, portNum, tc, dp)
    local ret, val, hwDpValue, entryIndex

    -- convert string to HW value of Drop Precedence
    if (dp == "green") then
        hwDpValue = 0
    elseif (dp == "yellow") then
        hwDpValue = 1
    elseif (dp == "red") then
        hwDpValue = 2
    else
        printLog("ERROR: wrong DP: " .. dp)
        setFailState()
        return LGT_BAD_PARAM, "ERROR: wrong DP: " .. dp
    end

    -- check portNum
    if ((portNum < 0) or (portNum > 16)) then
        printLog("ERROR: wrong port number: " .. portNum)
        setFailState()
        return LGT_BAD_PARAM, "ERROR: wrong port number: " .. portNum
    end

    -- check Traffic Class
    if ((tc < 0) or (tc > 7)) then
        printLog("ERROR: wrong Traffic Class: " .. tc)
        setFailState()
        return LGT_BAD_PARAM, "ERROR: wrong Traffic Class: " .. tc
    end

    entryIndex = bit_or(bit_or(hwDpValue, bit_shl(tc, 2)), bit_shl(portNum, 5))

    -- Read CNC counter
    ret, val = myGenWrapper("cpssPxCncCounterGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN",  "GT_U32", "blockNum", 0 },
            { "IN",  "GT_U32", "index", entryIndex },
            { "IN",  "CPSS_PX_CNC_COUNTER_FORMAT_ENT", "format",
                     "CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E" },
            { "OUT", "CPSS_PX_CNC_COUNTER_STC", "counterPtr" }
        })
    if (ret ~= LGT_OK) then
        printLog("ERROR: cpssPxCncCounterGet failed: " .. returnCodes[ret])
        setFailState()

        return ret, "cpssPxCncCounterGet failed: " .. returnCodes[ret]
    end

    return LGT_OK, val["counterPtr"]["packetCount"]["l"][0]
end


--******************************************************************************
-- prvLuaTgfPacketTypeKeyEntrySet
--
-- @description Wrapper around cpssPxIngressPacketTypeKeyEntrySet C API
--
-- @param devNum     - device number
-- @param packetType - index of the packet type key in the table.
-- @param keyData    - 17 bytes packet type key data.
-- @param keyMask    - 17 bytes packet type key mask.
--
-- @return LGT_OK on success. Otherwise error status
--
--******************************************************************************
local function prvLuaTgfPacketTypeKeyEntrySet(devNum, packetType, keyData, keyMask)
    local ret, val

    ret, val = myGenWrapper("cpssPxIngressPacketTypeKeyEntrySet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_U32", "packetType", packetType },
            { "IN", "CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC", "keyDataPtr",
              keyData },
            { "IN", "CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC", "keyMaskPtr",
              keyMask }
        })

    return ret
end


--******************************************************************************
-- cosExample_testMain
--
-- @description Main test function. Set configuration and verify correctness of
--              traffic flows.
--
--******************************************************************************
local function cosExample_testMain(testConfig)
    local ret, val
    local keyData, keyMask
    local devNum = devEnv.dev

    --
    -- SET CONFIGURATION
    --

    -- Set Ingress Packet Type key entry
    -- Needed to classify frame's Packet Type by MAC DA
    keyData = {
        profileIndex = testConfig.txPortNum,    -- use profile as ingress port
        macDa = string2mac(testConfig.macDa)    -- configure keyData to classify
                                                -- frame's Packet Type by MAC DA
    }

    keyMask = {
        profileIndex = 0x7F,                -- profile mask
        macDa = string2mac("FFFFFFFFFFFF")  -- configure keyMask to classify
                                            -- frame's Packet Type by MAC DA
    }

    ret = prvLuaTgfPacketTypeKeyEntrySet(devNum, testConfig.txPortNum,
                                         keyData, keyMask)
    if (ret ~= LGT_OK) then
        printLog("ERROR: cpssPxIngressPacketTypeKeyEntrySet failed: " ..
                 returnCodes[ret])
        setFailState()
    end


    --
    -- TRANSMITTING
    --

    -- Transmit frame
    local counterValues = {
        {
            portNum = COS_EXAMPLE_RX_PORTNUM,
            packetCount = 1
        }
    }

    local transmitInfo = {
        portNum = testConfig.txPortNum,
        pktInfo = {
            fullPacket = testConfig.macDa .. COS_EXAMPLE_FRAME_WITHOUT_MAC_DA
        }
    }

    ret = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, counterValues)
    if (ret ~= LGT_OK) then
        setFailState()
    end

    -- Read counter
    ret, val = readCncCounter(devNum, COS_EXAMPLE_RX_PORTNUM,
                              testConfig.tc,
                              testConfig.dp)
    if ((ret == LGT_OK) and (val ~= 1)) then
        printLog("ERROR: Wrong counter value:")
        printLog("   Expected: 1")
        printLog("   Received: " .. val)
        setFailState()
    end


    --
    -- RESTORE CONFIGURATION
    --

    -- restore Ingress Packet Type key entry
    keyData = {
        profileIndex = testConfig.txPortNum
    }

    keyMask = {
        profileIndex = 0x7F
    }

    ret = prvLuaTgfPacketTypeKeyEntrySet(devNum, testConfig.txPortNum,
                                         keyData, keyMask)
    if (ret ~= LGT_OK) then
        printLog("ERROR: cpssPxIngressPacketTypeKeyEntrySet failed: " ..
                 returnCodes[ret])
        setFailState()
    end
end


--******************************************************************************
-- cosExample_setCosFormatEntry
--
-- @description Verify CoS configuration per Format Entry.
--
--******************************************************************************
local function cosExample_setCosFormatEntry()
    printLog("")
    printLog("========= START check of CoS configuration per Format Entry")
    printLog("")

    cosExample_testMain(COS_EXAMPLE_FORMAT_ENTRY)

    if (isTestFailed) then
        printLog("")
        printLog("========= FAILED " ..
                 "check of CoS configuration per Format Entry")
        printLog("")
    else
        printLog("")
        printLog("========= SUCCESSFULLY finished " ..
                 "check of CoS configuration per Format Entry")
        printLog("")
    end
end


--******************************************************************************
-- cosExample_setCosPort
--
-- @description Verify CoS configuration per Port.
--
--******************************************************************************
local function cosExample_setCosPort()
    printLog("")
    printLog("========= START check of CoS configuration per Port")
    printLog("")

    cosExample_testMain(COS_EXAMPLE_PORT)

    if (isTestFailed) then
        printLog("")
        printLog("========= FAILED " ..
                 "check of CoS configuration per Port")
        printLog("")
    else
        printLog("")
        printLog("========= SUCCESSFULLY finished " ..
                 "check of CoS configuration per Port")
        printLog("")
    end
end


--******************************************************************************
-- cosExample_setCosPacket
--
-- @description Verify CoS configuration per Packet.
--
--******************************************************************************
local function cosExample_setCosPacket()
    printLog("")
    printLog("========= START check of CoS configuration per Packet")
    printLog("")

    cosExample_testMain(COS_EXAMPLE_PACKET)

    if (isTestFailed) then
        printLog("")
        printLog("========= FAILED " ..
                 "check of CoS configuration per Packet")
        printLog("")
    else
        printLog("")
        printLog("========= SUCCESSFULLY finished " ..
                 "check of CoS configuration per Packet")
        printLog("")
    end
end



--******************************************************************************
-- MAIN
--******************************************************************************

-- execute config
executeLocalConfig(luaTgfBuildConfigFileName("PX_cos_config"))

-- run CoS Format Entry test
cosExample_setCosFormatEntry()

-- execute reset counters
executeLocalConfig(luaTgfBuildConfigFileName("PX_cos_reset_counters"))

-- run CoS Port test
cosExample_setCosPort()

-- execute reset counters
executeLocalConfig(luaTgfBuildConfigFileName("PX_cos_reset_counters"))

-- run CoS Packet test
cosExample_setCosPacket()

-- execute deconfig
executeLocalConfig(luaTgfBuildConfigFileName("PX_cos_deconfig"))


-- Clear defined on test global variables
COS_EXAMPLE_FORMAT_ENTRY = nil
COS_EXAMPLE_PORT = nil
COS_EXAMPLE_PACKET = nil
