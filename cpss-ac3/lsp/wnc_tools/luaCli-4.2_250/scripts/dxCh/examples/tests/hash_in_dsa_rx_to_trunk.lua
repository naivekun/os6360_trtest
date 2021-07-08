--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hash_in_dsa_rx_to_trunk.lua
--*
--* DESCRIPTION:
--*       Test a hash reading from DSA FORWARD tag of incoming packet.
--*       Send several DSA tagged packets on cascade port 0 with enabled
--*       "hash reading from DSA FORWARD" feature. Create trunk with 3 tx ports
--*       and ensure the outgoing traffic depends on the hash value read from
--*       incoming packet.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devNum,"SIP_5_20")

local function trafficGenAndCheck()
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

    local vlanId = 1

    local rc, hwDevNum = device_to_hardware_format_convert(devNum)
    if (rc ~= 0) then
        setFailState()
        printLog(hwDevNum)      -- hwDevNum will store error line if rc ~= 0
        return
    end

    local dsaStc = {
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams={ dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT", vid = vlanId },
      dsaInfo={
        forward={
          srcHwDev            = hwDevNum + 1,
          srcIsTagged         = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          srcIsTrunk          = false,
          isTrgPhyPortValid   = true,
          dstInterface={ type="CPSS_INTERFACE_VID_E", vlanId=vlanId },
          hash = hash
        }
      }
    }


    local trunkPortsArr = {port2, port3, port4}
    -- do not sort as the config added the ports to trunk in same explicit way
    --table.sort(trunkPortsArr)
    print(to_string(trunkPortsArr))

    local step = isGmUsed() and 7 or 1
    for hash=0, 0x3f-1,step do          -- all possible 6-bit hash values
        dsaStc.dsaInfo.forward.hash = hash
        local rc, dsaBin =  wrlDxChDsaToString(devNum, dsaStc)
        if (rc ~=0) then
            setFailState()
            print(string.format("Error on DSA struct -> binary, hash==%x:", hash))
            return
        end

        local rxPacket = macDa .. macSa .. dsaBin .. etherType .. payload
        -- local txPacket = macDa .. macSa .. etherType .. payload

        -- fill table with expected packets via ports
        local expected = {}
        for ix=1,#trunkPortsArr do
            -- trunk members (added with cpssDxChTrunkMemberAdd used by channel-group CLI command)
            -- are assigned to be a 'designated member' in course.
            -- I.e. if trunk consists of ports 0,15,33. The "Designated Port Table" will
            -- look like
            -- index 0 - a designated trunk port is 0
            -- index 1 - a designated trunk port is 15
            -- index 2 - a designated trunk port is 33
            -- index 3 - a designated trunk port is 0
            -- index 4 - a designated trunk port is 15
            -- index 5 - a designated trunk port is 33
            --  ...
            --  Default value of field <Multicast Trunk Hash Mode> (Egress Filters Global Configuration Register) is 0
            --  So the index in "Designated Port Table" equal to hash[5..0] by default.

            if ix == hash % #trunkPortsArr + 1 then -- trunkPortsArr is sorted (see above)
                table.insert(expected, {portNum = trunkPortsArr[ix], packetCount = 1})
            else
                table.insert(expected, {portNum = trunkPortsArr[ix], packetCount = 0})
            end
        end

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
            {  -- transmitInfo
                portNum = port1, pktInfo = {fullPacket = rxPacket}
            },
            expected
        )
        if rc ~= 0 then
            setFailState()
            print(string.format("luaTgfTransmitPacketsWithExpectedEgressInfo error: \"%s\"",
                                returnCodes[rc]))
            return
        end
    end
end

luaTgfSimpleTest(
    "dxCh/examples/configurations/hash_in_dsa_rx_to_trunk_config.txt",
    "dxCh/examples/configurations/hash_in_dsa_rx_to_trunk_deconfig.txt",
    trafficGenAndCheck
)
