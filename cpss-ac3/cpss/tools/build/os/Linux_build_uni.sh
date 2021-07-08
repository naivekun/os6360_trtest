#
# Build without LSP (patched source tree available)
#
# required environment:
#   LINUX_DIST_PATH
#   LSP_USER_BASE
#   LINUX_KERNEL_VERSION  (or DEFAULT_LINUX_KERNEL_VERSION from CPU defs)
#   LINUX_LSP_NAME        (or DEFAULT_LINUX_LSP_NAME from CPU defs)
#   LINUX_BUILD_KERNEL    set to YES to build kernel
#
#   UIMAGE_NO_COPY        don't copy uImage to rootfs
#
#   LSP_KERNEL_TYPE       uImage zImage Image, default zImage
#   LSP_KERNEL_CONFIG     oldconfig or config file name (3.4+)
#   LSP_KERNEL_DTB_NAME   .DTB file name
#   LSP_KERNEL_UDTB_NAME  uImage name for zImage + .dtb
#   DONT_CREATE_ROOTFS    Don't build busybox when == "YES" (for LSP devel)
#

#----------------------------------------------------------------
# Function os_linux_uni_configure_path
#
# Description:
#       Get path to UNI directory
#
os_linux_uni_configure_path()
{
    #TODO: use $LINUX_SRC_DIST ==  $LINUX_DIST_PATH/uni/$LINUX_LSP_NAME ???
    cd $LINUX_DIST_PATH

    info_message "LINUX_CLONE_PATH=${LINUX_CLONE_PATH}"
    info_message "CASE_BUILD_IMAGES=${CASE_BUILD_IMAGES}"
    info_message " >> UNI_DIR=${UNI_DIR} "

    if [ -z ${LINUX_CLONE_PATH} ]
    then
       if [ \! -z ${UNI_DIR} ]
       then
           LSP_USER_BASE=$LSP_USER_BASE/${UNI_DIR}
       fi
    else
       if [ \! -z ${CASE_BUILD_IMAGES} ]
       then
          if [ \! -z ${UNI_DIR} ]
          then
              LSP_USER_BASE=$LSP_USER_BASE/${UNI_DIR}
          fi
       fi
    fi

    # APPLY LSP_SUBDIR
    info_message " >> LSP_USER_BASE=${LSP_USER_BASE} "
    info_message " >> LSP_SUBDIR=${LSP_SUBDIR} "

    if [ "$LSP_SUBDIR" != "" -a -d $LSP_USER_BASE/$LSP_SUBDIR/$LINUX_LSP_NAME ]
    then
        LSP_USER_BASE=$LSP_USER_BASE/$LSP_SUBDIR
    fi
    info_message " >> LSP_USER_BASE=${LSP_USER_BASE}/${LSP_SUBDIR}"

    UNI_SRC_DIST=${LSP_USER_BASE}/${LINUX_LSP_NAME}
    export LINUX_SRC_DIST=${LSP_USER_BASE}/${LINUX_LSP_NAME}
    info_message " >> UNI_SRC_DIST=${UNI_SRC_DIST} "
}

#----------------------------------------------------------------
# Function os_linux_prepare_uni
#
# Description:
#       Copy kernel to build directory
#
os_linux_prepare_uni()
{
    info_message " >> os_linux_prepare_uni "
    os_linux_uni_configure_path
    export LINUX_SRC_DIST=${UNI_SRC_DIST}
    DIST=$LINUX_DIST_PATH/uni/$LINUX_LSP_NAME/${DEFCONFIG}
    if [ "$LINUX_CONFIG_NOPRESTERA" = "YES" ]; then
        DIST=${DIST%_defconfig}_noprestera_defconfig
    fi
    DIST_HOME=${DIST}

    # check if rebuild required
    config_file_uni=${LINUX_SRC_DIST}/arch/${CPU_TOOL}/configs/${DEFCONFIG}
    config_md5_uni="`cat ${config_file_uni} | md5sum | cut -d ' ' -f 1`"

    if [ -f $DIST/config_file ]; then
        config_md5="`cat ${DIST}/config_file | md5sum | cut -d ' ' -f 1`"
        if [ "${config_md5}" = "${config_md5_uni}" ]; then
            info_message -e "END os_linux_prepare_uni -> config unchanged"
            return 0
        fi
    fi
    rm -rf $DIST
    mkdir -p $DIST ||
    {
        error_message "can't mkdir $DIST"
        return 1
    }

    cp ${config_file_uni} ${DIST}/config_file

    return 0
}

#----------------------------------------------------------------
# Function os_linux_copy_kernel
#
# Description:
#       Copy kernel to build directory
#
os_linux_copy_kernel()
{

    info_message " >> os_linux_copy_kernel "
    os_linux_uni_configure_path

    if [ \! -d $CPU_BOARD/$LINUX_LSP_NAME ]
    then
        mkdir -p $CPU_BOARD/$LINUX_LSP_NAME ||
        {
            error_message "can't mkdir $CPU_BOARD/$LINUX_LSP_NAME"
            return 1
        }
    fi

    if [ \! -d $DIST ]
    then
        mkdir -p $DIST ||
        {
            error_message "can't mkdir $DIST"
            return 1
        }
    fi

    # kernel not yet compiled or lsp doesn't match
    trace_message -e "\trm -rf "$DIST
    rm -rf $DIST/.[^.]* $DIST/*

    cd $CPU_BOARD/$LINUX_LSP_NAME
    if [ ! -d $LINUX_DIST_PATH ]
    then
      mkdir $LINUX_DIST_PATH
    fi
    info_message " >> LINUX_DIST_PATH=${LINUX_DIST_PATH} "

    cd $LINUX_DIST_PATH

    #Dist O.K ?
    if [ ! -d $DIST ]
    then
      error_message -e "\tDISTRIBUTION NAME ?"
      return 1
    fi

    #Apply the LSP
    #trace_message  "cp -rf --reply=yes ${LSP_USER_BASE}/${LINUX_LSP_NAME}/* ${DIST}"
    #cp -rf --reply=yes ${LSP_USER_BASE}/${LINUX_LSP_NAME}/* ${DIST}

    trace_message  "yes | cp -rpfL ${LSP_USER_BASE}/${LINUX_LSP_NAME}/* ${DIST}"

    info message " >> yes | cp -rpfL ${LSP_USER_BASE}/${LINUX_LSP_NAME}/* ${DIST} "
    yes | cp -rpfL ${LSP_USER_BASE}/${LINUX_LSP_NAME}/* ${DIST}

    if [ $? -ne 0 ]
    then
        error_message -e "\tCopy kernel failed"
        return 1
    else
        info_message -e "\tCopy kernel...done"
    fi

    LINUX_SRC_DIST=$DIST
    return 0
}

#----------------------------------------------------------------
# Function os_linux_kernel_is_already_built
#
# Description:
#       Check if kernel alredy built
#
# Returns
#       0 if yes
#       1 if not built
#
os_linux_kernel_is_already_built()
{
    if [ \! -f $DIST/.kernel_already_built ]; then
        return 1
    fi
    local vers_built="`cat $DIST/.kernel_already_built`"
    local vers_lsp=""
    if [ -f $LINUX_SRC_DIST/git_HEAD ]; then
        vers_lsp="`cat $LINUX_SRC_DIST/git_HEAD`"
    elif [ -f $LINUX_SRC_DIST/.git ]; then
        vers_lsp="`cd $LINUX_SRC_DIST;git rev-parse HEAD`"
    fi
    if [ "$vers_built" == "$vers_lsp" ]; then
        info_message "Kernel already built, skip"
        return 0
    fi
    return 1
}
#----------------------------------------------------------------
# Function os_linux_kernel_mark_already_built
#
# Description:
#       Mark kernel as already built
#
os_linux_kernel_mark_already_built()
{
    local vers_lsp=""
    if [ -f $LINUX_SRC_DIST/git_HEAD ]; then
        vers_lsp="`cat $LINUX_SRC_DIST/git_HEAD`"
    elif [ -f $LINUX_SRC_DIST/.git ]; then
        vers_lsp="`cd $LINUX_SRC_DIST;git rev-parse HEAD`"
    fi
    if [ "$vers_lsp" != "" ]; then
        echo $vers_lsp >$DIST/.kernel_already_built
    fi
}



#----------------------------------------------------------------
# Function os_pre_build_impl
#
# Description:
#       Build kernel and rootfs for target:
#       1. Build zImage
#       2. Build .dtb file if required
#       3. if zImage build uImage based on .dtb and zImage
#
os_pre_build_int()
{
    # build linux kernel
    #os_linux_copy_kernel || return 1
    #OBJS=
    # Build objects in serarate directory (common for MSYS_3_10 and AMC_385)
    # uni/ makefiles need to be fixed first
    os_linux_prepare_uni || return 1
    OBJS="O=${DIST}"

    if os_linux_kernel_is_already_built ; then
        # check if all required DTB's exists
        all_dtbs=1
        for dtb in ${LSP_KERNEL_DTB_NAME[*]}; do
            test -f $DIST/arch/arm/boot/dts/$dtb || all_dtbs=0
        done
        if [ "$all_dtbs" != "1" ]; then
            # WA: till dts ch-sets merged
            # When merged - remove files
            extra_dts_dir=$tool_build/toolkit/${TOOLKIT}/dts/$LINUX_KERNEL_VERSION
            if [ -d $extra_dts_dir -a $CPU_TOOL = arm ]; then
                cp -f $extra_dts_dir/* ${LINUX_SRC_DIST}/arch/$CPU_TOOL/boot/dts
            fi
            I make ${MAKE_FLAGS} ${OBJS} -C ${LINUX_SRC_DIST} ${LSP_KERNEL_DTB_NAME[*]} $mkopts
        fi
        if [ "$CPU_TOOL" != "arm64" ]; then
            all_uimages=1
            for uimg in ${LSP_KERNEL_UDTB_NAME[*]}; do
                test -f $DIST/arch/arm/boot/uImage_$uimg || all_uimages=0
            done
            if [ "$all_uimages" != "1" ]; then
                os_pre_build_mk_uImage $LINUX_SRC_DIST $DIST
            fi
        fi
        return 0
    fi

    if [ \! -f $DIST/config_done ]; then

        # WA: till dts ch-sets merged
        # When merged - remove files
        extra_dts_dir=$tool_build/toolkit/${TOOLKIT}/dts/$LINUX_KERNEL_VERSION
        if [ -d $extra_dts_dir -a $CPU_TOOL = arm ]; then
            cp -f $extra_dts_dir/* ${LINUX_SRC_DIST}/arch/$CPU_TOOL/boot/dts
        fi

        # configure kernel
        I make ${MAKE_FLAGS} ${OBJS} -C ${LINUX_SRC_DIST} ${DEFCONFIG}
        if [ $? -ne 0 ]
        then
            error_message -e "\tmake config ${DEFCONFIG} failed"
            return 1
        else
            info_message -e "\tmake config $config...done"
        fi
        if [ "$LINUX_CONFIG_NOPRESTERA" == "YES" ]; then
            I ${LINUX_SRC_DIST}/scripts/config --file $DIST/.config \
                --disable CONFIG_MV_INCLUDE_PRESTERA \
                --disable CONFIG_MV_INCLUDE_PRESTERA_PCI \
                --disable CONFIG_MV_INCLUDE_PRESTERA_KERNELEXT \
                --disable CONFIG_MV_INCLUDE_SERVICECPU \
                --disable CONFIG_MV_INCLUDE_DRAGONITE_XCAT \
                --disable CONFIG_MV_DRAGONITE_IPC_SUPPORT \
                --disable CONFIG_MVEBU_CM3 \
                --disable CONFIG_MV_CM3_IPC_SUPPORT \
                --disable CONFIG_MV_INCLUDE_SERVICECPU \
                --disable CONFIG_MV_SERVICECPU_IPC_SUPPORT \
                --disable CONFIG_MV_IPC_FREERTOS_DRIVER
        else
            I ${LINUX_SRC_DIST}/scripts/config --file $DIST/.config \
                --disable CONFIG_MV_INCLUDE_SERVICECPU \
                --disable CONFIG_MV_INCLUDE_DRAGONITE_XCAT \
                --disable CONFIG_MV_DRAGONITE_IPC_SUPPORT \
                --disable CONFIG_MVEBU_CM3 \
                --disable CONFIG_MV_CM3_IPC_SUPPORT \
                --disable CONFIG_MV_INCLUDE_SERVICECPU \
                --disable CONFIG_MV_SERVICECPU_IPC_SUPPORT \
                --disable CONFIG_MV_IPC_FREERTOS_DRIVER
        fi

        if [ "x$DONT_CREATE_ROOTFS" != "xYES" ]; then
            # create rootfs
            os_linux_make_rootfs || return 1
            crfs_script=$tool_build/toolkit/${TOOLKIT}/$LINUX_CRFS_SCRIPT_NAME
            test -f $crfs_script || crfs_script=${LINUX_SRC_DIST}/scripts/${LINUX_CRFS_SCRIPT_NAME}
            I bash $crfs_script \
                        ${DIST}/rootfs ${TOOLKIT_PATH} \
                        ${LINUX_CRFS_ARCH_ARG} ${LINUX_DIST_PATH}
            if [ $? -ne 0 ]
            then
                error_message -e "\tROOTFS CREATION FAILED"
                return 1
            fi

            os_pre_build_patch_init ${DIST}/rootfs
        else
            # Skip busybox
            mkdir -p ${DIST}/rootfs/{dev,usr/bin,lib/modules}
        fi
        touch $DIST/config_done
    fi


    # now build kernel
    I make ${MAKE_FLAGS} ${OBJS} -C ${LINUX_SRC_DIST} $LSP_KERNEL_TYPE $mkopts
    if [ $? -ne 0 ]; then
        error_message -e "\tmake $LSP_KERNEL_TYPE failed"
        return 1
    else
        info_message -e "\tmake $LSP_KERNEL_TYPE...done"
    fi
    I make ${MAKE_FLAGS} ${OBJS} -C ${LINUX_SRC_DIST} ${LSP_KERNEL_DTB_NAME[*]} $mkopts
    if [ $? -ne 0 ]; then
         error_message -e "\tmake .dtb files failed"
         return 1
    fi

    if [ "$CPU_TOOL" != "arm64" ]; then
        os_pre_build_mk_uImage $LINUX_SRC_DIST $DIST
    fi

    #mark compilation is done
    os_linux_kernel_mark_already_built

    info_message -e "END   buildLinux"
    return 0
}

#----------------------------------------------------------------
# Function os_post_build_int
#
# Description:
#       Build image to burn on target flash
#
os_post_build_int()
{
    for index in ${!LSP_KERNEL_UDTB_NAME[*]}; do
        dtb=${LSP_KERNEL_DTB_NAME[$index]}
        if [ "CPU_TOOL" != "arm64" ]; then
            udtb=${LSP_KERNEL_UDTB_NAME[$index]}

            linux_2_6_cr_jffs2_params $dtb

            target_uImage=uImage_$udtb

            if [ "${UIMAGE_NO_COPY}" = "TRUE" ]
            then
                info_message -e "\tuImage not included in rootfs."
            else
                T cp ${UIMAGE_PATH}/$target_uImage ${DIST}/rootfs
                T cp ${DIST}/arch/arm/boot/dts/$dtb ${DIST}/rootfs/
            fi

            # Build images for MTDBURN
            if [ "$DONT_BUILD_IMAGES" != "YES" -a "$BUILD_MTDBURN_IMAGES" == "YES" ]; then
                cr_script=$tool_build/toolkit/${TOOLKIT}/${CR_JFFS2_FILE_NAME}
                test -f $cr_script || cr_script=${LINUX_SRC_DIST}/scripts/${CR_JFFS2_FILE_NAME}
                I bash $cr_script ${CR_JFFS2_PARAMS}
                if [ $? -ne 0 ] # test result
                then
                    error_message -e "\tCreate jffs2_img failed"
                    return 1
                fi

                for img in jffs2_arm.image ubifs_arm_256eb_nand_v2_5.image ubifs_arm_512eb_nand.image
                do
                    if [ -f $IMAGE_PATH/$img ]; then
                        mv $IMAGE_PATH/$img $IMAGE_PATH/${udtb}_$img
                    fi
                done

                info_message "END buildImage"
                # TODO: rename images
            fi
            # copy uImage $TARGET_DIR_ROOTFS
            if [ -d "$TARGET_DIR_ROOTFS" ]; then
                I cp ${DIST}/arch/${CPU_TOOL}/boot/uImage_${udtb} $TARGET_DIR_ROOTFS
            fi
        fi # !arm64

        # copy dtb $TARGET_DIR_ROOTFS
        if [ -d "$TARGET_DIR_ROOTFS" ]; then
            I cp ${DIST}/arch/${CPU_TOOL}/boot/dts/$dtb       $TARGET_DIR_ROOTFS
        fi
    done
    # Build new image
    if [ "$DONT_BUILD_IMAGES" != "YES" -a -f $tool_build/toolkit/${TOOLKIT}/cr_ubi_img.sh ]; then
        I bash $tool_build/toolkit/${TOOLKIT}/cr_ubi_img.sh \
                $DIST $CPU_TOOL $LINUX_KERNEL_VERSION \
                $DIST/rootfs $IMAGE_PATH
    fi

    if [ -d "$TARGET_DIR_ROOTFS" ]; then
        I cp ${DIST}/arch/${CPU_TOOL}/boot/$LSP_KERNEL_TYPE   $TARGET_DIR_ROOTFS
        I tar -czf $TARGET_DIR_ROOTFS/rootfs.tgz -C $DIST/rootfs .
    fi

    return 0
}
