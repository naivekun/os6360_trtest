--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rxtx_reply_protocol.lua
--*
--* DESCRIPTION:
--*       rxtx reply protocol
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  rxtxReply
--        @description  The rx to tx implementation
--
--        @param rxdata         - received data
--
function rxtxReply(rxdata)
    sendPacket_devport(rxdata.devNum,rxdata.portNum, rxdata.packet.data)
end

protocolRxTxReply = {
    name="RxTxReply",
    func="rxtxReply",
    ident={
        noCpuPort = true -- don't do for CPU port to prevent flooding
    }
}
function proto_rxtx_reply_enable()
    print("Activating RxTx proto")
    protocolRxTxReply.disabled = nil
    protocolAdd("RxTxReply",protocolRxTxReply)
end
function proto_rxtx_reply_disable()
    protocolEnable("RxTxReply",false)
end
