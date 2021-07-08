--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policer_envelope.lua
--*
--* DESCRIPTION:
--*       test configuration and traffic related to MEF10.3.
--*
--*
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

local ingressPort = port2
local egressPort  = port4
local cpuPort = 63

-- this test is relevant for SIP_5_15 devices (BOBK and above)
SUPPORTED_FEATURE_DECLARE(devNum,"MEF10.3")
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload =
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f";
-- unicast macDa
local macDa   = "001122334455";
local macSa   = "000000001111";
local etherType = "3333";

local packet = macDa .. macSa .. etherType .. packetPayload;


--##################################
--##################################

local function startSection(sectionName, purpose)
    printLog (delimiter);
    printLog("START : Section " .. sectionName);
    printLog("Purpose of section : " .. purpose);
end

local function crashSection(sectionName, reason)
    local error_string = "Section " .. sectionName .. " FAILED";
    printLog ("ENDED : " .. error_string .. "\n")
    testAddErrorString(error_string)
end


local function packetSend(sectionName)
    local transmitInfo      = {portNum = ingressPort, pktInfo = {fullPacket = packet} };
    local egressInfoTable   = {{portNum = egressPort , packetCount = 1},};

    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        transmitInfo, egressInfoTable)

    printLog (delimiter)
    if rc ~= 0 then
        crashSection(sectionName, "packet not passed");
    end
end

local function convert_stage(stage)
    local stage_voc = {
        ingress0 = "CPSS_DXCH_POLICER_STAGE_INGRESS_0_E",
        ingress1 = "CPSS_DXCH_POLICER_STAGE_INGRESS_1_E",
        egress   = "CPSS_DXCH_POLICER_STAGE_EGRESS_E",
    };
    if stage_voc[stage] then
        return stage_voc[stage];
    end
    return stage;
end

local function check_mng_counters(dev, stage, set, expected)
    stage = convert_stage(stage);
	local command_data = Command_Data();
    local function cb_func(command_data, params)
        local isError, actualValue, expectedValue, description;
        local out_val, k, v;
        isError, out_val = policy_mng_counters_get(
            command_data, params.dev, params.stage, params.set);
        actualValue = {};
        for k,v in pairs(out_val) do
            actualValue[k] = out_val[k].packetMngCntr;
        end
        expectedValue = params.expected;
        description = "Policer Managment Counters Set " .. params.set;
        return isError, actualValue, expectedValue, description;
    end
    testCheckExpectedValues(
        command_data,cb_func,
        {{dev = dev, stage = stage, set = set, expected = expected}});
end

local function check_billing_counters(dev, stage, entry_index, expected)
    stage = convert_stage(stage);
	local command_data = Command_Data();
    local function cb_func(command_data, params)
        local isError, result, actualValue, expectedValue, description;
        local out_val, k, v;
        isError, result, out_val = policy_billing_get(
            command_data, params.dev, params.stage, params.entry_index, false);
        actualValue = {
            green = out_val.greenCntr.l[0];
            yellow = out_val.yellowCntr.l[0];
            red = out_val.redCntr.l[0];
        };
        expectedValue = params.expected;
        description = "Policer Billing Counters index " .. params.entry_index;
        return isError, actualValue, expectedValue, description;
    end
    testCheckExpectedValues(
        command_data,cb_func,
        {{dev = dev, stage = stage, entry_index = entry_index, expected = expected}});
end

-- those 2 moved from the test to support EPCL in BC3 that starts at high indexes
local test_ingress_acl_index = 6
local test_egress_acl_index  = 12


-- params:
-- section
-- stage
-- init_color
-- entry_rel_index
-- cbs
-- ebs
-- color_mode
-- envelope_size
-- envelope_base
-- coupling_flag0
-- expexted_color
local function gen_test(param)
    local stage_to_direction = {
        ingress0 = "ingress",
        ingress1 = "ingress",
        egress   = "egress",
    };
    local direction_to_port_dir = {
        ingress  = "",
        egress   = "egress",
    };
    local section = param.section
    local stage = param.stage;
    local direction = stage_to_direction[param.stage];
    local init_color = param.init_color
    local entry_rel_index = param.entry_rel_index
    local cbs = param.cbs
    local ebs = param.ebs
    local color_mode = param.color_mode
    local envelope_size = param.envelope_size
    local envelope_base = param.envelope_base
    local coupling_flag0 = param.coupling_flag0
    local port_dir = direction_to_port_dir[direction];
    local expexted_color = param.expexted_color
    local billing_index = 10 + entry_rel_index;
    local policy_index = envelope_base + entry_rel_index;
    if envelope_base >= 64 then
        policy_index = envelope_base;
    end
    
    local ingressTcamClientInfo = sharedResourceNamedPclRulesMemoInfoGet("ipcl0")
    local egressTcamClientInfo = sharedResourceNamedPclRulesMemoInfoGet("epcl")

    local ingressBaseIndex
    local egressBaseIndex
    
    if(ingressTcamClientInfo and ingressTcamClientInfo[1]) then
        ingressBaseIndex = ingressTcamClientInfo[1].base
    else
        ingressBaseIndex = 0
    end

    if(egressTcamClientInfo and egressTcamClientInfo[1]) then
        egressBaseIndex = egressTcamClientInfo[1].base
    else
        egressBaseIndex = 0
    end
    
    -- add the base index of the PCL client
    local egress_acl_index  = test_egress_acl_index  + egressBaseIndex
    local ingress_acl_index = test_ingress_acl_index + ingressBaseIndex

    -- global variable used by configuration file
    test_env =
    {
        stage               = tostring(stage),
        direction           = tostring(direction),
        init_color          = tostring(init_color),
        entry_rel_index     = tostring(entry_rel_index),
        cbs                 = tostring(cbs),
        ebs                 = tostring(ebs),
        color_mode          = tostring(color_mode),
        envelope_size       = tostring(envelope_size),
        envelope_base       = tostring(envelope_base),
        coupling_flag0      = tostring(coupling_flag0),
        port_dir            = tostring(port_dir),
        expexted_color      = tostring(expexted_color),
        billing_index       = tostring(billing_index),
        policy_index        = tostring(policy_index),
        priority            = tostring(entry_rel_index),
        ingress_acl_index   = tostring(ingress_acl_index),
        egress_acl_index    = tostring(egress_acl_index),
    };

    startSection(section, "check");
    executeLocalConfig("dxCh/examples/configurations/policer_envelope.txt")
    -- first packet send causes not correct result (per buckets entry)
    packetSend(section);
    -- rewrite backets, clear counters, resend packet
    executeLocalConfig("dxCh/examples/configurations/policer_envelope_reinit.txt")
    -- second packet send and check
    packetSend(section);
    check_mng_counters(devNum, test_env.stage, 0, {[test_env.expexted_color] = 1});
    check_billing_counters(devNum, test_env.stage, billing_index, {[test_env.expexted_color] = 1});
    --restore configuration
    executeLocalConfig("dxCh/examples/configurations/policer_envelope_deconfig.txt")
end

local function  test1()
    local param =
    {
        section          = "ingress0, flow, color-blind, to-yellow, no-CF0, size-5, rel-index-0",
        stage            = "ingress0",
        init_color       = "yellow",
        entry_rel_index  = 0,
        cbs              = 10000,
        ebs              = 20000,
        color_mode       = "blind",
        envelope_size    = 5,
        envelope_base    = 8,
        coupling_flag0   = false,
        expexted_color   = "yellow",
    };

    local rc, values

    -- Disable Egress Policer to avoid using of non initialized EPLR meter entries in Ingress test cases
    -- Egress test case will enable it back
    rc, values = myGenWrapper("cpssDxCh3PolicerMeteringEnableSet",{
		                {"IN", "GT_U8", "devNum", devNum},
		                {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", "CPSS_DXCH_POLICER_STAGE_EGRESS_E"},
		                {"IN", "GT_BOOL","enable",false},
		                })
    if rc ~= 0 then
	print("cpssDxCh3PolicerMeteringEnableSet failed");
	return false;
    end
    gen_test(param);
end

local function  test2()
    local param =
    {
        section          = "ingress1, flow, color-aware, to-yellow, CF0, size-4, rel-index-1",
        stage            = "ingress1",
        init_color       = "green",
        entry_rel_index  = 1,
        cbs              = 10000,
        ebs              = 20000,
        color_mode       = "aware",
        envelope_size    = 4,
        envelope_base    = 8,
        coupling_flag0   = true,
        expexted_color   = "yellow",
    };

    gen_test(param);
end

local function  test3()
    local param =
    {
        section          = "egress, priority, color-aware, to-red, NO_CF0, size-8, rel-index-7",
        stage            = "egress",
        init_color       = "yellow",
        entry_rel_index  = 7,
        cbs              = 20000,
        ebs              = 10000,
        color_mode       = "aware",
        envelope_size    = 8,
        envelope_base    = 72,
        coupling_flag0   = false,
        expexted_color   = "red",
    };
    local rc, values

    -- Enable Egress Policer
    rc, values = myGenWrapper("cpssDxCh3PolicerMeteringEnableSet",{
                                {"IN", "GT_U8", "devNum", devNum},
                                {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", "CPSS_DXCH_POLICER_STAGE_EGRESS_E"},
                                {"IN", "GT_BOOL","enable",true},
                                })
    if rc ~= 0 then
        print("cpssDxCh3PolicerMeteringEnableSet failed");
        return false;
    end

    gen_test(param);
end

local function  test4()
    local param =
    {
        section          = "ingress0, priority, color-aware, to-red, NO_CF0, size-2, rel-index-1",
        stage            = "ingress0",
        init_color       = "red",
        entry_rel_index  = 1,
        cbs              = 20000,
        ebs              = 20000,
        color_mode       = "aware",
        envelope_size    = 2,
        envelope_base    = 72,
        coupling_flag0   = false,
        expexted_color   = "red",
    };

    gen_test(param);
end

local function  test5()
    local param =
    {
        section          = "ingress1, priority, color-aware, to-red, NO_CF0, size-3, rel-index-2",
        stage            = "ingress1",
        init_color       = "green",
        entry_rel_index  = 2,
        cbs              = 10000,
        ebs              = 10000,
        color_mode       = "aware",
        envelope_size    = 3,
        envelope_base    = 72,
        coupling_flag0   = false,
        expexted_color   = "red",
    };

    gen_test(param);
end

local function  test6()
    local param =
    {
        section          = "egress, flow, color-blind, to-red, CF0, size-4, rel-index-0",
        stage            = "egress",
        init_color       = "green",
        entry_rel_index  = 0,
        cbs              = 10000,
        ebs              = 10000,
        color_mode       = "blind",
        envelope_size    = 4,
        envelope_base    = 8,
        coupling_flag0   = true,
        expexted_color   = "red",
    };

    gen_test(param);
end



test1();
test2();
test3();
test4();
test5();
test6();
pcall(
    executeStringCliCommands,
    "end\n" .. "clear bridge type " .. to_string(devNum) .. " all");





