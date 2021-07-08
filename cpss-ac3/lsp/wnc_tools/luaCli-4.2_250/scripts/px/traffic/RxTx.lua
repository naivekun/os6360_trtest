--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* RxTx.lua
--*
--* DESCRIPTION:
--*       sending of a packet
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("px/traffic/protocols_control")

--constants
cmdLuaCLI_registerCfunction("prvLuaTgfPxNetIfSdmaSyncTxPacketSend")

-- ************************************************************************
--
--  sendPacket
--        @description  This function receives a device number, if
--                      successful return 0, if not returns ret~=0 and a
--                      status
--
--        @param packetData     - packet data
--        @param minLen         - minimum packet length
--
function sendPacket(dev, packetData, minLen)
    local ret = 0

    if (minLen==nil) then minLen=64 end
    if (minLen < 64) then minLen=64 end
    -- add padding to minLen
    if string.len(packetData) < minLen*2 then
        packetData = packetData .. string.rep("0",minLen*2-string.len(packetData))
    end

    ret = prvLuaTgfPxNetIfSdmaSyncTxPacketSend(dev, packetData)
    luaTaskDelay(50)
    return ret
end

-- ************************************************************************
--
--  configureDestinationPorts
--        @description Configures destination ports that woul allow packet to
--          egress from that ports
--
--        @param commandData - command data
--        @param params      - command parameters
--        @param config      - comfigure or restore
--                             true - configure
--                             false - restore
--
local function configureDestinationPorts(commandData, params, config)
    local devNum = params["config"]["devNum"]
    local ports = params["config"]["ports"]
    local portBmp = 0
    local k,v
    local isErr
    local result
    local values
    local cpuPort = 16

    for k,v in pairs(ports) do
        portBmp = bit_or(portBmp, bit_shl(1, v))
    end

    if config == true then
        isErr, params["config"]["portKey"] = luaWrap_cpssPxIngressPortPacketTypeKeyGet(commandData, devNum, cpuPort)
        if isErr then return isErr end

        isErr, params["config"]["keyData"], params["config"]["keyMask"] = luaWrap_cpssPxIngressPacketTypeKeyEntryGet(
            commandData, devNum, cpuPort)
        if isErr then return isErr end

        isErr, params["config"]["srcEntryTable"] = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(
            commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E", cpuPort)
        if isErr then return isErr end

        isErr, params["config"]["dstEntryTable"] = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(
            commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E", cpuPort)
        if isErr then return isErr end

        isErr, params["config"]["dstPortBmp"] = luaWrap_cpssPxIngressPortMapEntryGet(commandData, devNum,
            "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E", 256 + cpuPort)
        if isErr then return isErr end

        isErr, params["config"]["srcPortBmp"] = luaWrap_cpssPxIngressPortMapEntryGet(commandData, devNum,
        "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E", 256 + 16*cpuPort)
        if isErr then return isErr end

        isErr, result, values = genericCpssApiWithErrorHandler(
            commandData, "cpssPxIngressPacketTypeKeyEntryEnableGet", {
            {"IN", "GT_SW_DEV_NUM"       , "devNum",     devNum},
            {"IN", "CPSS_PX_PACKET_TYPE" , "packetType", cpuPort},
            {"OUT","GT_BOOL"             , "enablePtr"}
        })
        if isErr then
            local error_string = "ERROR calling function cpssPxIngressPacketTypeKeyEntryEnableGet"
            commandData:addErrorAndPrint(error_string)
        end
        params["config"]["enabledPt"] = values["enablePtr"]

        params["config"]["enPorts"] = {}
        for k, port in pairs(ports) do
            local isErr, result, values = genericCpssApiWithErrorHandler(
                commandData, "cpssPxIngressPortTargetEnableGet", {
                {"IN", "GT_SW_DEV_NUM"       , "devNum",  devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                {"OUT","GT_BOOL"             , "enablePtr"}
            })
            if isErr then
                local error_string = "ERROR calling function cpssPxIngressPortTargetEnableGet"
                commandData:addErrorAndPrint(error_string)
            end
            params["config"]["enPorts"][port] = values["enablePtr"]
        end

        isErr, result, values = genericCpssApiWithErrorHandler(
            commandData, "cpssPxIngressPortRedirectSet", {
            {"IN", "GT_SW_DEV_NUM"        , "devNum",         devNum},
            {"IN", "GT_PHYSICAL_PORT_NUM" , "portNum",        cpuPort},
            {"IN", "CPSS_PX_PORTS_BMP"    , "targetPortsBmp", portBmp}
        })
        if isErr then
            local error_string = "ERROR calling function cpssPxIngressPortRedirectSet"
            commandData:addErrorAndPrint(error_string)
        end
    else
        isErr = luaWrap_cpssPxIngressPortMapEntrySet(commandData, devNum, 1, 256 + cpuPort,
            params["config"]["dstPortBmp"])
        if isErr then return isErr end

        isErr = luaWrap_cpssPxIngressPortMapEntrySet(commandData, devNum, 0, 256 + 16*cpuPort,
            params["config"]["srcPortBmp"])
        if isErr then return isErr end

        isErr = luaWrap_cpssPxIngressPortPacketTypeKeySet(commandData, devNum,
            cpuPort, params["config"]["portKey"])
        if isErr then return isErr end

        isErr = luaWrap_cpssPxIngressPacketTypeKeyEntrySet(commandData, devNum,
            cpuPort, params["config"]["keyData"], params["config"]["keyMask"])
        if isErr then return isErr end

        isErr = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(
            commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E",
            cpuPort, params["config"]["srcEntryTable"])
        if isErr then return isErr end

        isErr = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(
            commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E",
            cpuPort, params["config"]["dstEntryTable"])
        if isErr then return isErr end

        for k,port in pairs(ports) do
            local isErr, result, values = genericCpssApiWithErrorHandler(
                commandData, "cpssPxIngressPortTargetEnableSet", {
                {"IN", "GT_SW_DEV_NUM"       , "devNum",  devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                {"IN", "GT_BOOL"             , "enable",  params["config"]["enPorts"][port]}
            })
            if isErr then
                local error_string = "ERROR calling function cpssPxIngressPortTargetEnableSet"
                commandData:addErrorAndPrint(error_string)
            end
        end
    end
    return isErr
end

-- ************************************************************************
--
--  cmdSendPacket
--        @description  sending of packets
--
--        @param params         - packet command parameters
--
function cmdSendPacket(params)

    local ret, status
    local fd
    local data
    local commandData = Command_Data()
    local devNum
    local ports
    local isErr

    if nil~=params.file then
        filename=params.file

        fd,e=fs.open(filename)

        if fd == nil then
            commandData:addErrorAndPrint("Failed to open file " .. filename .. ": "..e)
            return false
        end

        data=""
        str, e = fs.gets(fd)
        while (str~=nil) do
            str=string.gsub(str,"\n","")
            data=data..str
            str, e = fs.gets(fd)
        end
        fs.close(fd)
        if e ~= "<EOF>" then
            print("error reading file: " .. e)
        else
            data = stripHexDataSequence(data);
            if data == nil then
                commandData:addErrorAndPrint("error: data for sending is not hexadecimal")
                return;
            end
            if params["toPorts"] == true then
                for devNum, ports in pairs(params["port-range"]) do
                    params["config"] = {}
                    params["config"]["devNum"] = devNum
                    params["config"]["ports"] = ports
                    isErr = configureDestinationPorts(commandData, params, true)
                    if isErr then return isErr end
                    ret,status=sendPacket(devNum, data)
                    if (ret~=0) then
                        commandData:addErrorAndPrint("Error: "..returnCodes[ret])
                        return true
                    end
                    isErr = configureDestinationPorts(commandData, params, false)
                    if isErr then return isErr end
                end
            else
                ret,status=sendPacket(params.devID, data)
                if (ret~=0) then
                    commandData:addErrorAndPrint("Error: "..returnCodes[ret])
                    return true
                end
            end
        end
    end


    if nil~=params.data then
        data = stripHexDataSequence(params.data);
        if data == nil then
            commandData:addErrorAndPrint("error: data for sending is not hexadecimal")
            return;
        end
        if params["toPorts"] == true then
            for devNum, ports in pairs(params["port-range"]) do
                params["config"] = {}
                params["config"]["devNum"] = devNum
                params["config"]["ports"] = ports
                isErr = configureDestinationPorts(commandData, params, true)
                if isErr then return isErr end
                ret,status=sendPacket(devNum, data)
                if (ret~=0) then
                    commandData:addErrorAndPrint("Error: "..returnCodes[ret])
                    return true
                end
                isErr = configureDestinationPorts(commandData, params, false)
                if isErr then return isErr end
            end
        else
            ret,status=sendPacket(params.devID, data)
            if (ret~=0) then
                commandData:addErrorAndPrint("Error: "..returnCodes[ret])
                return true
            end
        end
    end
end

-- ************************************************************************
---
--  setRcv
--        @description  This function enables/disables receiving of packets
--                      through the specified ports if the rcv is set to
--                      false the function will check for existing rxtx
--                      queue and ENABLE all the ports back to this queue
--
--        @param devPortTable   - interface
--        @param enable         - enabling property
--
--        @usage __global       - __global["ifRange"]: iterface range
--
--        @return       true on success, otherwise false and error message
--
function setRcv(devPortTable, enable)
    local i, dev, portTable, result, value, isErr, config
    local commandData = Command_Data()
    local cpuPort = 16
    local port

    if (devPortTable==nil) then devPortTable=luaCLI_getDevInfo() end  --nil means all

    for dev, portTable in pairs(devPortTable) do
        for i=1,#portTable do
            port = portTable[i]
            if enable == true then
                config = luaGlobalGet("captureConfig")
                if config == nil then config = {} end
                if config[dev] == nil then config[dev] = {} end
                if config[dev][port] == nil then config[dev][port] = {} end
            else
                config = luaGlobalGet("captureConfig")
                if config == nil then return end
                if config[dev] == nil then break end
                if config[dev][port] == nil then break end
            end
            if enable == true then
                config[dev][port] = {}
                isErr, config[dev][port]["portKey"] = luaWrap_cpssPxIngressPortPacketTypeKeyGet(commandData, dev, port)
                if isErr then return isErr end

                isErr, config[dev][port]["keyData"], config[dev][port]["keyMask"] =
                    luaWrap_cpssPxIngressPacketTypeKeyEntryGet(commandData, dev, port)
                if isErr then return isErr end

                isErr, config[dev][port]["srcEntryTable"] = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(
                    commandData, dev, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E", port)
                if isErr then return isErr end

                isErr, config[dev][port]["dstEntryTable"] = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(
                    commandData, dev, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E", port)
                if isErr then return isErr end

                isErr, config[dev][port]["dstPortBmp"] = luaWrap_cpssPxIngressPortMapEntryGet(commandData, dev,
                    "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E", 256 + port)
                if isErr then return isErr end

                isErr, config[dev][port]["srcPortBmp"] = luaWrap_cpssPxIngressPortMapEntryGet(commandData, dev,
                    "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E", 256 + 16 * port)
                if isErr then return isErr end

                isErr, result, values = genericCpssApiWithErrorHandler(
                    commandData, "cpssPxIngressPacketTypeKeyEntryEnableGet", {
                    {"IN", "GT_SW_DEV_NUM"       , "devNum",     dev},
                    {"IN", "CPSS_PX_PACKET_TYPE" , "packetType", port},
                    {"OUT","GT_BOOL"             , "enablePtr"}
                })
                if isErr then
                    local error_string = "ERROR calling function cpssPxIngressPacketTypeKeyEntryEnableGet"
                    commandData:addErrorAndPrint(error_string)
                end
                config[dev][port]["enabledPt"] = values["enablePtr"]

                isErr, result, values = genericCpssApiWithErrorHandler(
                    commandData, "cpssPxIngressPortRedirectSet", {
                    {"IN", "GT_SW_DEV_NUM"        , "devNum",         dev},
                    {"IN", "GT_PHYSICAL_PORT_NUM" , "portNum",        port},
                    {"IN", "CPSS_PX_PORTS_BMP"    , "targetPortsBmp", 0x10000}
                })
                if isErr then
                    local error_string = "ERROR calling function cpssPxIngressPortRedirectSet"
                    commandData:addErrorAndPrint(error_string)
                end
            else
                isErr = luaWrap_cpssPxIngressPortMapEntrySet(commandData, devNum, 1, 256 + port,
                config[dev][port]["dstPortBmp"])
                if isErr then return isErr end

                isErr = luaWrap_cpssPxIngressPortMapEntrySet(commandData, devNum, 0, 256 + 16*port,
                    config[dev][port]["srcPortBmp"])
                if isErr then return isErr end

                isErr = luaWrap_cpssPxIngressPortPacketTypeKeySet(commandData, devNum,
                    port, config[dev][port]["portKey"])
                if isErr then return isErr end

                isErr = luaWrap_cpssPxIngressPacketTypeKeyEntrySet(commandData, devNum,
                    port, config[dev][port]["keyData"], config[dev][port]["keyMask"])
                if isErr then return isErr end

                isErr = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(
                    commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E",
                    port, config[dev][port]["srcEntryTable"])
                if isErr then return isErr end

                isErr = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(
                    commandData, devNum, "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E",
                    port, config[dev][port]["dstEntryTable"])
                if isErr then return isErr end
            end
        end
    end
    if enable == true then
        luaGlobalStore("captureConfig", config)
    else
        luaGlobalStore("captureConfig", nil)
    end

    signalReceiverTask()
end

CLI_addCommand("traffic", "send device", {
    func=function(params)
        params["toPorts"] = false
        cmdSendPacket(params)
    end,
    help="Send a packet from CPU port",
    params={
        { type="values", "%devID"},
        { type="named",
            "#dataOrFilename",
            mandatory={"inputString"}
    }}}
)

CLI_addCommand("traffic", "send port", {
    func=function(params)
        params["toPorts"] = true
        cmdSendPacket(params)
    end,
    help="Send a packet from CPU port to a specific port",
    params={
        { type="values", "%port-range"},
        { type="named",
            "#dataOrFilename",
            mandatory={"inputString"}
    }}}
)
