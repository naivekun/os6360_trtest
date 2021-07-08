--
-- IPv4 packets
--

------------Sent IPv4 packet with correct header--------------------------------
local cs = string.format("%4.4x", luaTgfCalculate16bitCheckSum({"45000064000000004000", "C0A80101E10000010000"})) ;
local sent_correct_ipv4_packet = ""..
        "01005e000001"..             --mac da 01:00:5E:00:00:01
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000005"..                 --vlan 5
        "0800"..                     --ethertype
        "45000064000000004000"..cs.. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80101"..                 --source ip 192.168.1.1
        "E1000001"..                 --destination ip 225.0.0.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F4041424344454647"..
        "48494A4B4C4D4E4F"

local expected_vlan7_ipv4_packet = ""..
        "01005e000001"..             --mac da 01:00:5E:00:00:01
        "000102030405"..             --mac sa 00:04:05:06:07:11
        "81000007"..                 --vlan 7
        "0800"..                     --ethertype
        "45000064000000004000"..cs.. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80101"..                 --source ip 192.168.1.1
        "E1000001"..                 --destination ip 225.0.0.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F4041424344454647"..
        "48494A4B4C4D4E4F"

local expected_vlan6_ipv4_packet = ""..
        "01005e000001"..             --mac da 01:00:5E:00:00:01
        "000102030405"..             --mac sa 00:04:05:06:07:11
        "81000006"..                 --vlan 6
        "0800"..                     --ethertype
        "45000064000000004000"..cs.. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80101"..                 --source ip 192.168.1.1
        "E1000001"..                 --destination ip 225.0.0.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F4041424344454647"..
        "48494A4B4C4D4E4F"

return
{
    sent_correct_header_packet = string.lower(sent_correct_ipv4_packet),
    expected_header_packet1     = string.lower(expected_vlan7_ipv4_packet),
    expected_header_packet2     = string.lower(expected_vlan6_ipv4_packet)
}
