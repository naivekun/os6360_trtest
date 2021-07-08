--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packetNumberToTransmit.lua
--*
--* DESCRIPTION:
--*       packet number to transmit type
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  check_packet_number
--        @description  ckecks packet number
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--
--        @return       true and portNum on success, otherwise false and
--                      error message
--


local function check_packet_number(param, name)
    local device,ports
	local temp
	local integral, fractional
	--local key, multiplier
    param = tonumber(param)
    if nil == param then
        return false, name .. " not a number"
    end
	
	if 0>param then
		return false, name.. " should be positive"
	end
	
	local sizeTable={1,256,512,1024,4096,65536,1048576,16777216}

	for key, multiplier in pairs(sizeTable) do
		temp=param/multiplier
		if temp>0 and temp<8192 then
			integral, fractional=math.modf(temp)
			if 0==fractional then
				return true, param
			end
			return false, "Cannot send "..param.." packets, closest number is "..multiplier*integral.." or ".. multiplier*(integral+1)
		end
		
	
	
	end
	
	return false, name.. " is too big"

end





-------------------------------------------------------
-- type registration: packetNumberTransmit
-------------------------------------------------------
CLI_type_dict["packetNumberTransmit"] =
{
    checker  = check_packet_number,
    help     = "Enter number of packets to transmit"
}
