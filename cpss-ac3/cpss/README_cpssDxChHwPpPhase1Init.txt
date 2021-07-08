cpssDxChHwPpPhase1Init() API change
===================================

To support new devices and improve flexibility
CPSS_DXCH_PP_PHASE1_INIT_INFO_STC structure was changed


The summary of changes:
        Removed:
            busBaseAddr
            internalPciBase
            intVecNum
            intMask
            portGroupsInfo

        New field:
            hwInfo[CPSS_MAX_PORT_GROUPS_CNS] - The HW connectivity info:
                -   physical bus type
                -   bus address (for example device’s PCIe address)
                -   resources (cnm,switching,resetAndInitController, etc)
                -   interrupts
                -   interrupt masks

        For non-multi-portgroup device hwInfo[0] must be filled, numOfPortGroups == 1
        For multi-portgroup device numOfPortGroups must be set as well as hwInfo[portGroup]





Migration from legacy Phase1Params structure:
=============================================

Single port group devices
    1. newPhase1Params.numOfPortGroups = 1
    2. newPhase1Params.hwInfo[0] should be zeroed then filled:
        .busType    - physical bus type:
                      CPSS_HW_INFO_BUS_TYPE_PEX_E for PEX connected devices
                      CPSS_HW_INFO_BUS_TYPE_MBUS_E for MSYS
                      (AC3,BC2,BobK internal CPU)

        .hwAddr.busNo             == legacyPhase1Params.hwAddr.busNo
        .hwAddr.devSel            == legacyPhase1Params.hwAddr.devSel
        .hwAddr.funcNo            == legacyPhase1Params.hwAddr.funcNo
        .irq.switching            == legacyPhase1Params.intVecNum
        .intMask.switching        == legacyPhase1Params.intMask
        .resource.cnm.start       == legacyPhase1Params.internalPciBase
        .resource.switching.start == legacyPhase1Params.busBaseAddr
        .resource.resetAndInitController.start
                                  == legacyPhase1Params.resetAndInitControllerBase
        .resource.sram.start         (external CPU + AC3,BC2,BobK only):
                                  == legacyPhase1Params.resetAndInitControllerBase + 0x200000

       Please note: If Marvell's reference code is used (mainExtDrv)
	                extDrvPexConfigure() can be used to fill this structure


Multi-port-group devices
    1.  newPhase1Params.numOfPortGroups = <num_of_port_groups>
    2.  newPhase1Params.hwInfo[<portGroupNo>] will be filled for each portgroup

