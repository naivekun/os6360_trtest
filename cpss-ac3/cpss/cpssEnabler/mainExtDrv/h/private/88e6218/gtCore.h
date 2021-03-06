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
* @file gtCore.h
*
* @brief This header file contains simple read/write macros for addressing
* the SDRAM, devices, MV`s registers and PCI (using the PCI`s
* address space). The macros take care of Big/Little endian conversions.
*
*/

#ifndef __gtCoreh
#define __gtCoreh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 
/* includes */

/* globals */
    
/* typedefs */

/* defines  */

#define  mvInternalRegBaseAddr  (0x00000000)

/****************************************/
/*          GENERAL Definitions			*/
/****************************************/
#define NO_BIT          0x00000000
#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000
    /* Size defintions */
#define _1K             0x00000400
#define _2K             0x00000800
#define _4K             0x00001000
#define _8K             0x00002000
#define _16K            0x00004000
#define _32K            0x00008000
#define _64K            0x00010000
#define _128K           0x00020000
#define _256K           0x00040000
#define _512K           0x00080000
#define _1M             0x00100000
#define _2M             0x00200000
#define _3M             0x00300000
#define _4M             0x00400000
#define _5M             0x00500000
#define _6M             0x00600000
#define _7M             0x00700000
#define _8M             0x00800000
#define _9M             0x00900000
#define _10M            0x00a00000
#define _11M            0x00b00000
#define _12M            0x00c00000
#define _13M            0x00d00000
#define _14M            0x00e00000
#define _15M            0x00f00000
#define _16M            0x01000000
#define _32M            0x02000000
#define _64M            0x04000000
#define _128M           0x08000000
#define _256M           0x10000000
#define _512M           0x20000000
#define _1G             0x40000000

#ifndef NULL
    #define NULL 0
#endif /* NULL */

#define NONE_CACHEABLE(X)		(0x00000000 | X)
                  
/* Read/Write 8/16/32 bits to/from GT`s registers */

#ifndef MV_REG_READ
    #define MV_REG_READ(offset, pData)                                         \
            (*(pData) = *(volatile unsigned int *)                             \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                
#endif /* MV_REG_READ */                                                       
                                                                               
#ifndef MV_REG_READ_CHAR                                                       
#define MV_REG_READ_CHAR(offset, pData)                                        \
        (*(pData) = *(volatile unsigned char *)                                \
        (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                    
#endif /* MV_REG_READ_CHAR */                                                  
                                                                               
#ifndef MV_REG_READ_SHORT                                                      
#define MV_REG_READ_SHORT(offset, pData)                                       \
        (*(pData) = *(volatile unsigned short *)                               \
        (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                    
#endif /* MV_REG_READ_SHORT */                                                 
                                                                               
#ifndef MVREGREAD                                                              
    #define MVREGREAD(offset)                                                  \
            (*(volatile unsigned int *)                                        \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                
#endif /* MVREGREAD */                                                         
                                                                               
#ifndef MVREGREAD_CHAR                                                         
    #define MVREGREAD_CHAR(offset)                                             \
            (*(volatile unsigned char *)                                       \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                
#endif /* MVREGREAD_CHAR */                                                    
                                                                               
#ifndef MVREGREAD_SHORT                                                        
    #define MVREGREAD_SHORT(offset)                                            \
            (*(volatile unsigned short *)                                      \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) | (offset)))                
#endif /* MVREGREAD_SHORT */                                                   
                                                                               
#ifndef MV_REG_WRITE                                                           
    #define MV_REG_WRITE(offset, data)                                         \
            ((*((volatile unsigned int *)                                      \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) |                           \
            (offset)))) = (data))                                              
#endif /* MV_REG_WRITE */                                                      
                                                                               
#ifndef MV_REG_WRITE_CHAR                                                      
    #define MV_REG_WRITE_CHAR(offset, data)                                    \
            ((*((volatile unsigned char *)                                     \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) |                           \
            (offset)))) = (data))                                              
#endif /* MV_REG_WRITE_CHAR */                                                 
                                                                               
#ifndef MV_REG_WRITE_SHORT                                                     
    #define MV_REG_WRITE_SHORT(offset, data)                                   \
            ((*((volatile unsigned short *)                                    \
            (NONE_CACHEABLE(mvInternalRegBaseAddr) |                           \
            (offset)))) = (data))                                              
#endif /* MV_REG_WRITE_SHORT */

/* Write 32/16/8 bit NonCacheable */

#define MV_WRITE_CHAR(address, data)                                           \
        ((*((volatile unsigned char *)NONE_CACHEABLE((address))))=             \
        ((unsigned char)(data)))
         
#define MV_WRITE_SHORT(address, data)                                          \
        ((*((volatile unsigned short *)NONE_CACHEABLE((address)))) =           \
        ((unsigned short)(data)))

#define MV_WRITE_WORD(address, data)                                           \
        ((*((volatile unsigned int *)NONE_CACHEABLE((address)))) =             \
        ((unsigned int)(data)))

/* Write 32/16/8 bit Cacheable */
#define MV_WRITE_CHAR_CACHEABLE(address, data)                                 \
        ((*((volatile unsigned char *)CACHEABLE(address))) =                   \
        ((unsigned char)(data)))

#define MV_WRITE_SHORT_CACHEABLE(address, data)                                \
        ((*((volatile unsigned short *)CACHEABLE(address))) =                  \
        ((unsigned short)(data)))

#define MV_WRITE_WORD_CACHEABLE(address, data)                                 \
        ((*((volatile unsigned int *)CACHEABLE(address))) =                    \
        ((unsigned int)(data)))

/* Read 32/16/8 bits NonCacheable - returns data in variable. */
#define MV_READ_CHAR(address, pData)                                           \
        (*(pData) = (*((volatile unsigned char *)(NONE_CACHEABLE(address)))))

#define MV_READ_SHORT(address, pData)                                          \
        (*(pData) = (*((volatile unsigned short *)NONE_CACHEABLE(address))))

#define MV_READ_WORD(address, pData)                                           \
        (*(pData) = (*((volatile unsigned int *)NONE_CACHEABLE(address))))

/* Read 32/16/8 bit NonCacheable - returns data direct. */
#define MV_READCHAR(address)                                                   \
        ((*((volatile unsigned char *)NONE_CACHEABLE(address))))

#define MV_READSHORT(address)                                                  \
        ((*((volatile unsigned short *)NONE_CACHEABLE(address))))

#define MV_READWORD(address)                                                   \
        ((*((volatile unsigned int *)NONE_CACHEABLE(address))))

/* Read 32/16/8 bit Cacheable */
#define MV_READ_CHAR_CACHEABLE(address, pData)                                 \
        (*(pData) = (*((unsigned char *)CACHEABLE(address))))

#define MV_READ_SHORT_CACHEABLE(address, pData)                                \
        (*(pData) = (*((unsigned short *)CACHEABLE(address))))

#define MV_READ_WORD_CACHEABLE(address, pData)                                 \
        (*(pData) = (*((unsigned int *)CACHEABLE(address))))

/* Read 32/16/8 bit Cacheable - returns data direct. */
#define MV_READCHAR_CACHEABLE(address)                                         \
        ((*((unsigned char *)CACHEABLE(address))))

#define MV_READSHORT_CACHEABLE(address)                                        \
        ((*((unsigned short *)CACHEABLE(address))))

#define MV_READWORD_CACHEABLE(address)                                         \
        ((*((unsigned int *)CACHEABLE(address))))

/* gets register offset and bits: a 32bit value. It set to logic '1' in the  
   register the bits which given as an input example:
   MV_SET_REG_BITS(0x840,BIT3 | BIT24 | BIT30) - set bits: 3,24 and 30 to logic
   '1' in register 0x840 while the other bits stays as is. */
#define MV_SET_REG_BITS(regOffset,bits)                                        \
        ((*((volatile unsigned int*)(NONE_CACHEABLE(mvInternalRegBaseAddr) |   \
        (regOffset)))) |= ((unsigned int)(bits)))

/* gets register offset and bits: a 32bit value. It set to logic '0' in the  
   register the bits which given as an input example:
   MV_RESET_REG_BITS(0x840,BIT3 | BIT24 | BIT30) - set bits: 3,24 and 30 to 
   logic '0' in register 0x840 while the other bits stays as is. */
#define MV_RESET_REG_BITS(regOffset,bits)                                      \
        ((*((volatile unsigned int*)(NONE_CACHEABLE(mvInternalRegBaseAddr) |   \
        (regOffset)))) &= ~((unsigned int)(bits)))
        

/* Functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __gtCoreh */


