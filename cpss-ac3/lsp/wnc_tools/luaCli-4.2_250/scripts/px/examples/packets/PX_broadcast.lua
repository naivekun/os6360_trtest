--
-- broadcast packet generator
-- 
local px_broadcast_packet =
"ffffffffffff".. --DST MAC
"000000000001".. --SRC MAC
"5555"..         --Ethertype
"000102030405060708090a0b"..--PAYLOAD
"0c0d0e0f1011121314151617"..
"18191a1b1c1d1e1f20212223"..
"2425262728292a2b2c2d2e2f"..
"303132333435363738393a3b"..
"3c3d3e3f"

return px_broadcast_packet