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
* @file prvExtDrvLinuxMapping.h
*
* @brief This file contains a set of userspace virtual addresses used in CPSS
* (Linux only)
*
* @version   1
********************************************************************************
*/
#ifndef __prvExtDrvLinuxMapping_h
#define __prvExtDrvLinuxMapping_h


/*****************************/
/*****************************/
/*** INTEL64               ***/
/*****************************/
/*****************************/
#if defined(CPU_ARMARCH5) || defined(CPU_ARMARCH7) || defined(CPU_AARCH64v8) || (defined(INTEL64_DRV) && defined(CPU_i386))
/* addresses which must fit in 32bit address space */
/* DMA memory (2M) */
#define LINUX_VMA_DMABASE       0x1c800000 /* 2M */

/* Defined in mainOs/h/gtOs/gtOsSharedMemoryRemapper.h:
SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x14800000
*/


#define LINUX_VMA_PP_CONF_BASE  0x1cc00000
#define LINUX_VMA_PP_DFX_BASE   0x1e000000
#define LINUX_VMA_PP_REGS_BASE  0x20000000

/* XCAT only */
#define LINUX_VMA_DRAGONITE     0x80000000

/* HAS_HSU */
/* if xcat then 0x90000000 else 0x40000000 */
#if (defined(__KERNEL__) && defined(CONFIG_ARCH_FEROCEON_KW)) || (!defined(__KERNEL__) && defined(XCAT_DRV))
#define LINUX_VMA_HSU           0x90000000
#else
#define LINUX_VMA_HSU           0x40000000
#endif
#endif /* defined(CPU_ARMARCH5) || defined(CPU_ARMARCH7) */


#if defined(INTEL64_DRV) && !defined(CPU_i386)
#define LINUX_VMA_DMABASE       0x1fc00000 /* 4M */
#define LINUX_VMA_PP_REGS_BASE  0x20000000
#endif



/*****************************/
/*****************************/
/*** MIPS64                ***/
/*****************************/
/*****************************/
#if defined(MIPS64_CPU)
/* addresses which must fit in 32bit address space */
/* DMA memory (2M) */
#define LINUX_VMA_DMABASE       0x2c800000 /* 2M */

/* Defined in mainOs/h/gtOs/gtOsSharedMemoryRemapper.h:
SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x24800000
*/


#define LINUX_VMA_PP_CONF_BASE  0x2cc00000
#define LINUX_VMA_PP_DFX_BASE   0x2e000000
#define LINUX_VMA_PP_REGS_BASE  0x30000000

/* HAS_HSU */
#define LINUX_VMA_HSU           0x50000000
#endif /* defined(MIPS64_CPU) */




/**********************************/
/**********************************/
/*** legacy address arrangement ***/
/**********************************/
/**********************************/
#ifndef LINUX_VMA_DMABASE
/* addresses which must fit in 32bit address space */
/* DMA memory (2M) */
#define LINUX_VMA_DMABASE       0x19000000 /* 2M */

/* Defined in mainOs/h/gtOs/gtOsSharedMemoryRemapper.h:
SHARED_MEMORY_MALLOC_VIRT_ADDR_MAC 0x11000000
*/


#define LINUX_VMA_PP_CONF_BASE  0x19400000
#define LINUX_VMA_PP_DFX_BASE   0x1b000000
#define LINUX_VMA_PP_REGS_BASE  0x20000000

/* XCAT only */
#define LINUX_VMA_DRAGONITE     0x80000000

/* HAS_HSU */
/* if xcat then 0x90000000 else 0x40000000 */
#if (defined(__KERNEL__) && defined(CONFIG_ARCH_FEROCEON_KW)) || (!defined(__KERNEL__) && defined(XCAT_DRV))
#define LINUX_VMA_HSU           0x90000000
#else
#define LINUX_VMA_HSU           0x40000000
#endif

#endif /* legacy address arrangement */

#endif /* __prvExtDrvLinuxMapping_h */


