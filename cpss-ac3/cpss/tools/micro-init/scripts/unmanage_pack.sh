#!/bin/sh

make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi- -C freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC WITH_LABSERVER=y IPC_LIB=y BOARD=ALDRIN_DEV DISABLE_CLI=n MICRO_INIT=y PROC=CM3 clean
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi- -C freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC WITH_LABSERVER=y IPC_LIB=y BOARD=ALDRIN_DEV DISABLE_CLI=n MICRO_INIT=y PROC=CM3 RTOSDemo-cm3_Aldrin.a -j 4
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/aldrin
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/switch-phy
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs
mkdir -p unmanaged/common/h/cpss/common/labServices/port/gop/port/private
mkdir -p unmanaged/common/h/cpss/common/labServices/port/gop/common/os
mkdir -p unmanaged/common/h/cpss/common/labServices/port/gop/common/siliconIf
mkdir -p unmanaged/common/src/cpss/common/labServices/port/gop/src/portCtrl/h
mkdir -p unmanaged/common/h/cpss/common/labServices/port/gop/port/serdes
mkdir -p unmanaged/common/src/cpss/common/labServices/port/gop/port/serdes
mkdir -p unmanaged/common/h/cpss/common/labServices/port/gop/common/configElementDb
mkdir -p unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/Generic
mkdir -p unmanaged/common/h/cpss/extServices/os/gtOs
mkdir -p unmanaged/common/h/cpss/generic/hwDriver


cp common/h/cpss/extServices/os/gtOs/gtGenTypes.h                               unmanaged/common/h/cpss/extServices/os/gtOs
cp common/h/cpss/generic/cpssHwInfo.h                                           unmanaged/common/h/cpss/generic
cp common/h/cpss/generic/hwDriver/cpssHwDriver*                                 unmanaged/common/h/cpss/generic/hwDriver
cp common/h/cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h                unmanaged/common/h/cpss/common/labServices/port/gop/port
cp common/h/cpss/common/labServices/port/gop/common/os/hwsEnv.h                 unmanaged/common/h/cpss/common/labServices/port/gop/common/os
cp common/h/cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h     unmanaged/common/h/cpss/common/labServices/port/gop/common/siliconIf
cp common/h/cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h         unmanaged/common/h/cpss/common/labServices/port/gop/port
cp common/h/cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h             unmanaged/common/h/cpss/common/labServices/port/gop/port
cp common/h/cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h     unmanaged/common/h/cpss/common/labServices/port/gop/port/private
cp common/h/cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h      unmanaged/common/h/cpss/common/labServices/port/gop/port/private
cp common/h/cpss/common/labServices/port/gop/port/private/mvPortModeElements.h  unmanaged/common/h/cpss/common/labServices/port/gop/port/private
cp common/h/cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h     unmanaged/common/h/cpss/common/labServices/port/gop/port/serdes
cp common/h/cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h        unmanaged/common/h/cpss/common/labServices/port/gop/port/serdes
cp common/h/cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h unmanaged/common/h/cpss/common/labServices/port/gop/common/configElementDb
cp common/src/cpss/common/labServices/port/gop/src/portCtrl/h/mvHwsPortCtrlDefines.h unmanaged/common/src/cpss/common/labServices/port/gop/src/portCtrl/h
cp common/src/cpss/common/labServices/port/gop/src/portCtrl/h/mvHwsPortCtrlAp.h unmanaged/common/src/cpss/common/labServices/port/gop/src/portCtrl/h
cp common/src/cpss/common/labServices/port/gop/src/portCtrl/h/mvHwsPortCtrlDb.h unmanaged/common/src/cpss/common/labServices/port/gop/src/portCtrl/h
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/FreeRTOSConfig.h            unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/Makefile                    unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/RTOSDemo-cm3_Aldrin.a       unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/template_Aldrin.ld          unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/RTOSDemo-cm3.ld             unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC/printf.h                    unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/labservice.a               unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs/gtEnvDep.h   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs/gtGenTypes.h unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs/gtOsSem.h    unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/labservice/hwServices/mainOs/h/gtOs
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/NetPort.h     unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/aldrin/unq.c  unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/aldrin
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/Generic/mvTwsi.h        unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/Generic
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/unq.h         unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include/FreeRTOS.h                   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include/mpu_wrappers.h               unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include/portable.h                   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include/projdefs.h                   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/include
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw/common.h     unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw/errno.h      unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw/mvCommon.h   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/hw
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3/portmacro.h     unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Source/portable/GCC/ARM_CM3
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/switch-phy/mvSwitchPhy.h unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/switch-phy
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/fileCommon.h  unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/fileOps.h     unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init
cp freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/port_ctrl.h   unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init

make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi- -C unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC WITH_LABSERVER=y IPC_LIB=y BOARD=ALDRIN_DEV DISABLE_CLI=n MICRO_INIT=y PROC=CM3 RTOSDemo-cm3.ld
make ARCH=arm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi- -C unmanaged/freeRTOS/FreeRTOSV7.3.0/FreeRTOS/App/ARM_CM3_GCC WITH_LABSERVER=y IPC_LIB=y BOARD=ALDRIN_DEV DISABLE_CLI=n MICRO_INIT=y PROC=CM3 unmanaged_Aldrin.bin -j 4
tar cjf unmanaged-aldrin.tar.bz2 unmanaged --remove-files
echo `pwd`/unmanaged-aldrin.tar.bz2 ready!
