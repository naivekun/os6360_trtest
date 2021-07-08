/*************************************************************************
*
* Copyright (c) Microchip Technology Inc. All rights reserved.
* Licensed under the MIT License. See LICENSE in the project root for license information.
*
*************************************************************************
*
*  File Revision: 1.0
*
*************************************************************************
*
*  Description: contain the implementation of the functions.
*
*************************************************************************/



/*=========================================================================
/ INCLUDES
/========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>

#include "../../host_poe_api/inc/host_poe_api.h"
#include "../../host_poe_api/inc/host_poe_communication.h"
#include "../inc/host_poe_comm_device.h"
#include "../lib/i2cbusses.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/*=========================================================================
/ LOCAL VARIABLES
/========================================================================*/

/** UART***********/

#define BAUDRATE B19200
#define I2C_BUFFER_SIZE 50

static fd_set readfs;    /* file descriptor set */
static int    maxfd;     /* maximum file desciptor used */

static int tty_fd;
static struct termios oldtio, newtio;


/** i2cT***********/

/*static char i2cBbuf[I2C_BUFFER_SIZE];*/

void* fd = NULL ;
int file;

/** Queue ***********/
char name[10];  /* CONS or PROD */
int msgqid_tx = -1, msgqid_rx = -1;

#define MAX_SIZE 15
#define IPC_WAIT 0

typedef struct
{
	long mtype;
	char mtext[MAX_SIZE];
} msg_buf_t;


/*=========================================================================
/ EXTERN VARIABLES
/========================================================================*/


extern UINT8 pTxParams[POE_MESSAGE_SIZE];
extern UINT8 pTxParams_SWDownload[55];
extern UINT8 pRxBuf[SIZE_OF_COM_READ_BUF];
extern POE_HOST_COMM_DEVICE commDevice;
/*=========================================================================
/ FUNCTIONS
/========================================================================*/


PoE_RC_e poeCommDevOpenUART(void)
{

	tty_fd = open(commDevice.deviceName, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (tty_fd < 0)
		{
			perror(commDevice.deviceName);
			return PoE_RC_ERR_COMM_DEVICE;
		}
	
		tcgetattr(tty_fd, &oldtio); /* save current port settings */
	
		bzero(&newtio, sizeof(newtio));
		newtio.c_cflag = BAUDRATE | CS8 | CREAD | CLOCAL;
		newtio.c_iflag = 0;
		newtio.c_oflag = 0;
		newtio.c_lflag = 0;   /* set input mode (non-canonical, no echo,...) */
		newtio.c_cc[VTIME]	  = 0;	 /* inter-character timer unused */
		newtio.c_cc[VMIN]	  = 0;	 /* blocking read until 5 chars received */
	
		tcflush(tty_fd, TCIFLUSH);
		tcsetattr(tty_fd, TCSANOW, &newtio);

		if(commDevice.devType == POE_COMM_UART_SYNC)
		{
			fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);		/* make the reads non-blocking*/
			maxfd = tty_fd + 1; /* maximum bit entry (fd) to test */
		}
		return PoE_RC_SUCSESS;


}


PoE_RC_e poeCommDevCloseUART(void)
{

	memset(&commDevice,0,sizeof(commDevice));
	close(tty_fd);
	
	/* restore old port settings */
	tcsetattr(tty_fd, TCSANOW, &oldtio);
	
	return PoE_RC_SUCSESS;



}

PoE_RC_e poeCommDevWriteUART(void)
{
	
	int num_bytes_written = write(tty_fd, pTxParams, POE_MESSAGE_SIZE);
	
	if (num_bytes_written < 0)
	{
		return PoE_RC_ERR_COMM_DEVICE;
	}

	return PoE_RC_SUCSESS;
}




PoE_RC_e poeCommDevReadUART(UINT8 bufLen, UINT8* bufLenRead,BOOL readTimeout)
{


	struct timeval timeout;
	int numOfReadBytes,numOfTry;;

	
	FD_SET(tty_fd, &readfs);  /* set for source 1 */

	if (readTimeout == FALSE)/* no timeout - immediate read*/
		{
			*bufLenRead = read(tty_fd, pRxBuf , bufLen);  /* returns after 5 chars have been input */
	
			if ((*bufLenRead) == 0)
				return PoE_RC_ERR_COMM_DEVICE;
	
			return PoE_RC_SUCSESS;
		}

	/* not immediate - set timeout value  */

	timeout.tv_usec = 0;  /* milliseconds */
	timeout.tv_sec	= 3;  /* seconds */
	
	int rc = 1;
	rc = select(maxfd, &readfs, NULL, NULL, &timeout);
	if (rc < 0 ) /* -1 for errors */
	{
		return PoE_RC_ERR_COMM_DEVICE;
	}
	if (rc == 0 ) /* timeout :no data read */
	{
		return PoE_RC_ERR_COMM_TIMEOUT;
	}


	/* wait for 15 bytes to arrive */

	numOfReadBytes = numOfTry =0;

	while ((numOfReadBytes < bufLen) && (numOfTry < 5))
	{
		numOfTry++;
		usleep(10000);
		ioctl(tty_fd, FIONREAD, &numOfReadBytes);
	}
	
	if (numOfReadBytes == 0)
	{
		return PoE_RC_ERR_COMM_TIMEOUT;
	}
	
	if (numOfReadBytes < bufLen)
	{
		bufLen = numOfReadBytes;
	}

	/* read the requested amount of bytes  after all bufLen size had received or numOfReadBytes after timeout */
	*bufLenRead = read(tty_fd, pRxBuf, bufLen);
	
	if ((*bufLenRead) == 0)
	{
		return PoE_RC_ERR_COMM_TIMEOUT;
	}
	
	return PoE_RC_SUCSESS;



}


PoE_RC_e poeCommDevClearUART(void)
{


	PoE_RC_e rc;
	UINT8 bufLenRead;
    int zeroByteRec=0;


	do
	{
		/* Read 1 byte each time from PoE Device - to clear buffer from data */
		rc= poeCommDevReadUART(BUF_LEN_TO_READ, &bufLenRead,FALSE);

		if ((rc == PoE_RC_SUCSESS) && (pRxBuf[0] == 0) && (bufLenRead == 1))
			zeroByteRec++;
		else
			zeroByteRec = 0;

	}
	while ((rc == PoE_RC_SUCSESS) && (zeroByteRec < 1000));
	return rc;
}



PoE_RC_e poeCommDevOpenI2C(void)
{
    int i2cbus;
    char filename[20];

    P_POE_SET_I2C_PARAMS i2cParams = (P_POE_SET_I2C_PARAMS)(commDevice.params);
    i2cbus = i2cParams->I2CBitRate;

    file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
    if (file < 0)
        return PoE_RC_ERR_COMM_DEVICE;

    return PoE_RC_SUCSESS;
}


PoE_RC_e poeCommDevCloseI2C()
{
    return PoE_RC_SUCSESS;
}



PoE_RC_e poeCommDevWriteI2C(UINT16 bufLen)
{
	struct i2c_msg msgs[I2C_RDRW_IOCTL_MAX_MSGS];
	struct i2c_rdwr_ioctl_data rdwr;
	int nmsgs = 0, nmsgs_sent;
	FILE *output = stdout;
	__u8 *buf;

	P_POE_SET_I2C_PARAMS i2cParams = (P_POE_SET_I2C_PARAMS)(commDevice.params);

	msgs[nmsgs].addr = i2cParams->I2CAddress;
	msgs[nmsgs].flags = 0;
	msgs[nmsgs].len = bufLen;

	buf = malloc(bufLen);
	memset(buf, 0, bufLen);

	if (bufLen > 15)
		msgs[nmsgs].buf = pTxParams_SWDownload;
	else
		msgs[nmsgs].buf = pTxParams;

	rdwr.msgs = msgs;
	rdwr.nmsgs = 1;
	nmsgs_sent = ioctl(file, I2C_RDWR, &rdwr);

	return PoE_RC_SUCSESS ;
}




PoE_RC_e poeCommDevReadI2C ( UINT16 bufLen, UINT16 *bufLenRead, BOOL readTimeout)
{
	struct i2c_msg msgs[I2C_RDRW_IOCTL_MAX_MSGS]={0};
	struct i2c_rdwr_ioctl_data rdwr;
	__u8 *buf;
	int nmsgs = 0, nmsgs_sent;
	FILE *output = stdout;
	unsigned i;

	P_POE_SET_I2C_PARAMS i2cParams = (P_POE_SET_I2C_PARAMS)(commDevice.params);

	msgs[nmsgs].addr = i2cParams->I2CAddress;
	msgs[nmsgs].flags |= I2C_M_RD;
	msgs[nmsgs].len = POE_MESSAGE_SIZE;

	buf = malloc(POE_MESSAGE_SIZE);
	memset(buf, 0, POE_MESSAGE_SIZE);
	msgs[nmsgs].buf = buf;

	rdwr.msgs = msgs;
	rdwr.nmsgs = 1;
	nmsgs_sent = ioctl(file, I2C_RDWR, &rdwr);

	for (i = 0; i < POE_MESSAGE_SIZE; i++) {
		pRxBuf[i] = msgs[0].buf[i];
	}

	return PoE_RC_SUCSESS;
}

/*#define _MSCC_HAL_IPC_QUEUE_C_*/


PoE_RC_e poeCommDevClearQueue()
{
	int rc = 0;
	msg_buf_t msg_buf;
	msg_buf.mtype = 2;
	if (msgqid_rx < 0)
		return PoE_RC_ERR_COMM_DEVICE;

	do
	{
		rc = msgrcv(msgqid_rx, &msg_buf, sizeof(msg_buf.mtext), 2, IPC_NOWAIT);
	}while(rc != -1);


	do
	{
		rc = msgrcv(msgqid_tx, &msg_buf, sizeof(msg_buf.mtext), 2, IPC_NOWAIT);
	}while(rc != -1);

	return PoE_RC_SUCSESS;
}



PoE_RC_e createMessageQueue(char *_name, key_t key_tx, key_t key_rx)
{
	int msgflg = S_IRWXU | IPC_CREAT;

	memset(name, '\0', sizeof(name));
	strcpy(name, _name);

	if (-1 == (msgqid_tx = msgget(key_tx, msgflg)))
	{
		perror("failed to open tx message queue\n");
		fprintf(stderr, "failed to open tx message queue errno [%d]\n", errno);
		return PoE_RC_ERR_COMM_DEVICE;
	}

	if (-1 == (msgqid_rx = msgget(key_rx, msgflg)))
	{
		perror("failed to open rx message queue\n");
		fprintf(stderr, "failed to open rx message queue errno [%d]\n", errno);
		return PoE_RC_ERR_COMM_DEVICE;
	}

	PoE_RC_e ePoE_Status = poeCommDevClearQueue();
	if(ePoE_Status != PoE_RC_SUCSESS)
	{
		printf("Rx queue cleared operation failed \n");
		return ePoE_Status;
	}

	printf("queues opened and cleared successfully. tx key: %d , rx key: %d \n", key_tx, key_rx);

	return PoE_RC_SUCSESS;
}


PoE_RC_e closeMessageQueue( void )
{
	if ((msgqid_tx) >= 0)
	{
		if (-1 == msgctl(msgqid_tx, IPC_RMID, 0))
		{
			printf("CLOSING Tx queue error (msgctl failed)\n");
		}
		msgqid_tx = -1;
	}

	if ((msgqid_rx) >= 0)
	{
		if (-1 == msgctl(msgqid_rx, IPC_RMID, 0))
		{
			printf("CLOSING Rx queue error (msgctl failed)\n");
		}
		msgqid_rx = -1;
	}

	printf("queues closed \n");

	return PoE_RC_SUCSESS;
}


PoE_RC_e sendMsg(char *p, int size)
{
	int i;
	msg_buf_t msg_buf;
	if (size > MAX_SIZE)
	{
		printf("Message size [%d] too large", size);
		return PoE_RC_ERR_COMM_DEVICE;
	}

	msg_buf.mtype = 2;
	memcpy(msg_buf.mtext, p, MAX_SIZE);

	printf("%s queue snd msg: ", name);
	for(i=0;i<MAX_SIZE;i++){
		printf("%2x, ", (UINT8)msg_buf.mtext[i]);
	}
	time_t  current_time = time(NULL);
	printf(" %s",ctime(&current_time));

	if (-1 == msgsnd(msgqid_tx, &msg_buf, sizeof(msg_buf.mtext), IPC_NOWAIT))
	{
		if (EAGAIN == errno) return 0;	/* No message available */
		else
		{
			printf("Queue TX msg failed (msgsnd failed) \n");
			return PoE_RC_ERR_COMM_DEVICE;
		}
	}

	return PoE_RC_SUCSESS;
}


PoE_RC_e receiveMsg(char *p, int size , OUT UINT16 *number_of_bytes_actually_read)
{
	int num_rx_bytes;
	msg_buf_t msg_buf;
	msg_buf.mtype = 2;
	if (msgqid_rx < 0)
		return PoE_RC_ERR_COMM_DEVICE;

	int i;
	for(i=0;i<400;i++)
	{
		if (-1 == (num_rx_bytes = msgrcv(msgqid_rx, &msg_buf, sizeof(msg_buf.mtext), 2, IPC_NOWAIT)))
		{
			if ((ENOMSG == errno) || (EAGAIN == errno))/* No message available */
			{
				usleep(1000 * 5); /* 5ms */
			}
			else
			{
				printf("Producer Queue error (msgrcv failed) \n");
				return PoE_RC_ERR_COMM_DEVICE;
			}
		}
		else
		{
			memcpy(p, msg_buf.mtext, size);
			*number_of_bytes_actually_read = num_rx_bytes;

			printf("%s queue rcv msg: ", name);
			for(i=0;i<MAX_SIZE;i++){
				printf("%2x, ", (UINT8)msg_buf.mtext[i]);
			}
			time_t  current_time = time(NULL);
			printf(" %s", ctime(&current_time));

			return PoE_RC_SUCSESS;
		}
	}

	return PoE_RC_ERR_COMM_TIMEOUT;
}



PoE_RC_e poeCommDevOpenQueue(void)
{
	return createMessageQueue("PROD", PRODUCER_TX_KEY,  PRODUCER_RX_KEY);
}


PoE_RC_e poeCommDevCloseQueue()
{
	return closeMessageQueue();
}


PoE_RC_e poeCommDevWriteQueue(void)
{
	return sendMsg((char*)pTxParams,  POE_MESSAGE_SIZE);
}


PoE_RC_e poeCommDevReadQueue(UINT16 bufLen, UINT16 *bufLenRead, BOOL readTimeout)
{
	return receiveMsg((char*) pRxBuf,  bufLen , bufLenRead);
}


/*#undef _MSCC_HAL_IPC_QUEUE_C_*/


