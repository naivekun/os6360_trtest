--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vpls_mc_bc.lua
--*
--* DESCRIPTION:
--*       The test for testing vpls MC/BC and nni to nni interfaces
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

SUPPORTED_FEATURE_DECLARE(devNum, "VPLS")
--allow to debug send of packet for cases before debugged case (needed for learning of MAC SA , that used by macDa of debugged case)
local debug_allow_prev_case_to_run = true
--allow to debug send of packet only for specific case 
local debug_specific_case = nil--"3.1" <-- example how to use
local current_case_id = "1.1"
--function to check if can run current case 
local function debug_is_allow_run_current_case()
    return (debug_specific_case == current_case_id)
end

--function to check if can run current case 
local function debug_is_allow_prev_case()
    
    return (debug_allow_prev_case_to_run and (debug_specific_case > current_case_id))
end

--function to run current case 
local function run_current_case(transmitInfo,egressInfoTable)
    printLog ("Current case " .. current_case_id .. " :")
    if(debug_specific_case)then
        if (debug_is_allow_prev_case()) then
            -- we not care about the egress ports , we only want to send packet for the SA learning
            egressInfoTable = {}
            printLog ("(Allow SA learning) \n")
        elseif(not debug_is_allow_run_current_case()) then
            printLog ("skipped \n")
            return
        end
    end
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("passed \n")
    else
        printLog ("failed \n")
        setFailState()
    end
end


local pkt
local pktInfo
local payloads
local status

local delimiter    = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
local errors_array = {}
local pass_array   = {} 
local srcMacAddr   = {"000000240001","000000240002","000000240003","000000240004"}

local configuration_2 = [[
end
configure

interface ethernet ${dev}/${port[1]}
no uni vid 20 src-mac 00:00:00:24:00:02
uni vsi 1000 assign-eport 662 eport-pvid 220 vid 20 tag-state untagged
exit

interface ethernet ${dev}/${port[2]}
no uni vid 30 src-mac 00:00:00:24:00:03
uni vsi 1000 assign-eport 663 eport-pvid 230 vid 30 tag-state tagged
exit

end
]] 

-- check that the entry is in the FDB with the expected MAC-SA
local function checkFdbEntry()
    
    -- do visualization of the FDB table .. for this mac address
    local command_showFdbEntry = 
    [[
    end
    show mac address-table all device ${dev}
    end
    ]]
    
    local expectedVid  = 4001

    executeStringCliCommands(command_showFdbEntry)
    
    for ii = 1,4 do
        local mac_filter = 
        { ["key"] =
            { ["entryType"] = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
              ["key"] = 
               { ["macVlan"] =
                   {
                      ["macAddr"]               = srcMacAddr[ii],
                      ["vlanId"]                = expectedVid
                   }
               }
            }
        } 

        result, values, mac_entry_index, dummyVidx =
            wrlCpssDxChBrgIsEntryExists(devNum, 0, mac_filter) 
                                        
        if result ~= 0 or not values then
            -- error , no entry with this macAddr was found
            local error_string = "Check entry in the FDB with the expected MAC-SA (".. srcMacAddr[ii] ..") FAILED. FDB entry not found"
            errors_array[#errors_array+1] = error_string
            setFailState()
        else
            -- OK .. as expected
            local pass_string = "Check entry in the FDB with the expected MAC-SA (".. srcMacAddr[ii] ..") PASSED"
            pass_array[#pass_array+1] = pass_string
        end
    end                                
    return
end

-- print results summary
local function printResultSummary()
    local numPass = #pass_array
    local headerName = "FDB learn from VPLS"
    printLog (delimiter)
    
    if numPass ~= 0 then
        printLog("-------- " .. headerName .. " PASS summary : " .. numPass .. " cases: ")
        
        for ii = 1,numPass do
            local myString = "["..ii.."] " .. to_string(pass_array[ii])
            printLog(myString)
        end
    else
        printLog("-------- ALL " .. headerName .. " TESTS FAILED .")
    end

    local numFail = #errors_array
    if numFail ~= 0 then
        printLog (delimiter)

        printLog("-------- " .. headerName .. " FAIL summary : " .. numFail .. " cases: ")
        for ii = 1,numFail do
            local myString = "["..ii.."] " .. to_string(errors_array[ii])
            printLog(myString)
        end
    end
    pass_array = {}
    errors_array = {}

    if numPass == 0 and numFail == 0 then
        setFailState()
        printLog (delimiter)
        printLog("-------- ERROR no " .. headerName .. " passed or failed")
    end
    printLog (delimiter)
    printLog("-------- " .. headerName .. " test ended --- \n") 
end 

--generate vpls test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/vpls_mc_bc.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

printLog("============================== Set configuration ===============================")
executeLocalConfig("dxCh/examples/configurations/vpls_mc_bc.txt")

if is_multi_fdb_instance(devNum) then
    -- we must not allow the multiple FDB instances to hold different entries.
    -- so to synch between them we need to allow NA messages to the CPU
    -- meaning that the test will run in auto learning + NA to CPU (and CPU will synch the multiple FDB instances)
    local apiName = "cpssDxChBrgFdbNaToCpuPerPortSet"
    local enable = true
    local eports = {662,663,651,653}
    for _ii,portNum in pairs(eports) do
        cpssPerPortParamSet(
            apiName,
            devNum, portNum, enable, "enable",
            "GT_BOOL")
    end
end



local transmitInfo;
local egressInfoTable;
local rc = 0;

xcat3x_set_ingress_port_vpt(port1,0)
xcat3x_set_ingress_port_vpt(port2,0)
xcat3x_set_ingress_port_vpt(port3,0)
xcat3x_set_ingress_port_vpt(port4,0)

-- BC/Unknown DA scenarios - Flooding
printLog("====================== BC/Unknown DA scenarios - Flooding ======================")
printLog("======================== Test case 1: Traffic from UNI =========================")
printLog("================= Test case 1.1: Send tagged packet to port 1 ==================")
current_case_id = "1.1"
-- tagged packet to port 1, check on ports 2, 3, 4
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_case_1_1"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged"] }},
    {portNum = port3  , pktInfo = {fullPacket = payloads["expected_mpls_c_tag_p_tag"] }},
    {portNum = port4  , pktInfo = {fullPacket = payloads["expected_mpls_untagged"] }}
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("================ Test case 1.2: Send untagged packet  to port 1 ================")
current_case_id = "1.2"
-- tagged packet to port 1, check on ports 2, 3, 4
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["untagged_case_1_2"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_1"] }},
    {portNum = port3  , pktInfo = {fullPacket = payloads["expected_mpls_c_tag_p_tag_2"] }},
    {portNum = port4  , pktInfo = {fullPacket = payloads["expected_mpls_untagged"] }}
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("================ Test case 1.3: Send packet with different VID =================")
current_case_id = "1.3"
-- tagged packet to port 1, check on ports 2, 3, 4
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_dif_vid_case_1_3"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("======================== Test case 2: Traffic from NNI =========================")
printLog("================= Test case 2.1: Send tagged packet to port 3 ==================")
current_case_id = "2.1"
-- tagged packet to port 3, check on ports 1, 2, 4
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_c_tag_p_tag_case_2_1"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged_1"] }},
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_3"] }},
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

--[[
printLog("================ Test case 2.2: Send p-tagged packet  to port 3 ================")
current_case_id = "2.2"
-- tagged packet to port 3, check on ports 1, 2, 4
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_p_tag_case_2_2"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged_1"] }},
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_2"] }},
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)
]]
printLog("======= Test case 2.3: Send packet with inner-label != 0x55552 to port 3 =======")
current_case_id = "2.3"
-- tagged packet to port 3, check on ports 1, 2, 4
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_c_tag_p_tag_case_2_3"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 },
    {portNum = port2  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("======= Test case 2.4: Send packet with outer-label != 0x66662 to port 3 ========")
current_case_id = "2.4"
-- tagged packet to port 3, check on ports 1, 2, 4
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_c_tag_p_tag_case_2_4"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 },
    {portNum = port2  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("============== Test case 2.5: Send packet without p-tag to port 3 ==============")
current_case_id = "2.5"
-- tagged packet to port 3, check on ports 1, 2, 4
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_c_tag_case_2_5"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 },
    {portNum = port2  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("================= Test case 2.6: Send tagged packet to port 2 ==================")
current_case_id = "2.6"
-- tagged packet to port 2, check on ports 1, 3, 4
transmitInfo = {devNum = devNum, portNum = port2 , pktInfo = {fullPacket = payloads["tagged_case_2_6"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged_3"] }},
    {portNum = port3  , pktInfo = {fullPacket = payloads["expected_mpls_c_tag_p_tag_4"] }},
    {portNum = port4  , pktInfo = {fullPacket = payloads["expected_mpls_untagged_2"] }}
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

--[[
printLog("============== Test case 2.7: Send untagged MPLS packet to port 4 ==============")
current_case_id = "2.7"
-- tagged packet to port 4, check on ports 1, 2, 3
transmitInfo = {devNum = devNum, portNum = port4 , pktInfo = {fullPacket = payloads["mpls_untagged_case_2_7"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged_4"] }},
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_7"] }},
    {portNum = port3  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("====================== Test case 2.8: Verify MAC learning ======================")

-- check that all expected entries is in the FDB
checkFdbEntry()
printResultSummary()
]]
-- Known DA scenarios
printLog("============================== Known DA scenarios ==============================")

-- Configure UNI ports to not use MAC-SA as criteria
executeStringCliCommands(configuration_2)

printLog("== Test case 3.1: Send c-tagged packet with DA = 00:00:00:24:00:02 to port 1 ===")
current_case_id = "3.1"
-- tagged packet to port 1, check on port 2
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_case_3_1"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_4"] }},
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("== Test case 3.2: Send c-tagged packet with DA = 00:00:00:24:00:01 to port 1 ===")
current_case_id = "3.2"
-- tagged packet to port 1, check on port 3
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_case_3_2"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port3  , pktInfo = {fullPacket = payloads["expected_mpls_c_tag_p_tag_3"] }},
    {portNum = port2  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

--[[
printLog("== Test case 3.3: Send c-tagged packet with DA = 00:00:00:24:00:04 to port 1 ===")
current_case_id = "3.3"
-- tagged packet to port 1, check on port 4
transmitInfo = {devNum = devNum, portNum = port1 , pktInfo = {fullPacket = payloads["tagged_case_3_3"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port4  , pktInfo = {fullPacket = payloads["expected_mpls_c_tag"] }},
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)
]]
printLog("============= Test case 3.4: Send c-tagged and p-tagged to port 3 ==============")
current_case_id = "3.4"
-- tagged packet to port 3, check on ports 1
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_c_tag_p_tag_case_3_4"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged"] }},
    {portNum = port2  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

--[[
printLog("==================== Test case 3.5: Send p-tagged to port 3 ====================")
current_case_id = "3.5"
-- tagged packet to port 3, check on ports 2
transmitInfo = {devNum = devNum, portNum = port3 , pktInfo = {fullPacket = payloads["mpls_p_tag_case_3_5"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_5"] }},
    {portNum = port1  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)
]]
printLog("============= Test case 3.6: Send c-tagged and p-tagged to port 4 ==============")
current_case_id = "3.6"
-- tagged packet to port 4, check on ports 1,2,3
transmitInfo = {devNum = devNum, portNum = port4 , pktInfo = {fullPacket = payloads["mpls_p_tag_case_3_6"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , packetCount = 0 },
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

printLog("==================== Test case 3.7: Send c-tagged to port 4 ====================")
current_case_id = "3.7"
-- tagged packet to port 4, check on port 1
transmitInfo = {devNum = devNum, portNum = port4 , pktInfo = {fullPacket = payloads["mpls_c_tag_case_3_7"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  , pktInfo = {fullPacket = payloads["expected_untagged_2"] }},
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)

--[[
printLog("======== Test case 3.8: Send c-tagged to port 4 with differ MPLS label =========")
current_case_id = "3.8"
-- tagged packet to port 4, check on port 2
transmitInfo = {devNum = devNum, portNum = port4 , pktInfo = {fullPacket = payloads["mpls_c_tag_case_3_8"]} }
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2  , pktInfo = {fullPacket = payloads["expected_tagged_6"] }},
    {portNum = port1  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
run_current_case(transmitInfo,egressInfoTable)
]]
-- restore the defaults
xcat3x_set_ingress_port_vpt(port1,1)
xcat3x_set_ingress_port_vpt(port2,1)
xcat3x_set_ingress_port_vpt(port3,1)
xcat3x_set_ingress_port_vpt(port4,1)

printLog("============================ Restore configuration =============================")
executeLocalConfig("dxCh/examples/configurations/vpls_mc_bc_deconfig.txt")
