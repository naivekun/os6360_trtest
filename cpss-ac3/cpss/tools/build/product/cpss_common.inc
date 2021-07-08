#
# CPSS common settings
#
##############################################################################
# CPSS related environment variables
##############################################################################
# USER_BASE - the directory where the CPSS code exists
# USER_MAKE - name of the make file to run for the different compilation flags
# COMPILATION_ROOT - directory to output the compiled fise and libs
#
# FA_VENDOR - the FA vendor                          -- empty / PRESTERA
# BSP_VENDOR - the vendor of the BSP                 -- empty / DUNE
#
# NO_PP_USED - is it non-PP compilation ?            -- empty / NOT_USED
# CHX_FAMILY - is it Cheetah-X compilation           -- empty / EXISTS
# CH3_FAMILY - is it Cheetah-3 extra compilation     -- empty / EXISTS
# CH3P_FAMILY - is it Cheetah-3P extra compilation   -- empty / EXISTS
# EXDXMX_DRIVER - do we need the ex/mx/dx cpssDriver -- empty / EXISTS
#
# GT_SMI - do we need SMI support                    -- empty / EXISTS
# GT_PCI - do we need PCI support                    -- empty / EXISTS
# GT_I2C - do we need I2C support                    -- empty / EXISTS
#
# GALTIS_NO_APP_DEMO - do we compile and build image for 'Galtis without AppDemo'
#
# **************************************************************************************
#

export USER_BASE=$CPSS_PATH
export USER_MAKE=presteraTopMake

#############################################################

case $FAMILY in
    DX )
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY CH3_FAMILY"
        export CHX_FAMILY="EXISTS"
        export CH3_FAMILY="EXISTS"
        ;;
    DXALL | DX_ALL)
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY CH3_FAMILY"
        export CHX_FAMILY="EXISTS"
        export CH3_FAMILY="EXISTS"
        #export CH3P_FAMILY="EXISTS"
        #export SAL_FAMILY="EXISTS"
        ;;
    DX_CHX )
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY"
        export CHX_FAMILY="EXISTS"
        ;;
    DX_CH3 )
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY CH3_FAMILY"
        export CHX_FAMILY="EXISTS"
        export CH3_FAMILY="EXISTS"
        ;;
    DX_CH3P )
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY CH3_FAMILY CH3P_FAMILY"
        export CHX_FAMILY="EXISTS"
        export CH3_FAMILY="EXISTS"
        export CH3P_FAMILY="EXISTS"
        ;;
    PX_ALL)
        PROJECT_DEFS="PX_FAMILY"
        export PX_FAMILY="EXISTS"
        $verbose && echo "PX_FAMILY =${PX_FAMILY}"
        MAKEFILE_FAMILY_SUFFIX=Px
        ;;
    DXPX )
        PROJECT_DEFS="$PROJECT_DEFS CHX_FAMILY CH3_FAMILY PX_FAMILY"
        export CHX_FAMILY="EXISTS"
        export CH3_FAMILY="EXISTS"
        export PX_FAMILY="EXISTS"
        MAKEFILE_FAMILY_SUFFIX=DxPx
        ;;
    NONE )
        export NO_PP_USED=NOT_USED
        ;;
    * )
        error_message "Unknown PP FAMILY $PP_FAMILY"
        return 1
        ;;
esac
MAKEFILE_FAMILY_SUFFIX=${MAKEFILE_FAMILY_SUFFIX:-Dx}

if [[ "$NO_PP_USED" != "NOT_USED" ]]; then
    PROJECT_DEFS="$PROJECT_DEFS IMPL_PP"
fi

if [[ ${CHX_FAMILY} = "EXISTS" ]]
then
    export EXDXMX_DRIVER="EXISTS"
    PROJECT_DEFS="$PROJECT_DEFS INCL_EXDXMX_DRIVER"
fi

# if MANAGEMENT_BUS_PARAM is not set by -b options
# By default we set MANAGEMENT_BUS_PARAM to PCI, except of Dx on ARM5181
if [ -z "$MANAGEMENT_BUS_PARAM" ]; then
    if [ "$CPU_BOARD" = "ARM5181" ] && [ "$CHX_FAMILY" = "EXISTS" ]
    then 
        MANAGEMENT_BUS_PARAM=PCI_SMI
     else
        MANAGEMENT_BUS_PARAM=$DEFAULT_MANAGEMENT_BUS
    fi
fi

case $MANAGEMENT_BUS_PARAM in
    PCI )
        PROJECT_DEFS="$PROJECT_DEFS GT_PCI"
        export GT_PCI="EXISTS"
        export MANAGEMENT_BUS=GT_MNG_PCI
        ;;
    SMI )
        PROJECT_DEFS="$PROJECT_DEFS GT_SMI"
        export GT_SMI="EXISTS"
        export MANAGEMENT_BUS=GT_MNG_SMI
        ;;
    PCI_SMI )
        PROJECT_DEFS="$PROJECT_DEFS GT_PCI GT_SMI"
        export GT_PCI="EXISTS"
        export GT_SMI="EXISTS"
        export MANAGEMENT_BUS=GT_MNG_PCI_SMI
        ;;
    PCI_SMI_I2C )
        PROJECT_DEFS="$PROJECT_DEFS GT_PCI GT_SMI GT_I2C"
        export GT_PCI="EXISTS"
        export GT_SMI="EXISTS"
        export GT_I2C="EXISTS"
        export MANAGEMENT_BUS=GT_MNG_PCI_SMI_I2C
        ;;
    PEX )
        PROJECT_DEFS="$PROJECT_DEFS GT_PCI"
        #export GT_PEX="EXISTS"
        export GT_PCI="EXISTS"
        ;;
    * )
        error_message "Unknown MANAGEMENT BUS $MANAGEMENT_BUS"
        return 1
        ;;
esac

export BIN_BS=128
export BIN_SKIP=1
export AUTO_INIT_SYS=ENABLE
unset  DPSS_CONFIG

# FA/XBAR defs
if [ "$FA_VENDOR" = "PRESTERA" ]; then
    PROJECT_DEFS="$PROJECT_DEFS IMPL_FA"
fi

# UT features
if [ "$INCLUDE_ENHANCED_UTF" = "YES" ]; then
	export IMPL_TGF=EXISTS
fi
export IMPL_TGF=${IMPL_TGF:-EXISTS}
if [ "$IMPL_TGF" = "EXISTS" ]; then
    PROJECT_DEFS="$PROJECT_DEFS IMPL_TGF"
fi


. $tool_build/product/simulation.inc
