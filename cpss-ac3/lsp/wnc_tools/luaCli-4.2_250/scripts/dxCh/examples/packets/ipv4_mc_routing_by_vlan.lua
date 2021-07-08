--
-- IPv4 packet
--

------------ IPv4 multicase packet with correct header--------------------------------
local cs = string.format("%4.4x", luaTgfCalculate16bitCheckSum({"45000064000000004000", "C0A80101E00101010000"})) ;
local ipv4_mc_packet = ""..
        "01005e010101"..             --mac da 01:00:5E:01:01:01
        "000405060711"..             --mac sa 00:04:05:06:07:11
        "81000064"..                 --vlan 100
        "0800"..                     --ethertype
        "45000064000000004000"..cs.. --ipv4 Header: timetoleave=0x40=64, protocol=0x0
        "C0A80101"..                 --source ip 192.168.1.1
        "E0010101"..                 --destination ip 224.1.1.1
        "000102030405060708090A0B".. --payload
        "0C0D0E0F1011121314151617"..
        "18191A1B1C1D1E1F20212223"..
        "2425262728292A2B2C2D2E2F"..
        "303132333435363738393A3B"..
        "3C3D3E3F4041424344454647"..
        "48494A4B4C4D4E4F"

return string.lower(ipv4_mc_packet)
