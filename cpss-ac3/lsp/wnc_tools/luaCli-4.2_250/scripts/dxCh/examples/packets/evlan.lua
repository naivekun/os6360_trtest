local my_untagged_packet = ""..
"000000001122"..             --mac da 00:00:00:00:11:22
"000000001123"..             --mac sa 00:00:00:00:11:23
"0800"..                     --ethertype
"4500006400000000400094CB".. --ipv4 Header: timetoleave=0x40=64, protocol=0x11
"11121314"..                 --source ip 17.18.19.20
"C0A80101"..                 --destination ip 192.168.1.1
"000102030405060708090A0B".. --payload
"0C0D0E0F1011121314151617"..
"18191A1B1C1D1E1F20212223"..
"2425262728292A2B2C2D2E2F"..
"303132333435363738393A3B"..
"3C3D3E3F4041424344454647"..
"48494A4B4C4D4E4F"

return { my_untagged_packet = my_untagged_packet }
