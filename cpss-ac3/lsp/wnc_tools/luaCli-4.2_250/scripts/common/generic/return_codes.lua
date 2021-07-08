--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* return_codes.lua
--*
--* DESCRIPTION:
--*       return codes of cpss functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

returnCodes =         {
    [-1]    = "GT_ERROR = generic error",
    [0x00]  = "GT_OK = Operation succeeded",
    [0x01]  = "GT_FAIL = Operation failed",
    [0x02]  = "GT_BAD_VALUE = Illegal value",
    [0x03]  = "GT_OUT_OF_RANGE = Value is out of range",
    [0x04]  = "GT_BAD_PARAM = Illegal parameter in function called",
    [0x05]  = "GT_BAD_PTR = Illegal pointer value",
    [0x06]  = "GT_BAD_SIZE = Illegal size",
    [0x07]  = "GT_BAD_STATE = Illegal state of state machine",
    [0x08]  = "GT_SET_ERROR = Set operation failed",
    [0x09]  = "GT_GET_ERROR = Get operation failed",
    [0x0A]  = "GT_CREATE_ERROR = Fail while creating an item",
    [0x0B]  = "GT_NOT_FOUND = Item not found",
    [0x0C]  = "GT_NO_MORE = No more items found",
    [0x0D]  = "GT_NO_SUCH = No such item",
    [0x0E]  = "GT_TIMEOUT = Time Out",
    [0x0F]  = "GT_NO_CHANGE = The parameter is already in this value",
    [0x10]  = "GT_NOT_SUPPORTED = This request is not supported",
    [0x11]  = "GT_NOT_IMPLEMENTED = This request is not implemented",
    [0x12]  = "GT_NOT_INITIALIZED = The item is not initialized",
    [0x13]  = "GT_NO_RESOURCE = Resource not available (memory ...)",
    [0x14]  = "GT_FULL = Item is full (Queue or table etc...)",
    [0x15]  = "GT_EMPTY = Item is empty (Queue or table etc...)",
    [0x16]  = "GT_INIT_ERROR = Error occurred while INIT process",
    [0x1A]  = "GT_NOT_READY = The other side is not ready yet",
    [0x1B]  = "GT_ALREADY_EXIST = Tried to create existing item",
    [0x1C]  = "GT_OUT_OF_CPU_MEM = Cpu memory allocation failed.",
    [0x1D]  = "GT_ABORTED = Operation has been aborted.",
    [0x1E]  = "GT_NOT_APPLICABLE_DEVICE = API not applicable to device.",
    [0x1F]  = "GT_UNFIXABLE_ECC_ERROR = the CPSS detected ECC error that can't be fixed when reading from the memory which is protected by ECC. NOTE: relevant only when the table resides in the CSU, the ECC is used , and the CPSS emulates the ECC detection and correction for 'Read entry' operations",
    [0x20]  = "GT_UNFIXABLE_BIST_ERROR = Built-in self-test detected unfixable error",
    [0x21]  = "GT_CHECKSUM_ERROR = checksum doesn't fits received data",
    [0x22]  = "GT_DSA_PARSING_ERROR = DSA tag parsing error",
    [0x10017] = "GT_HW_ERROR = Hardware error",
    [0x10018] = "GT_TX_ERROR = Transmit operation not succeeded",
    [0x10019] = "GT_RCV_ERROR = Receive operation not succeeded",
    [0x1001E] = "GT_REDO = Re-perform the interrupt handling",
    [0x1001D] = "GT_OUT_OF_PP_MEM = PP memory allocation failed",
    [0x1001F] = "GT_STOP_INT = interrupts cannot be processed and should not be unmasked",
}


LGT_OK                      = 0x00  -- Operation succeeded
LGT_FAIL                    = 0x01  -- Operation failed
LGT_BAD_VALUE               = 0x02  -- Illegal value
LGT_OUT_OF_RANGE            = 0x03  -- Value is out of range
LGT_BAD_PARAM               = 0x04  -- Illegal parameter in function called
LGT_BAD_PTR                 = 0x05  -- Illegal pointer value
LGT_BAD_SIZE                = 0x06  -- Illegal size
LGT_BAD_STATE               = 0x07  -- Illegal state of state machine
LGT_SET_ERROR               = 0x08  -- Set operation failed
LGT_GET_ERROR               = 0x09  -- Get operation failed
LGT_CREATE_ERROR            = 0x0A  -- Fail while creating an item
LGT_NOT_FOUND               = 0x0B  -- Item not found
LGT_NO_MORE                 = 0x0C  -- No more items found
LGT_NO_SUCH                 = 0x0D  -- No such item
LGT_TIMEOUT                 = 0x0E  -- Time Out
LGT_NO_CHANGE               = 0x0F  -- The parameter is already in this value
LGT_NOT_SUPPORTED           = 0x10  -- This request is not support
LGT_NOT_IMPLEMENTED         = 0x11  -- This request is not implemented
LGT_NOT_INITIALIZED         = 0x12  -- The item is not initialized
LGT_NO_RESOURCE             = 0x13  -- Resource not available (memory ...)
LGT_FULL                    = 0x14  -- Item is full (Queue or table etc...)
LGT_EMPTY                   = 0x15  -- Item is empty (Queue or table etc...)
LGT_INIT_ERROR              = 0x16  -- Error occurred while INIT process
LGT_NOT_READY               = 0x1A  -- The other side is not ready yet
LGT_ALREADY_EXIST           = 0x1B  -- Tried to create existing item
LGT_OUT_OF_CPU_MEM          = 0x1C  -- Cpu memory allocation failed.
LGT_ABORTED                 = 0x1D  -- Operation has been aborted.
LGT_NOT_APPLICABLE_DEVICE   = 0x1E  --[[ API not applicable to device, can
                                         be returned only on devNum parameter  ]]
LGT_UNFIXABLE_ECC_ERROR     = 0x1F  --[[ the CPSS detected ECC error that can't
                                         be fixed when reading from the memory
                                         which is protected by ECC.
                                         NOTE: relevant only when the table resides
                                         in the CSU, the ECC is used and the CPSS
                                         emulates the ECC detection and correction
                                         for 'Read entry' operations  ]]
LGT_UNFIXABLE_BIST_ERROR    = 0x20  -- Built-in self-test detected unfixable error
LGT_CHECKSUM_ERROR          = 0x21  -- Checksum doesn't fits received data
LGT_DSA_PARSING_ERROR       = 0x22  -- DSA tag parsing error
