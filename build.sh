#!/bin/bash
set -e

# docker definition
DOCKER_IMAGE_BASE="os6360_base"
DOCKER_BASE_TAG=`sha1sum docker/Dockerfile | awk '{print substr($1,1,11)}'`
DOCKER_MOUNT_FOLDER=`pwd`
DOCKER_MOUNT_TOOLCHAIN="/opt/armv7-marvell-linux-gnueabi-softfp-4.8.5_i686_20150706"
TOOLCHAIN="/opt/armv7-marvell-linux-gnueabi-softfp"
KERNEL_PATH="cpss-ac3/linux/uni/4.4/kernel/mvebu_v7_lsp_noprestera_defconfig"
DEFAULT_KERNEL_CONFIG="cpss-ac3/lsp/uni/4.4/kernel/arch/arm/configs/mvebu_v7_lsp_defconfig"
BUSYBOX_SRC="cpss-ac3/linux/busybox-1.25.0.tar.bz2"
BUSYBOX_DIR="busybox-1.25.0"
BUSYBOX_CONFIG="cpss-ac3/cpss/tools/build/toolkit/marvell_v7sft/busybox-1.25.0.config"
BUILT_KERNEL="cpss-ac3/linux/uni/4.4/kernel/mvebu_v7_lsp_noprestera_defconfig"
SUBMODULE="cpss-ac3/lsp/wnc_tools/diag/diag_code"
COMPILATION_ROOT="cpss-ac3/compilation_root"
GIT_REVISION=`git show | grep ^commit | awk '{print substr($2,1,11)}'`
GIT_REVISION_DIAG=`cd ${SUBMODULE};git show | grep ^commit | awk '{print substr($2,1,11)}';`
GIT_REVISION_FUNC=`cd ${SUBMODULE}/funcs;git show | grep ^commit | awk '{print substr($2,1,11)}';`
VER_NUM="${GIT_REVISION}-${GIT_REVISION_DIAG}-${GIT_REVISION_FUNC}"
UBOOT_DIR="u-boot_2"
UBOOT_VER="${UBOOT_DIR}/include/configs/mv_version.h"
BUILDROOT_PATH="buildroot/buildroot-2015.11-16.08.0"
DEFAULT_BUILDROOT_CONFIG="${BUILDROOT_PATH}/configs/os6360_defconfig"

DOCKER_BASE_BUILD="docker build --no-cache \
-t ${DOCKER_IMAGE_BASE} \
-f docker/Dockerfile \
docker"
DOCKER_BASE_RUN="docker run -it \
--network=host \
--rm=true \
-v /etc/localtime:/etc/localtime:ro \
-v ${DOCKER_MOUNT_FOLDER}:${DOCKER_MOUNT_FOLDER} \
-v ${DOCKER_MOUNT_TOOLCHAIN}:${TOOLCHAIN} \
-w ${DOCKER_MOUNT_FOLDER} \
-e TOOLCHAIN=${TOOLCHAIN} \
-e CROSS_COMPILE=${TOOLCHAIN}/bin/arm-marvell-linux-gnueabi- \
-e CROSS_COMPILE_BH=${TOOLCHAIN}/bin/arm-marvell-linux-gnueabi- \
-e KERNEL_PATH=${KERNEL_PATH} \
-e DEBUG_INFO=D_ON \
-e CMDFS_NIX=yes \
-e VER_NUM=${VER_NUM} \
${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG}"

# help message definition
RED_BOLD=$(tput bold)$(tput setaf 1)
GREEN_BOLD=$(tput bold)$(tput setaf 2)
BLUE_BOLD=$(tput bold)$(tput setaf 4)
YELLOW_BOLD=$(tput bold)$(tput setaf 3)
RESET=$(tput sgr 0)

# check docekr images and tags docker image to the latest
docker_image_build () {
  docker inspect --type image ${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG} &> /dev/null || \
  { echo Image ${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG} not found. Building... ; \
    ${DOCKER_BASE_BUILD} && \
    docker tag ${DOCKER_IMAGE_BASE}:latest ${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG}; \
  }
  # insure docker base is specified image
  docker tag ${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG} ${DOCKER_IMAGE_BASE}:latest
  echo "Desired Docker image does exist."
}

source_code_clean_bootloader () {
  echo "Clean Bootloader ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "cd ${UBOOT_DIR};./myclean.pl > /dev/null"
  echo "Exit Clean Bootloader."
}

source_code_clean_cpss () {
  echo "Clean CPSS ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "rm -rf ${PWD}/${BUILT_KERNEL} ${PWD}/${COMPILATION_ROOT};rm -f ${PWD}/cpss-ac3/output/*.img"
  echo "Exit Clean CPSS."
}

source_code_clean_buildroot () {
  echo "Clean buildroot ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "cd ${BUILDROOT_PATH};make clean;"
  echo "Exit Clean buildroot."
}

source_code_build_bootloader () {
  echo "Build Bootloader ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "cd ${UBOOT_DIR};./build.pl -b ac3s_rd -f spi -i spi:nand -m 3" | tee build-u-boot.log
  echo "Exit Build Bootloader."
  set +e; grep --color=auto -n -e "error:" -e "Error [0-9]*" build-u-boot.log; set -e
}

source_code_build_cpss () {
  echo "Build CPSS ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "cd cpss-ac3;./os6360.sh" | tee build-os6360.log
  echo "Exit Build CPSS."
  set +e; grep --color=auto -n -e "error:" -e "Error [0-9]*" build-os6360.log; set -e
}

source_code_build_buildroot () {
  echo "Build buildroot ..."
  if [ ! -f ${PWD}/${BUILDROOT_PATH}/.config ]; then
    time ${DOCKER_BASE_RUN} \
    bash -c "cd ${BUILDROOT_PATH};make os6360_defconfig;make" | tee build-buildroot.log
  else
    time ${DOCKER_BASE_RUN} \
    bash -c "cd ${BUILDROOT_PATH};make" | tee build-buildroot.log
  fi
  echo "Exit Build buildroot."
  echo "Compose Nadiag_GUI.img ..."
  time ${DOCKER_BASE_RUN} \
  bash -c "cd cpss-ac3;./pkt_gen_gui_based_on_cpss_rootfs.sh" | tee -a build-buildroot.log
  echo "Finish ${YELLOW_BOLD}Nadiag_GUI.img${RESET}, please go to ${YELLOW_BOLD}./cpss-ac3/output/${RESET} to get it."
}

source_code_update_submodule () {
  echo "Update Diganostic Submodule ..."
  git submodule update --init --recursive ${SUBMODULE}
  cd ${SUBMODULE}/funcs
  git pull origin master
  cd - >/dev/null 2>&1
  cd ${SUBMODULE}
  git pull origin qsa11_oam_al
  cd - >/dev/null 2>&1
  echo "Done."
}

source_code_specified_submodule() {
  local i=0
  
  if [ ! -z "$1" ] && [ `git tag | grep "$1"` ]; then
    echo "Checkout to tag $1 ..."
    git checkout tags/$1 > /dev/null 2>&1
    DOCKER_BASE_RUN="docker run -it \
    --network=host \
    --rm=true \
    -v /etc/localtime:/etc/localtime:ro \
    -v ${DOCKER_MOUNT_FOLDER}:${DOCKER_MOUNT_FOLDER} \
    -v ${DOCKER_MOUNT_TOOLCHAIN}:${TOOLCHAIN} \
    -w ${DOCKER_MOUNT_FOLDER} \
    -e TOOLCHAIN=${TOOLCHAIN} \
    -e CROSS_COMPILE=${TOOLCHAIN}/bin/arm-marvell-linux-gnueabi- \
    -e CROSS_COMPILE_BH=${TOOLCHAIN}/bin/arm-marvell-linux-gnueabi- \
    -e KERNEL_PATH=${KERNEL_PATH} \
    -e DEBUG_INFO=D_ON \
    -e CMDFS_NIX=yes \
    -e VER_NUM="$1" \
    ${DOCKER_IMAGE_BASE}:${DOCKER_BASE_TAG}"
  else
    echo ${RED_BOLD}"There is no tag $1!"${RESET}
    echo "Available tags are as follows:"
    tags=`git tag`
    for tag in ${tags}; do
      if [ $(($i % 2)) -eq 0 ]; then
        echo -n ${YELLOW_BOLD}"${tag}" ${RESET}
      else
        echo -n ${GREEN_BOLD}"${tag}" ${RESET}
      fi
      i=$((i+1))
    done
    echo
    exit 1
  fi
}

source_code_fetch_tags () {
  local i=0
  
  echo "Fetch Tags ..."
  git fetch --tags
  echo "Done."
  echo "Available tags are as follows:"
  tags=`git tag`
  for tag in ${tags}; do
    if [ $(($i % 2)) -eq 0 ]; then
      echo -n ${YELLOW_BOLD}"${tag}" ${RESET}
    else
      echo -n ${GREEN_BOLD}"${tag}" ${RESET}
    fi
    i=$((i+1))
  done
  echo
}

msg_help () {
  echo "##################################################" >&2
  echo "#                                                #" >&2
  echo "# ${RED_BOLD}Usage${RESET}:                                         #" >&2
  echo "#       ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh ${YELLOW_BOLD}<${BLUE_BOLD}target${RESET}${YELLOW_BOLD}>${RESET}                      #" >&2
  echo "#                                                #" >&2
  echo "# Available target: ${BLUE_BOLD}all clean-all ${RESET}               #" >&2
  echo "#                   ${BLUE_BOLD}uboot clean-uboot${RESET}            #" >&2
  echo "#                   ${BLUE_BOLD}cpss clean-cpss${RESET}              #" >&2
  echo "#                   ${BLUE_BOLD}docker-image enter-container${RESET} #" >&2
  echo "#                   ${BLUE_BOLD}kernel_menuconfig${RESET}            #" >&2
  echo "#                   ${BLUE_BOLD}busybox_menuconfig${RESET}           #" >&2
  echo "#                   ${BLUE_BOLD}update_submodule${RESET}             #" >&2
  echo "#                   ${BLUE_BOLD}fetch_tags${RESET}                   #" >&2
  echo "#                   ${BLUE_BOLD}release_cpss <tag>${RESET}           #" >&2
  echo "#                   ${BLUE_BOLD}release_uboot <tag>${RESET}          #" >&2
  echo "#                   ${BLUE_BOLD}gui_env clean-gui_env${RESET}        #" >&2
  echo "#                   ${BLUE_BOLD}gui_env_menuconfig${RESET}           #" >&2
  echo "#                                                #" >&2
  echo "# ${RED_BOLD}Example${RESET}:                                       #" >&2
  echo "# ${GREEN_BOLD}Build uboot and cpss${RESET}                           #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} all${RESET}                             #" >&2
  echo "# ${GREEN_BOLD}Build uboot${RESET}                                    #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} uboot${RESET}                           #" >&2
  echo "# ${GREEN_BOLD}Build cpss${RESET}                                     #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} cpss${RESET}                            #" >&2
  echo "# ${GREEN_BOLD}Modify kernel config${RESET}                           #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} kernel_menuconfig${RESET}               #" >&2
  echo "# ${GREEN_BOLD}Modify busybox config${RESET}                          #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} busybox_menuconfig${RESET}              #" >&2
  echo "# ${GREEN_BOLD}Enter the bash of container${RESET}                    #" >&2
  echo "#     ${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} enter-container${RESET}                 #" >&2
  echo "##################################################" >&2
}

case "$1" in
  all)
    docker_image_build
    echo "Build All ..."
    #source_code_update_submodule
    source_code_clean_bootloader
        cat << EOF > tmp_file
#ifndef __MV_VERSION_H
#define __MV_VERSION_H
#define CONFIG_IDENT_STRING	"${GIT_REVISION}"
#endif /* __MV_VERSION_H */
EOF
    mv tmp_file ${UBOOT_VER}
    source_code_build_bootloader
    git checkout ${UBOOT_VER}
    source_code_clean_cpss
    source_code_build_cpss
    echo "Exit Build All."
  ;;
  clean-all)
    docker_image_build
    source_code_clean_bootloader
    source_code_clean_cpss
  ;;
  uboot)
    docker_image_build
    source_code_clean_bootloader
        cat << EOF > tmp_file
#ifndef __MV_VERSION_H
#define __MV_VERSION_H
#define CONFIG_IDENT_STRING	"${GIT_REVISION}"
#endif /* __MV_VERSION_H */
EOF
    mv tmp_file ${UBOOT_VER}
    source_code_build_bootloader
    git checkout ${UBOOT_VER}
  ;;
  clean-uboot)
    docker_image_build
    source_code_clean_bootloader
  ;;
  cpss)
    docker_image_build
    source_code_update_submodule
    source_code_clean_cpss
    source_code_build_cpss
  ;;
  clean-cpss)
    docker_image_build
    source_code_clean_cpss
  ;;
  docker-image)
    docker_image_build
  ;;
  enter-container)
    docker_image_build
    echo "Enter Container ..."
    ${DOCKER_BASE_RUN} bash
    echo "Exit Container"
  ;;
  kernel_menuconfig)
    docker_image_build
    ${DOCKER_BASE_RUN} bash -c "cd ${PWD}/${KERNEL_PATH}; ARCH=arm make menuconfig;"
    if [ -f ${DOCKER_MOUNT_FOLDER}/${KERNEL_PATH}/.config.old ]; then
      echo "${RED_BOLD}You have modified kernel config!${RESET}"
      echo "If you want to keep your modification, please merge/commit the difference between"
      echo "${BLUE_BOLD}./${KERNEL_PATH}/.config${RESET}"
      echo "and"
      echo "${BLUE_BOLD}./${KERNEL_PATH}/.config.old${RESET}"
      echo "to"
      echo "${BLUE_BOLD}./${DEFAULT_KERNEL_CONFIG}${RESET}"
      echo ""
      echo "${RED_BOLD}Please execute following command to apply your kernel config.${RESET}"
      echo "${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} cpss${RESET}"
    fi
  ;;
  busybox_menuconfig)
    docker_image_build
    ${DOCKER_BASE_RUN} bash -c "tar jxf ${PWD}/${BUSYBOX_SRC} -C /tmp; cd /tmp/${BUSYBOX_DIR}; \
        cp -v ${PWD}/${BUSYBOX_CONFIG} .config; ARCH=arm make menuconfig; \
    cp -vf .config ${PWD}/${BUSYBOX_CONFIG}"
  ;;
  update_submodule)
    source_code_update_submodule
  ;;
  fetch_tags)
    source_code_fetch_tags
  ;;
  release_cpss)
    docker_image_build
    source_code_specified_submodule $2
    source_code_clean_cpss
    source_code_build_cpss
    echo "Checkout to master ..."
    git checkout master > /dev/null 2>&1
  ;;
  release_uboot)
    docker_image_build
    source_code_specified_submodule $2
    source_code_clean_bootloader
        cat << EOF > tmp_file
#ifndef __MV_VERSION_H
#define __MV_VERSION_H
#define CONFIG_IDENT_STRING	"$2"
#endif /* __MV_VERSION_H */
EOF
    mv tmp_file ${UBOOT_VER}
    source_code_build_bootloader
    git checkout ${UBOOT_VER}
    echo "Checkout to master ..."
    git checkout master > /dev/null 2>&1
  ;;
  gui_env)
    docker_image_build
    if [ ! -f ${BUILT_KERNEL}/rootfs/usr/bin/appDemo ]; then
      echo "${RED_BOLD}You have to build all first!${RESET}"
      exit 4
    fi
    source_code_build_buildroot
  ;;
  clean-gui_env)
    docker_image_build
    source_code_clean_buildroot
  ;;
  gui_env_menuconfig)
    docker_image_build
    if [ ! -f ${PWD}/${BUILDROOT_PATH}/.config ]; then
      echo "Configure buildroot for OS6360 ..."
      time ${DOCKER_BASE_RUN} \
      bash -c "cd ${BUILDROOT_PATH};make os6360_defconfig;"
      echo "Finish the configuration of OS6360."
    fi
    
    ${DOCKER_BASE_RUN} bash -c "cd ${PWD}/${BUILDROOT_PATH}; make menuconfig;"
    if [ -f ${PWD}/${BUILDROOT_PATH}/.config.old ]; then
      echo "${RED_BOLD}You have modified buildroot config!${RESET}"
      echo "If you want to keep your modification, please merge/commit the difference between"
      echo "${BLUE_BOLD}./${BUILDROOT_PATH}/.config${RESET}"
      echo "and"
      echo "${BLUE_BOLD}./${BUILDROOT_PATH}/.config.old${RESET}"
      echo "to"
      echo "${BLUE_BOLD}./${DEFAULT_BUILDROOT_CONFIG}${RESET}"
      echo ""
      echo "${RED_BOLD}Please execute following command to apply your buildroot config.${RESET}"
      echo "${RED_BOLD}.${GREEN_BOLD}/${RESET}build.sh${BLUE_BOLD} gui_env${RESET}"
    fi
  ;;
  *)
    msg_help
    exit 3
  ;;
esac
