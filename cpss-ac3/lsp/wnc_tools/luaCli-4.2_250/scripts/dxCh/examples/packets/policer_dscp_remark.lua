--
-- Policer Remarking DSCP packets
-- 
local policerDscpRemarkContent = ""..
"001122334455"..                   --mac da 00:11:22:33:44:55 
"00AABBCCDDEE"..                   --mac sa 00:AA:BB:CC:DD:EE 
"8100"..                           --ethertype 
"0065"..                           --vlan id 101
"0800"..                           --ethertype 
"453c0064000000004011947E"..       --ipv4 Header: dscp=15, totoalen=0x64=100, timetoleave=0x40=64, protocol=0x11 
"11121314"..                       --source ip 17.18.19.20 
"C0A80101"..                       --destination ip 192.168.1.1 
"000102030405060708090A0B"..       --payload 
"0C0D0E0F1011121314151617".. 
"18191A1B1C1D1E1F20212223".. 
"2425262728292A2B2C2D2E2F".. 
"303132333435363738393A3B".. 
"3C3D3E3F4041424344454647".. 
"48494A4B4C4D4E4F"     

local policerDscpRemarkExpectedContent = ""..
"001122334455"..                   --mac da 00:11:22:33:44:55 
"00AABBCCDDEE"..                   --mac sa 00:AA:BB:CC:DD:EE 
"0800"..                           --ethertype 
"455400640000000040119466"..       --ipv4 Header: dscp=21, totoalen=0x64=100, timetoleave=0x40=64, protocol=0x11 
"11121314"..                       --source ip 17.18.19.20 
"C0A80101"..                       --destination ip 192.168.1.1 
"000102030405060708090A0B"..       --payload 
"0C0D0E0F1011121314151617".. 
"18191A1B1C1D1E1F20212223".. 
"2425262728292A2B2C2D2E2F".. 
"303132333435363738393A3B".. 
"3C3D3E3F4041424344454647".. 
"48494A4B4C4D4E4F"     

return { policerDscpRemark = policerDscpRemarkContent, policerDscpRemarkExpected = policerDscpRemarkExpectedContent }
