--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* src_id_dsa.lua
--*
--* DESCRIPTION:
--*       Test configuration and traffic related to srcId assignement from DSA tag.
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

--- AC3X need proper ports for this test
-- only devEnv.port[2] must support the "DSA_CASCADE"
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[2] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {devEnv.port[2]})

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

local ingressPort = port2

-- this test is relevant for SIP_5 devices
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")

--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"
-- broadcast macDa
local macDa = "FFFFFFFFFFFF"
local macSa = "000000001111"

local function buildPacketBc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end
    
    return macDa .. macSa .. partAfterMac .. packetPayload
end


-- set/unset port as cascade
local function generic_cscd_port_set(devNum, portNum, unset , portType)
    local command_data = Command_Data()

    if unset == true then
        portType = "CPSS_CSCD_PORT_NETWORK_E"
    elseif portType then
        -- already have portType value
    elseif is_sip_5(devNum) then
        portType = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    else
        portType = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    end

    apiName = "cpssDxChCscdPortTypeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
        { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType", portType}
    })
end 

local function port_src_id_force_enable_set(devNum, portNum, enable, srcId)
    local command_data = Command_Data()

    apiName = "cpssDxChBrgSrcIdPortSrcIdForceEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "GT_BOOL", "enable", enable}
    })

    apiName = "cpssDxChBrgSrcIdPortDefaultSrcIdSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "GT_U32", "defaultSrcId", srcId}
    })

    if enable == true then
        apiName = "cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT", "mode", "CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E"},
        })

        apiName = "cpssDxChBrgSrcIdGroupPortDelete"
        -- filter port4 for source ID
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "sourceId", srcId},
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port4}
        })
    else
        if srcId == 0 then
            apiName = "cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT", "mode", "CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E"},
            })

            apiName = "cpssDxChBrgSrcIdGroupPortAdd"
            -- remove port4 for source ID filtering
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_U32", "sourceId", srcId},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port4}
            })
        end
    end
end

--##################################
--##################################
-- function to run section
local function testSection(sectionInfo)
    local sectionName       = "dsa_tag_with_src_id"
    local transmitInfo      = sectionInfo.transmitInfo   
    local egressInfoTable   = sectionInfo.egressInfoTable

    local dropped_on_ingress = {
        -- expect ALL ports NOT to get it
        {portNum = port1  , packetCount = 0},
        {portNum = port2  , packetCount = 0},
        {portNum = port3  , packetCount = 0},
        {portNum = port4  , packetCount = 0}
    }
    local ingressPort = transmitInfo.portNum

    local index
    -- remove ingress port from dropped_on_ingress
    for ii,entry in pairs(dropped_on_ingress) do
        if entry.portNum == ingressPort then
            -- move last entry to override this entry
            index = ii
            break
        end
    end

    -- remove last index
    if index then
        dropped_on_ingress[index] = nil
    end

    printLog (delimiter)

    printLog("START : Section " .. sectionName)

    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED , when "
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED , when "
        printLog ("ENDED : " .. error_string .. "\n")
    end 

    printLog (delimiter)

end

--##################################
--##################################
local function test_run( packetsToSend, sectionName)
    local srcId = 5
    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = nil --[[filled in runtime]]} }
    local egressInfoTableForce = {
        {portNum = port1  , packetCount = 1},
        {portNum = port3  , packetCount = 1},
        {portNum = port4  , packetCount = 0},
    }
    local egressInfoTableDsaSrcId = {
        {portNum = port1  , packetCount = 1},
        {portNum = port3  , packetCount = 1},
        {portNum = port4  , packetCount = 1},
    }

    local sectionInfo = {
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTableForce,
    }

    -- force src ID assignement on ingress port and remove port4 from group
    port_src_id_force_enable_set(devNum, ingressPort, true, srcId)

    for index,entry in pairs(packetsToSend) do
        printLog (delimiter)
        
        sectionInfo.egressInfoTable = egressInfoTableForce
        
        -- the ingress packet
        sectionInfo.transmitInfo.pktInfo.fullPacket = entry.packet
        generic_cscd_port_set(devNum, ingressPort,false,entry.cascade)

        sectionInfo.egressInfoTable[1].pktInfo = nil
        sectionInfo.egressInfoTable[1].packetCount = 1 -- we do not want to check the content of packet that removed the DSA tag

        if index == 1 then
            --startSimulationLog()
        end
        

        -->> test now
        testSection(sectionInfo)

        generic_cscd_port_set(devNum, ingressPort, true)

        -- debug specific part 
        if index == 1 then
            --stopSimulationLog()
        end

    end

    -- reset src ID assignement on ingress port
    port_src_id_force_enable_set(devNum, ingressPort, false, srcId)

    for index,entry in pairs(packetsToSend) do
        printLog (delimiter)

        sectionInfo.egressInfoTable = egressInfoTableDsaSrcId

        -- the ingress packet
        sectionInfo.transmitInfo.pktInfo.fullPacket = entry.packet
        generic_cscd_port_set(devNum, ingressPort,false,entry.cascade)

        sectionInfo.egressInfoTable[1].pktInfo = nil
        sectionInfo.egressInfoTable[1].packetCount = 1 -- we do not want to check the content of packet that removed the DSA tag

        if index == 1 then
            -- startSimulationLog()
        end


        -->> test now
        testSection(sectionInfo)

        generic_cscd_port_set(devNum, ingressPort,true--[[unset]])

        -- debug specific part 
        if index == 1 then
            -- stopSimulationLog()
        end

    end

    -- reset src ID assignement on ingress port and add port4 to group
    port_src_id_force_enable_set(devNum, ingressPort, false)

end

local function buildPacketDsaTag_FRW(isEdsa , srcId)
    local vlanId = 1
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local dsaTagType

    if not isEdsa then
        dsaTagType = "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
    else
        dsaTagType = "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end

    local srcEPort = 0
    local origSrcPhyPort = 0

    dsaTagStc = {
      dsaInfo={
        forward={
          source={
            portNum=srcEPort
          },
          tag0TpidIndex=0,
          phySrcMcFilterEnable=false,
          srcIsTagged="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          origSrcPhy={
            portNum=origSrcPhyPort
          },
          srcIsTrunk=false,
          egrFilterRegistered=false,
          srcId=srcId,
          isTrgPhyPortValid = 1,
          dstInterface={
            type="CPSS_INTERFACE_VID_E",
            vlanId=vlanId
          },
          srcHwDev=srcHwDevNum
        }
      },
      commonParams={
        dsaTagType=dsaTagType,
        vpt=1,
        vid=vlanId
      },
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
    }

    local dsaString = luaTgfDsaStringFromSTC(devNum,dsaTagStc)

    return buildPacketBc(dsaString)
end

--packet classifications dsa-srcId
local function test_dsa_classifications()
--[[
a.	FRW DSA , 2 words , srcId = 17
b.	FRW DSA , 4 words , srcId = 17

]]--
    local test_packets = {
        {type = dsa_tag  , value = 5    , packet = buildPacketDsaTag_FRW(true--[[eDsa]] , 17--[[srcId]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"},
        {type = dsa_tag  , value = 5    , packet = buildPacketDsaTag_FRW(false--[[EXT-dsa]] , 17--[[srcId]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"},
    }
    
    test_run(test_packets)
end

--packet classifications dsa-srcId
test_dsa_classifications()


