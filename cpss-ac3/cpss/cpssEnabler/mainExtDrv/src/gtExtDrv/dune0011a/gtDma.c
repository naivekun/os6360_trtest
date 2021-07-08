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
* @file gtDma.c
*
* @brief This file gives the user a complete interface to the powerful DMA engines,
* including functions for controling the priority mechanism.
* To fully understand the capabilities of the DMA engines please spare some
* time to go trough the spec.
*
*/

/* includes */

#include "gtDma.h"

#define GT64131_ADDRESS 0x14000000

#define NONE_CACHEABLE(address) (address)

#ifndef GT_REG_WRITE
    #define GT_REG_WRITE(offset, data)                                      \
            (volatile unsigned int)*((unsigned int *)                       \
            (NONE_CACHEABLE(GT64131_ADDRESS) |                        \
            (offset))) = (data)

#endif

#define GT_WRITE_WORD(address, data)                                        \
        (volatile unsigned int)*((unsigned int *)                           \
        (NONE_CACHEABLE(address))) = (unsigned int)(data)

#define VIRTUAL_TO_PHY(address) ((unsigned int)address)
#define PHY_TO_VIRTUAL(address) ((unsigned int)address)

#define GT_WORD_SWAP(X) (X)


/* GT_SET_REG_BITS(regOffset,bits) -
   gets register offset and bits: a 32bit value. It set to logic '1' in the
   register the bits which given as an input example:
   GT_SET_REG_BITS(0x840,BIT3 | BIT24 | BIT30) - set bits: 3,24 and 30 to logic
   '1' in register 0x840 while the other bits stays as is. */
#define GT_SET_REG_BITS(regOffset,bits)                                     \
        (volatile unsigned int)*(unsigned int*)                             \
        (NONE_CACHEABLE(GT64131_ADDRESS) | (regOffset)) |=            \
        (unsigned int)GT_WORD_SWAP(bits)



/* Read/Write to/from GT`s registers */
#ifndef GT_REG_READ
    #define GT_REG_READ(offset, pData)                                      \
            *pData = ( (volatile unsigned int)*((unsigned int *)            \
                     (NONE_CACHEABLE(GT64131_ADDRESS) | (offset))) ); \
            *pData = GT_WORD_SWAP(*pData)
#endif

/****************************************/
/* DMA Record                           */
/****************************************/

#define DMA_CHANNEL0_BYTE_COUNT                               0x800
#define DMA_CHANNEL1_BYTE_COUNT                               0x804
#define DMA_CHANNEL2_BYTE_COUNT                               0x808
#define DMA_CHANNEL3_BYTE_COUNT                               0x80C

#define DMA_CHANNEL0_SOURCE_ADDR                              0x810
#define DMA_CHANNEL1_SOURCE_ADDR                              0x814
#define DMA_CHANNEL2_SOURCE_ADDR                              0x818
#define DMA_CHANNEL3_SOURCE_ADDR                              0x81C

#define DMA_CHANNEL0_DESTINATION_ADDR                         0x820
#define DMA_CHANNEL1_DESTINATION_ADDR                         0x824
#define DMA_CHANNEL2_DESTINATION_ADDR                         0x828
#define DMA_CHANNEL3_DESTINATION_ADDR                         0x82C

#define DMA_CHANNEL0_NEXT_RECORD_POINTER                      0x830
#define DMA_CHANNEL1_NEXT_RECORD_POINTER                      0x834
#define DMA_CHANNEL2_NEXT_RECORD_POINTER                      0x838
#define DMA_CHANNEL3_NEXT_RECORD_POINTER                      0x83C

#define DMA_CHANNEL0_CURRENT_DESCRIPTOR_POINTER               0x870
#define DMA_CHANNEL1_CURRENT_DESCRIPTOR_POINTER               0x874
#define DMA_CHANNEL2_CURRENT_DESCRIPTOR_POINTER               0x878
#define DMA_CHANNEL3_CURRENT_DESCRIPTOR_POINTER               0x87C

/****************************************/
/* DMA Channel Control                  */
/****************************************/

#define DMA_CHANNEL0_CONTROL                                  0x840
#define DMA_CHANNEL1_CONTROL                                  0x844
#define DMA_CHANNEL2_CONTROL                                  0x848
#define DMA_CHANNEL3_CONTROL                                  0x84C


/****************************************/
/* DMA Arbiter                          */
/****************************************/

#define DMA_ARBITER_CONTROL_0_3                               0x860
#define DMA_ARBITER_CONTROL_4_7                               0x960



/****************************************/
/*          GENERAL Definitions         */
/****************************************/

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

/**
* @internal gtDmaCommand function
* @endinternal
*
*/
GT_BOOL gtDmaCommand(DMA_ENGINE channel,unsigned int command)
{
    unsigned int    offset=0;
    if(channel > DMA_LAST_ENGINE )
        return GT_FALSE;
    if (channel > DMA_ENG_3 )
        offset = 0x100;       /* Access to the four additional gtDma engine. */
    GT_REG_WRITE(DMA_CHANNEL0_CONTROL + (channel % 4)*4 + offset,command);
    return GT_TRUE;
}

/**
* @internal gtDmaTransfer function
* @endinternal
*
*/
DMA_STATUS gtDmaTransfer (DMA_ENGINE channel,unsigned int sourceAddr,
                        unsigned int destAddr,unsigned int numOfBytes,
                        unsigned int command,DMA_RECORD *nextRecordPointer)
{
    unsigned int offset=0;
    unsigned int tempData, checkBits, alignmentOffset = 0;
    unsigned int descripOwner = NO_BIT;
    unsigned int flagDTL = 0x0;
    DMA_RECORD *next = nextRecordPointer;

    if(channel > DMA_LAST_ENGINE )
        return DMA_NO_SUCH_CHANNEL;
    if (numOfBytes > 0xffff)
    {
            command = command | DMA_NEW_DESCRIPTOR_MODE;
            descripOwner = BIT31;
    }
    if ( numOfBytes > (_16M-1) )
        return DMA_GENERAL_ERROR;
    if (channel > DMA_ENG_3 )
        offset = 0x100;       /* Access to the four additional gtDma engine. */
    if( gtDmaIsChannelActive(channel) )
        return DMA_CHANNEL_BUSY;

    if((command & (BIT0 | BIT1 | BIT2)) != 0x0)
    {
        flagDTL = 0x1;         /* different DTL for source and destination*/
        if(((command & BIT1) !=0) && ((command & (BIT0 | BIT2)) == 0x0))
            command = command & ~BIT1;   /* real definition of DMA_DTL_8BYTES */
    }

    if(next != NULL) /* case of chain Mode */
    {
        alignmentOffset = ((unsigned int)next % 16);
    }
    checkBits = command & (DMA_REC_ADDR_IN_PCI0 | DMA_REC_ADDR_IN_PCI1);
    if(checkBits == 0)
    {
        while(next != NULL)
        {
            tempData = (unsigned int)next->ByteCnt;
            GT_WRITE_WORD(VIRTUAL_TO_PHY((unsigned int)next - alignmentOffset),
                        GT_WORD_SWAP(tempData | descripOwner));
            GT_WRITE_WORD(((unsigned int)next - alignmentOffset),
                        GT_WORD_SWAP(tempData | descripOwner));
            tempData = (unsigned int)next->SrcAdd;
            GT_WRITE_WORD(VIRTUAL_TO_PHY((unsigned int)next + 4  -
                                         alignmentOffset),
                       GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData)));
            GT_WRITE_WORD(((unsigned int)next + 4  - alignmentOffset),
                       GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData)));
            tempData = (unsigned int)next->DestAdd;
            GT_WRITE_WORD(VIRTUAL_TO_PHY((unsigned int)next + 8  -
                                         alignmentOffset),
                       GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData)));
            GT_WRITE_WORD(((unsigned int)next + 8  - alignmentOffset),
                       GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData)));
            tempData = (unsigned int)next->NextRecPtr;
            if(tempData == 0)
            {
                GT_WRITE_WORD(VIRTUAL_TO_PHY((unsigned int)next + 12 -
                                             alignmentOffset),NULL);
                GT_WRITE_WORD(((unsigned int)next + 12 - alignmentOffset),NULL);
            }
            else
            {
                GT_WRITE_WORD(VIRTUAL_TO_PHY((unsigned int)next + 12 -
                                             alignmentOffset),
                        GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData -
                                                    alignmentOffset)));
                GT_WRITE_WORD(((unsigned int)next + 12 - alignmentOffset),
                        GT_WORD_SWAP(VIRTUAL_TO_PHY(tempData -
                                                    alignmentOffset)));
            }
            next = (DMA_RECORD *)tempData;
            if(next == nextRecordPointer)
                next = NULL;
        }
    }
    GT_REG_WRITE(DMA_CHANNEL0_BYTE_COUNT + (channel % 4)*4 + offset,numOfBytes);
    tempData = sourceAddr;
    if(command & (DMA_SOURCE_ADDR_IN_PCI0 | DMA_SOURCE_ADDR_IN_PCI1))
    {
        GT_REG_WRITE(DMA_CHANNEL0_SOURCE_ADDR + (channel % 4)*4 + offset,
                     (tempData));
    }
    else
    {
        GT_REG_WRITE(DMA_CHANNEL0_SOURCE_ADDR + (channel % 4)*4 + offset,
                     VIRTUAL_TO_PHY(tempData));
    }
    tempData = destAddr;
    if(command & (DMA_DEST_ADDR_IN_PCI0 | DMA_DEST_ADDR_IN_PCI1))
    {
        GT_REG_WRITE(DMA_CHANNEL0_DESTINATION_ADDR + (channel % 4)*4 + offset,
                    (tempData));
    }
    else
    {
        GT_REG_WRITE(DMA_CHANNEL0_DESTINATION_ADDR + (channel % 4)*4 + offset,
                    VIRTUAL_TO_PHY(tempData));
    }
    if(nextRecordPointer != NULL)
    {
        tempData = (unsigned int)nextRecordPointer - alignmentOffset;
        if(command & (DMA_REC_ADDR_IN_PCI0 | DMA_REC_ADDR_IN_PCI1))
        {
            GT_REG_WRITE(DMA_CHANNEL0_NEXT_RECORD_POINTER +
                         4*(channel % 4) + offset, (tempData));
        }
        else
        {
            GT_REG_WRITE(DMA_CHANNEL0_NEXT_RECORD_POINTER +
                         4*(channel % 4) + offset, VIRTUAL_TO_PHY(tempData));
        }
        command = command | DMA_CHANNEL_ENABLE;
    }
    else
    {
        command = command | DMA_CHANNEL_ENABLE | DMA_NON_CHAIN_MODE;
    }
#if 0
    if(flagDTL == 0x1)
    {
        /* Enable DMA transfer with different DTL for Source and Destination
           in case that Source and Destination DTL are really different */
        if((command & (BIT0 | BIT1 | BIT2)) !=
           ((command & (BIT6 | BIT7 | BIT8)) >> 6))
            GT_SET_REG_BITS(DMA_CHANNEL0_CONTROL_HIGH + (channel % 4)*4 +
                            offset, BIT31);
    }
#endif
    /* Activate DMA engine By writting to gtDmaControlRegister */
    GT_REG_WRITE(DMA_CHANNEL0_CONTROL + (channel % 4)*4 + offset,command);
    return DMA_OK;
}

/**
* @internal gtDmaIsChannelActive function
* @endinternal
*
*/
DMA_STATUS gtDmaIsChannelActive(DMA_ENGINE channel)
{
    unsigned int    offset=0;
    unsigned int    data;
    if(channel > DMA_LAST_ENGINE)
        return DMA_NO_SUCH_CHANNEL;
    if (channel > DMA_ENG_3 )
        offset = 0x100;   /* Access to the four additional gtDma engine. */
    GT_REG_READ(DMA_CHANNEL0_CONTROL + 4*(channel % 4) + offset,&data);
    if(data & DMA_ACTIVITY_STATUS)
    {
        return DMA_CHANNEL_BUSY;
    }
    return DMA_CHANNEL_IDLE;
}

/**
* @internal gtDmaEngineDisable function
* @endinternal
*
*/
GT_BOOL gtDmaEngineDisable(DMA_ENGINE engine)
{
    unsigned int    offset=0;

    if(engine > DMA_LAST_ENGINE)
        return GT_FALSE;
    if (engine > DMA_ENG_3 )
        offset = 0x100;   /* Access to the four additional gtDma engine. */
    GT_SET_REG_BITS(DMA_CHANNEL0_CONTROL + 4*(engine % 4) + offset,BIT20);
    return GT_TRUE;

}

/**
* @internal gtDmaUpdateArbiter0_3 function
* @endinternal
*
*/
GT_BOOL gtDmaUpdateArbiter0_3(DMA_PIZZA0_3* gtDmaPizza)
{
    unsigned int i;
    unsigned int arbiterPrio = 0;
    unsigned int baseAddress=(unsigned int)(gtDmaPizza);
    unsigned int temp;

    for (i=0 ; i<16 ;i++)
    {
        temp = *(unsigned char *)( baseAddress + i );
        if ( temp>3 )
        {
            return GT_FALSE;
        }
        arbiterPrio = arbiterPrio << 2;
        arbiterPrio = arbiterPrio + temp;
    }
    GT_REG_WRITE(DMA_ARBITER_CONTROL_0_3,arbiterPrio);
    return GT_TRUE;
}

/**
* @internal gtDmaUpdateArbiter4_7 function
* @endinternal
*
*/
GT_BOOL gtDmaUpdateArbiter4_7(DMA_PIZZA4_7* gtDmaPizza)
{
    unsigned int i;
    unsigned int arbiterPrio = 0;
    unsigned int baseAddress=(unsigned int)(gtDmaPizza);
    unsigned int temp;

    for (i=0 ; i<16 ;i++)
    {
        temp = *(unsigned int *)( baseAddress + i );
        temp = temp - 4;
        if (temp>3)
        {
            return GT_FALSE;
        }
        arbiterPrio = arbiterPrio << 2;
        arbiterPrio = arbiterPrio + temp;
    }
    GT_REG_WRITE(DMA_ARBITER_CONTROL_4_7,arbiterPrio);
    return GT_TRUE;
}



