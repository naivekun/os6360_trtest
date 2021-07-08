--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_type_prbs.lua
--*
--* DESCRIPTION:
--*       prbs types dec;arations
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes


--constants

--------------------------------------------
-- type registration: prbs_mode
--------------------------------------------

CLI_type_dict["prbs_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PRBS mode",
    enum = {
           ["regular"]      = {value="CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E",    help="REGULAR"     },
           ["prbs"]         = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS_E",       help="PRBS"        },
           ["zeros"]        = {value="CPSS_PX_DIAG_TRANSMIT_MODE_ZEROS_E",      help="ZEROS"       },
           ["ones"]         = {value="CPSS_PX_DIAG_TRANSMIT_MODE_ONES_E",       help="ONES"        },
           ["cyclic"]       = {value="CPSS_PX_DIAG_TRANSMIT_MODE_CYCLIC_E",     help="CYCLIC"      },
           ["7"]            = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS7_E",      help="PRBS 7"      },
           ["9"]            = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS9_E",      help="PRBS 9"      },
           ["15"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS15_E",     help="PRBS 15"     },
           ["23"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS23_E",     help="PRBS 23"     },
           ["31"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_PRBS31_E",     help="PRBS 31"     },
           ["1t"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_1T_E",         help="MODE 1T"     },
           ["2t"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_2T_E",         help="MODE 2T"     },
           ["5t"]           = {value="CPSS_PX_DIAG_TRANSMIT_MODE_5T_E",         help="MODE 5T"     },
           ["10t"]          = {value="CPSS_PX_DIAG_TRANSMIT_MODE_10T_E",        help="MODE 10T"    },
           ["dfet_raining"] = {value="CPSS_PX_DIAG_TRANSMIT_MODE_DFETraining",  help="DFETraining" }
    }
}

