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

#include "string.h"
#include "stdio.h"
#include <mvTwsi.h>

#include <time.h>

#undef MV_DEBUG
//#define MV_DEBUG

static void mv_twsi_isr(void);
#ifdef MV_DEBUG
static void mv_twsi_add_dbg_str(char *str);
#endif /*MV_DEBUG*/

/* GLOBALS */
static MV_TWSI_CONTROLLER_STATE  mv_twsi_controller_state = MV_TWSI_STATE_IDLE;
static UINT8         mv_twsi_slave_address;
static UINT8         mv_twsi_buffer[MV_TWSI_EXTERNAL_BUFFER_SIZE];
static UINT32        mv_twsi_length;
static UINT32        mv_twsi_buffer_offset;
static UINT8*        mv_twsi_user_buffer_ptr;
static UINT32        mv_twsi_reset_counter = 0;
static UINT32        mv_twsi_offset_length;
static MV_STATUS     mv_twsi_status;

#ifdef MV_DEBUG
char                 mv_twsi_dbg_buf[0x5000];
static char          dbg_str[0x100];
#endif /*MV_DEBUG*/

extern void *mg_space_ptr;

extern void sysfs_pci_write(void* vaddr, int offset, uint32_t value);
extern uint32_t sysfs_pci_read(void* vaddr, int offset);

const static struct timespec mv_twsi_delay = {0/*sec*/, 300000/*nSec*/};
const static struct timespec mv_twsi_delay_tx = {0/*sec*/, 5000000/*nSec*/};

/***********************************************************************
 * MV_TWSI_RESET()                                                     *
 *    Reset controller in case of error: calls init as well            *
 ***********************************************************************/
static void mv_twsi_reset(void)
{
  MV_TWSI_WRITE_REG(MV_TWSI_REG_RESET, 0);
}

/***********************************************************************
 * MV_TWSI_RELINQUISH()                                                *
 *    When called from ISR - abandon transaction and quit              *
 ***********************************************************************/
static void mv_twsi_relinquish(void)
{
  mv_twsi_controller_state = MV_TWSI_STATE_IDLE;

/* DEBUG - remove */
#ifdef MV_DEBUG
  if(mv_twsi_status != MV_FAIL)
  {
    mv_twsi_reset_counter = 0;
    printf("TWSI_DBG %s\n",dbg_str);
    //mv_twsi_add_dbg_str(dbg_str);
  }
  else
  {
    if(mv_twsi_reset_counter < 10)
    {
      mv_twsi_reset_counter++;
      printf("TWSI_DBG %s\n",dbg_str);
      //mv_twsi_add_dbg_str(dbg_str);
    }
  }
#endif
/* End of DEBUG */

}


/***********************************************************************
 * MV_TWSI_INIT()                                                      *
 *    Initialize controller and global regisers                        *
 ***********************************************************************/
UINT32 mv_twsi_init(UINT32 frequency, UINT32 tclk)
{
  UINT32  power, n, actualN, m, actualM, freq, actualFreq, minMargin = 0xffffffff;
  UINT32  val;

  /* Set state machine to IDLE */
  mv_twsi_controller_state = MV_TWSI_STATE_IDLE;

  if(frequency > 100000)
  {
    printf("Warning TWSI frequency is too high, please use up tp 100Khz. \n");
  }

  /* Calucalte N and M for the TWSI clock baud rate */
  for(n = 0 ; n < 8 ; n++)
    for(m = 0 ; m < 16 ; m++)
    {
      power = 2 << n; /* power = 2^(n+1) */
      freq = tclk/(10*(m+1)*power);
      if(MV_ABS(frequency - freq) < minMargin)
      {
        minMargin   = MV_ABS(frequency - freq);
        actualFreq  = freq;
        actualN     = n;
        actualM     = m;
      }
    }

  /* Reset controller */
  mv_twsi_reset();

  /* Set the baud rate */
  MV_TWSI_WRITE_REG(MV_TWSI_REG_BAUD, MV_TWSI_BAUD(actualM, actualN));
  /* Enable TWSI */
  MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, MV_TWSI_CTRL_BIT_SLAVE); 

  /* DEBUG */
#ifdef MV_DEBUG
      memset(mv_twsi_dbg_buf, 0, sizeof(mv_twsi_dbg_buf));
#endif /*MV_DEBUG*/
  /* DEBUG */

  return actualFreq;
}


/***********************************************************************
 * MV_TWSI_MASTER_TRANSCEIVE(BUFFER, LENGTH, SLAVE, TRNS_REC)          *
 *    Transmit / receive BUFFER on I2C bus to / from SLAVE_ADDRESS     *
 *    Initialize globals, prepares buffer, set start bit and loop for  *
 *    completion.                                                      *
 *    inputs:                                                          *
 *      TX_RX    = 0 for transmit, 1 for receive.                      *
 *    output:                                                          *
 *      0 on success, 1 if not ready                                   *
 ***********************************************************************/
UINT32 mv_twsi_master_transceive( UINT8 *buffer,
                                  UINT32 length,
                                  MV_TWSI_SLAVE *slave,
                                  UINT32 tx_rx)
{
  UINT8 temp, i=0;

  if(MV_TWSI_STATE_IDLE != mv_twsi_controller_state)
    return MV_FAIL;

  if(length > MV_TWSI_EXTERNAL_BUFFER_SIZE)
    return MV_FAIL;

  /* Set global values for ISR, copy buffer, set START signal */
  temp = mv_twsi_slave_address = slave->slaveAddr.address | tx_rx;
  mv_twsi_controller_state = MV_TWSI_STATE_STA;
  mv_twsi_length = length;
  mv_twsi_buffer_offset = 0;

  /* Copy the slave register offset bytes to transmit buffer */
  if(slave->validOffset)
  {
    mv_twsi_offset_length = slave->offset_length;
    /* Copy offset bytes to transmit buffer */
    for(i=0; i<mv_twsi_offset_length ;i++)
      mv_twsi_buffer[i] = (UINT8)(((slave->offset) >> ((mv_twsi_offset_length - 1 - i) * 8)) & 0xFF);
  }

  if(MV_TWSI_TRANSMIT == tx_rx) {
    memcpy(mv_twsi_buffer + i, buffer, length);
    if(slave->validOffset) {
      length += mv_twsi_offset_length;
      mv_twsi_length = length;
    }
  }
  else/* Receiver mode */
  {
    if(slave->validOffset)
      temp = slave->slaveAddr.address | MV_TWSI_TRANSMIT;
    mv_twsi_user_buffer_ptr = buffer;
  }/* Receiver mode */

  /* Load slave address + W/R bit */
  MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, temp);

  /* Set START signal on bus */
  temp = MV_TWSI_READ_REG(MV_TWSI_REG_CTRL);
  MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, temp | MV_TWSI_CTRL_BIT_STA);

  mv_twsi_status = MV_FAIL;

/* DEBUG - remove */
#ifdef MV_DEBUG
    memset(dbg_str, 0, sizeof(dbg_str));
    sprintf(dbg_str, "Addr 0x%02x %d bytes, offs 0x%x - ",
                      mv_twsi_slave_address,
                      length,
                      slave->offset);
#endif /*MV_DEBUG*/

  while(MV_TWSI_STATE_IDLE != mv_twsi_controller_state)
  {
    nanosleep(&mv_twsi_delay, NULL);
    mv_twsi_isr();
  }

  if(MV_TWSI_TRANSMIT == tx_rx) {
    nanosleep(&mv_twsi_delay_tx, NULL);
  }

  return mv_twsi_status;
}



/***********************************************************************
 * MV_TWSI_ISR()                                                       *
 ***********************************************************************/
static void mv_twsi_isr(void)
{
  MV_TWSI_CONTROLLER_STATUS status;
  UINT8 ctrl;
  UINT32 cause;

  /* Read ctrl & status registers */
  status = MV_TWSI_READ_REG(MV_TWSI_REG_STATUS);
  ctrl  = MV_TWSI_READ_REG(MV_TWSI_REG_CTRL) & ~MV_TWSI_CTRL_BIT_IF;

/* DEBUG - remove */
#ifdef MV_DEBUG
  sprintf(dbg_str + strlen(dbg_str), "%d 0x%02x %02d, ", mv_twsi_controller_state, ctrl, status);
  if((mv_twsi_controller_state != MV_TWSI_STATE_STA) && (mv_twsi_controller_state != MV_TWSI_STATE_DONE))
    sprintf(dbg_str + strlen(dbg_str), "data 0x%02x, ", mv_twsi_buffer[mv_twsi_buffer_offset]);
#endif /*MV_DEBUG*/
  switch(mv_twsi_controller_state)
  {


/*--------------  Master start transmitted  --------------*/
/* This iteration will only clear the interrupt flag 
 * so slave address can be transmitted
 */
    case MV_TWSI_STATE_STA:
      /* Start signal transmitted */
      switch(status)
      {
        case MV_TWSI_STATUS_MSTR_STA:
        case MV_TWSI_STATUS_MSTR_REP_STA:
          /* 
           * Start signal transmitted successfully.
           * Set next state to master tx or rx.
           */
          if(mv_twsi_slave_address & MV_TWSI_RECEIVE)
          {
             if(mv_twsi_offset_length == 0)
               mv_twsi_controller_state = MV_TWSI_STATE_REC;
             else
               mv_twsi_controller_state = MV_TWSI_STATE_OFFS;
          }
          else   /* Master transmitter */
            mv_twsi_controller_state = MV_TWSI_STATE_TRAN;
          break;

        default:
          /* Error or Bytes have been transmitted and nacked
           * - end of transmition 
           */
          mv_twsi_reset();
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
      }
      break;


/*--------------  Master transmitter  --------------*/
    case MV_TWSI_STATE_TRAN:

      switch(status)
      {
        case MV_TWSI_STATUS_MSTR_TRAN_SLA_ACK:
        case MV_TWSI_STATUS_MSTR_TRAN_DATA_ACK:
          /*
           * Either slave acked it's address or data byte/s
           * transmitted successfully. Load next byte/s or stop.
           */
          if(mv_twsi_length <= mv_twsi_buffer_offset)
          { /* No more bytes - end transmition */
            ctrl |= MV_TWSI_CTRL_BIT_STO;
            mv_twsi_status = MV_OK;
            mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          }
          else  /* Transmit next byte if any left */
          {           
            /* Load bytes to data for TX */
            MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA,
                                     mv_twsi_buffer[mv_twsi_buffer_offset++]);
          }
          break;
            
        case MV_TWSI_STATUS_MSTR_TRAN_SLA_NACK:
          /* No Slave acked it's address. */
          ctrl |= MV_TWSI_CTRL_BIT_STO;
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
          
        default:
          /* Error or Bytes have been transmitted and nacked
           * - end of transmition 
           */
          mv_twsi_reset();
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
      }
      break;






/*--------------  Transmit offset  --------------*/

    case MV_TWSI_STATE_OFFS:

      switch(status)
      {
        case MV_TWSI_STATUS_MSTR_TRAN_SLA_ACK:
        case MV_TWSI_STATUS_MSTR_TRAN_DATA_ACK:
          /*
           * Either slave acked it's address or offset byte/s
           * transmitted successfully. Load next byte/s or restart.
           */
          if(mv_twsi_offset_length > 0)
          {
            mv_twsi_offset_length--;
            /* Load bytes to data for TX */
            MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA,
                                     mv_twsi_buffer[mv_twsi_buffer_offset++]);
          }
          else
          {
            /* Offset has been transmitted - set repeated start, go to receiver mode */
            mv_twsi_buffer_offset = 0;
            ctrl |= MV_TWSI_CTRL_BIT_STA;
            /* Load slave address + W/R bit */
            MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, mv_twsi_slave_address);
            mv_twsi_controller_state = MV_TWSI_STATE_STA;
          }
          break;
        case MV_TWSI_STATUS_MSTR_TRAN_SLA_NACK:
          /* No Slave acked it's address. */
          ctrl |= MV_TWSI_CTRL_BIT_STO;
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
          
        default:
          /* Error or Bytes have been transmitted and nacked
           * - end of transmition 
           */
          mv_twsi_reset();
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
      }
      break;


/*--------------  Master receiver  --------------*/
    case MV_TWSI_STATE_REC:

      switch(status)
      {
        case MV_TWSI_STATUS_MSTR_RECV_DATA_NACK:
          /* We're done receiving. Read byte/s, set stop bit, return to IDLE */
        case MV_TWSI_STATUS_MSTR_RECV_DATA_ACK:
          /*
           * Slave sent byte/s, we acked. Read byte/s to external buffer.
           * Don't ack last byte if no more space in external buffer.
           */

          mv_twsi_buffer[mv_twsi_buffer_offset] = MV_TWSI_READ_REG(MV_TWSI_REG_DATA);

          if(MV_TWSI_STATUS_MSTR_RECV_DATA_NACK == status)
          { /* End of communication - copy buffer, stop bit and IDLE */
            memcpy(mv_twsi_user_buffer_ptr, mv_twsi_buffer, mv_twsi_length);
            ctrl |= MV_TWSI_CTRL_BIT_STO;
            mv_twsi_status = MV_OK;
            mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          }
          else /* Continue to receive next byte */
            if(mv_twsi_length - mv_twsi_buffer_offset == 1)
              ctrl &= ~MV_TWSI_CTRL_BIT_ACK; /* Don't acknowledge next byte */
            else
              ctrl |= MV_TWSI_CTRL_BIT_ACK; /* Acknowledge next byte */

          mv_twsi_buffer_offset++;
          break;

        case MV_TWSI_STATUS_MSTR_RECV_SLA_ACK:
            /* Slave acked it's address. Set ack if needed, and receive first byte */
            if(mv_twsi_length > 1)
              ctrl |= MV_TWSI_CTRL_BIT_ACK; /* Acknowledge next byte */
          break;

        case MV_TWSI_STATUS_MSTR_RECV_SLA_NACK:
          /* Slave nacked it's address. */
          ctrl |= MV_TWSI_CTRL_BIT_STO;
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;

        default:
          /* Error or Bytes have been transmitted and nacked
           * - end of transmition 
           */
          mv_twsi_reset();
          mv_twsi_controller_state = MV_TWSI_STATE_DONE;
          break;
      }

      break;

/*--------------  Master done  --------------*/
    case MV_TWSI_STATE_DONE:

      MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, ctrl);
      mv_twsi_relinquish();
      return;


  default:
    mv_twsi_reset();
    mv_twsi_controller_state = MV_TWSI_STATE_DONE;
  }

  /* Clear interrupt */
  MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, ctrl);
}


/* DEBUG - remove */
#ifdef MV_DEBUG
static void mv_twsi_add_dbg_str(char *str)
{
  static UINT32 index = 0, current_pos = 0;
  UINT32 size, space_left;

  space_left = sizeof(mv_twsi_dbg_buf) - current_pos;

  /* Roll over if not enough space left */
  size = strlen(str);
  /*If not enough space left - set space to zero and roll over */
  if(space_left < size + 12)
  {
    memset(mv_twsi_dbg_buf + current_pos, 0, space_left);
    current_pos = 0;
  }

  /* Start new line with serial number. */
  sprintf(mv_twsi_dbg_buf + current_pos, "\n%08x: ", index++);
  current_pos += 11;

  /* Copy first part of string to current position */
  memcpy(mv_twsi_dbg_buf + current_pos, str, size);

  /* update positions */
  current_pos += size;
}
#endif /*MV_DEBUG*/
