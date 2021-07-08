--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vxlan_nvgre.lua
--*
--* DESCRIPTION:
--*       The test for testing vxlan and nvgre uni and nni
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local ports = {port1,port2,port3,port4}

local transmitInfo;
local egressInfoTable;
local rc = 0;
local test_case_key, test_case_data;
local send_port_index, send_packet;
local check_key, check_data;
local check_num, check_port_index;
local packet_num, packet_data;
local masked_bytes_src, masked_bytes;
local field_num, field_key, field_data;

--generate vxlan and nvgre test packets
do
    rc, test_cases = pcall(dofile, "dxCh/examples/packets/vxlan_nvgre.lua")
    if not rc then
        printLog ("Error in file of packet definitons")
        setFailState()
        return
    end
end

executeLocalConfig("dxCh/examples/configurations/vxlan_nvgre.txt")

for test_case_key, test_case_data in pairs(test_cases) do
    send_port_index = test_case_data["send"]["port_index"];
    send_packet = test_case_data["send"]["packet"];
    printLog("case: " .. test_case_data["name"]);
    transmitInfo =
    {
        devNum = devNum,
        portNum = ports[send_port_index] ,
        pktInfo = {fullPacket = send_packet}
    };
    check_num = 1;
    egressInfoTable = {};
    for check_key, check_data in pairs(test_case_data["check"]) do
        check_port_index = check_data["port_index"];
        packet_num  = check_data["packet_num"];
        packet_data = check_data["packet"];
        masked_bytes_src = check_data["no_compare_bytes"];
        if (packet_num ~= nil) then
            egressInfoTable[check_num] =
            {
                portNum = ports[check_port_index],
                packetCount = packet_num
            };
        else
            masked_bytes = nil;
            if (masked_bytes_src ~= nil) then
                masked_bytes = {};
                field_num = 1;
                for field_key, field_data in pairs(masked_bytes_src) do
                    masked_bytes[field_num] =
                    {
                        startByte = field_data[1],
                        endByte   = field_data[2]
                    };
                    field_num = field_num + 1;
                end
            end
            egressInfoTable[check_num] =
            {
                portNum = ports[check_port_index],
                pktInfo =
                {
                    fullPacket = packet_data,
                    maskedBytesTable = masked_bytes
                }
            };
        end
        check_num = check_num + 1;
    end

    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test passed: " .. test_case_data["name"] .. "\n")
    else
        printLog ("Test failed: " .. test_case_data["name"] .. "\n")
        setFailState()
    end
end

-------------------------------------------------------------------------------

executeLocalConfig("dxCh/examples/configurations/vxlan_nvgre_deconfig.txt")




