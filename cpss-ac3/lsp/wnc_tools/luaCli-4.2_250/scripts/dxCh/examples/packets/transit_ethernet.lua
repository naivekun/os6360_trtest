--[[ Test Description:
Ingress: ethernet packet with 2 vlans.
Action: The packet is not TT (transit).
        TTI rule based on the values of mac-sa, vlan0 and vlan1 directs the packet to the egress port and replace vlan0, vlan1.
Tag state: The egress tag state is {inner tag0, outer tag1}
Egress: ethernet packet with vlans [x,y] would be sent with vlans [a,b]. ]]--

--
-- transit ethernet packet with tti vlan manipulations
-- 

--[[
0000  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 05
0010  88 47 00 0A B1 15 00 09 0A 0B 0C 22 00 0D 0E 0F
0020  00 25 66 66 00 01 02 03 04 05 06 07 08 09 0A 0B
0030  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B
0040  1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B
0050  2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B
0060  3C 3D 3E 3F
--]]

local transit_ethernet_packet = ""..
      "000102033402"..            --mac da 00:01:02:03:34:02
      "000405060711"..            --mac sa 00:04:05:06:07:11
      "81000005"..                --vlan 5
      "81000006"..                --vlan 6
      "6666"..                    --ethertype 0x6666
      "000102030405060708090A0B"..--payload
      "0C0D0E0F1011121314151617"..
      "18191A1B1C1D1E1F20212223"..
      "2425262728292A2B2C2D2E2F"..
      "303132333435363738393A3B"..
      "3C3D3E3F"

--[[ the 'egress packet'
0x0000 :  00 01 02 03 34 02 00 04 05 06 07 11 81 00 00 07
0x0010 :  81 00 00 08 66 66 00 01 02 03 04 05 06 07 08 09
0x0020 :  0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19
0x0030 :  1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29
0x0040 :  2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39
0x0050 :  3a 3b 3c 3d 3e 3f
--]]

local egress_transit_ethernet_packet = ""..
      "00".."01".."02".."03".."34".."02".."00".."04".."05".."06".."07".."11".."81".."00".."00".."07"..
      "81".."00".."00".."08".."66".."66".."00".."01".."02".."03".."04".."05".."06".."07".."08".."09"..
      "0a".."0b".."0c".."0d".."0e".."0f".."10".."11".."12".."13".."14".."15".."16".."17".."18".."19"..
      "1a".."1b".."1c".."1d".."1e".."1f".."20".."21".."22".."23".."24".."25".."26".."27".."28".."29"..
      "2a".."2b".."2c".."2d".."2e".."2f".."30".."31".."32".."33".."34".."35".."36".."37".."38".."39"..
      "3a".."3b".."3c".."3d".."3e".."3f"

return  { ingress=transit_ethernet_packet, egress =egress_transit_ethernet_packet }
