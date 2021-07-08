#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "i2cbusses.h"
#include <signal.h>
#include "util.h"
#include "wnc-tool.h"
#include <fcntl.h>
#define PAGE_SIZE 32

unsigned char buffer[PAGE_SIZE + 2]; // + 2 for  offset
int i2cbus= AC3_I2C_ADAPTOR; //dev/i2c-1
unsigned char i2c_read_buff[2048];
char i2c_read_buff_str[4096];
int file, force=0, res=0;
double fabs(double x);
char mtd_dev[12];


int ac3_aikido_get_sfp_present(int port)
{
	int fd = -1;

    unsigned char read_buf[4] = {0x00};               

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

    /*Todo: Serach mtd device*/

	lseek(fd, AIKIDO_SFP_PRESENT_ADDR, SEEK_SET);              
	read(fd, read_buf, sizeof(read_buf)); //read original data

	close(fd);

	//printf ("Read original data 0x%02x\n", read_buf[0]);

	return U32_GET_FIELD_MAC(read_buf[0],port-1,1);
	
}

int ac3_aikido_get_sfp_rx_los(int port)
{
	int fd = -1;

    unsigned char read_buf[4] = {0x00};               

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

    /*Todo: Serach mtd device*/

	lseek(fd, AIKIDO_SFP_RX_LOS_ADDR, SEEK_SET);              
	read(fd, read_buf, sizeof(read_buf)); //read original data

	close(fd);

	//printf ("Read original data 0x%02x\n", read_buf[0]);

	return U32_GET_FIELD_MAC(read_buf[0],port-1,1);
}

int ac3_aikido_set_sfp_tx_dis(int port,int data)
{
	int fd = -1;

    unsigned char read_buf[4] = {0x00}; 
	unsigned char data_buf[4] = {0x00,0x00,0x00,0x00};	

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

    /*Todo: Serach mtd device*/

	lseek(fd, AIKIDO_SFP_TX_DIS_ADDR, SEEK_SET);              
	read(fd, read_buf, sizeof(read_buf)); //read original data
	
	/*printf ("Read original data 0x%02x\n", read_buf[0]);*/

	U32_SET_FIELD_MAC(read_buf[0], port-1, 1, data); /*Change the bit number only*/

	data_buf[0] = read_buf[0];
	lseek(fd, AIKIDO_SFP_TX_DIS_ADDR, SEEK_SET);
	//printf("Write Aikido 0x%02x bit %d = %d\n",AIKIDO_SFP_TX_DIS_ADDR,port-1,data);

	write(fd, data_buf, sizeof(data_buf)); // write our message

	close(fd);

	//printf ("Read original data 0x%02x\n", read_buf[0]);

	return U32_GET_FIELD_MAC(read_buf[0],port-1,1);
}


void ac3_aikido_clear_i2c_mux(void)
{
	
	int fd = -1;

	unsigned char data_buf[4] = {0x00,0x00,0x00,0x00};			  

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

	/*Todo: Serach mtd device*/

	lseek(fd, AIKIDO_I2C_MUX_ADDR, SEEK_SET);
	//printf("Write Aikido 0x%02x = 0x%02x\n",address,data_buf[0]);

	write(fd, data_buf, sizeof(data_buf)); // write our message

	close(fd);
	
	return;
}


int ac3_aikido_set_i2c_mux(int port)
{
    int i;
	
	int fd = -1;

    unsigned char data_buf[4] = {0x00,0x00,0x00,0x00};
    unsigned char read_buf[4] = {0x00};               

	if ((fd = open(mtd_dev, O_RDWR)) == -1)
		sys_errmsg_die("%s", mtd_dev);

    /*Todo: Serach mtd device*/

 
	lseek(fd, AIKIDO_I2C_MUX_ADDR, SEEK_SET);              
	read(fd, read_buf, sizeof(read_buf)); //read original data

	//printf ("Read original data 0x%02x\n", read_buf[0]);

	for(i=0;i<4;i++)
	{
		if(i == port-1)
			U32_SET_FIELD_MAC(read_buf[0],i, 1, 1);
		else
			U32_SET_FIELD_MAC(read_buf[0],i, 1, 0);
	}

	 /*Change the bit number only*/

	data_buf[0] = read_buf[0];
	lseek(fd, AIKIDO_I2C_MUX_ADDR, SEEK_SET);
	//printf("Write Aikido 0x%02x = 0x%02x\n",AIKIDO_I2C_MUX_ADDR,data_buf[0]);

	write(fd, data_buf, sizeof(data_buf)); // write our message


	close(fd);
		
	return 0;
}


void read_port_status(int port)
{

	if(port < 1 || port > 4)
	{
		//Port number error
		printf("Port %d is out of range.\n",port);
	}
	else
	{
		int status=1;

		status=ac3_aikido_get_sfp_present(port);
		printf("SFP Port %02d cable %s present.\n",port,status==0?"is":"is NOT");
  
		if ( status== 0) //Cable is present
		{
			
			ac3_aikido_set_i2c_mux(port);
			
			
			if ( check_i2c_address(MODE_AUTO,AC3_SFP_EE_ADDR))
			{	
				printf("SFP Port %02d cable EEPROM=%s\n",port,wnc_i2c_dev_read_char(AC3_SFP_EE_ADDR, CABLE_MODEL_START, CABLE_MODEL_SIZE));
			}	
			else
				printf("SFP Port %02d cable EEPROM is NOT present.\n",port);

			
            //Read Serial number   
			if ( check_i2c_address(MODE_AUTO,AC3_SFP_EE_ADDR))
			{	
				printf("SFP Port %02d cable serial number=%s\n",port,wnc_i2c_dev_read_char(AC3_SFP_EE_ADDR, CABLE_SERIAL_START, CABLE_SERIAL_SIZE));
			}	
			else
				printf("SFP Port %02d cable EEPROM is NOT present.\n",port);

			ac3_aikido_clear_i2c_mux();
		
		}

	}

}

int tx_dis_test(int port)
{	

	int r_data,out_data;
	int tx_dis,tx_dis_bit;

	/*Config input / outpu pin*/
	wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_CFG_REG,0x39);

    /*Pull high*/
	ac3_aikido_set_sfp_tx_dis(port,1);

	tx_dis=wnc_i2c_dev_read_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_IN_REG);
	//printf("tx_dis 0x%2x\n",tx_dis);
	tx_dis_bit=U32_GET_FIELD_MAC(tx_dis,5,1);
	//printf("tx_dis %d\n",tx_dis_bit);

	if(tx_dis_bit != 1)
	{
		printf("SFP Port %02d TX_DIS pull high test fail.\n",port);
		return 1; 
	}
	
    /*Pull low*/
	ac3_aikido_set_sfp_tx_dis(port,0);

	tx_dis=wnc_i2c_dev_read_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_IN_REG);
	//printf("tx_dis 0x%2x\n",tx_dis);
	tx_dis_bit=U32_GET_FIELD_MAC(tx_dis,5,1);
	//printf("tx_dis %d\n",tx_dis_bit);

	if(tx_dis_bit != 0)
	{
		printf("SFP Port %02d TX_DIS pull low test fail.\n",port);
		return 1; 
	}	

	printf("SFP Port %02d TX_DIS TEST PASS.\n",port);
	
	return 0;
}
int rx_los_test(int port)
{

	int r_data;
	int out_data;

	/*Config input / outpu pin*/
	wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_CFG_REG,0x39);

	r_data=wnc_i2c_dev_read_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_OUT_REG);
	//printf("Out 0x%2x\n",r_data);

	
	/*Pull high*/
	out_data=U32_SET_FIELD_MAC(r_data,6, 1, 1);	
	//printf("Out 0x%2x\n",out_data);
	wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_OUT_REG,out_data);

	if ( ac3_aikido_get_sfp_rx_los(port) != 1)
	{
		printf("SFP Port %02d RX_LOS pull high test fail.\n",port);
		return 1; 	
	}	
	
	/*Pull low*/
	out_data=U32_SET_FIELD_MAC(r_data,6, 1, 0);
	//printf("Out 0x%2x\n",out_data);
	wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_OUT_REG,out_data);

	if ( ac3_aikido_get_sfp_rx_los(port) != 0)
	{
		printf("SFP Port %02d RX_LOS pull low test fail.\n",port);
		return 1; 	
	}	

	printf("SFP Port %02d RX_LOS TEST PASS.\n",port);

	return 0;
}


void sfp_io_test(int port)
{
	int i;
	int error=0;

	if(port < 0 || port > 4)
	{
		//Port number error
		printf("Port %d is out of range.\n",port);
	}
	else
	{
		if(port == 0)
		{
			for(i=1;i<5;i++)
			{
				ac3_aikido_set_i2c_mux(i);

				if ( check_i2c_address(MODE_AUTO,LBM_PCA9554_ADDR))
				{	
					//printf("SFP Port %02d cable PCA9554 is present.\n",i);
					
					if( tx_dis_test(i)== 1)
						error++;
					if( rx_los_test(i)== 1)
						error++;
				}	
				else
				{
					error++;
					printf("SFP Port %02d cable PCA9554 is NOT present.\n",i);
				}	
			}
			
		}
		else
		{
			ac3_aikido_set_i2c_mux(port);
			
			if ( check_i2c_address(MODE_AUTO,LBM_PCA9554_ADDR))
			{	
				/*printf("SFP Port %02d cable PCA9554 is present.\n",port);*/
				if( tx_dis_test(port)== 1)
					error++;
				if( rx_los_test(port)== 1)
					error++;
			}	
			else
			{
				error++;
				printf("SFP Port %02d cable PCA9554 is NOT present.\n",port);
			}	
		}

		ac3_aikido_clear_i2c_mux();
	}

	if ( error != 0 )
		printf("SFP IO TEST FAIL! CHECK PREVIOUS LOG\n");
	else
		printf("SFP IO TEST PASS!\n");
	
}


void set_sfp_loading(int loading)
{
	int i;
	int r_data,out_data;
	int error=0;

	if(loading < 0 || loading > 3)
	{
		//Loading number error
		printf("Loading %d is out of range.\n",loading);
		return;
	}
	else
	{
		for(i=1;i<5;i++)
			{
				ac3_aikido_set_i2c_mux(i);

				if ( check_i2c_address(MODE_AUTO,LBM_PCA9554_ADDR))
				{	
				
					//printf("SFP Port %02d cable PCA9554 is present.\n",i);
					/*Config input / outpu pin*/
					wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_CFG_REG,0x39);
					
					r_data=wnc_i2c_dev_read_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_OUT_REG);
					//printf("Out 0x%2x\n",r_data);

					if(loading == 0)
					{
						out_data=U32_SET_FIELD_MAC(r_data,1, 1, 0);	 //HI_POW_G
						out_data=U32_SET_FIELD_MAC(out_data,2, 1, 0); //LO_POW_G	
					}	
					else if(loading == 1)
					{
						out_data=U32_SET_FIELD_MAC(r_data,1, 1, 0);	//HI_POW_G
						out_data=U32_SET_FIELD_MAC(out_data,2, 1, 1); //LO_POW_G	
					}	
					else if(loading == 2)
					{
						out_data=U32_SET_FIELD_MAC(r_data,1, 1, 1);	//HI_POW_G
						out_data=U32_SET_FIELD_MAC(out_data,2, 1, 0); //LO_POW_G	
					}	
					else if(loading == 3)
					{
						out_data=U32_SET_FIELD_MAC(r_data,1, 1, 1);	//HI_POW_G
						out_data=U32_SET_FIELD_MAC(out_data,2, 1, 1); //LO_POW_G	
					}
					//printf("Out 0x%2x\n",out_data);

					wnc_i2c_dev_write_byte_data(LBM_PCA9554_ADDR,LBM_PCA9554_OUT_REG,out_data);

					printf("SFP Port %02d cable loading is set to %d.\n",i,loading);
	
				}	
				else
				{
				    error++;
					printf("SFP Port %02d cable PCA9554 is NOT present.\n",i);
				}	
			}
	}	

	if ( error != 0 )
		printf("SET SFP LOADING FAIL! CHECK PREVIOUS LOG\n");
	else
		printf("SET SFP LOADING PASS!\n");

}

void show_usage(void)
{
  printf("Usage of %s tool. Version: %s\n",tool_name,tool_version);
  printf(" %s -h               : help.\n", tool_name);
  printf(" %s -v               : print tool version info.\n", tool_name);
  printf(" %s -p port (1-4)    : show SFP port status and eeprom information.\n", tool_name);
  printf(" %s -s port (0-4)    : test SFP port io (TX_DIS and RX_LOS). 0=All.\n", tool_name);
  printf(" %s -l load (0-3)    : set loading for all SFP ports.\n", tool_name);
  
}

int main(int argc, char *argv[])
{

  int port_num=0;
  int loading=0;
  int ch;
  int mtd_num;

  if (argc < 2)
  {
	show_usage();
	return 0;
  }


  FILE *fp = popen("/usr/sbin/get_aikido_mtd", "r");

  fscanf(fp, "%d", &mtd_num);
  pclose(fp);

  sprintf(mtd_dev,"/dev/mtd%d",mtd_num);	

  i2cbus = lookup_i2c_bus(I2C_BUS_NAME);	

  if (i2cbus < 0)
  {
	printf("Error: Can't find i2c bus\n");
	i2cbus = AC3_I2C_ADAPTOR;
  } 

  //Debug information
  //	printf("%s bus = %d\n",I2C_BUS_NAME,i2cbus);

  signal(SIGINT, intHandler);
  wnc_i2c_dev_open();	

  while ((ch = getopt(argc, argv, "hvp:s:l:")) != -1) {
    switch (ch) {
    case 'h':
      show_usage();
      break;
    case 'v':
	  printf("%s\n",tool_version);
	  break;
	case 'p':
      port_num= atoi(optarg);
	  read_port_status(port_num);
      break; 
	case 's':
      port_num= atoi(optarg);
	  sfp_io_test(port_num);
      break;   
	case 'l':
	  loading= atoi(optarg);
	  set_sfp_loading(loading);
      break;     
    default:                   // wrong input
      show_usage();
      break;
    }
  }
  wnc_i2c_dev_close();
 exit(0);
}


