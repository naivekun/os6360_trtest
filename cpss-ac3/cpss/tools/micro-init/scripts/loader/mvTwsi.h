/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#ifndef __INCmvTwsiH
#define __INCmvTwsiH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "defs.h"

/* Codes for controller HW status byte (taken from DataSheet) */
typedef enum mv_twsi_controller_status{

  MV_TWSI_STATUS_IDLE                  = 0xF8, /* Idle */

  MV_TWSI_STATUS_MSTR_STA              = 0x08, /* A START condition transmitted */
  MV_TWSI_STATUS_MSTR_REP_STA          = 0x10, /* A repeated START condition transmitted */
  MV_TWSI_STATUS_MSTR_TRAN_SLA_ACK     = 0x18, /* SLA+W transmitted; ACK received */
  MV_TWSI_STATUS_MSTR_TRAN_SLA_NACK    = 0x20, /* SLA+W transmitted; NACK received */
  MV_TWSI_STATUS_MSTR_TRAN_DATA_ACK    = 0x28, /* Data byte/s in I2CDAT/buffer transmitted; ACK received for all bytes*/
  MV_TWSI_STATUS_MSTR_TRAN_DATA_NACK   = 0x30, /* Data byte/s in I2CDAT/buffertransmitted; NACK received for last byte*/
  MV_TWSI_STATUS_MSTR_ARB_LOST         = 0x38, /* Arbitration lost in SLA+W or Data bytes */
  
  MV_TWSI_STATUS_MSTR_RECV_SLA_ACK     = 0x40, /* SLA+R transmitted; ACK received */
  MV_TWSI_STATUS_MSTR_RECV_SLA_NACK    = 0x48, /* SLA+R transmitted; NACK received */
  MV_TWSI_STATUS_MSTR_RECV_DATA_ACK    = 0x50, /* Data byte received; ACK returned */
  MV_TWSI_STATUS_MSTR_RECV_DATA_NACK   = 0x58, /* Data byte received; NACK returned (possibly end of transmition) */
  
  MV_TWSI_STATUS_SLAV_REC_OWN_SLA      = 0x60, /* Own SLA+W received; ACK returned */
  MV_TWSI_STATUS_SLAV_REC_ARB_LOST     = 0x68, /* Arbitration lost in SLA+R/W as master; Own SLA+W received, ACK returned */
  MV_TWSI_STATUS_SLAV_GEN_CAL          = 0xD0, /* General Call address (00h) received; ACK returned. */
  MV_TWSI_STATUS_SLAV_GEN_CAL_ARB_LOST = 0xD8, /* Arbitration lost in SLA = R/W as master; General Call address received; ACK bit returned. */
  MV_TWSI_STATUS_SLAV_REC_DATA_ACK     = 0x80, /* Previously addressed with own slave address; DATA received; ACK returned */
  MV_TWSI_STATUS_SLAV_REC_DATA_NACK    = 0x88, /* Previously addressed with own slave address; DATA byte received; NACK returned for last byte */
  MV_TWSI_STATUS_SLAV_GEN_DATA_ACK     = 0xE0, /* Previously addressed with General Call; Data received; ACK returned */
  MV_TWSI_STATUS_SLAV_GEN_DATA_NACK    = 0xE8, /* Previously addressed with General Call; Data received; NACK returned for last byte */
  MV_TWSI_STATUS_SLAV_STO_REP_STA      = 0xA0, /* A STOP/repeated START received while still addressed as Slave Receiver */
 
  MV_TWSI_STATUS_SLAV_TRA_OWN_SLA      = 0xA8, /* Own SLA+R received; ACK returned */
  MV_TWSI_STATUS_SLAV_TRA_ARB_LOST     = 0xB0, /* Arbitration lost in SLA+R/W as master; Own SLA+W received, ACK returned */
  MV_TWSI_STATUS_SLAV_TRA_DATA_ACK     = 0xB8, /* Data bytes transmitted; ACK received */
  MV_TWSI_STATUS_SLAV_TRA_DATA_NACK    = 0xC0, /* Data bytes transmitted; NACK received for last byte */
  MV_TWSI_STATUS_SLAV_REC_DATA_ACK_NA  = 0xC8, /* Data bytes transmitted; ACK received (AA = 0) */

  MV_TWSI_STATUS_BUS_ERROR_SDA         = 0x70,  /* Bus error SDA stuck low */
  MV_TWSI_STATUS_BUS_ERROR_SCL         = 0x78,  /* Bus error SCL stuck low */
  MV_TWSI_STATUS_INVALID_COUNT_VALUE   = 0xFC,  /* Invalid value has been loaded to I2CCOUNT register */
  MV_TWSI_STATUS_BUS_ERROR_ILLEGAL     = 0x00   /* Bus error due to illegal start/stop */
  
 }MV_TWSI_CONTROLLER_STATUS;

/* Internal registers offsets */
typedef enum mv_twsi_registers{
  MV_TWSI_REG_SLV_ADDR = 0x00, /* slave (own) address */
  MV_TWSI_REG_DATA     = 0x04, /* data */
  MV_TWSI_REG_CTRL     = 0x08, /* control */
  MV_TWSI_REG_STATUS   = 0x0C, /* status [Read only]*/
  MV_TWSI_REG_BAUD     = 0x0C, /* BAUD rate */
  MV_TWSI_REG_RESET    = 0x1C  /* soft reset */
}MV_TWSI_REGISTERS;

/* Control register bits */
typedef enum mv_twsi_control_bit{
  MV_TWSI_CTRL_BIT_IE    = 0x80, /* Interrupt enable */
  MV_TWSI_CTRL_BIT_SLAVE = 0x40, /* Enable slave */
  MV_TWSI_CTRL_BIT_STA   = 0x20, /* Set START bit */
  MV_TWSI_CTRL_BIT_STO   = 0x10, /* Set STOP bit */
  MV_TWSI_CTRL_BIT_IF    = 0x08, /* Interrupt flag */
  MV_TWSI_CTRL_BIT_ACK   = 0x04  /* Ack */
}MV_TWSI_CONTROL_BIT;

/* Interrupt machine states */
typedef enum mv_twsi_controller_state{
  MV_TWSI_STATE_IDLE           , /* Idle */
  MV_TWSI_STATE_STA            , /* A START condition should have been transmitted */
  MV_TWSI_STATE_TRAN           , /* In master transmitter mode, slave address + bytes transmitted */
  MV_TWSI_STATE_REC            , /* In master receiver mode, slave address transmitted and bytes received */
  MV_TWSI_STATE_OFFS           , /* In master receiver mode, transmitting offset */
  MV_TWSI_STATE_DONE             /* Master transaction ended */
 }MV_TWSI_CONTROLLER_STATE;

/* The TWSI interface supports both 7-bit and 10-bit addressing. */
/* This enumerator describes addressing type.                    */
typedef enum _mvTwsiAddrType
{
  ADDR7_BIT,                            /* 7 bit address  */
  ADDR10_BIT                            /* 10 bit address */
}MV_TWSI_ADDR_TYPE;

/* This structure describes TWSI address. */
typedef struct _mvTwsiAddr
{
  MV_U32            address;            /* address      */
  MV_TWSI_ADDR_TYPE type;               /* Address type */
}MV_TWSI_ADDR;

/* This structure describes a TWSI slave. */
typedef struct _mvTwsiSlave
{
  MV_TWSI_ADDR slaveAddr;
  MV_BOOL      validOffset;             /* whether the slave has offset (i.e. Eeprom  etc.) */
  MV_U32       offset;                  /* offset in the slave. */
  MV_U32       offset_length;           /* # of bytes that compose the offset */
}MV_TWSI_SLAVE;                  
 
/* Definitions */
#define MV_TWSI_EXTERNAL_BUFFER_SIZE 128
/*#define MV_TWSI_BASE_ADDR        0xF1011000*/
#define MV_TWSI_BASE_ADDR        0x11000

#define MV_TWSI_TRANSMIT     0x00
#define MV_TWSI_RECEIVE      0x01

/* TWSI access MACROS */

/* Direct TWSI access */
/*
#define MV_TWSI_WRITE_REG(addr,data) \
        *(volatile UINT32 *)(MV_TWSI_BASE_ADDR + addr)=data
       
#define MV_TWSI_READ_REG(addr) \
        (*(volatile UINT32 *)(MV_TWSI_BASE_ADDR + addr))
*/

/* Through PCI TWSI access */
#define MV_TWSI_WRITE_REG(addr,data) \
          sysfs_pci_write(mg_space_ptr, MV_TWSI_BASE_ADDR + addr, data)

#define MV_TWSI_READ_REG(addr) \
        sysfs_pci_read(mg_space_ptr, MV_TWSI_BASE_ADDR + addr)


#define MV_TWSI_BAUD(M, N)  ((M << 3) | (N & 0x7))
/***********************************************************************
 * MV_TWSI_INIT()                                                      *
 *    Initialize controller and global regisers                        *
 ***********************************************************************/
UINT32 mv_twsi_init(UINT32 frequency, UINT32 tclk);

/***********************************************************************
 * MV_TWSI_MASTER_TRANSCEIVE(BUFFER, LENGTH, SLAVE_ADDRESS, OFFSET,
 *                                           OFFSET_LENGTH, TRNS_REC)  *
 *    Transmit / receive BUFFER on I2C bus to / from SLAVE_ADDRESS     *
 *    inputs:                                                          *
 *      TX_RX    = 1 for transmit, 0 for receive.                      *
 *      OFFSET   = offset of register in slave.                        *
 *      OFFSET_LENGTH = # of bytes that compose the offset.            *
 *    output:                                                          *
 *      0 on success, 1 if not ready                                   *
 ***********************************************************************/
UINT32 mv_twsi_master_transceive( UINT8 *buffer,
                                  UINT32 length,
                                  MV_TWSI_SLAVE *slave,
                                  UINT32 tx_rx);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvTwsiH */
