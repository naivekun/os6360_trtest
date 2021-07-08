-- TCP options: |Hrd Len| Rsvd | URG | ACK | PSH | RST | SYN | FIN
--                4b      6b     1b    1b    1b    1b    1b    1b


------------TCP/UDP packets with SIP address equal to DIP address for sanity check test------------
local sipIsDipContent =
        "001122334455"..             --mac da 00:11:22:33:44:55
        "00A0BBCCDDEE"..             --mac sa 00:A0:BB:CC:DD:EE
        "0800"..                     --ethertype
        "4500002E000000004006F965".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C0A8000A"..                 --source ip 192.168.0.10
        "C0A8000A"..                 --destination ip 192.168.0.10
        "0001000700000080000000FF".. --source port=1, dest port=7, seq=0x80=128, ackSeq=0xFF=255
        "0502FFFFF956"..             --header len=5, options=0x2=SYN set, window size=0xFFFF=65535
        "000000000000000000000000"   --payload

------------TCP/UDP packets with source TCP/UDP port equal to destination TCP/UDP port------------
local tcpUdpSportIsDportContent =
        "001122334455"..             --mac da 00:11:22:33:44:55
        "00A0BBCCDDEE"..             --mac sa 00:A0:BB:CC:DD:EE
        "0800"..                     --ethertype
        "4500002E000000004006F96E".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C0A80001"..                 --source ip 192.168.0.1
        "C0A8000A"..                 --destination ip 192.168.0.10
        "0007000700000080000000FF".. --source port=7, dest port=7, seq=0x80=128, ackSeq=0xFF=255
        "0502FFFFF950"..             --header len=5, options=0x2=SYN set, window size=0xFFFF=65535
        "000000000000000000000000"   --payload

------------TCP packets with the TCP FIN flag set and the TCP ACK not set------------------------
local tcpFinWithoutAckContent =
        "001122334455"..             --mac da 00:11:22:33:44:55
        "00A0BBCCDDEE"..             --mac sa 00:A0:BB:CC:DD:EE
        "0800"..                     --ethertype
        "4500002E000000004006F96E".. --ipv4 Header: timetoleave=0x40=64, protocol=0x6
        "C0A80001"..                 --source ip 192.168.0.1
        "C0A8000A"..                 --destination ip 192.168.0.10
        "0001000700000080000000FF".. --source port=1, dest port=7, seq=0x80=128, ackSeq=0xFF=255
        "0501FFFFF957"..             --header len=5, options=0x1=FIN set, window size=0xFFFF=65535
        "000000000000000000000000"   --payload

return { sipIsDip = sipIsDipContent, tcpUdpSportIsDport = tcpUdpSportIsDportContent, tcpFinWithoutAck = tcpFinWithoutAckContent}
