--
-- IPv4 packet
--

------------ IPv4 unicast packet with correct header--------------------------------
local cs = string.format("%4.4x", luaTgfCalculate16bitCheckSum({"45000064000000004000", "01010001010101010000"})) ;
local ipv4_uc_packet = ""..
        "000000010101"..             --mac da 00:00:00:01:01:01
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "810000c8"..                 --vlan 200
        "0800"..                     --ethertype
        "45000064000000004000"..cs.. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "01010001"..                 --source ip 1.1.0.1
        "01010101"..                 --destination ip 1.1.1.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F4041424344454647"..
        "48494A4B4C4D4E4F"

return string.lower(ipv4_uc_packet)
