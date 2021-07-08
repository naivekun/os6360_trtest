/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtOsSharedMemoryRemapper.h
*
* @brief This file contains declarations for remapping routines which remaps
* regular libcpss.so shared object (TEXT section is common) to
* CPSS Shared Library (also BSS and DATA sections are common).
*
* @version   3
********************************************************************************
*/
#ifndef __gtOsSharedMemoryRemapperh
#define __gtOsSharedMemoryRemapperh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>

/************* Defines ********************************************************/
#define SHARED_MEMORY_MALLOC_DATA_CNS  "/dev/shm/CPSS_SHM_MALLOC"
/* The address of osMalloc() allocated memory.
 * Must be the same for all instances of libcpss code
 *
 * Should fit in 32bit address space
 */


#define SHARED_MEMORY_DATA_CNS	"/dev/shm/CPSS_SHM_DATA"
#define SHARED_MEMORY_MAINOS_DATA_CNS	"/dev/shm/MAINOS_SHM_DATA"
#define SHARED_MEMORY_SEM_CNS	"/CPSS_SHM_SEM"

/* Address of shared BSS/DATA section
 *
 *      SHARED_DATA_ADDR_CNS        - Address of CPSS lib shared data block
 *      SHARED_MAINOS_DATA_ADDR_CNS - Address of libhelper shared data block
 *
 * Must be syncronized with
 *      cpssEnabler/mainOs/src/gtOs/sharedMemory/scriptsld/linux{,Sim}/
 *              libcpss_ld_script.${CPU}
 *              libhelper_ld_script.${CPU}
 *
 */
#ifdef ASIC_SIMULATION
/* For simulation TEXT section can be too large (over 16Mb with Golden Module)
 * so we should have enough space to avoid section overlapping */
#   define SHARED_DATA_ADDR_CNS	0x26000000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x36000000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x54000000
#else
#  if defined(CPU_ARMARCH5) || defined(CPU_ARMARCH7) || defined(CPU_AARCH64v8) || (defined(INTEL64_DRV) && defined(CPU_i386))
#   define SHARED_DATA_ADDR_CNS	0x13000000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x14680000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x14800000
#  endif
#  if defined(INTEL64_DRV) && !defined(CPU_i386)
#   define SHARED_DATA_ADDR_CNS	0x13000000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x14a00000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x14c00000
#  endif
#  if defined(MIPS64_CPU)
#   define SHARED_DATA_ADDR_CNS	0x23000000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x24680000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x24800000
#  endif
#  if defined(CPU_PPC85XX)
#   define SHARED_DATA_ADDR_CNS	0x63000000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x64680000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x64800000
#  endif
   /* legacy address mapping */
#  ifndef SHARED_DATA_ADDR_CNS
#   define SHARED_DATA_ADDR_CNS	0x10b00000
#   define SHARED_MAINOS_DATA_ADDR_CNS	0x10f80000
#   define SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x11000000
#  endif
#endif


/************* Typedefs ********************************************************/
/*
 * Typedef: struct SECTIONS_STCT
 *
 * Description:
 *      This structure presents addresses begin and end of section.
 *
 * Fields:
 *      startPtr	- began an address a section.
 *      donePtr		- address of end of section.
 *
 *******************************************************************************/
typedef struct SECTION_STCT
{
    GT_VOID	*startPtr;
    GT_VOID	*donePtr;
} SECTION_STC;

/************* Functions ******************************************************/

/**
* @internal shrMemUnlinkShmObj function
* @endinternal
*
* @brief   Unlink shared files and semaphore
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*/
GT_STATUS shrMemUnlinkShmObj
(
    GT_VOID
);

/**
* @internal shrMemDoShmMap function
* @endinternal
*
* @brief   Do sharing data for all processes
*
* @param[in] isFirstClient            - is client is the first CPSS client(Enabler) or not.
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*/
GT_STATUS shrMemDoShmMap
(
    GT_BOOL isFirstClient
);

/**
* @internal shrMemPrintMapsDebugInfo function
* @endinternal
*
* @brief   Ouputs /proc/self/maps to stderr for debug purpose
*
* @note Should be used only for debugging on SHARED_MEMORY=1
*
*/
GT_VOID shrMemPrintMapsDebugInfo(GT_VOID);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __gtOsSharedMemoryRemapperh */


