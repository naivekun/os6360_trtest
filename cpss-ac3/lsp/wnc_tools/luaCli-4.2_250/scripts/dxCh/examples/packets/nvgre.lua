--
-- nvgre packet
--

local uni_mac_da   = "00112233445B";
local uni_mac_sa   = "000000240011";
local uni_mac_vtag = "810000C9";
local uni_eth_type = "4444";
local uni_l2_payload =
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"..
    "0123456789ABCDEF0123456789ABCDEF"

local nvgre_direction1_packet_from_port_1 =
    uni_mac_da .. uni_mac_sa .. uni_mac_vtag .. uni_eth_type .. uni_l2_payload;

local nni_mac_da_port3   = "000000B01A21";
local nni_mac_da_port4   = "000000B01A22";
local nni_mac_sa   = "000000102030";
local ipv4_eth_type = "0800"
local ipv4_hdr_part1 =  "4500" -- version 5, header length - words, DSCP 0, ECN - 0
-- ommited 16 bit total length
local ipv4_hdr_part2 =  ""
    .. "00000000" -- identification(16), flags(3), fragment offset(13)
    .. "402F" -- time to live(8), GRE-protocol(8)
local ipv4_hdr_checksum;
local ipv4_hdr_before_cs;
local ipv4_hdr_after_cs;
local nni_port3_local_ip  = "C0B0140A"
local nni_port3_remote_ip = "C0B0140B"
local nni_port4_local_ip  = "C0B0140A"
local nni_port4_remote_ip = "C0B0140C"

local nni_untagged_payload =
    uni_mac_da .. uni_mac_sa .. uni_eth_type .. uni_l2_payload;

local nni_ipv4_total_length = 0
    + 20 --[[IPV4 header--]]
    + 8 --[[NV_GRE header]]
    + math.floor(string.len(nni_untagged_payload) / 2)

local nni_nvgre_header = "200065580F424100"

--------------------------------------------------------------
local nvgre_direction1_expected_packet_to_port_2 = nni_untagged_payload;

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_ipv4_total_length)
    .. ipv4_hdr_part2;
ipv4_hdr_after_cs = nni_port3_local_ip .. nni_port3_remote_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nvgre_direction1_expected_packet_to_port_3 = ""
    .. nni_mac_da_port3 .. nni_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_nvgre_header
    .. nni_untagged_payload;

ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_ipv4_total_length)
    .. ipv4_hdr_part2;
ipv4_hdr_after_cs = nni_port4_local_ip .. nni_port4_remote_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nvgre_direction1_expected_packet_to_port_4 = ""
    .. nni_mac_da_port4 .. nni_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_nvgre_header
    .. nni_untagged_payload;

-- as nvgre_direction1_expected_packet_to_port_3, but swappes Source IP and destination IP
ipv4_hdr_before_cs = ipv4_hdr_part1
    .. string.format("%04X", nni_ipv4_total_length)
    .. ipv4_hdr_part2;
ipv4_hdr_after_cs = nni_port3_remote_ip .. nni_port3_local_ip
ipv4_hdr_checksum = luaTgfCalculate16bitCheckSum(
    {ipv4_hdr_before_cs, ipv4_hdr_after_cs});

local nvgre_direction2_packet_from_port_3 = ""
    .. nni_mac_da_port3 .. nni_mac_sa .. ipv4_eth_type
    .. ipv4_hdr_before_cs .. string.format("%04X", ipv4_hdr_checksum) .. ipv4_hdr_after_cs
    .. nni_nvgre_header
    .. nni_untagged_payload;


return
{
    direction1_from_port1      = string.lower(nvgre_direction1_packet_from_port_1),
    direction1_expected_port_2 = string.lower(nvgre_direction1_expected_packet_to_port_2),
    direction1_expected_port_3 = string.lower(nvgre_direction1_expected_packet_to_port_3),
    direction1_expected_port_4 = string.lower(nvgre_direction1_expected_packet_to_port_4),
    direction2_from_port3      = string.lower(nvgre_direction2_packet_from_port_3),
    direction2_expected_port_1 = string.lower(nvgre_direction1_expected_packet_to_port_2),
    direction2_expected_port_2 = string.lower(nvgre_direction1_expected_packet_to_port_2)
}


