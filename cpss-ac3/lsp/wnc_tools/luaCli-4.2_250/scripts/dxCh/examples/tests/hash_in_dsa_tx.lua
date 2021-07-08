--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hash_in_dsa_tx.lua
--*
--* DESCRIPTION:
--*       Test a storing of hash into DSA FORWARD tag of outgoing packet.
--*       Send several packets (differ by MAC DA, MAC SA to affect the hash value)
--*        on port 0 and check the resulting DSA.hash in the outgoing packets
--*        tx port configured to be cascade port with enabled feature
--*        "store hash into DSA FORWARD tag".
--*
--*
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
      dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
      commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                     vpt = 1,
                     vid = vlanId },
      dsaInfo = {
        forward = {
          srcHwDev             = hwDevNum,
          srcIsTagged          = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          srcIsTrunk           = false,
          isTrgPhyPortValid    = true,
          dstInterface         ={ type="CPSS_INTERFACE_VID_E", vlanId=vlanId },
          phySrcMcFilterEnable = true,
          hash                 = hash,

          source = {portNum = devEnv.port[1]},-- src eport
          origSrcPhy = {portNum = devEnv.port[1]}   -- 12 bits in eDSA because muxed with origTrunkId
        }
      }
    }
    local step = isGmUsed() and 7 or 1
    for n=0, 0x3f-1,step do          -- all possible 6-bit hash values
        local numDa = 0x3402 + n
        local numSa = 0x22   + n + n

        local macDa = string.format("%012x",numDa)
        local macSa = string.format("%012x",numSa)
        local hash   = bit_and(bit_xor(numDa, numSa),
                               0x3F)  -- mac_sa[5:0]^mac_da[5:0]

        dsaStc.dsaInfo.forward.hash = hash
        local rc, dsaBin =  wrlDxChDsaToString(devNum, dsaStc)
        if (rc ~=0) then
            setFailState()
            print(string.format("Error on DSA struct -> binary, iter==%d:", n))
            return
        end


        local rxPacket = macDa .. macSa .. etherType .. payload
        local txPacket = macDa .. macSa .. dsaBin .. etherType .. payload
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
            {  -- transmitInfo
                portNum = port1, pktInfo = {fullPacket = rxPacket}
            },
            {  -- expected packets
                {portNum = port2, pktInfo = {fullPacket = txPacket}, packetCount = 1},
                {portNum = port3, packetCount = 1},
                {portNum = port4, packetCount = 1}
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


luaTgfSimpleTest(
    "dxCh/examples/configurations/hash_in_dsa_tx_config.txt",
    "dxCh/examples/configurations/hash_in_dsa_tx_deconfig.txt",
    trafficGenAndCheck
)
