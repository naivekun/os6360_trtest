/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssHwDriverAPI.h
*
* @brief generic HW driver APIs declaration and common functions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHwDriverAPI_h__
#define __cpssHwDriverAPI_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/* forward declaration */
struct CPSS_HW_DRIVER_STCT;

/*******************************************************************************
* CPSS_HW_DRIVER_METHOD_READ
*
* DESCRIPTION:
*       Read method of generic HW driver object
*
* INPUTS:
*       drv         - driver pointer
*       addrSpace   - address space selector
*                     CNM/SWITCHING/DFX/MG1, ... for prestera driver
*                     phyId for SMI
*                     etc
*       regAddr     - register address to read
*       count       - amount of registers to read
*
* OUTPUTS:
*       dataPtr     - pointer to registers data
*
* RETURNS:
*       GT_OK   - on success
*       != GT_OK - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_HW_DRIVER_METHOD_WRITE_MASK
*       2. CPSS_HW_DRIVER_METHOD_DESTROY
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_HW_DRIVER_METHOD_READ)(
    IN  struct CPSS_HW_DRIVER_STCT *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
);

/*******************************************************************************
* CPSS_HW_DRIVER_METHOD_WRITE_MASK
*
* DESCRIPTION:
*       Write method of generic HW driver object
*
* INPUTS:
*       drv         - driver pointer
*       addrSpace   - address space selector
*                     CNM/SWITCHING/DFX/MG1, ... for prestera driver
*                     phyId for SMI
*                     etc
*       regAddr     - register address to read
*       dataPtr     - pointer to data to write
*       count       - amount of registers to write
*       mask        - data mask, will be applied to each word
*                     Use 0xffffffff for direct write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       != GT_OK - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_HW_DRIVER_METHOD_READ
*       2. CPSS_HW_DRIVER_METHOD_DESTROY
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_HW_DRIVER_METHOD_WRITE_MASK)(
    IN  struct CPSS_HW_DRIVER_STCT *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
);

/*******************************************************************************
* CPSS_HW_DRIVER_METHOD_DESTROY
*
* DESCRIPTION:
*       Destroy method of generic HW driver object
*
* INPUTS:
*       drv         - driver pointer
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK   - on success
*       != GT_OK - on error
*
* COMMENTS:
*       This method should not care about .name and .parent cleanup
*
*******************************************************************************/
typedef void (*CPSS_HW_DRIVER_METHOD_DESTROY)(
    IN  struct CPSS_HW_DRIVER_STCT *drv
);

/*
 * typedef: struct CPSS_HW_DRIVER_STCT
 *
 * Description: Generic HW driver object
 *
 * Fields:
 *
 *   read         - read method
 *   writeMask    - write masked method
 *   destroy      - destroy method
 *   name         - driver name. For example: "smi0".
 *                  The full path example: "/SoC/smi0"
 *                  Will be filled by cpssHwDriverRegister()
 *   numRefs      - the use count: list + childs. Used by
 *                  cpssHwDriverRegister(),
 *                  cpssHwDriverDestroy()
 *                  Driver will not really be destroyed
 *                  untill all childs destroyed
 *   parent       - Lower-level driver or NULL
 *
 * Comments:
 *
 */
typedef struct CPSS_HW_DRIVER_STCT {
    CPSS_HW_DRIVER_METHOD_READ       read;
    CPSS_HW_DRIVER_METHOD_WRITE_MASK writeMask;
    CPSS_HW_DRIVER_METHOD_DESTROY    destroy;
    char *name;
    GT_U32 numRefs;
    struct CPSS_HW_DRIVER_STCT *parent;
} CPSS_HW_DRIVER_STC;


/**
* @enum CPSS_HW_DRIVER_AS_ENT
 *
 * @brief Defines Standard address spaces
*/
typedef enum{

    /** Control And Management AS */
    CPSS_HW_DRIVER_AS_CNM_E       = 0x0,

    /** Switching core unit 0 (MG0) */
    CPSS_HW_DRIVER_AS_SWITCHING_E = 0x3,

    /** Switching core unit 0 (MG0) - alias */
    CPSS_HW_DRIVER_AS_MG0_E       = 0x3,

    /** Switching core unit 1 (MG1) */
    CPSS_HW_DRIVER_AS_MG1_E       = 0x5,

    /** Switching core unit 2 (MG2) */
    CPSS_HW_DRIVER_AS_MG2_E       = 0x6,

    /** Switching core unit 3 (MG2) */
    CPSS_HW_DRIVER_AS_MG3_E       = 0x7,

    /** Reset And Init Controller */
    CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E = 0x8,

    /** Reset And Init Controller - alias */
    CPSS_HW_DRIVER_AS_DFX_E       = 0x8,

    /** Dragonite unit */
    CPSS_HW_DRIVER_AS_DRAGONITE_E = 0xa,

    /** DRAM Unit */
    CPSS_HW_DRIVER_AS_DRAM_E      = 0x10,

    /** PCI Configuration Space */
    CPSS_HW_DRIVER_AS_PCI_CONFIG_E  = 0x11,

    /** Address Translation Unit (ATU in PEX IP) */
    CPSS_HW_DRIVER_AS_ATU_E         = 0x12,


} CPSS_HW_DRIVER_AS_ENT;



/* Common APIs: */
/**
* @internal cpssHwDriverRegister function
* @endinternal
*
* @brief   Add driver instance to registry
*         The full path will be constructed as parent.path + '/' + name
* @param[in] drv                      - driver pointer
* @param[in] name                     - driver name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the parent is not registered
*                                       GT_OUT_OF_CPU_MEM
*                                       GT_ALREADY_EXIST
*/
GT_STATUS cpssHwDriverRegister(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  const char         *name
);

/**
* @internal cpssHwDriverAddAlias function
* @endinternal
*
* @brief   Add alias for the driver
*
* @param[in] drv                      - driver pointer
* @param[in] alias                    -  path: relative ("smi0") or absolute ("/smi0")
*
* @retval GT_OK                    - on success
*                                       GT_OUT_OF_CPU_MEM
*/
GT_STATUS cpssHwDriverAddAlias(
    IN  CPSS_HW_DRIVER_STC *drv,
    IN  const char         *alias
);

/**
* @internal cpssHwDriverDestroy function
* @endinternal
*
* @brief   Remove driver instance from registry
*         Destroy it if refCount == 0
* @param[in] drv                      - driver pointer
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssHwDriverDestroy(
    IN  CPSS_HW_DRIVER_STC *drv
);

/*******************************************************************************
* cpssHwDriverLookup
*
* DESCRIPTION:
*       Lookup the driver in registry using full path
*
* INPUTS:
*       path        - registry path
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Driver pointer or NULL
*
* COMMENTS:
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* cpssHwDriverLookup(
    IN  const char *path
);

/*******************************************************************************
* cpssHwDriverLookupRelative
*
* DESCRIPTION:
*       Lookup the driver in registry using relative path
*
* INPUTS:
*       parent      - the base for relative search
*       path        - registry relative path
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Driver pointer or NULL
*
* COMMENTS:
*       For example:
*       prestera_drv_path == "/PEX01:00.0"
*       smi_drv_path == "/PEX01:00.0/smi0/smi0"
*       cpssHwDriverLookupRelative(prestera_drv, "smi0/smi0");
*
*******************************************************************************/
CPSS_HW_DRIVER_STC* cpssHwDriverLookupRelative(
    IN  CPSS_HW_DRIVER_STC *parent,
    IN  const char         *path
);

/**
* @internal cpssHwDriverPrintList function
* @endinternal
*
* @brief   Print driver registry
*/
GT_STATUS cpssHwDriverPrintList(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(__cpssHwDriverAPI_h__) */

