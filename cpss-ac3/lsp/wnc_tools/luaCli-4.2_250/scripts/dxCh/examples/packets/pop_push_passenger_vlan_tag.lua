--[[Test 2) Pop+Push:
Ingress: A passenger packet with 3 tags encapsulated in a tunnel.
Action: The packet is TT. The outer tag is popped and the inner tags are classified to tag0 and tag1. IPCL rule based on the values of tag0 and tag1 classifies the packet to an eVLAN. The packet is bridged to a target ePort.
Tag state: The egress tag state of the ePort is {tag1, tag0}. The ePort is associated with a Push Tag to passenger and with a Tunnel.
Egress: So a passenger packet with tags [x,y,z] encapsulated in tunnel, and the egress push tag [w], would be sent with tags [w,z,eVLAN] encapsulated in a new tunnel.]]--

--
-- pop push passenger vlan tag packet
--

--[[
0000  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 04
0010  88 47 00 11 10 15 00 22 21 15 00 09 0A 0B 0C 22
0020  00 0D 0E 0F 00 25 88 88 00 05 88 A8 00 06 81 00
0030  00 07 06 00 00 01 02 03 04 05 06 07 08 09 0A 0B
0040  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B
0050  1C 1D 1E 1F 20 21 49 0E 19 99
--]]

local pop_push_passenger_packet1 = ""..
"00010203340200040506071181000004"..
"8847001110150022211500090A0B0C22"..
"000D0E0F00258888000588A800068100"..
"00070600000102030405060708090A0B"..
"0C0D0E0F101112131415161718191A1B"..
"1C1D1E1F2021490E1999"

--[[ the 'egress packet'
0x0000 :  00 04 05 06 07 12 00 01 02 03 34 03 81 00 00 04
0x0010 :  88 47 00 33 31 15 00 09 0a 0b 0c 22 00 0d 0e 0f
0x0020 :  00 25 99 99 00 09 81 00 00 07 88 a8 00 08 06 00
0x0030 :  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
0x0040 :  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
0x0050 :  20 21
--]]

local egress_pop_push_passenger_packet1 = ""..
    "00".."04".."05".."06".."07".."12".."00".."01".."02".."03".."34".."03".."81".."00".."00".."04"..
    "88".."47".."00".."33".."31".."15".."00".."09".."0a".."0b".."0c".."22".."00".."0d".."0e".."0f"..
    "00".."25".."99".."99".."00".."09".."81".."00".."00".."07".."88".."a8".."00".."08".."06".."00"..
    "00".."01".."02".."03".."04".."05".."06".."07".."08".."09".."0a".."0b".."0c".."0d".."0e".."0f"..
    "10".."11".."12".."13".."14".."15".."16".."17".."18".."19".."1a".."1b".."1c".."1d".."1e".."1f"..
    "20".."21"

--[[
0000  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 04
0010  88 47 00 11 10 15 00 22 21 15 00 09 0A 0B 0C 22
0020  00 0D 0E 0F 00 25 88 88 00 05 77 66 00 06 99 88
0030  00 07 06 00 00 01 02 03 04 05 06 07 08 09 0A 0B
0040  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B
0050  1C 1D 1E 1F 20 21 56 B0 D4 53
--]]

local pop_push_passenger_packet2 = ""..
        "00010203340200040506071181000004"..
        "8847001110150022211500090A0B0C22"..
        "000D0E0F002588880005776600069988"..
        "00070600000102030405060708090A0B"..
        "0C0D0E0F101112131415161718191A1B"..
        "1C1D1E1F202156B0D453"

return  { ingress1=pop_push_passenger_packet1, egress1 =egress_pop_push_passenger_packet1, ingress2=pop_push_passenger_packet2 }
