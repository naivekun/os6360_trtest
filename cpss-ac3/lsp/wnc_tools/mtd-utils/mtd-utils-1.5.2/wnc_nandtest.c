#define PROGRAM_NAME "wnc_nandtest"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>

#include <asm/types.h>
#include "mtd/mtd-user.h"

#define test_case_1 0x00
#define test_case_2 0x55
#define test_case_3 0xAA
#define test_case_4 0xFF

int nr_passes = 0;
int non_stop = 0;
uint32_t length = 524288; /*512KB*/
char test_file[128];
int kb_length = 512;

void usage(int status)
{
	fprintf(status ? stderr : stdout,
		"usage: %s [OPTIONS] <folder>\n"
		"  -h, --help\t   Display this help output\n"
		"  -n, --non-stop   Non-stop when failed.\n"
		"  -p, --passes\t   Number of passes\n"
		"  -l, --length\t   Length of file to test (KB), max=15MB=15360KB\n",
		PROGRAM_NAME);
	exit(status);
}

int write_test_data(unsigned char test_case)
{
	FILE *fptr;
	 uint32_t n=0;
	
	if ((fptr = fopen(test_file,"wb")) == NULL){
		printf("Error! opening file %s\n",test_file);
		// Program exits if the file pointer returns NULL.
		goto failexit;
	}

	/*printf("write_test_data! case %d\n",case_num);*/
	
	for(n = 0; n < length; n++)
	{
		fprintf(fptr,"%c",test_case);		
	 }

       fclose(fptr); 
       system("sync");
		
        return 0;

failexit:
	fclose(fptr); 
         system("sync");
		 
	return 1;	 
}

int read_and_compare(unsigned char test_case)
{

	 FILE         *fptr;
    	 unsigned char  read_byte;
	 uint32_t n;	 

	if ((fptr = fopen(test_file,"rb")) == NULL){
		printf("Error! opening file %s\n",test_file);
		// Program exits if the file pointer returns NULL.
		goto failexit;
	}

    	/*fprintf(stdout, "hexdump of `%s': ", test_file);*/

	for(n = 0; n < length; n++)
	{
		if(fread(&read_byte, 1, 1, fptr) != 1)
		{
			printf("Error! Read %d byte fail.",n+1);
			goto failexit;
		}
		else
		{
			if( read_byte != test_case)
			{	
			 	fprintf(stdout, "Error! %d byte fail !Read %02x Test Case %02x ", n+1,read_byte,test_case);
				goto failexit;
			}	
		}
	}

  	/*fprintf(stdout, "\n");*/

	fclose(fptr); 
	system("sync");

	return 0;

failexit:
	fclose(fptr); 
	system("sync");

	return 1;	 

}


/*
 * Main program
 */
int main(int argc, char **argv)
{
	int data_case;
	unsigned char test_case;
	int test_times;
	int rc=0;
	  
	   for (;;) {
			   static const char short_options[] = "hnl:p:";
			   static const struct option long_options[] = {
				   { "help", no_argument, 0, 'h' },
				   { "non-stop", no_argument, 0, 'n' },
				   { "passes", required_argument, 0, 'p' },
				   { "length", required_argument, 0, 'l' },
				   {0, 0, 0, 0},
			   };
			   int option_index = 0;
			   int c = getopt_long(argc, argv, short_options, long_options, &option_index);
			   if (c == EOF)
				   break;
	   
			   switch (c) {
			   case 'h':
				   usage(0);
				   break;
			  case 'n':
				   non_stop=1;
				   break;	   
			   case '?':
				   usage(1);
				   break;
			   case 'p':
				   nr_passes = atol(optarg);
				   break;
			   case 'l':
				   kb_length = strtol(optarg, NULL, 0);
				   if(kb_length>15360)
				   {
				   	printf("Error! exceed maximum!\n");
				   	usage(1);
				   }
				   length=kb_length*1024;
				   break;
			   }
		   }
		   if (argc - optind != 1)
			   usage(1);

	            sprintf(test_file,"%s/test.bin",argv[optind]);
		  /* printf("Test file = %s\n",test_file);*/

		if(nr_passes == 0)
		{
			test_times=1;
			  for (;;) 
			  {
			  	printf("Testing times: %d\n",test_times);
			  	
				 for(	data_case=1;data_case<5;data_case++)
				 {
						switch(data_case)
						{
							case 1:
							   test_case=test_case_1;
							   break;
							case 2:
							   test_case=test_case_2;
							   break;
							case 3:
							   test_case=test_case_3;
							   break;
							case 4:
							   test_case=test_case_4;
							   break;
							default:
							   break;
						}		
				 
				 	rc = write_test_data(test_case);
					if (non_stop == 1 && rc == 1)
					{
						printf("Error! Test case 0x%02x . Write data fail!\n",test_case);
						continue;
					}	
					else if ( non_stop == 0 && rc == 1 )
					{
						printf("Error! Test case 0x%02x . Write data fail!\n",test_case);
						return 1;
					}
					rc = read_and_compare(test_case);
					if (non_stop == 1 && rc == 1)
					{
						printf("Error! Test case 0x%02x . Compare data fail!\n",test_case);
						continue;
					}	
					else if ( non_stop == 0 && rc == 1 )
					{
						printf("Error! Test case 0x%02x . Compare data fail!\n",test_case);
						return 1;
					}
					else
						printf("Pass! Test case 0x%02x. Test length %d KB\n",test_case, kb_length);
					
				 }
				 test_times++;
			  }
		}
		else /*nr_passes != 0*/
		{
			for (test_times=1;test_times<nr_passes+1;test_times++) 
			  {
			  	printf("Testing times: %d\n",test_times);
			  	
				 for(	data_case=1;data_case<5;data_case++)
				 {
				 	switch(data_case)
					{
						case 1:
						   test_case=test_case_1;
						   break;
						case 2:
						   test_case=test_case_2;
						   break;
						case 3:
						   test_case=test_case_3;
						   break;
						case 4:
						   test_case=test_case_4;
						   break;
						default:
						   break;
					}		
					
				 	rc = write_test_data(test_case);
					if (non_stop == 1 && rc == 1)
					{
						printf("Error! Test case 0x%02x . Write data fail!\n",test_case);
						continue;
					}	
					else if ( non_stop == 0 && rc == 1 )
					{
						printf("Error! Test case 0x%02x . Write data fail!\n",test_case);
						return 1;
					}
					rc = read_and_compare(test_case);
					if (non_stop == 1 && rc == 1)
					{
						printf("Error! Test case 0x%02x . Compare data fail!\n",test_case);
						continue;
					}	
					else if ( non_stop == 0 && rc == 1 )
					{
						printf("Error! Test case 0x%02x . Compare data fail!\n",test_case);
						return 1;
					}
					else
						printf("Pass! Test case 0x%02x. Test length %d KB\n",test_case, kb_length);
				 }
			  }
		}
			
	/* Return happy */
	return 0;
}
