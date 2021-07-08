
#################################################################
#################################################################
###  Include path definitions                                 ###
#################################################################
#################################################################

#common
PATH_COMMON      = common/h
PATH_COMMON     += common/h/cpss/extServices/os
PATH_COMMON     += common/src/cpss/common/labServices/port/gop/src/port
PATH_COMMON     += common/h/cpss/common/labServices/port/gop

#mainPpDrv
PATH_PP_DRV = mainPpDrv/h

#mainPxDrv
PATH_PX_DRV = mainPxDrv/h

#labservices
PATH_LABSERVICE += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/port/gop/h
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/port/gop/h/port
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver/ddr3
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Silicon
PATH_LABSERVICE += mainPpDrv/src/cpss/generic/labServices/ddr/bap/h
ifneq (EXISTS, $(ASIC_SIMULATION))
PATH_LABSERVICE += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago
PATH_LABSERVICE += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/include
endif


PATH_SD28FIRMWARE = common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/sd28firmware
PATH_PP_88e1690     = mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include
PATH_PP_88e1690    += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/msApi
PATH_PP_88e1690    += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/platform
PATH_PP_88e1690    += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/driver

#mainExtUtils
PATH_EXT_UTILS   = mainExtUtils/h

ifeq (YES, $(INCLUDE_UTF))
#mainUT/utfTool
PATH_UTFTOOL     = mainUT/utfTool/h
#mainUT/mainPpDrv
PATH_UTF_PP_DRV  = mainUT/mainPpDrv/h
#mainUT/utfTraffic
PATH_UTFTRAFFIC  = mainUT/utfTraffic/h
#mainUT/mainPxDrv
PATH_UTF_PX_DRV  = mainUT/mainPxDrv/h
endif


#TM API
PATH_TM_API          = mainTmDrv/h
PATH_TM_API_PRV      = mainTmDrv/h/private
PATH_TM_API_CORE     = mainTmDrv/h/core
PATH_TM_API_PLATFORM = mainTmDrv/h/platform

PATH_SIM		     = $(sort $(SIM_INCLUDES))

#cpssEnabler - OS , sysconfig , extDrv , cmd
PATH_MAINOS     = cpssEnabler/mainOs/h
PATH_SYSCONFIG  = cpssEnabler/mainSysConfig/h
PATH_EXT_DRV    = cpssEnabler/mainExtDrv/h
PATH_EXT_MAC    = cpssEnabler/mainExtMac/h
PATH_PHY_DRV 	= cpssEnabler/mainPhy
PATH_MAINCMD    = cpssEnabler/mainCmd/h

#galtis
PATH_GALTIS     = mainGaltisWrapper/h


PATH_LUAWRAPPERS= mainLuaWrapper/h
PATH_LUA        = mainExtUtils/src/extUtils/luaCLI/lua-5.1/src
PATH_MXML       = mainExtUtils/h/extUtils/mxml


TM_API_PATH     = mainTmDrv/h

#TM API Private
TM_API_PATH_PRV     = mainTmDrv/h/private

#TM API Core
TM_API_PATH_CORE     = mainTmDrv/h/core

#TM API Platform
TM_API_PATH_PLATFORM     = mainTmDrv/h/platform
