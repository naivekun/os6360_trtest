--
-- vxlan and nvgre packets
--

-- common
local ipv4_hdr_checksum;   -- work variable
local ipv4_hdr_before_cs;  -- work variable
local ipv4_hdr_after_cs;   -- work variable
local ipv4_eth_type = "0800"
local ipv4_hdr_part1 =  "4500" -- version 5, header length - words, DSCP 0, ECN - 0

------------------------------------------------------------------------------------------------
-- vxlan

local uni_vxlan_mac_da   = "00112233445A";
local uni_vxlan_mac_sa   = "000000240001";
local uni_vxlan_mac_vtag = "81000065";
local uni_vxlan_eth_type = "3333";
local uni_vxlan_l2_payload =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

local uni_vxlan_ingress_packet =
    uni_vxlan_mac_da .. uni_vxlan_mac_sa .. uni_vxlan_mac_vtag
    .. uni_vxlan_eth_type .. uni_vxlan_l2_payload;

local nni_vxlan_mac_da   = "000000B01A11";
local nni_vxlan_mac_sa   = "000000102030";
-- ommited 16 bit total length
local ipv4_vxlan_hdr_part2 =  ""
    .. "00000000" -- identification(16), flags(3), fragment offset(13)
    .. "4011" -- time to live(8), UDP-protocol(8)
local nni_vxlan_local_ip  = "C0B00A0A"
local nni_vxlan_remote_ip = "C0B00A0B"

local nni_vxlan_payload = uni_vxlan_ingress_packet;

local nni_vxlan_udp_total_length = 0
    + 8 --[[UDP header--]]
    + 8 --[[VX_LAN header]]
    + math.floor(string.len(nni_vxlan_payload) / 2)

local nni_vxlan_ipv4_total_length = 0
    + 20 --[[IPV4 header--]]
    + nni_vxlan_udp_total_length

local nni_vxlan_udp_header = ""
    .. "000012B5" --Src port(16) (not compared), Dst port(16)
    .. string.format("%04X", nni_vxlan_udp_total_length)
    .. "0000" -- Check Sum Placeholder

local nni_vxlan_header = "080000000F424000"

-- vxlan egress packet

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_vxlan_ipv4_total_length)
    .. ipv4_vxlan_hdr_part2;
ipv4_hdr_after_cs = nni_vxlan_local_ip .. nni_vxlan_remote_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nni_vxlan_egress_packet = ""
    .. nni_vxlan_mac_da .. nni_vxlan_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_vxlan_udp_header
    .. nni_vxlan_header
    .. uni_vxlan_ingress_packet;

-- vxlan ingress packet  (swapped IP addresses)

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_vxlan_ipv4_total_length)
    .. ipv4_vxlan_hdr_part2;
ipv4_hdr_after_cs =  nni_vxlan_remote_ip .. nni_vxlan_local_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nni_vxlan_ingress_packet = ""
    .. nni_vxlan_mac_da .. nni_vxlan_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_vxlan_udp_header
    .. nni_vxlan_header
    .. uni_vxlan_ingress_packet;

------------------------------------------------------------------------------------------------
-- nvgre

local uni_nvgre_mac_da   = "00112233445B";
local uni_nvgre_mac_sa   = "000000240012";
local uni_nvgre_eth_type = "4444";
local uni_nvgre_l2_payload =
    "FEDCBA9876543210FEDCBA9876543210"..
    "FEDCBA9876543210FEDCBA9876543210"..
    "FEDCBA9876543210FEDCBA9876543210"

local uni_nvgre_ingress_packet =
    uni_nvgre_mac_da .. uni_nvgre_mac_sa
    .. uni_nvgre_eth_type .. uni_nvgre_l2_payload;

local nni_nvgre_mac_da   = "000000B01A22";
local nni_nvgre_mac_sa   = "000000102030";
-- ommited 16 bit total length
local ipv4_nvgre_hdr_part2 =  ""
    .. "00000000" -- identification(16), flags(3), fragment offset(13)
    .. "402F" -- time to live(8), GRE-protocol(8)
local nni_nvgre_local_ip  = "C0B0140A"
local nni_nvgre_remote_ip = "C0B0140C"

local nni_ipv4_nvgre_total_length = 0
    + 20 --[[IPV4 header--]]
    + 8 --[[NV_GRE header]]
    + math.floor(string.len(uni_nvgre_ingress_packet) / 2)

local nni_nvgre_header = "200065580F424100"

-- nvgre egress packet

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_ipv4_nvgre_total_length)
    .. ipv4_nvgre_hdr_part2;
ipv4_hdr_after_cs = nni_nvgre_local_ip .. nni_nvgre_remote_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nni_nvgre_egress_packet = ""
    .. nni_nvgre_mac_da .. nni_nvgre_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_nvgre_header
    .. uni_nvgre_ingress_packet;

-- nvgre ingress packet (swapped IP addresses)

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_ipv4_nvgre_total_length)
    .. ipv4_nvgre_hdr_part2;
ipv4_hdr_after_cs = nni_nvgre_remote_ip .. nni_nvgre_local_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nni_nvgre_ingress_packet = ""
    .. nni_nvgre_mac_da .. nni_nvgre_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_nvgre_header
    .. uni_nvgre_ingress_packet;


return
{
    {
        name = "vxlan_uni_to_nni",
        send = {port_index = 1, packet = string.lower(uni_vxlan_ingress_packet)},
        check =
        {
            {port_index = 2, packet_num = 0},
            {port_index = 3, packet = string.lower(nni_vxlan_egress_packet),
                no_compare_bytes =
                { -- vlan untagged packet
                     {18, 19 } -- ipv4 header - 2 bytes 'identification field'
                    ,{24, 25 } -- ipv4 header - 2 bytes 'checksum'
                    ,{34, 35 } -- UDP  header - 2 bytes UDP Source port
                }
            },
            {port_index = 4, packet_num = 0}
        },
    },
    {
        name = "vxlan_nni_to_uni",
        send = {port_index = 3, packet = string.lower(nni_vxlan_ingress_packet)},
        check =
        {
            {port_index = 1, packet = string.lower(uni_vxlan_ingress_packet) },
            {port_index = 2, packet_num = 0},
            {port_index = 4, packet_num = 0}
        },
    },
    {
        name = "nvgre_uni_to_nni",
        send = {port_index = 2, packet = string.lower(uni_nvgre_ingress_packet)},
        check =
        {
            {port_index = 1, packet_num = 0},
            {port_index = 3, packet_num = 0},
            {port_index = 4, packet = string.lower(nni_nvgre_egress_packet),
                no_compare_bytes =
                { -- vlan untagged packet
                    {18, 19 } -- ipv4 header - 2 bytes 'identification field'
                   ,{24, 25 } -- ipv4 header - 2 bytes 'checksum'
                }
            }
        },
    },
    {
        name = "nvgre_nni_to_uni",
        send = {port_index = 4, packet = string.lower(nni_nvgre_ingress_packet)},
        check =
        {
            {port_index = 1, packet_num = 0},
            {port_index = 2, packet = string.lower(uni_nvgre_ingress_packet) },
            {port_index = 3, packet_num = 0}
        },
    },
}



