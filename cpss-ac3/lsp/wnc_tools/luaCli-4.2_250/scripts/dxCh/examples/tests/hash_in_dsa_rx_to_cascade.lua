--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hash_in_dsa_rx_to_cascade.lua
--*
--* DESCRIPTION:
--*       Test a hash reading from DSA FORWARD tag of incoming packet.
--*       Send several DSA tagged packets on cascade port 0 with enabled
--*       "hash reading from DSA FORWARD" feature. Check tx cascade port
--*       (with disabled "store hash in DSA") to ensure  the packet is the same.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devNum,"SIP_5_20")


local function trafficGenAndCheck(srcHwDevNum, trgHwDevNum)
    local devNum = devEnv.dev
    local port1 = devEnv.port[1]
    local port2 = devEnv.port[2]
    local port3 = devEnv.port[3]
    local port4 = devEnv.port[4]
    -- packet  fields
    local macDa       = "000000003402"
    local macSa       = "000000000022"
    local etherType   = "0030"          -- payload's length 0x30(48)
    local payload     = ""              -- 48 bytes
        .."ffeeddccbbaa998877665544"
        .."332211000000000000000000000000000000000000000000000000000000000000000000"

    -- DSA tag
    local vlanId = 1


    local dsaStc = {
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams={ dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT", vid = vlanId },
      dsaInfo={
        forward={
          srcHwDev            = srcHwDevNum,
          srcIsTagged         = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          srcIsTrunk          = false,
          isTrgPhyPortValid   = true,
          dstInterface={ type = "CPSS_INTERFACE_PORT_E",
                         devPort = {devNum = trgHwDevNum, portNum = 0}}
        }
      }
    }

    -- for hash=13,23 do -- TODO: uncomment
    for hash=15,15 do -- TODO: delete
        dsaStc.dsaInfo.forward.hash = hash
        local rc, dsaBin =  wrlDxChDsaToString(devNum, dsaStc)
        if (rc ~=0) then
            setFailState()
            print(string.format("Error on DSA struct -> binary, iter==%d:", i))
            return
        end

        rxPacket = macDa .. macSa .. dsaBin .. etherType .. payload
        txPacket = rxPacket

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
            {  -- transmitInfo
                portNum = port1, pktInfo = {fullPacket = rxPacket}
            },
            {
                {portNum = port2, pktInfo = {fullPacket = txPacket}, packetCount = 1},
                {portNum = port3, packetCount = 0},
                {portNum = port4, packetCount = 0}
            }
        )
        if rc ~= 0 then
            setFailState()
            print(string.format("luaTgfTransmitPacketsWithExpectedEgressInfo error: \"%s\"",
                                returnCodes[rc]))
            return
        end
    end
end


-- ============================== MAIN =========================================

-- fill global table used by the test's config-files
if test_env == nil then
    test_env = {}
end
local rc, hwDevNum = device_to_hardware_format_convert(devEnv.dev)
if (rc ~= 0) then
    setFailState()
    printLog(hwDevNum)      -- hwDevNum will store error line if rc ~= 0
    return
end
local srcHwDevNum = hwDevNum - 1
local trgHwDevNum = hwDevNum + 1
test_env.srcHwDev = srcHwDevNum
test_env.trgHwDev = trgHwDevNum


luaTgfSimpleTest(
    "dxCh/examples/configurations/hash_in_dsa_rx_to_cascade_config.txt",
    "dxCh/examples/configurations/hash_in_dsa_rx_to_cascade_deconfig.txt",
    trafficGenAndCheck, srcHwDevNum, trgHwDevNum
)
