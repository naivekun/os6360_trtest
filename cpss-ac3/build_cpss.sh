#!/usr/bin/env bash
##################################################################################
##  build_cpss.sh
##################################################################################
##
##  APPDEMO IMAGE BUILD WRAPPER
##
##################################################################################
## ARGUMENTS         :  Can be defined in any order
##
##     CPU and  PP_TYPE  are the mandatory parameters  -  the rest are optional
##
##      MANDATORY PARAMETERS
##      -----------------------------------
## CPU      : ARM5181 ARM5281 PPC603 PPC85XX EP3041A XCAT XCAT_BE i386 i386_64 XCAT_2.6.31
##            XCAT_BE_2.6.31 ARM78200 ARM78200_BE ARMADAXP ARMADAXP_BE XCAT_3.4 XCAT_3.4_BE
##            MIPS64 INTEL64 INTEL64_32 INTEL32 MSYS MSYS_2_6_32 ARMADAXP_3.4.69 ARMADA_385
##            MSYS_3_10 ARMADA_390 MSYS_BE ARMv7_LK_4_4 ARMv7_LK_3_14 ARMv7_LK_3_10 AARCH64v8
##
## PP_TYPE  : DX_ALL PX_ALL DXPX
##
##      OPTIONAL PARAMETERS
##      -------------------------------
##
## PRODUCT           :  CPSS_ENABLER  CPSS_LIB  CPSS_ENABLER_NOKM CPSS_BLOB CPSS_USE_MUTEX_PROFILER
##                      CPSS_ENABLER_SHARED CPSS_LIB_SHARED CPSS_ENABLER_SHARED_NOKM
##                      - default value CPSS_ENABLER
##
## UT                :  UTF_YES UTF_NO       - default value UTF_YES  ( build UT )
## TM                :  TM_YES TM_NO         - default value TM_YES   ( build TM )
## CPSS_LOG          :  CPSS_LOG_YES CPSS_LOG_NO - default value  CPSS_LOG_YES
##
## LUA               :  NOLUA (the same as LUA_NO) -  by default LUA code is included, NOLUA removes LUA support
##
## NO_EXAMPLES       :  remove examples from LUA code
##
## GALTIS            :  NOGALTIS - galtis code not used
##
## TOOLKIT           :  SDK_5_V5 SDK_5_V7 SDK_3.2 SDK_1.2.4 ELDK_3.1 YOCTO
##
## Build options     :  UNZIP   ENVBASE  CUST  SHLIB SO_LIB     - no defaulf value
##
## Build options     :  UNZIP   ENVBASE  CUST  SHLIB      - no defaulf value
##
## GM_LION           :  specify using of   GM libs for DX LION simulation image build
##
## GM_BOBCAT2        :  specify using of   GM libs for GM_BOBCAT2 simulation image build
##
## GM_BOBCAT2_B0     :  specify using of   GM libs for GM_BOBCAT2_B0 simulation image build
##
## GM_BOBCAT3        :  specify using of   GM libs for GM_BOBCAT3 simulation image build
##
## GM_ALDRIN2        :  specify using of   GM libs for GM_ALDRIN2 simulation image build
##
## GM_FALCON         :  specify using of   GM libs for GM_FALCON simulation image build
##
## NOKERNEL 		   :  Do not build kernel
##
## NOLINK			   :  Only build libs - do not try to link
##
## LEGACY|legacy     :  legacy "slow" compilation options (no defaulf value) 
##
## Build CPSS log compilation type
##                   :   silent|SILENT  - not output of compilation log
##                   :   info|INFO   - output c-files only in compilation log (as default)
##                   :   verbose|VERBOSE - full output compilation log
##
##  DEPENDENCY       : DEP_YES - image will be compiled with dependency files (as default) 
##                   : DEP_NO  - without dependency files 
##
##
## HIR               : Build App for Hot Insertion/Removal
##################################################
##
##  ENVIRONMENT VARIABLES
##
##  The following parameters can be define as environment variables:
##
## --------------------------------------------------------------------------------------
## variable                         default value
## --------------------------------------------------------------------------------------
##  WORKING_DIRECTORY             currect directory (pwd)
##  CPSS_PATH                     /cpss/SW/prestera
##  LINUX_DIST_PATH               $WORKING_DIRECTORY/linux
##  LSP_USER_BASE                 $WORKING_DIRECTORY/lsp
##  COMPILATION_ROOT              $WORKING_DIRECTORY/compilation_root
##  IMAGE_PATH                    $WORKING_DIRECTORY/compilation_root
##  LINUX_BUILD_KERNEL            ""
##  DONT_LINK                     ""
##  TARGET_DIR_ROOTFS             ""
## -----------------------------------------------------------------------------------------
##
##       OPTIONS:
##
##               UNZIP  - this option can be used by customer to:
##                                 - unzip CPSS components
##                                 - compile CPSS modules
##                                 - build CPSS appDemo
##
##
##               ENVBASE  - this option can be used by customer to set CPSS build environment
##
##
##      EXAMPLES:
##
##-----------------------------------------------------------------------------------------------
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC603
##
##              appDemo  goes to ./compilation_root , objects   in  ./compilation_root
##
##------------------------------------------------------------------------------------------------
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC85XX UNZIP
##
##          The following  zip files should be located in current working directory:
##              - CPSS-PP-DxCh***.zip
##              - CPSS-FA-Xbar***.zip
##              - CPSS-FX***.zip
##              - EnablerSuite***.zip
##              - ExtUtils***.zip
##              - GaltisSuite***.zip
##              - ReferenceCode***.zip
##              - UT*.zip
##              - lsp*.zip
##
##              The script :
##                  - unpack CPSS code to directory     ./unpack/cpss
##                  - unpack lsp to directory ./lsp
##                  - defines
##                              export  CPSS_PATH= ./unpack/cpss
##                              export  LSP_USER_BASE= ./unpack/lsp
##                  - build   DX_ALL   appDemo   in  ./compilation_root
##
##------------------------------------------------------------------------------------------------
##
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC85XX   ENVBASE
##             The script set CPSS build environment for defined CPU and PP_TYPE and
##             gives the prompt. In thia case a customer can work in the CPSS build
##             environment
##
##------------------------------------------------------------------------------------------------
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL  XCAT SHLIB
##              Build CPSS Shared Lib appDemo in developer mode
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL  XCAT SHLIB UNZIP
##              Build CPSS Shared Lib appDemo in client mode
##
##------------------------------------------------------------------------------------------------
##      No linux kernel build:
##             export LINUX_BUILD_KERNEL=NO
##      No appDemo link:
##             export DONT_LINK=NO
##      Directory to copy uImage and rootfs.tgz:
##             export TARGET_DIR_ROOTFS=path_Directory_to_copy
##
##          $Revision: 51 $
##
##------------------------------------------------------------------------------------------------
##
##  C_SINGLE_COMPILATION -  C file path for single compilation (no defaulf value) 
##                   for example:
##                       export C_SINGLE_COMPILATION=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.c
##
##  OBJ_FILE_PATH -  Object file path for it's single compilation (no defaulf value) 
##                   for example:
##                       export OBJ_FILE_PATH=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.o
##
##################################################################



#--------------------------------------------------

function Check_Baseline_Version ()
{
   WORKING_DIRECTORY=${WORKING_DIRECTORY:-`pwd`}
   ZIP_FILES=$(ls -r ${WORKING_DIRECTORY}/*.zip)
   BASE_LINE=''
   ZIP_FILE=''

   echo "----------------------------------------------------"
   echo "Zip files found:"
   echo "----------------------------------------------------"
   for line in ${ZIP_FILES}; do
       echo "${line}"
       if [ "${BASE_LINE}" = '' ]; then
           BASE_LINE=$(echo -n ${line} | tail -c 11)
           ZIP_FILE=${line}
       fi
       BL=$(echo -n ${line} | tail -c 11)

       if [ "${BASE_LINE}" != "${BL}" ]; then
           echo "**************************************************************"
           echo "*** Different baselines are used: ${BL} != ${BASE_LINE} ***"
           echo "**************************************************************"
           exit 1 
       fi
   done
   echo "----------------------------------------------------"
}

#--------------------------------------------------

function DurationTime ()
{
    STARTTIME=$1
    TN=$2

    d=$( date +%T )
    TN1="$(date +%s%N)"
    ENDTIME="$d"
    # Time interval in nanoseconds
    T="$(($(date +%s%N)-TN))"

    # Seconds
    S="$((T/1000000000))"
    # Milliseconds
    M="$((T/1000000))"
    Min="$((S/60%60))"  
    echo          ---$3------------
    echo   "Start    : $STARTTIME"
    echo   "Finish   : $ENDTIME"
    printf "Duration : %02d:%02d:%02d,%02d\n" "$((S/3600%24))" "$((S/60%60))" "$((S%60))" "$(($M - $S*1000))"
    echo          ---------------
}
#--------------------------------------------------
TN="$(date +%s%N)"
d=$( date +%T )
STARTTIME="$d"
echo "Start    : $STARTTIME $0"
GHIR_TYPE=0
#--------------------------------------------------

export FAST_KERNEL_COMP=TRUE

# set environment variables
if [ -z "${WORKING_DIRECTORY}" ]
then
    export WORKING_DIRECTORY=`pwd`
fi


# set LINUX_DIST_PATH
if [ -z $LINUX_DIST_PATH ]
then
    export LINUX_DIST_PATH="$WORKING_DIRECTORY/linux"
fi


# set COMPILATION_ROOT
if [ -z  $COMPILATION_ROOT ]
then
    export COMPILATION_ROOT="$WORKING_DIRECTORY/compilation_root"
    if [ ! -d $WORKING_DIRECTORY/compilation_root ]
    then
         mkdir -p $WORKING_DIRECTORY/compilation_root
    fi
    #echo "COMPILATION_ROOT=$WORKING_DIRECTORY/compilation_root"
fi

# set IMAGE_PATH
if [ -z  $IMAGE_PATH ]
then
    export IMAGE_PATH="$COMPILATION_ROOT"
fi

obj_dir=$COMPILATION_ROOT

# default settings
options=""
host_os=`uname`
target_cpu="unknown"
target_os=`uname`
pp_family="unknown"
pp_type="HW"
product="CPSS_ENABLER"
unpack_cpss="NO"
tm_opt="TM_YES"
utf_opt="UTF_YES"
cust_mode="NO"
endianess=""
bus_option=""
#environment_mode="DEV"
environment_mode=""
if [ "$LINUX_BUILD_KERNEL" = NO -o "$APP_DEMO_ONLY" = YES ]; then
    NOKERNEL=YES
fi

if [ -z  $DEPENDENCY ]
then
    export DEPENDENCY="DEP_YES"
fi

if [ "$IPC_TO_CPSS" == "y" ]; then
    export WNC_CPSS_IPC="yes"
fi

####################
# HELP option
####################
if [ "$1" = "help" ]; then
   echo ""
   echo "   COMMAND LINE FORMAT:"

   echo "      build_cpss.sh < Packet Processor > < CPU/Compiler > < Options >"
   echo ""
   echo "   CPU/Compiler:"
   echo "      ARMADAXP ARMADAXP_BE ARMADAXP_3.4.69 ARMADA_385 ARMADA_390"
   echo "      ARM5181 ARM5281 ARM78200 ARM78200_BE ARM78200RD ARM78200RD_BE "
   echo "      EP3041A INTEL64 INTEL64_32 INTEL32 i386 i386_BE MIPS64 MSYS MSYS_2_6_32 PPC603 PPC85XX"
   echo "      XCAT_3.4 XCAT_3.4_BE XCAT XCAT_BE XCAT_2.6.31 XCAT_BE_2.6.31 MSYS_3_10 MSYS_BE"
   echo ""
   echo "   Packet Processor:"
   echo "      DX_ALL"
   echo ""
   echo "   OPTIONS:"
   echo "      Toolkit options:"
   echo "         eldk gnueabi gnueabi_be gnu eldk_3.1 sdk_1.2.4 sdk_3.2 sdk_5_v5 sdk_5_v7 "
   echo ""
   echo "      UT options:"
   echo "         UT_NO         - UT code not included "
   echo ""
   echo "      TM options:"
   echo "         TM_NO         - TM code not included "
   echo ""
   echo "      LUA options:"
   echo "         NOLUA         - Lua code not included "
   echo ""
   echo "         NO_EXAMPLES   - remove examples from LUA code "
   echo ""
   echo "      Galtis options:"
   echo "         NOGALTIS      - Galtis code not included "
   echo ""
   echo "      GM options:"
   echo "         GM_LION GM_BOBCAT2 GM_BOBCAT2_B0 GM_BOBK_CAELUM GM_BOBCAT3 GM_ALDRIN2 GM_FALCON"
   echo ""
   echo "      Shared lib options:"
   echo "         shlib "
   echo ""
   echo "      Target product options:"
   echo "         CPSS_LIB CPSS_ENABLER CPSS_ENABLER_NOKM CPSS_BLOB CPSS_CHECK_CPP FREERTOS"
   echo ""
   echo "      SCRIPT MODE options:"
   echo "         UNZIP      - specifies the following steps: "
   echo "                       - unzip CPSS zip files to working directory "
   echo "                       - compile unziped source files "
   echo "                       - build CPSS appDemo "
   echo "         CUST       - specifies the following steps: "
   echo "                       - compile unziped source files "
   echo "                       - build CPSS appDemo "
   echo "     HIR options:"
   echo "         HIR     - specifies that Hot Insertion/Removal appDemo has to be built"
   echo ""
   echo "      Legacy slow compilation options:"
   echo "         LEGACY|legacy "
   echo "      Clean object options:"
   echo "         CLEAN|clean "

    exit 0
fi


####################
# for Linux lua by default is inside
####################
if [ "${target_os}" != FreeBSD ]; then
    lua_options="-l"
fi

# Build lua with examples by default
no_examples=


# Non-Recursive compilation by default
fast_compile_option=""

#----------------------------------------------------------------

for arg in "$@"
do
    case $arg in
      UNZIP | unzip)
         unpack_cpss="YES"
         environment_mode="CUST"
         ;;
      CUST | cust )
         cust_mode="YES"
         environment_mode="CUST"
         ;;
esac
done

#----------------------------------------------------------------
if [ ! -z ${environment_mode} ]
then

    # set CPSS_UNPACK directory
    if [ -z  $CPSS_UNPACK ]
    then
        export CPSS_UNPACK="${WORKING_DIRECTORY}"
    fi

    # set LSP_UNPACK directory
    if [ -z  $LSP_UNPACK ]
    then
        export LSP_UNPACK="${WORKING_DIRECTORY}/lsp"
    fi

    if [ -z  ${CPSS_PATH} ]
    then
        export CPSS_PATH=$CPSS_UNPACK/cpss
    fi

    if [ -z  ${LSP_UNPACK} ]
    then
        export LSP_USER_BASE=$LSP_UNPACK
    fi

    # set LSP_USER_BASE
    if [ -z  $LSP_USER_BASE ]
    then
        export LSP_USER_BASE="$WORKING_DIRECTORY/lsp"
    fi

    export LIB_BASE="$CPSS_PATH"

else # if [ ${environment_mode}  == "CUST" ]

       host_os="linux_marvell_cpss_dev"

       # For CC
       export RELEASE_DEP=/cpss/SW/prestera

       # set CPSS_PATH
       if [ -z $CPSS_PATH ]
       then
           #export CPSS_PATH="$WORKING_DIRECTORY/cpss"
           export CPSS_PATH=$(cd $(dirname $0) && pwd);
       fi

       export  CLONE_PATH=$(cd $(dirname $CPSS_PATH) && pwd);
       echo "CLONE_PATH=$CLONE_PATH"

       export  LINUX_CLONE_PATH=$(cd $(dirname $CLONE_PATH) && pwd);
       echo "LINUX_CLONE_PATH=$LINUX_CLONE_PATH"


fi # if [ ${environment_mode}  == "CUST" ]

echo "CPSS_PATH=${CPSS_PATH}"
if [ ! -d ${CPSS_PATH}/tools/build/cpu ]
then

   if [ -d ${CPSS_PATH}/cpss/tools/build/cpu ]
   then
      echo "Not found cpu folder: ${CPSS_PATH}/tools/build/cpu"
      echo "           but found: ${CPSS_PATH}/cpss//tools/build/cpu"
      echo "Please   - check parameters: CUST or UNZIP"
      exit 1
   else
      echo "Not found cpu folder: ${CPSS_PATH}/tools/build/cpu - Please check command line argument"
      exit 1
   fi

fi

#----------------------------------------------------------------


#----------------------------------------------------------------
for arg in "$@"
do
    if [ -f ${CPSS_PATH}/tools/build/cpu/${arg}.sh ]; then
        cpu_found=1

        if [ "${arg%_BE}" != "${arg}" -o "${arg%_BE_*}" != "${arg}" ]; then
            endianess="_be"
        fi

        if [ "${arg}" = ARM78200RD -o "${arg}" = ARM78200RD_BE ]
         then export LION_RD=1
        fi

        if [ "${arg}" = i386 -o "${arg}" = i386_64 ]; then
            target_os=${target_os}WM
            pp_type=simulation
        fi

         target_cpu=$arg
         if [ $target_cpu = i386 -o $target_cpu = i386_64 ]; then
             NOKERNEL=YES
         fi
        continue
    fi

    #echo "${arg} cpu_found=${cpu_found}"
    #----------------------------------------------------------------

   case $arg in

      DX_ALL | PX_ALL | DXPX)
         pp_family=$arg
         if [ $pp_family = "DX_ALL" -o $pp_family = "PX_ALL" -o $pp_family = "DXPX" ];then
            lua_options="-l"
         else
            lua_options=""
         fi
         if [ $pp_family = PX_ALL ]; then
            tm_opt=TM_NO
            options="$options -t TM_NO"
         fi
         ;;

      ELDK | eldk | GNUEABI | gnueabi | GNUEABI_BE | gnueabi_be | GNU | gnu )
         arg_lo=`echo $arg | tr 'A-Z' 'a-z'`
         options="$options -T $arg_lo"
         ;;

     ELDK_3.1 | eldk_3.1 )
         options="$options -T eldk$endianess"
         ;;

     YOCTO )
         options="$options -T yocto"
         ;;

     SDK_1.2.4 | sdk_1.2.4 )
         options="$options -T gnueabi$endianess"
         ;;

     SDK_3.2 | sdk_3.2 )
         options="$options -T mv5sft$endianess"
         ;;

     SDK_5_V5 | sdk_5_v5 )
         options="$options -T marvell_v5sft$endianess"
         ;;

     SDK_5_V7 | sdk_5_v7 )
         options="$options -T marvell_v7sft$endianess"
         ;;

      CPSS_ENABLER | CPSS_LIB | CPSS_ENABLER_NOKM | CPSS_BLOB | CPSS_CHECK_CPP|FREERTOS)
         product=$arg
         ;;
      CPSS_ENABLER_SHARED | CPSS_LIB_SHARED | CPSS_ENABLER_SHARED_NOKM | CPSS_USE_MUTEX_PROFILER|CPSS_TRAFFIC_API_LOCK_DISABLE)
         product=$arg
	     ;;

      UTF_NO | UTF_YES  )
         utf_opt=$arg
         options="$options -u $arg"
         ;;

      TM_NO | TM_YES  )
         tm_opt=$arg
         options="$options -t $arg"
         ;;

      CPSS_LOG_YES | CPSS_LOG_NO )
         options="$options -a $arg"
         ;;

      SO_LIB  )
                        so_opt=$arg
         options="$options -o $arg"
         ;;

      UNZIP | unzip)
         unpack_cpss="YES"
         ;;
      ENVBASE | envbase )
         #options="$options  -Denvbase"
         environment_mode=envbase
         ;;
      CUST | cust )
         cust_mode="YES"
         ;;
      SHLIB | shlib )
	     echo "The option $arg is deprecated. Just specify product:"
		 echo "CPSS_ENABLER_SHARED"
		 echo "CPSS_ENABLER_SHARED_NOKM"
		 echo "CPSS_LIB_SHARED"
         exit 1
         ;;
      NOLUA | LUA_NO )
         lua_options=
         ;;
      LEGACY | legacy)
         fast_compile_option="-L"
         ;;
      CLEAN | clean)
         options="$options -E"
         export C_SINGLE_COMPILATION=
         export OBJ_FILE_PATH=
         unset C_SINGLE_COMPILATION
         unset OBJ_FILE_PATH
         ;;

      NO_EXAMPLES )
         no_examples="-e"
         ;;
      NOGALTIS )
         galtis_options="NOGALTIS"
         options="$options  -Gno_galtis"
         ;;
      GM_LION )
         gm_options="GM_LION"
         export FORCE_GM_LION_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_BOBCAT2 )
         gm_options="GM_BOBCAT2"
         export FORCE_GM_BOBCAT2_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_BOBCAT2_B0 )
         gm_options="GM_BOBCAT2_B0"
         export FORCE_GM_BOBCAT2_B0_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_BOBK_CAELUM )
         gm_options="GM_BOBK_CAELUM"
         export FORCE_GM_BOBK_CAELUM_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_BOBCAT3 )
         gm_options="GM_BOBCAT3"
         export FORCE_GM_BOBCAT3_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_ALDRIN2 )
         gm_options="GM_ALDRIN2"
         export FORCE_GM_ALDRIN2_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      GM_FALCON )
         gm_options="GM_FALCON"
         export FORCE_GM_FALCON_USE="EXISTS"
         export FORCE_GM_USE="EXISTS"
         ;;
      NOKERNEL | NOKERNEL_NOLINK )
         options="$options -N $arg"
         NOKERNEL=YES
         ;;
      NOLINK )
         options="$options -N $arg"
         ;;
      DEP_YES | DEP_NO )
        options="$options -P $arg"
        export DEPENDENCY=$arg
        ;;
      silent | SILENT)
        options="$options -S"
        ;;
      info | INFO)
        options="$options -i"
        ;;
      verbose | VERBOSE)
        options="$options -w"
        ;;


      PCI | SMI | PCI_SMI | PCI_SMI_I2C)
        options="$options -b $arg"
        ;;

      HIR | hir)
      hir_type="YES"
      GHIR_TYPE=1
       ;;

      * )

        if [ "${cpu_found}" != 1 ]; then
            echo "Wrong command line argument - $arg"
            exit 1
        else
           echo "Wrong command line argument - $arg"
           exit 1
        fi
        ;;

   esac
done

export GHIR_TYPE


#----------------------------------------
#export C_SINGLE_COMPILATION=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.c
#export OBJ_FILE_PATH=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.obj

if [ -z ${OBJ_FILE_PATH} ]
then
	if [ ! -z ${C_SINGLE_COMPILATION} ]
	then
		OBJ_FILE_PATH="${C_SINGLE_COMPILATION/.c/.o}"
	fi
fi

if [ ! -z ${OBJ_FILE_PATH} ]
then
	options="$options -O ${OBJ_FILE_PATH}"

fi
export C_SINGLE_COMPILATION=
export OBJ_FILE_PATH=
unset C_SINGLE_COMPILATION
unset OBJ_FILE_PATH

#----------------------------------------
if [ ! -z ${environment_mode} ]
then
   options="$options  -D ${environment_mode}"
fi

#----------------------------------------
if [ -z ${environment_mode} ]
then

       # set LSP_USER_BASE
       if [ -z  $LSP_USER_BASE ]
       then
           #export LSP_USER_BASE="$WORKING_DIRECTORY/lsp"
           export  LSP_USER_BASE="$CLONE_PATH/lsp"
           if grep -q UNI_DIR ${CPSS_PATH}/tools/build/cpu/${target_cpu}.sh
           then
               export  LSP_USER_BASE="$LINUX_CLONE_PATH/uni"
           fi
       fi
       #echo "LSP_USER_BASE=$LSP_USER_BASE"

       if [ -z  $TARGET_OS ]
       then
           export TARGET_OS=Linux
       fi

       echo "WORKING_DIRECTORY=$WORKING_DIRECTORY"
       echo "CPSS_PATH=$CPSS_PATH"
       #echo "TARGET_OS=$TARGET_OS"

       if [ $TARGET_OS \!= "win32" -a TARGET_OS \!= "vxWorks" ]
       then

           eval LIB_BASE="/swdev/fileril103/Objects/cpss/bin/lib"
           export LIB_BASE="/swdev/fileril103/Objects/cpss/bin/lib"
           echo "LIB_BASE=$LIB_BASE"

           if [ -z  $NO_COPY_OBJECTS ]
           then
               export NO_COPY_OBJECTS="TRUE"
           fi

           ##################
           echo "CPSS_PATH=$CPSS_PATH"
           cd $CPSS_PATH

           export TMP_DIRECTORY=~/tmp/ctemp
           #if [ ! -d $TMP_DIRECTORY ]
           #then
                mkdir -p $TMP_DIRECTORY
           #fi

           cs_config_path=~/tmp/ctemp/git_cs_config.sh
           changed_files_path=~/tmp/ctemp/changed_files.txt

           TARGET_OS="Linux"

           LOG_LEVEL=60
           FILE_LOG_LEVEL=60

           if [ -z  $LOG_LEVEL ]
           then
               export LOG_LEVEL=10
           fi

           if [ -z  $FILE_LOG_LEVEL ]
           then
               export FILE_LOG_LEVEL=10
           fi

           if [ -z  $GET_GIT_MODE ]
           then
               export GET_GIT_MODE=1
           fi

           #echo "get_git_param.py" "$GET_GIT_MODE" "$TARGET_OS" "$CPSS_PATH" "$cs_config_path" "$changed_files_path" "$LOG_LEVEL" "$FILE_LOG_LEVEL"
           python ./get_git_param.py "$GET_GIT_MODE" "$TARGET_OS" "$CPSS_PATH" "$cs_config_path" "$changed_files_path" "$LOG_LEVEL" "$FILE_LOG_LEVEL"

           unset LOG_LEVEL
           unset FILE_LOG_LEVEL
           unset GET_GIT_MODE


           chmod 777  $cs_config_path
           .  $cs_config_path

           set_cs_parameters

           echo "CC_CURRENT_CHANGE_SET =$CC_CURRENT_CHANGE_SET"
           echo "CC_CURRENT_BASE_LINE  =$CC_CURRENT_BASE_LINE"
           echo "CC_RELEASE_STREAM     =$CC_RELEASE_STREAM"
        fi

fi 

#------------------------------------------------------------------------------------
# add options for build
options="$options $fast_compile_option $lua_options $no_examples"

# threat warnings as errors if i386 && !CUST && !UNZIP
if [ "$pp_type" == "simulation" -a "$cust_mode" != "YES" -a "$unpack_cpss" != "YES" ]; then
   export THREAT_WARNINGS_AS_ERRORS=YES
fi

if [ "$target_cpu" = "unknown" ]; then
   echo "CPU parameter required"
   exit 1
fi

if [ "$pp_family" = "unknown" ]; then
   echo "PP_TYPE parameter required"
   exit 1
fi


#echo "target_cpu   =${target_cpu}"
#echo "LSP_USER_BASE=${LSP_USER_BASE}"
#----------------------------------------------------------------

##################################################
##   check  :
##                  - directory LINUX_DIST_PATH exists
##                   - linux tarball exists in LINUX_DIST_PATH
##                   - busybox tarball exists in LINUX_DIST_PATH
##  in developer mode :
##                    - copy linux tarballs from server
##                    - copy busybox tarball from server
##################################################
if  [  -d  $LINUX_DIST_PATH   ]
then
    if [ $unpack_cpss  != "YES" ] &&  [  $cust_mode  != "YES"  ]
    then
        echo "Check linux tarballs in  $LINUX_DIST_PATH"

        (
            cd /swtools/devsources/kernel
            for f in linux-*; do
                test -e $LINUX_DIST_PATH/$f ||
                    ln -s /swtools/devsources/kernel/$f $LINUX_DIST_PATH/$f
            done
        )

        for bbversion in 1.01 1.2.1 1.25.0
        do
            bbtar=busybox-$bbversion.tar.bz2
            bbdist=/swtools/devsources/root_fs/files/busybox
            if [ -f $bbdist/$bbtar -a ! -e  $LINUX_DIST_PATH/$bbtar ]
            then
                echo "Copy busybox $bbversion tarball from server"
                echo "ln -s $bbdist/$bbtar $LINUX_DIST_PATH"
                ln -s $bbdist/$bbtar $LINUX_DIST_PATH
            fi
        done
    fi
fi

###############################
##  unpack   cpss and lsp zips
###############################
if [ $unpack_cpss  = YES ]
then

        ##########################
        # check  if  ALL zips  exist
        ########################

        Check_Baseline_Version

        FILES_TO_UNZIP="all"

        for file in ./Cpss-PP*.zip
        do
            if [ ! -f "$file" ]
            then
                echo "Not found $file"
                FILES_TO_UNZIP="not_all"
            fi
        done


        if [ $pp_family  = DX_ALL  ];then
            if [ ! -f  ./Cpss-PP-DxCh*.zip ]; then
                echo " >> Not found Cpss-PP-DxCh*.zip"
                FILES_TO_UNZIP="not_all"
            fi

        fi
        if [ $pp_family  = PX  ];then
            if [ ! -f  ./Cpss-PP-Px*.zip ]; then
                echo " >> Not found Cpss-PP-Px*.zip"
                FILES_TO_UNZIP="not_all"
            fi

        fi
        if [ $pp_family  = DXPX  ];then
            if [ ! -f  ./Cpss-PP-Px*.zip ]; then
                echo " >> Not found Cpss-PP-Px*.zip"
                FILES_TO_UNZIP="not_all"
            fi
            if [ ! -f  ./Cpss-PP-DxCh*.zip ]; then
                echo " >> Not found Cpss-PP-DxCh*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi


        if [ "$NOKERNEL" != YES ]; then
            if [ ! -f  ./lsp*.zip ]
            then
                echo " >> Not found lsp*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi

        if [ $pp_family  = EX  ];then
            if [ ! -f  ./Cpss-FX*.zip ]; then
                echo " >> Not found Cpss-FX*.zip"
                FILES_TO_UNZIP="not_all"
            fi
            if [ ! -f  ./Cpss-FA*.zip ];then
                echo " >> Not found Cpss-FA*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi

        if  [ $utf_opt = UTF_YES  ]; then
            if [ ! -f  ./UT*.zip ]; then
                echo " >> Not found UT*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi

        if  [ $tm_opt = TM_YES  ]; then
            if [ ! -f  ./TM*.zip ]; then
                echo " >> Not found TM*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi

        if  [ "$lua_options" = "-l"  ]; then
            if [ ! -f  ./LuaSuite*.zip ]; then
                echo " >> Not found LuaSuite*.zip"
                FILES_TO_UNZIP="not_all"
            fi
        fi

        # check EnablerSuite*.zip
        if [ ! -f  ./EnablerSuite*.zip ]; then
            echo " >> Not found EnablerSuite*.zip"
            FILES_TO_UNZIP="not_all"
        fi

        # check ExtUtils*.zip
        if [ ! -f  ./ExtUtils*.zip ];  then
            echo " >> Not found ExtUtils*.zip"
            FILES_TO_UNZIP="not_all"
        fi

        # check  ReferenceCode*.zip
        if [ ! -f  ./ReferenceCode*.zip ]; then
            echo " >> Not found ReferenceCode*.zip"
            FILES_TO_UNZIP="not_all"
        fi

        # check GaltisSuite*.zip
        if [ ! -f  ./GaltisSuite*.zip ]&&[ "$galtis_options" != "NOGALTIS" ]; then
            echo " >> Not found GaltisSuite*.zip"
            FILES_TO_UNZIP="not_all"
        fi

        # check Simulation*.zip
        if [ $target_cpu  = i386 -o $target_cpu  = i386_64 ]
        then

            for file in ./Simulation*.zip
            do
                if [ ! -f "$file" ]
                then
                    echo "Not found $file"
                    FILES_TO_UNZIP="not_all"
                fi
            done

            if [ $pp_family  = DX_ALL  -a  "$gm_options" = "GM_LION"  ]
            then
                if [ -f  ./GM_DX*.zip  ]
                then
                    echo "Not found GM_DX*.zip"
                    FILES_TO_UNZIP="not_all"
                fi
            fi


        fi

        if [ $FILES_TO_UNZIP != "all" ];then
            echo " >> ERROR - not all required zip files exist"
            exit 0
        fi



        ##########################
        # check  if  zips  exist and unpack
        ##########################

        if [ $pp_family  = DX_ALL  ];then
            if [ ! -d ${CPSS_PATH}/cpss/mainPpDrv ]
            then
               echo "Not found folder: ${CPSS_UNPACK}/cpss/mainPpDrv"
               if [ -f  ./Cpss-PP-DxCh*.zip ]; then
                 echo "unzip  -d $CPSS_UNPACK  -o  Cpss-PP-DxCh*.zip"
                       unzip  -d $CPSS_UNPACK  -o  Cpss-PP-DxCh*.zip
               else
                 echo "Not found Cpss-PP-DxCh*.zip"
                 exit 0
               fi
            fi
        fi

        if [ $pp_family  = PX  ];then
            if [ ! -d ${CPSS_PATH}/cpss/mainPpDrv ]
            then
               echo "Not found folder: ${CPSS_UNPACK}/cpss/mainPxDrv"
               if [ -f  ./Cpss-PP-Px*.zip ]; then
                 echo "unzip  -d $CPSS_UNPACK  -o  Cpss-PP-Px*.zip"
                       unzip  -d $CPSS_UNPACK  -o  Cpss-PP-Px*.zip
               else
                 echo "Not found Cpss-PP-Px*.zip"
                 exit 0
               fi
            fi
        fi

        if [ $pp_family  = DXPX  ];then
            if [ ! -d ${CPSS_PATH}/cpss/mainPpDrv ]
            then
               echo "Not found folder: ${CPSS_UNPACK}/cpss/mainPpDrv"
               if [ -f  ./Cpss-PP-DxCh*.zip ]; then
                 echo "unzip  -d $CPSS_UNPACK  -o  Cpss-PP-DxCh*.zip"
                       unzip  -d $CPSS_UNPACK  -o  Cpss-PP-DxCh*.zip
               else
                 echo "Not found Cpss-PP-DxCh*.zip"
                 exit 0
               fi
            fi
            if [ ! -d ${CPSS_PATH}/cpss/mainPpDrv ]
            then
               echo "Not found folder: ${CPSS_UNPACK}}/cpss/mainPxDrv"
               if [ -f  ./Cpss-PP-Px*.zip ]; then
                 echo "unzip  -d $CPSS_UNPACK  -o  Cpss-PP-Px*.zip"
                       unzip  -d $CPSS_UNPACK  -o  Cpss-PP-Px*.zip
               else
                 echo "Not found Cpss-PP-Px*.zip"
                 exit 0
               fi
            fi
        fi
        
        # unpack lsp*.zip
        if [ "$NOKERNEL" != YES ]
        then
            if [ -f  ./lsp*.zip ]
            then
                unzip  -d $LSP_UNPACK  -o  lsp*.zip
            else
                echo "Not found lsp*.zip"
                exit 0
            fi
        fi

        if [ $pp_family  = EX  ]
        then
            if [ -f  ./Cpss-FX*.zip ]
            then
                unzip  -d $CPSS_UNPACK/cpss  -o  Cpss-FX*.zip
            else
                echo "Not found Cpss-FX*.zip"
                exit 0
            fi
            if [ -f  ./Cpss-FA*.zip ]
            then
                unzip  -d $CPSS_UNPACK/cpss  -o  Cpss-FA*.zip
            else
                echo "Not found Cpss-FA*.zip"
                exit 0
            fi
        fi

        if  [ $utf_opt = UTF_YES  ]
        then
            if [ -f  ./UT*.zip ]
            then
                unzip     -d $CPSS_UNPACK/cpss   -o UT*.zip
            else
                echo "Not found UT*.zip"
                exit 0
            fi
        else
            echo "UTF_OPTION - $utf_opt  :  UT*.zip not unpacked"
        fi

        if  [ $tm_opt = TM_YES  ]
        then
            if [ -f  ./TM*.zip ]
            then
                unzip     -d $CPSS_UNPACK/cpss   -o TM*.zip
            else
                echo "Not found UT*.zip"
                exit 0
            fi
        else
            echo "UTF_OPTION - $utf_opt  :  UT*.zip not unpacked"
        fi

        if  [ "$lua_options" = "-l"  ]
        then
            if [ -f  ./LuaSuite*.zip ]
            then
                unzip     -d $CPSS_UNPACK/cpss   -o LuaSuite*.zip
            else
                echo "Not found LuaSuite*.zip"
                exit 0
            fi
        else
            echo "LUA_OPTIONS - $$lua_options  :  LuaSuite*.zip not unpacked"
        fi

        # unpack EnablerSuite*.zip
        if [ -f  ./EnablerSuite*.zip ]
        then
            unzip     -d $CPSS_UNPACK/cpss   -o  EnablerSuite*.zip
        else
            echo "Not found EnablerSuite*.zip"
            exit 0
        fi


        # unpack ExtUtils*.zip
        if [ -f  ./ExtUtils*.zip ]
        then
            unzip     -d $CPSS_UNPACK/cpss   -o ExtUtils*.zip
        else
            echo "Not found ExtUtils*.zip"
            exit 0
        fi

        # unpack ReferenceCode*.zip
        if [ -f  ./ReferenceCode*.zip ]
        then
           unzip     -d $CPSS_UNPACK/cpss -o ReferenceCode*.zip
        else
            echo "Not found ReferenceCode*.zip"
            exit 0
        fi

        # unpack embeddedCommands*.zip
        if [ -f  ./embeddedCommands*.zip ]
        then
           unzip     -d $CPSS_UNPACK/cpss -o embeddedCommands*.zip
        fi

        # unpack GaltisSuite*.zip
        if [ "$galtis_options" != "NOGALTIS" ];then

           if [ -f  ./GaltisSuite*.zip ];then
              unzip     -d $CPSS_UNPACK/cpss   -o GaltisSuite*.zip
           else
              echo "Not found GaltisSuite*.zip"
              exit 0
           fi
        fi

        if [ "$target_cpu"  = "i386"  -o "$target_cpu"  = "i386_64" ]
        then
            for file in ./Simulation*.zip
            do
              if [ -f "$file" ]
              then
                  echo "unzip  -d $CPSS_UNPACK/cpss  -o  $file"
                  unzip  -d $CPSS_UNPACK/cpss  -o  $file
                  continue
              else
                  echo "Not found $file"
                  exit 0
              fi
            done

            if [ $pp_family  = DX_ALL  -a  "$gm_options" = "GM_LION"  ]
            then
                if [ -f  ./GM_DX*.zip  ]
                then
                    unzip  -d $CPSS_UNPACK/cpss  -o  GM_DX*.zip
                else
                    echo "Not found GM_DX*.zip"
                    exit 0
                fi
            fi
        fi

    cd cpss
    chmod -R -c 777 *
    cd ..
fi

######################################################
##          CUST mode
##     should be used after UNZIP required  CPSS zip files
######################################################
if [ $cust_mode  = YES ]
then


    # check if CPSS was unziped
    if [ ! -d ${CPSS_PATH} ]
    then
        echo " CPSS not unziped "
        exit 0
    fi

    # check if  LSP was unziped

    if [ "$NOKERNEL" != YES ]
    then
        if [ ! -d $LSP_UNPACK ]
        then
            echo " LSP not unziped "
            exit 0
        fi
    fi
fi

#---------------------------------------------------------
# Enable parallel build
export MAKEFLAGS=${MAKEFLAGS:--j24}

# for CleaCase mode - to allow copy of objects from server
if [ $unpack_cpss  != "YES" ] &&  [  $cust_mode  != "YES"  ]
then
   host_os="linux_marvell_cpss_dev"
fi

########################################
## RUN BUILD.SH WITH DEFINED PARAMETERS
########################################

echo "--------------------------------------"
if [ ! -z ${environment_mode} ]
then
   echo "environment_mode  - ${environment_mode}"
fi

echo "WORKING_DIRECTORY - $WORKING_DIRECTORY"
echo "COMPILATION_ROOT  - $COMPILATION_ROOT"
echo "CPSS_PATH         - $CPSS_PATH"
echo "LINUX_DIST_PATH   - $LINUX_DIST_PATH"
echo "LINUX_CLONE_PATH  - $LINUX_CLONE_PATH"
echo "LSP_USER_BASE     - $LSP_USER_BASE"

echo "options    - $options   "
echo "host_os    - $host_os   "
echo "target_cpu - $target_cpu"
echo "target_os  - $target_os "
echo "pp_family  - $pp_family "
echo "pp_type    - $pp_type   "
echo "product    - $product   "
echo "obj_dir    - $obj_dir   "
echo "IMAGE_PATH - $IMAGE_PATH   "
echo "--------------------------------------"

echo "bash ${CPSS_PATH}/Build.sh \
   $options   \
   $host_os $target_cpu $target_os \
   $pp_family $pp_type $product \
   $obj_dir $IMAGE_PATH"

bash ${CPSS_PATH}/Build.sh \
   $options   \
   $host_os $target_cpu $target_os \
   $pp_family $pp_type $product \
   $obj_dir $IMAGE_PATH

DurationTime  ${STARTTIME} ${TN}  $0

unset NO_COPY_OBJECTS
unset C_SINGLE_COMPILATION
unset OBJ_FILE_PATH



