####################################################################
#   Simulation related definitions required to Build/link project
#   This file PSS/CPSS shared
####################################################################

# $(USER_BASE) -> $(LIB_BASE)

### handle the Simulation files
ifeq (EXISTS, $(RTOS_ON_SIM))
    SIM_LIB = $(COMPILATION_ROOT_FIX)/simulation/libs/$(CPU_DIR)/simulation.$(LIBEXT)
else
    SIM_LIB = $(wildcard $(COMPILATION_ROOT_FIX)/simulation/libs/$(CPU_DIR)/*.$(LIBEXT))
    ifeq (EXISTS_Linux, $(FORCE_GM_LION_USE)_$(OS_TARGET))
        SIM_EXTRA += $(LIB_BASE)/simulation/libs/GM/Linux/PpInterfaceLibs/lionB/liblionB_gm.a
    endif

##############################
#                 win32
##############################

    ifeq (win32, $(OS_RUN))
        #extra libs when ASIC is needed

        #allow the environmet to explicitly give path to SLAN/SHOST lib that is not from the 'Server'
        #this file ignores the SLAN/SHOST from other sources.
    ifneq (, $(FORCE_SHOST_EXPLICIT_FILE))
        shostLib = $(FORCE_SHOST_EXPLICIT_FILE)
    else
            shostLib = $(LIB_BASE)\simulation\libs\shost\$(SHOST_LIB_DIR)\SHOST.lib
    endif

    ifneq (, $(FORCE_SLAN_EXPLICIT_FILE))
        slanLib = $(FORCE_SLAN_EXPLICIT_FILE)
    else
        slanLib = $(LIB_BASE)\simulation\libs\slan\$(SHOST_LIB_DIR)\SLAN.LIB
    endif

        SIM_EXTRA += $(shostLib)
        SIM_EXTRA += $(slanLib)

        ifeq (EXISTS, $(IMPL_PRINT_CALL_STACK))
            SIM_EXTRA += $(LIB_BASE)\simulation\libs\shost\$(SHOST_LIB_DIR)\StackWalker.obj
        endif

        #SUB-20 library
        ifeq (EXISTS, $(FORCE_SUB20_USE))
          SIM_EXTRA += $(LIB_BASE)\simulation\libs\CM\VC\sub20.lib
        endif

        gmLib = NOT_USED
        WIN_GM=$(LIB_BASE)/simulation/libs/GM/vc$(VC_VER)
        #check if need to add to the simulation the GM lib
        ifeq (EXISTS, $(FORCE_GM_LION_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/lionb/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_XCAT2_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/xcat2/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_LION2_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/lion2/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBCAT2_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/bobcat2/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBCAT3_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/bobcat3/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_ALDRIN2_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/aldrin2/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_FALCON_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/falcon/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBCAT2_B0_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/bobcat2_b0/PPInterface.lib
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBK_CAELUM_USE))
            gmLib = $(WIN_GM)/PpInterfaceLibs/bobk/PPInterface.lib
        endif

        #allow the environmet to explicitly give path to GM lib that is not from the 'Server'
        #this file ignores the 'family type' of the GM
        ifneq (NOT_USED, $(gmLib))
            ifneq (, $(FORCE_GM_EXPLICIT_FILE))
                gmLib = $(FORCE_GM_EXPLICIT_FILE)
            endif
        endif

        ifneq (NOT_USED, $(gmLib))
            SIM_EXTRA += $(gmLib)
        endif


    endif

    ifeq (1, $(LINUX_SIM))

        gmLib = NOT_USED
        LINUX_GM=$(LIB_BASE)/simulation/libs/GM/Linux

        #check if need to add to the simulation the GM lib
        ifeq (EXISTS, $(FORCE_GM_BOBCAT2_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/bobcat2/libBobcat2_gm.a
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBCAT2_B0_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/bobcat2_b0/libBobcat2b0_gm.a
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBCAT3_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/bobcat3/libBC3Gm_gm.a
        endif
        ifeq (EXISTS, $(FORCE_GM_ALDRIN2_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/aldrin2/libAldrin2Gm_gm.a
        endif
        ifeq (EXISTS, $(FORCE_GM_FALCON_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/falcon/libFalconGm_gm.a
        endif
        ifeq (EXISTS, $(FORCE_GM_BOBK_CAELUM_USE))
            gmLib = $(LINUX_GM)/PpInterfaceLibs/bobk/libBobk_caelum_gm.a
        endif

        #allow the environmet to explicitly give path to GM lib that is not from the 'Server'
        #this file ignores the 'family type' of the GM
        ifneq (NOT_USED, $(gmLib))
            ifneq (, $(FORCE_GM_EXPLICIT_FILE))
                gmLib = $(FORCE_GM_EXPLICIT_FILE)
            endif
        endif
        ifneq (NOT_USED, $(gmLib))
            SIM_EXTRA += $(gmLib)
        endif
    endif


endif


