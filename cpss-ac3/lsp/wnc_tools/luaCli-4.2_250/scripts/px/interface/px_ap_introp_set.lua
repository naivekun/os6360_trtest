--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ap_introp_set.lua
--*
--* DESCRIPTION:
--*     Set AP port introp information
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  ap_func
--        @description  set ap port introp information
--
--
--
--        @return       error message if fails
--
local function ap_introp_set_func(params)
    local command_data = Command_Data()
    local devNum, portNum, ret
    local GT_OK         = 0
    local apIntropPtr   = {}

    apIntropPtr.attrBitMask             = params.attrBitMask                    -- attrBitMask
    apIntropPtr.txDisDuration           = params.txDisDuration                  -- txDisDuration
    apIntropPtr.abilityDuration         = params.abilityDuration                -- abilityDuration
    apIntropPtr.abilityMaxInterval      = params.abilityMaxInterval             -- abilityMaxInterval
    apIntropPtr.abilityFailMaxInterval  = params.abilityFailMaxInterval         -- abilityFailMaxInterval
    apIntropPtr.apLinkDuration          = params.apLinkDuration                 -- apLinkDuration
    apIntropPtr.apLinkMaxInterval       = params.apLinkMaxInterval              -- apLinkMaxInterval
    apIntropPtr.pdLinkDuration          = params.pdLinkDuration                 -- pdLinkDuration
    apIntropPtr.pdLinkMaxInterval       = params.pdLinkMaxInterval              -- pdLinkMaxInterval


    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                        command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
            if(nil ~= devNum) then
                ret = myGenWrapper("cpssPxPortApIntropSet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PX_PORT_AP_INTROP_STC","apIntropPtr",apIntropPtr}})
                if (ret ~= GT_OK) then
                    local index = string.find(returnCodes[ret],"=")
                    print("Error at ap introp set command " .. devNum .. "/" .. portNum .. " : ".. string.sub(returnCodes[ret],index + 1))
                end
            end
        end
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: ap-introp set
--------------------------------------------
CLI_addHelp("interface", "ap-introp", "Set AP port introp information")
CLI_addCommand("interface", "ap-introp set", {
  func   = ap_introp_set_func,
  help   = "Set AP port introp information",
  params = {
        { type="named",
            { format="attrBitMask               %GT_U16", name="attrBitMask"            },
            { format="txDisDuration             %GT_U16", name="txDisDuration"          },
            { format="abilityDuration           %GT_U16", name="abilityDuration"        },
            { format="abilityMaxInterval        %GT_U16", name="abilityMaxInterval"     },
            { format="abilityFailMaxInterval    %GT_U16", name="abilityFailMaxInterval" },
            { format="apLinkDuration            %GT_U16", name="apLinkDuration"         },
            { format="apLinkMaxInterval         %GT_U16", name="apLinkMaxInterval"      },
            { format="pdLinkDuration            %GT_U16", name="pdLinkDuration"         },
            { format="pdLinkMaxInterval         %GT_U16", name="pdLinkMaxInterval"      },

            mandatory = {   "attrBitMask"           ,
                            "txDisDuration"         ,
                            "abilityDuration"       ,
                            "abilityMaxInterval"    ,
                            "abilityFailMaxInterval",
                            "apLinkDuration"        ,
                            "apLinkMaxInterval"     ,
                            "pdLinkDuration"        ,
                            "pdLinkMaxInterval"     }

    }}}
)

