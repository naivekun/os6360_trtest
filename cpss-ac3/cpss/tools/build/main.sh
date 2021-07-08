#!/usr/bin/env bash
#
# PSS/CPSS build script
#
# Enviroment variables:
# 1. Project depended
#   PSS_PATH                required for PSS* projects
#   CPSS_PATH               required for CPSS* projects and PSS_O_CPSS
#
# 2. OS depended
# 2.1. Linux BM
#   LINUX_DIST_PATH         required. Kernel sources should be found here,
#                           kernel will be compiled in this directory
#   LSP_USER_BASE           required.
#   LSP_SUBDIR            try $LSP_USER_BASE/$LSP_SUBDIR/$LINUX_LSP_NAME first
#                         Otherwise use $LSP_USER_BASE/$LINUX_LSP_NAME
#   LINUX_KERNEL_VERSION    (or DEFAULT_LINUX_KERNEL_VERSION from CPU defs)
#   LINUX_LSP_NAME          (or DEFAULT_LINUX_LSP_NAME from CPU defs)
#   LINUX_BUILD_KERNEL    set to YES to build kernel. Default is YES
#
# 2.2. vxWorks:
#   BSP_CONFIG_DIR          (Fx: Z:\cpss\Bsp\config)
#
#
# Options:
#   -a CPSS_LOG_YES|CPSS_LOG_NO
#                       Enable/disable CPSS API log. Default is CPSS_LOG_YES.
#
#   -b BUS              override management bus.
#                       Default is CPU specific
#
#   -C                  Do build in current directory only
#                       This option developed to be used with simple script
#                       Example  (bld_cpss_sim)
#                           #!/bin/bash
#                           export CPSS_PATH=/home/serg/tmp/Marvell/cpss
#                           $CPSS_PATH/Build.sh $1 \
#                               -u UTF_YES -L \
#                               Linux i386 LinuxWM \
#                               DX simulation CPSS_ENABLER \
#                               /home/serg/tmp/Marvell/objs -
#                       Run this script with no option to build whole project
#                       Add -C parameter to build in current directory only
#
#   -c                  Set COVERITY RUN
#
#   -D DEV | ENVBASE | CUST
#                       If DEV Run in developer mode - spawn an interactive shell
#                       after finishing of all steps (BSP copy, compile and link)
#
#                       If ENBASE - spawn an interactive
#                       shell after all environment set and host OS depended
#                       preparation actions is done
#
#                       If CUST - spawn an interactive shell
#                       after finishing compile and link
#
#   -d                  ClearCase baseline
#
#   -e                  LUA_CLI without examples
#
#   -E                  clean objects
#
#   -f  MAKE_FLAGS      Set make flags
#
#   -F FOLDER_PATH      folder path for compilation
#
#   -g                  force Golden Module Lib usage
#
#   -i                  Build CPSS log compilation type - info (as default)
#
#   -j MAKE_JOB_NUMBER  make file job number
#
#   -L                  Legacy "slow" compilation (no defaulf value)
#
#   -l                  lua CLI compilation and build
#
#   -N NOKERNEL|NOLINK|NOKERNEL_NOLINK
#                       no linux kernel build | no appDemo link
#
#   -O OBJ_FILE         Object file path for it's single compilation (no defaulf value)
#                       Example for windows:
#                            "mainPxDrv\src\cpss\px\port\cpssPxPortMapping.obj"
#   -o                  # Set SO LIB
#
#   -p                  Coverity Server Port Number
#
#   -P DEP_YES|DEP_NO   DEP_YES - image will be compiled with dependency files (as default) DEP_NO - without dependency files
#
#   -q                  Run script silently - only warning and error
#                       messages will be visible
#
#   -R                  The directory to copy uImage and rootfs.tgz
#
#   -S                  Build CPSS log compilation type - silent
#
#   -t TM_YES|TM_NO
#                       Incude TM. Default YES
#
#   -T toolkit          Choose toolkit.
#                       VC,eldk,GnuEabi,Tornado,gcc(native for simulation),etc
#                       refers $tool_build/toolkit/${toolkit_name}.sh
#
#                       Default toolkit for each supported OS defined in CPU file
#
#   -x XBAR             set XBAR type. Default NONE
#
#   -s SO_LIB           Build shared objects. Default NONE.
#
#   -U D_ON|D_OFF       D_ON - image will be compiled with debug information (as default) D_OFF - without debug
#
#   -u UTF_YES|UTF_NO
#                       Incude UTF. Default YES
#
#   -w                  Build CPSS log compilation type - verbose
#
# Parameters:
#   1. Host OS          Linux, WIN32, FreeBSD
#   2. Target CPU       PPC603, PPC6024, PPC603_VB, ARM_EV, ARM_RD, ARM_SP,
#                       ARM_GM, ARM5181, ARM5281, i386, etc
#                       refers $tool_build/cpu/${target_cpu}.sh
#   3. Target OS        Linux,vxWorks,uClinux,WIN32,FreeBSD
#                       refers $tool_build/os/${target_os}.sh
#
#   4. the prestera PP family (DX, SX, EX, MX, DXSX, PX, )
#   5. pp type: HW(BM), simulation, GM
#   6. product type: PSS, CPSS, POC, PSS_O_CPSS
#
#   7. compilation root
#   8. (image) destination path
##############################################
#  FILE VERSION: 23
##############################################
XBAR_TYPE=NONE
export INCLUDE_TM=YES
#export INCLUDE_UTF=YES
#export INCLUDE_ENHANCED_UTF=YES
export PROJECT_DEFS=""
export PROJECT_INCLUDES=""
export OS_DEFS=""
export OS_CFLAGS=""
export CPU_FLAGS=""
MANAGEMENT_BUS_PARAM=""
TOOLKIT=""
CC_BASELINE=""
COV_PORT="5500"
export FAST_COMPILE="YES"
export UNI_DIR=
mode=""
FORCE_GM=""
MAKE_FLAGS=""
LOG_COMPILATION_TYPE="-s"
export COMPILATION_TYPE=
MAKE_JOB_NUMBER=""
PARM_DEBUG_INFO=""
OBJ_FILE_PARM=""
export CLEAN_MODE=""
verbose=false

# $tool_build already defined in top level wrapper
all_args="$*"

#echo -E  " >>> all_args - ${all_args}"

while getopts ":ELiwqsSlevCosc:b:d:h:p:x:f:u:a:D:g:N:R:G:T:j:F:U:P:O:t:" opt;
do

    #echo -E "opt=$opt"
    case $opt in

    E ) # clean objects
         CLEAN_MODE="clean"
         if [ $verbose = true ]; then echo " >>> -E clean objects";fi
         ;;
    O ) #  Object file path for it's single compilation
        OBJ_FILE_PARM=${OPTARG}
        if [ $verbose = true ]; then echo " >>> -O Object file path  ${OBJ_FILE_PARM}";fi
        ;;
    P ) # dePendency: DEP_YES - image will be compiled with dependency files (as default) DEP_NO - without dependency files
        export DEPENDENCY=${OPTARG}
        if [ $verbose = true ]; then echo " >>> -P enable/disable compilation with dependency files ${DEPENDENCY}";fi
        ;;
    U ) # debUg: enable/disable compilation with debug information
        PARM_DEBUG_INFO=${OPTARG}
        if [ $verbose = true ]; then echo " >>> -U enable/disable compilation with debug information ${OPTARG}";fi
        ;;
    S ) # Set Build CPSS log compilation type - silent
         export COMPILATION_TYPE="silent"
         if [ $verbose = true ]; then echo " >>> -S Set Build CPSS log compilation COMPILATION_TYPE=${COMPILATION_TYPE}";fi
         ;;
    i ) # Set Build CPSS log compilation type - info
         echo " >>> -i Set Build CPSS log compilation - info"
         ;;
    w ) # Set Build CPSS log compilation - verbose
         LOG_COMPILATION_TYPE=""
         verbose=true
         if [ $verbose = true ]; then echo " >>> -w Build CPSS verbose";fi
         ;;
    j ) # Set MAKE_JOB_NUMBER
          MAKE_JOB_NUMBER="$OPTARG"
          if [ $verbose = true ]; then echo " >>> -j Set make file job number=${OPTARG}";fi
         ;;
    L ) # Set LEGACY
         export FAST_COMPILE=
         if [ $verbose = true ]; then echo " >>> -L LEGACY - YES";fi
         ;;
    C ) # Set BUILD_ONLY to (C)PSS relative path
        if [ $verbose = true ]; then echo " >>> -C Do build in current directory only";fi
        cur_dir="`(cd .;pwd)`"
        proj_dir="`(cd ${tool_build}/../..;pwd)`"
        if [ "$OSTYPE" = "Windows_NT" ]; then
            # lowercase $cur_dir, $proj_dir
            if [ \! -z "$ZSH_NAME" ]; then
                pth=${cur_dir##??}
                drive="${cur_dir%%${pth}}"
                cur_dir="${drive:l}${pth}"
                pth=${proj_dir##??}
                drive="${proj_dir%%${pth}}"
                proj_dir="${drive:l}${pth}"
            elif [ \! -z "$BASH" ]; then
                drive="${cur_dir:0:2}"
                cur_dir="${drive,,}${cur_dir:2}"
                drive="${proj_dir:0:2}"
                proj_dir="${drive,,}${proj_dir:2}"
            fi
        fi
        if [ "$cur_dir" != "${cur_dir#$proj_dir/}" ]; then
            BUILD_ONLY="${cur_dir#$proj_dir/}"
            export BUILD_ONLY
            echo -E "BUILD_ONLY=$BUILD_ONLY"
        fi
        if [ $verbose = true ]; then echo " >>> -C Set BUILD_ONLY to (C)PSS relative path proj_dir=${proj_dir} BUILD_ONLY=${BUILD_ONLY}";fi
        ;;
    T ) # Set toolkit
        export TOOLKIT="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -T Set toolkit TOOLKIT=${TOOLKIT}";fi
        ;;
    d ) # Set ClearCase baseline - for coverity description
        CC_BASELINE="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -d Set ClearCase baseline - CC_BASELINE=${OPTARG}";fi
        ;;
    p ) # Set Coverity Server Port Number
        COV_PORT="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -p Set Coverity Server Port Number - COV_PORT=${OPTARG}";fi
        ;;
    b ) # Set management bus
        MANAGEMENT_BUS_PARAM="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -T Set management MANAGEMENT_BUS_PARAM=${OPTARG}";fi
        ;;
    c ) # Set COVERITY RUN
        case $OPTARG in
        pss_dx_vc  | pss_dx_vx | pss_pm_vx | cpss_dx_vc  | cpss_dx_vx | cpss_pm_vx)
            RUN_PSS_CPSS_COVERITY=YES
            COV_PRODUCT="$OPTARG"
            if [ $verbose = true ]; then echo " >>> -T Set COVERITY RUN RUN_PSS_CPSS_COVERITY=${RUN_PSS_CPSS_COVERITY} COV_PRODUCT=${OPTARG}";fi
            ;;
        * )
            echo "Unsupported coverity case: $OPTARG passed with -c option."
            exit 1
            ;;
        esac
        if [ $verbose = true ]; then echo " >>> -c Set COVERITY RUN - RUN_PSS_CPSS_COVERITY=${RUN_PSS_CPSS_COVERITY} COV_PRODUCT=${OPTARG}";fi
        ;;
    l ) # set lua cli to include
         export CMD_LUA_CLI="yes"
         if [ $verbose = true ]; then echo " >>> -l set lua cli to include - CMD_LUA_CLI=${CMD_LUA_CLI}";fi
        ;;
    e ) # LUA_CLI without examples
         export NO_LUACLI_EXAMPLES="yes"
         if [ $verbose = true ]; then echo " >>> -e set lua cli to include - NO_LUACLI_EXAMPLES=${NO_LUACLI_EXAMPLES}";fi
        ;;
    N ) # no kernel build or no link
            case $OPTARG in
               NOKERNEL )
                  export LINUX_BUILD_KERNEL=NO
                  ;;
               NOLINK )
                  export DONT_LINK=YES
                  ;;
               NOKERNEL_NOLINK )
                  export LINUX_BUILD_KERNEL=NO
                  export DONT_LINK=YES
                  ;;
               esac
               if [ $verbose = true ]; then echo " >>> -N no kernel build or no link: $OPTARG";fi
               ;;

    R ) # The directory to copy uImage and rootfs.tgz
        if [ ! -d $OPTARG ]; then
            echo "The directory for uImage and rootfs doesn't exists: $OPTARG"
            exit 1
        fi
        TARGET_DIR_ROOTFS=$OPTARG
        if [ $verbose = true ]; then echo " >>> -R Set directory to copy uImage and rootfs.tgz TARGET_DIR_ROOTFS=${OPTARG}";fi
        ;;
    g ) # force GM lib usage
        # no galtis option

        FORCE_GM=${OPTARG^^}
        case $OPTARG in
         lion )
            export FORCE_GM_LION_USE="EXISTS"
            ;;
         lion2 )
            export FORCE_GM_LION2_USE="EXISTS"
            ;;
         xcat2 )
            export FORCE_GM_XCAT2_USE="EXISTS"
            ;;
         bobcat2 )
            export FORCE_GM_BOBCAT2_USE="EXISTS"
            ;;
         bobcat2_b0 )
            export FORCE_GM_BOBCAT2_B0_USE="EXISTS"
            ;;
         bobk_caelum )
            export FORCE_GM_BOBK_CAELUM_USE="EXISTS"
            ;;
         bobcat3 )
            export FORCE_GM_BOBCAT3_USE="EXISTS"
            ;;
         aldrin2 )
            export FORCE_GM_ALDRIN2_USE="EXISTS"
            ;;
         falcon )
            export FORCE_GM_FALCON_USE="EXISTS"
            ;;

        * )
            echo "Unsupported GM lib type: $OPTARG passed with -g option"
            exit 1
            ;;
        esac
        # force "no_stub_usage"
        export FORCE_GM_USE="EXISTS"
        if [ $verbose = true ]; then echo " >>> -g force GM lib usage FORCE_GM=${FORCE_GM}";fi
        ;;

    G )
        # no galtis option
      case $OPTARG in
        no_galtis )
           export EXCLUDE_GALTIS="EXCLUDE_LIB"
           ;;
        * )
            echo "Unsupported GALTIS option: $OPTARG passed with -G option"
            exit 1
           ;;
       esac
       if [ $verbose = true ]; then echo " >>> -G Set no galtis option EXCLUDE_GALTIS=${EXCLUDE_GALTIS}";fi
       ;;

    h ) # force GM lib usage
        case $OPTARG in
         sub20 )
            export FORCE_SUB20_USE="EXISTS"
            ;;
         * )
             echo "Unsupported option: $OPTARG passed as -h option"
             exit 1
             ;;
         esac
        if [ $verbose = true ]; then echo " >>> -h Set force force GM lib usage FORCE_SUB20_USE=${FORCE_SUB20_USE}";fi
        ;;
    o ) # Set SO LIB
        export SO_LIB=YES
          export EXCLUDE_GALTIS=EXCLUDE_LIB
        if [ $verbose = true ]; then echo " >>> -o Set SO LIB SO_LIB=${SO_LIB} EXCLUDE_GALTIS=${EXCLUDE_GALTIS}";fi
        ;;
    x ) # Set xbar
        XBAR_TYPE="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -x Set xbar XBAR_TYPE=${XBAR_TYPE}";fi
        ;;

    u ) # Set INCLUDE_UTF
        if [ $verbose = true ]; then echo " >>> -u Set INCLUDE_UTF - $OPTARG";fi
        case $OPTARG in
           NO | UTF_NO )
               export INCLUDE_UTF_OPTION=YES
               export INCLUDE_UTF=NO
               export INCLUDE_ENHANCED_UTF=NO
               #echo " >>>  UTF_NO"
           ;;
           YES | UTF_YES )
               export INCLUDE_UTF_OPTION=YES
               export INCLUDE_UTF=YES
               export INCLUDE_ENHANCED_UTF=YES
               #echo " >>>  UTF_YES"
           ;;
        * )
            echo "Unsupported UTF type: $OPTARG passed with -u option."
            exit 1
            ;;
        esac
        if [ $verbose = true ]; then echo " >>> -u Set INCLUDE_UTF=${INCLUDE_UTF}";fi
        ;;

    t ) # Set INCLUDE_TM
        echo " >>> -u Set INCLUDE_TM - $OPTARG"
        case $OPTARG in
           NO | TM_NO )
               export INCLUDE_TM=NO
               #echo " >>>  TM_NO"
           ;;
           YES | TM_YES )
               export INCLUDE_TM=YES
               #echo " >>>  TM_YES"
           ;;
        * )
            echo "Unsupported TM type: $OPTARG passed with -u option."
            exit 1
            ;;
        esac
        echo " >>> -u Set INCLUDE_TM=${INCLUDE_TM}"
        ;;


    a ) # enable/disable the CPSS API log
        case $OPTARG in
           CPSS_LOG_NO )
                export LOG_ENABLE="L_OFF"
                export API_LOG_ENABLE="NO" # any non-empty value is ok
                ;;
           CPSS_LOG_YES )
                export LOG_ENABLE="L_ON"
                export API_LOG_ENABLE=""   # should be emtpy
                ;;
        esac
        if [ $verbose = true ]; then echo " >>> -a enable/disable the CPSS API log LOG_ENABLE=${LOG_ENABLE} API_LOG_ENABLE=${API_LOG_ENABLE}";fi
        ;;

    f ) # Set make flags
        MAKE_FLAGS="$OPTARG"
        if [ $verbose = true ]; then echo " >>> -f Set make flags MAKE_FLAGS=${OPTARG}";fi
        ;;

    D )
        case $OPTARG in
        dev | DEV )  mode=developer  ;;
        envbase | ENVBASE ) mode=envbase ;;
        cust | CUST )  mode=customer  ;;
        * )
            echo "Unsupported argument: $OPTARG for -D option."
            exit 1
            ;;
        esac
        if [ $verbose = true ]; then echo " >>> -D mode=${mode} ";fi
        ;;
    q )
        SCRIPT_QUIET_MODE=YES
        if [ $verbose = true ]; then echo " >>> -q Set Run script silently  SCRIPT_QUIET_MODE=${OPSCRIPT_QUIET_MODETARG}";fi
        ;;
    s )
        export PSS_MODE=PSS_DPSS
        export DPSS_DIR=T2CDPSS
        if [ $verbose = true ]; then echo " >>> -s Build shared objects ${PSS_MODE} ${DPSS_DIR}";fi
        ;;

    * ) echo "Usupported option"
        exit 1
        ;;
    esac
    #echo "End of option case  "
done




if [ "$mode" = "" ]; then
   export environment_mode=developer
else
   export environment_mode=${mode}
fi

eval HOST_OS=\${$OPTIND}
shift $OPTIND
export CPU_BOARD=${1}
export TARGET_OS=${2}
FAMILY=${3}
export SIMULATION=${4}
export PRODUCT_TYPE=${5}
COMPILATION_ROOT="${6}"
IMAGE_PATH="${7}"

export BOARD_FAMILY=${FAMILY}

#echo "HOST_OS          - $HOST_OS   "
#echo "CPU_BOARD        - $CPU_BOARD"
#echo "TARGET_OS        - $TARGET_OS "
#echo "FAMILY           - $FAMILY "
#echo "SIMULATION       - $SIMULATION"
#echo "PRODUCT_TYPE     - $PRODUCT_TYPE   "
#echo "COMPILATION_ROOT - $COMPILATION_ROOT   "
#echo "IMAGE_PATH       - $IMAGE_PATH   "

#echo "THREAT_WARNINGS_AS_ERRORS = ${THREAT_WARNINGS_AS_ERRORS}   "


#echo "MAKEFILE_FAMILY_SUFFIX = ${MAKEFILE_FAMILY_SUFFIX}"
#-------------------------------------------------

if [ "$TARGET_OS" = "Linux" ]; then

   if [ "${PRODUCT_TYPE}" == "CPSS_LIB_SHARED" ]; then
      export PRODUCT_TYPE_FLAVOR=${PRODUCT_TYPE}
      export PRODUCT_TYPE=CPSS_ENABLER_SHARED_NOKM
      export SAVE_DONT_BUILD_IMAGES=${DONT_BUILD_IMAGES}
      export DONT_BUILD_IMAGES=YES
      echo "PRODUCT_TYPE_FLAVOR = ${PRODUCT_TYPE_FLAVOR}"

   fi

fi

#-------------------------------------------------
if [ "$THREAT_WARNINGS_AS_ERRORS" = "YES" ]; then
   if [ "$SIMULATION" = "simulation" ]; then
      if [ "$TARGET_OS" = "LinuxWM" ]; then

         gcc_version="$(gcc --version | head -n1 | cut -d" " -f4)"
         currentver="$(gcc -dumpversion)"
         requiredver=${requiredver:-"4.8"}

         if [ $verbose = true ]; then echo "currentver  = ${currentver}"  ;fi
         if [ $verbose = true ]; then echo "requiredver = ${requiredver}" ;fi

         if [ "$(printf '%s\n' "$requiredver" "$currentver" | sort -V | head -n1)" = "$requiredver" ]; then 
            if [ $verbose = true ]; then echo "gcc version ${gcc_version} greater than or equal to ${requiredver}" ;fi
         else
            if [ $verbose = true ]; then echo "gcc version ${gcc_version} less than ${requiredver}" ;fi
            unset THREAT_WARNINGS_AS_ERRORS
         fi

      fi
   fi
fi

#if [ $verbose = true ]; then echo "THREAT_WARNINGS_AS_ERRORS = ${THREAT_WARNINGS_AS_ERRORS}" ;fi
echo "THREAT_WARNINGS_AS_ERRORS = ${THREAT_WARNINGS_AS_ERRORS}"

if [ -z "${DEPENDENCY}" ]; then
   export DEPENDENCY=DEP_YES
fi

if [ \! -z "${PARM_DEBUG_INFO}" ]; then
   if [ "${TARGET_OS}" = "win32" ]; then
      export DEBUG_INFO=${PARM_DEBUG_INFO}
   else
      export DEBUG_INFO=${PARM_DEBUG_INFO}
      if [ "${PARM_DEBUG_INFO}" = "D_ON" ]; then
         export DEBUG_INFO=${PARM_DEBUG_INFO}
      else
         export DEBUG_INFO=
      fi
   fi
else
   if [ "${TARGET_OS}" = "win32" ]; then
      if [ -z "${DEBUG_INFO}" ]; then
         export DEBUG_INFO=D_ON
      fi
   fi
fi



if [ "${TARGET_OS}" = "win32" -a "${TOOLKIT}" = "VC10" ]; then
   if [ "${CPU_BOARD}" = "i386_64" ]; then
      export CPU_DIR=VC10_64
   else
      if [ "${FORCE_GM}" != "" ]; then
         export CPU_DIR=${TOOLKIT}_${FORCE_GM}
      else
         export CPU_DIR=${TOOLKIT}
      fi
   fi
else
      export CPU_DIR=${TOOLKIT}
fi

if [ -z ${NUM_CORES} ]; then
   if [ ! -z ${MAKE_JOB_NUMBER} ]; then
      NUM_CORES=${MAKE_JOB_NUMBER}
   else
      if [ "${TARGET_OS}" = "win32" ]; then
         if [ $verbose = true ]; then echo "main.sh 01 TARGET_OS  win32";fi
         NUM_CORES=${NUMBER_OF_PROCESSORS}
      else
         NUM_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
         if [ $verbose = true ]; then echo "main.sh 01 TARGET_OS ! win32";fi
      fi
   fi
fi

#----------------------------------------------------------------

$verbose && {
echo "main.sh 1 CPU_BOARD            =${CPU_BOARD}"
echo "main.sh 2 TARGET_OS            =${TARGET_OS}"
echo "main.sh 3 FAMILY               =${FAMILY}"
echo "main.sh 4 SIMULATION           =${SIMULATION}"
echo "main.sh 5 PRODUCT_TYPE         =${PRODUCT_TYPE}"
echo "main.sh 6 COMPILATION_ROOT     =${COMPILATION_ROOT}"
echo "main.sh 7 IMAGE_PATH           =${IMAGE_PATH}"
echo ""
echo "main.sh   TOOLKIT              =${TOOLKIT}"
echo "main.sh   CPU_DIR              =${CPU_DIR}"
echo "main.sh   COMPILATION_ROOT     =${COMPILATION_ROOT}"
echo "main.sh   environment_mode     =${environment_mode}"
echo "main.sh   NUM_CORES            =${NUM_CORES}"
echo "main.sh   LOG_COMPILATION_TYPE =${LOG_COMPILATION_TYPE}"
echo "main.sh   COMPILATION_TYPE     =${COMPILATION_TYPE}"
echo "main.sh   DEBUG_INFO           =${DEBUG_INFO}"
echo "main.sh   DEPENDENCY           =${DEPENDENCY}"
echo "main.sh   INCLUDE_TM           =${INCLUDE_TM}"

if [ "${CLEAN_MODE}" != "" ]; then
   echo "main.sh   CLEAN_MODE           =${CLEAN_MODE}"
fi

if [ "${OBJ_FILE_PARM}" != "" ]; then
   echo "main.sh   OBJ_FILE_PARM           =${OBJ_FILE_PARM}"
fi


echo ""
#echo "\n"

}

export OS_BUILD_STARTED=NO

###############################
# Set CPSS API_LOCK_PROTECTION
###############################
if [ -z $API_LOCK_PROTECTION ]; then
    export API_LOCK_PROTECTION="P_ON"
fi

###############################
# Set CPSS CPSS_USE_MUTEX_PROFILER
###############################
if [ -z $CPSS_USE_MUTEX_PROFILER ]; then
    export CPSS_USE_MUTEX_PROFILER="MP_OFF"
fi

###############################
# Set CPSS CPSS_USE_MUTEX_PROFILER
###############################
if [ -z $CPSS_TRAFFIC_API_LOCK ]; then
    export CPSS_TRAFFIC_API_LOCK="CPSS_TRAFFIC_API_LOCK_ENABLE"
fi

###############################
# Set PSS_PRODUCT_TYPE
###############################
if [ "$PRODUCT_TYPE" = "PSS" -o "$PRODUCT_TYPE" = "POC" ]; then
   PSS_PRODUCT_TYPE=${5}
else
   PSS_PRODUCT_TYPE="NONE"
fi



###############################
# no copy GM VC images to zip
# because IMAGE_PATH=""
###############################
#if [ "$FORCE_GM_USE" = "EXISTS" -o "$FORCE_SUB20_USE" = "EXISTS" ]; then
#   IMAGE_PATH=""
#fi

export COMPILATION_ROOT

###############################
trace_message()
{
    if [ \! -z "$SCRIPT_DEBUG_MODE" ]; then
        echo -E "$@"
    fi
    return 0
}

###############################
T()
{
    trace_message "$@"
    "$@"
}

###############################
info_message()
{
    if [ -z "$SCRIPT_QUIET_MODE" ]; then
        echo -E "$@"
    fi
    return 0
}
I()
{
    info_message "$@"
    "$@"
    return $?
}

warning_message()
{
    echo -E "$@"
    return 0
}

error_message()
{
    echo -E "$@" >&2
    return 0
}

##################################################
# declare all functions - they can be owerriden
##################################################

# host specific actions.
#   release unpacking, version control,
#   packing release, image, copying results, etc
host_shell()
{
    $SHELL
}

##################################################
host_pre_build()
{
    return 0
}

##################################################
host_post_build()
{
    return 0
}

##################################################
# target os specific actions
#   prepare to build (build kernel, libraries, rootfs, etc
#   build, link, make image
os_pre_build()
{
    return 0
}

##################################################
do_cleanup_if_required()
{
    if [ "${CLEAN_MODE}" == "clean" ]
    then
        export COMPILATION_MODE=clean
        if [ -d "${COMPILATION_ROOT}" ]; then
            rm -rf ${COMPILATION_ROOT}
        fi
        mkdir -p ${COMPILATION_ROOT}

    else
        if [ "${OBJ_FILE_PARM}" != "" ]
        then

            if [ "${TARGET_OS}" == "win32" ]; then
                OBJ_FILE_PARM=`win32_path_to_posix ${OBJ_FILE_PARM}`
            fi
            LAST_2_SYM=`echo "${OBJ_FILE_PARM}" | tail -c 3`
            #echo "LAST_2_SYM =${LAST_2_SYM}"
            if [ "${LAST_2_SYM}" == ".c" ];then
                OBJ_FILE_PARM=${OBJ_FILE_PARM%.c}.obj
            fi
            export COMPILATION_MODE=${COMPILATION_ROOT}/${OBJ_FILE_PARM}
        else
            export COMPILATION_MODE=link
        fi
    fi
    export OBJ_FILE_PARM=
}

pre_fast_build_win32()
{
    export OS_BUILD_STARTED=YES
    export COMPILATION_ROOT=`win32_path_to_posix "${COMPILATION_ROOT}"`

    case ${PRODUCT_TYPE} in
        CPSS_LIB)
                DONT_LINK=YES
                product_makefiles=Make_cpsa${MAKEFILE_FAMILY_SUFFIX}_Lib; ;;
        CPSS_ENABLER_SHARED | CPSS_ENABLER_SHARED_NOKM | CPSS_LIB_SHARED)
                error_message "The target $PRODUCT_TYPE not supported on Win32"
                return 1; ;;
        *)
                product_makefiles="${PRODUCT_MAKEFILES:-Make_cpss_${MAKEFILE_FAMILY_SUFFIX}}"; ;;
    esac
}
pre_fast_build_linux()
{
    if [ $verbose = true ]; then echo "main.sh LINUX_BUILD_KERNEL            =${LINUX_BUILD_KERNEL}";fi
    if [ $verbose = true ]; then echo "main.sh LINUX_NOKM                    =${LINUX_NOKM}";fi
    if [ $verbose = true ]; then echo "main.sh APP_DEMO_ONLY                 =${APP_DEMO_ONLY}";fi
    if [ "$ASIC_SIMULATION" != "EXISTS" -a -f $USER_BASE/freeRTOS/check_serviceCpuFw.sh ]; then
        # WA, to be moved to makefile
        if [ ${PRODUCT_TYPE} != CPSS_LIB_REFCODE ]; then
            ${USER_BASE}/freeRTOS/check_serviceCpuFw.sh
        fi
    fi

    case ${PRODUCT_TYPE} in
        CPSS_ENABLER_SHARED | CPSS_ENABLER_SHARED_NOKM)
                product_makefiles="Make_cpss_${MAKEFILE_FAMILY_SUFFIX} Make_cpss_Lnx_Lua"; ;;
        CPSS_LIB | CPSS_LIB_SHARED)
                DONT_LINK=YES
                product_makefiles=Make_cpss_${MAKEFILE_FAMILY_SUFFIX}_Lib; ;;
        *)
                product_makefiles="${PRODUCT_MAKEFILES:-Make_cpss_${MAKEFILE_FAMILY_SUFFIX}}"; ;;
    esac
    echo "product_makefiles = ${product_makefiles}"
}
fast_build()
{
    if [ "${TARGET_OS}" == "win32" ]; then
       pre_fast_build_win32 || return 1
    else # if [ "${TARGET_OS}" == "win32" ]; then
       pre_fast_build_linux || return 1
    fi # if [ "${TARGET_OS}" == "win32" ]; then
    GMAKE=${GMAKE:-${MAKE}}
    for mm in $product_makefiles; do
        info_message "main.sh $mm"
        I ${GMAKE} \
               -j ${NUM_CORES} \
               ${LOG_COMPILATION_TYPE} ${MAKE_FLAGS} \
               -C "${USER_BASE}" \
               -f $mm ${COMPILATION_MODE}

        if [ $? -ne 0 ]; then
           error_message -e "\tfailed to build $mm $USER_BASE"
           return 1
        fi
    done
}

os_build()
{
   # compile project
   if [ $verbose = true ]; then echo "main.sh *** os_build ***";fi
   do_cleanup_if_required
   if [ $verbose = true ]; then echo "COMPILATION_MODE  =${COMPILATION_MODE}";fi


   if [ "${FAST_COMPILE}" == "YES" ]; then
      fast_build || return 1
   else
      I ${MAKE} -j ${NUM_CORES} ${LOG_COMPILATION_TYPE} ${MAKE_FLAGS} \
            -C "${USER_BASE}" \
            -f ${USER_MAKE} \
            ${MAKE_TARGET:-full}
      if [ $? -ne 0 ]; then
         error_message -e "\tfailed to build $USER_BASE"
         return 1
      fi
   fi

   return 0
}

##################################################
os_pre_link()
{
    return 0
}

##################################################
os_link()
{
    return 0
}

##################################################
os_post_build()
{
    return 0
}

##################################################
# product specific actions
#
product_pre_build()
{
    if [ "$TARGET_OS" != "vxWorks" -o "$DRAGONITE_TYPE" != "A1" ]
    then
   dragoniteFwPath=${CPSS_PATH}/cpssEnabler/mainSysConfig/src/appDemo/dragonite/firmware
   if [ -e ${dragoniteFwPath}/drigonite_fw.s ]
   then
       info_message "Dragonite FW assembly delete:rm -f ${dragoniteFwPath}/drigonite_fw.s"
       rm -f ${dragoniteFwPath}/drigonite_fw.s
   fi
    fi

    return 0
}

##################################################
product_pre_link()
{
    return 0
}

##################################################
product_post_build()
{
    return 0
}

##################################################
# check parameters
check_param()
{
    if [ \! -f $tool_build/$1/$2.sh ]; then
        echo "Unknown $3: $2.sh not found $tool_build/$1/"
        exit 1
    fi
}

##################################################
check_param host ${HOST_OS} "host OS"
info_message "main HOST_OS=${HOST_OS}, including $tool_build/host/${HOST_OS}.sh"
. $tool_build/host/${HOST_OS}.sh

##################################################
check_param cpu ${CPU_BOARD} "cpu"
info_message "main CPU_BOARD=${CPU_BOARD}, including $tool_build/cpu/${CPU_BOARD}.sh"
. $tool_build/cpu/${CPU_BOARD}.sh
if [ -z "$TOOLKIT" ]
then
    # if no toolkit spefified then use default toolkit for this
    # CPU and target OS
    eval TOOLKIT=\${DEFAULT_TOOLKIT_${TARGET_OS}}
    if [ -z "$TOOLKIT" ]
    then
        echo "Toolkit not specified"
        exit 1
    fi
fi

##################################################
# after toolkit selected get value of $CPU_PRE_TOOLKIT_toolkit and execute it
# For example if TOOLKIT == vxWorks then eval results
# ${CPU_PRE_TOOLKIT_vxWorks}
# After that shell expands value of CPU_PRE_TOOLKIT_vxWorks and execute it
# If this variable is not defined then this will be evaluated to empty line
# and then shell will silently do nothing
##################################################

info_message "main eval \${CPU_PRE_TOOLKIT_${TOOLKIT}}"
eval \${CPU_PRE_TOOLKIT_${TOOLKIT}}

check_param toolkit ${TOOLKIT} "toolkit"
info_message "TOOLKIT=${TOOLKIT}, including $tool_build/toolkit/${TOOLKIT}.sh"
. $tool_build/toolkit/${TOOLKIT}.sh
info_message "TOOLKIT=${TOOLKIT} end"

check_param os ${TARGET_OS} "target os"
info_message "TARGET_OS=${TARGET_OS}, including $tool_build/os/${TARGET_OS}.sh"
. $tool_build/os/${TARGET_OS}.sh

info_message "main.sh check_param product ${PRODUCT_TYPE}"
check_param product ${PRODUCT_TYPE} "product type"

if [ -f $tool_build/quirks/${TOOLKIT}_${CPU_BOARD}.sh ]; then
    info_message "Applying toolkit==${TOOLKIT} + cpu=${CPU_BOARD} rules"
    . $tool_build/quirks/${TOOLKIT}_${CPU_BOARD}.sh
fi

# set default value of LOG_ENABLE if it wasn't set
export LOG_ENABLE=${LOG_ENABLE-L_ON}

# export host/target vars??

# product configuration based on parameters 6,7,8
info_message "PRODUCT_TYPE=${PRODUCT_TYPE}, including $tool_build/product/${PRODUCT_TYPE}.sh"
. $tool_build/product/${PRODUCT_TYPE}.sh

if [ "$PSS_PRODUCT_TYPE" != "POC" ]; then
   if [ "$INCLUDE_UTF_OPTION" != "YES" ]; then
      export INCLUDE_UTF=YES
      export INCLUDE_ENHANCED_UTF=YES
   fi
   if [ "$INCLUDE_UTF" = "YES" ]; then
      PROJECT_DEFS="$PROJECT_DEFS INCLUDE_UTF"
   fi
   if [ "$INCLUDE_ENHANCED_UTF" = "YES" ]; then
      PROJECT_DEFS="$PROJECT_DEFS INCLUDE_TGF"
   fi
fi

###############################
### Configured
###############################


###############################
### Run COVERITY
###############################

if [ "$RUN_PSS_CPSS_COVERITY" = "YES" ]; then

   echo "Run COVERITY"
   win32_add_paths $CPSS_TOOLS_PATH'\tools\bin'
   cd ${USER_BASE}

   info_message "rm -rf C:\\prevent\\"$COV_PRODUCT"\\*"
   rm -rf C://prevent//"$COV_PRODUCT"

   #compile for COVERITY CHECKING
   info_message "cov-build --dir C:\\prevent\\"$COV_PRODUCT" gmake -sf presteraTopMake full"
   cov-build --dir C:\\prevent\\"$COV_PRODUCT" gmake -f presteraTopMake full

   info_message "cov-analyze --checker-option DEADCODE:no_dead_default:true --dir  C:\\prevent\\"$COV_PRODUCT""
   cov-analyze --checker-option DEADCODE:no_dead_default:true --dir  C:\\prevent\\"$COV_PRODUCT"

   info_message "cov-commit-defects  --remote pt-pss01 --port "$COV_PORT" --description "$CC_BASELINE" --product "$COV_PRODUCT" --user admin --password password --dir C:\\prevent\\"$COV_PRODUCT""
   cov-commit-defects  --remote pt-pss01 --port "$COV_PORT" --description "$CC_BASELINE" --product "$COV_PRODUCT" --user admin --password password --dir C:\\prevent\\"$COV_PRODUCT"
   exit 0
fi

##################################################
trace_message "<= BEGIN host preparation"
host_pre_build || exit
trace_message "=> END host preparation"


##################################################
if [ "$mode" = "envbase" ]; then
    echo "Environment set, starting shell."
    echo 'Type "exit" to finish'
    host_shell
    exit 0
fi

##################################################
trace_message "<= BEGIN target OS preparation"
os_pre_build || exit
trace_message "=> END target OS preparation"

##################################################
trace_message "<= BEGIN product preparation"
product_pre_build || exit
trace_message "=> END product preparation"

#############################
# do build in customer mode
#############################
if [ "$RUN_COVERITY" = "YES" ]; then
   trace_message "<= BEGIN build"
   coverity_os_build || exit
   trace_message "=> END build"
   exit 0
fi

##################################################
#if [ "$mode" != "developer" -a "$mode" != "customer" ]; then
if [ "$mode" != "envbase" ]; then
   trace_message "<= BEGIN build"
   os_build || exit
   trace_message "=> END build"
fi

##################################################
if [ "$DONT_LINK" = YES ]; then
    exit 0
fi

##################################################
trace_message "<= BEGIN product link preparation"
product_pre_link || exit
trace_message "=> END product link preparation"

##################################################
trace_message "<= BEGIN target OS link preparation"
os_pre_link || exit
trace_message "=> END target OS link preparation"

##################################################
trace_message "<= BEGIN link"

if [ "${FAST_COMPILE}" = "YES" ]; then
   if [ "${TARGET_OS}" == "win32" ]; then
      if [ "${OS_BUILD_STARTED}" != "YES" ]; then
         os_build || exit
      fi
   fi
fi

if [ "${CLEAN_MODE}" != "clean" ]
then
   os_link || exit
fi

trace_message "=> END link"

##################################################
if [ "$RUN_COVERITY" = "YES" ]; then
    echo "End of COVERITY RUNNING - exit "
    exit 0
fi

##################################################
trace_message "<= BEGIN product post build phase"
product_post_build || exit
trace_message "=> END product post build phase"

##################################################
trace_message "<= BEGIN target OS post build phase"
os_post_build || exit
trace_message "=> END target OS post build phase"

##################################################
trace_message "<= BEGIN host post build phase"
host_post_build || exit
trace_message "=> END host post build phase"


##################################################

if [ "${PRODUCT_TYPE_FLAVOR}" == "CPSS_LIB_SHARED" ]; then
   unset PRODUCT_TYPE_FLAVOR
   export PRODUCT_TYPE=CPSS_LIB_SHARED
   export DONT_BUILD_IMAGES=${SAVE_DONT_BUILD_IMAGES}
   DONT_LINK=YES
   product_makefiles=Make_cpss_${MAKEFILE_FAMILY_SUFFIX}_Lib
   fast_build || return 1
fi

##################################################
if [ "${TARGET_OS}" = "win32" ]; then

   #############################################
   ## if defined developer mode or customer mode
   ## goto cmd shell
   #############################################
   if [ -z "$ROL_WM_BUILD_SYSTEM" ]; then
        if [ "$mode" = "developer" -o "$mode" = "customer" ]; then
            echo "Environment set, starting shell."
            echo 'Type "exit" to finish'
            host_shell
            exit 0
        fi
    fi
fi
