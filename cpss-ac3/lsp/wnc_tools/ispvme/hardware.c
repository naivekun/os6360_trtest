/*********************************************************************************
* Lattice Semiconductor Corp. Copyright 2000-2008
*
* This is the hardware.c of ispVME V12.1 for JTAG programmable devices.
* All the functions requiring customization are organized into this file for
* the convinience of porting.
*********************************************************************************/
/*********************************************************************************
* Revision History:
*
* 09/11/07 NN Type cast mismatch variables
* 09/24/07 NN Added calibration function.
*             Calibration will help to determine the system clock frequency
*             and the count value for one micro-second delay of the target
*             specific hardware.
*             Modified the ispVMDelay function
*             Removed Delay Percent support
*             Moved the sclock() function from ivm_core.c to hardware.c
*********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

/********************************************************************************
* Declaration of global variables
*
*********************************************************************************/

unsigned char  g_siIspPins        = 0x00;   /*Keeper of JTAG pin state*/
unsigned short g_usInPort         = 0x379;  /*Address of the TDO pin*/
unsigned short g_usOutPort        = 0x378;  /*Address of TDI, TMS, TCK pin*/
unsigned short g_usCpu_Frequency  = 1000;   /*Enter your CPU frequency here, unit in MHz.*/


/*********************************************************************************
* This is the definition of the bit locations of each respective
* signal in the global variable g_siIspPins.
*
* NOTE: Users must add their own implementation here to define
*       the bit location of the signal to target their hardware.
*       The example below is for the Lattice download cable on
*       on the parallel port.
*
*********************************************************************************/

const unsigned char g_ucPinTDI          = 0x01;    /* Bit address of TDI */
const unsigned char g_ucPinTCK          = 0x02;    /* Bit address of TCK */
const unsigned char g_ucPinTMS          = 0x04;    /* Bit address of TMS */
const unsigned char g_ucPinENABLE       = 0x08;    /* Bit address of ENABLE */
const unsigned char g_ucPinTRST         = 0x10;    /* Bit address of TRST */
const unsigned char g_ucPinTDO          = 0x40;    /* Bit address of TDO*/

/***************************************************************
*
* Functions declared in hardware.c module.
*
***************************************************************/
void writePort( unsigned char a_ucPins, unsigned char a_ucValue );
unsigned char readPort();
void sclock();
void ispVMDelay( unsigned short a_usTimeDelay );
void calibration(void);

static void _outp(unsigned char a_ucPins, unsigned char a_ucValue);

/********************************************************************************
* writePort
* To apply the specified value to the pins indicated. This routine will
* be modified for specific systems.
* As an example, this code uses the IBM-PC standard Parallel port, along with the
* schematic shown in Lattice documentation, to apply the signals to the
* JTAG pins.
*
* PC Parallel port pin    Signal name           Port bit address
*        2                g_ucPinTDI             1
*        3                g_ucPinTCK             2
*        4                g_ucPinTMS             4
*        5                g_ucPinENABLE          8
*        6                g_ucPinTRST            16
*        10               g_ucPinTDO             64
*
*  Parameters:
*   - a_ucPins, which is actually a set of bit flags (defined above)
*     that correspond to the bits of the data port. Each of the I/O port
*     bits that drives an isp programming pin is assigned a flag
*     (through a #define) corresponding to the signal it drives. To
*     change the value of more than one pin at once, the flags are added
*     together, much like file access flags are.
*
*     The bit flags are only set if the pin is to be changed. Bits that
*     do not have their flags set do not have their levels changed. The
*     state of the port is always manintained in the static global
*     variable g_siIspPins, so that each pin can be addressed individually
*     without disturbing the others.
*
*   - a_ucValue, which is either HIGH (0x01 ) or LOW (0x00 ). Only these two
*     values are valid. Any non-zero number sets the pin(s) high.
*
*********************************************************************************/

void writePort( unsigned char a_ucPins, unsigned char a_ucValue )
{
    if ( a_ucValue ) {
        g_siIspPins = (unsigned char) (a_ucPins | g_siIspPins);
    }
    else {
        g_siIspPins = (unsigned char) (~a_ucPins & g_siIspPins);
    }

    _outp( a_ucPins, a_ucValue );
}

/*********************************************************************************
*
* readPort
*
* Returns the value of the TDO from the device.
*
**********************************************************************************/
unsigned char readPort()
{
    int fd;
    char * gpio_value_path = "/sys/class/gpio/gpio7/value";
    char value = 0;

    if ((fd = open(gpio_value_path, O_RDONLY)) < 0)
    {
        printf("Cannot open %s\n", gpio_value_path);
        return 0x00;
    }

    read(fd, &value, 1);
    close(fd);

    if(value == '0')
    {
        /* Current GPIO status low */
        return 0x00;
    }
    else
    {
        /* Current GPIO status high */
        return 0x01;
    }
}

/*********************************************************************************
* sclock
*
* Apply a pulse to TCK.
*
* This function is located here so that users can modify to slow down TCK if
* it is too fast (> 25MHZ). Users can change the IdleTime assignment from 0 to
* 1, 2... to effectively slowing down TCK by half, quarter...
*
*********************************************************************************/
void sclock()
{
    unsigned short IdleTime    = 0; //change to > 0 if need to slow down TCK
    unsigned short usIdleIndex = 0;
    IdleTime++;
    for ( usIdleIndex = 0; usIdleIndex < IdleTime; usIdleIndex++ ) {
        writePort( g_ucPinTCK, 0x01 );
    }
    for ( usIdleIndex = 0; usIdleIndex < IdleTime; usIdleIndex++ ) {
        writePort( g_ucPinTCK, 0x00 );
    }
}
/********************************************************************************
*
* ispVMDelay
*
*
* Users must implement a delay to observe a_usTimeDelay, where
* bit 15 of the a_usTimeDelay defines the unit.
*      1 = milliseconds
*      0 = microseconds
* Example:
*      a_usTimeDelay = 0x0001 = 1 microsecond delay.
*      a_usTimeDelay = 0x8001 = 1 millisecond delay.
*
* This subroutine is called upon to provide a delay from 1 millisecond to a few
* hundreds milliseconds each time.
* It is understood that due to a_usTimeDelay is defined as unsigned short, a 16 bits
* integer, this function is restricted to produce a delay to 64000 micro-seconds
* or 32000 milli-second maximum. The VME file will never pass on to this function
* a delay time > those maximum number. If it needs more than those maximum, the VME
* file will launch the delay function several times to realize a larger delay time
* cummulatively.
* It is perfectly alright to provide a longer delay than required. It is not
* acceptable if the delay is shorter.
*
* Delay function example--using the machine clock signal of the native CPU------
* When porting ispVME to a native CPU environment, the speed of CPU or
* the system clock that drives the CPU is usually known.
* The speed or the time it takes for the native CPU to execute one for loop
* then can be calculated as follows:
*       The for loop usually is compiled into the ASSEMBLY code as shown below:
*       LOOP: DEC RA;
*             JNZ LOOP;
*       If each line of assembly code needs 4 machine cycles to execute,
*       the total number of machine cycles to execute the loop is 2 x 4 = 8.
*       Usually system clock = machine clock (the internal CPU clock).
*       Note: Some CPU has a clock multiplier to double the system clock for
              the machine clock.
*
*       Let the machine clock frequency of the CPU be F, or 1 machine cycle = 1/F.
*       The time it takes to execute one for loop = (1/F ) x 8.
*       Or one micro-second = F(MHz)/8;
*
* Example: The CPU internal clock is set to 100Mhz, then one micro-second = 100/8 = 12
*
* The C code shown below can be used to create the milli-second accuracy.
* Users only need to enter the speed of the cpu.
*
**********************************************************************************/
void ispVMDelay( unsigned short a_usTimeDelay )
{
    unsigned short loop_index     = 0;
    unsigned short ms_index       = 0;
    unsigned short us_index       = 0;
    struct timespec sleep, remain;

    if ( a_usTimeDelay & 0x8000 ) /*Test for unit*/
    {
        a_usTimeDelay &= ~0x8000; /*unit in milliseconds*/
    }
    else { /*unit in microseconds*/
        a_usTimeDelay = (unsigned short) (a_usTimeDelay/1000); /*convert to milliseconds*/
        if ( a_usTimeDelay <= 0 ) {
             a_usTimeDelay = 1; /*delay is 1 millisecond minimum*/
        }
    }
    /*Users can replace the following section of code by their own*/
    sleep.tv_nsec = a_usTimeDelay*1000*1000;
    sleep.tv_sec  = 0 ;
    nanosleep(&sleep, &remain);
}

/*********************************************************************************
*
* calibration
*
* It is important to confirm if the delay function is indeed providing
* the accuracy required. Also one other important parameter needed
* checking is the clock frequency.
* Calibration will help to determine the system clock frequency
* and the loop_per_micro value for one micro-second delay of the target
* specific hardware.
*
**********************************************************************************/
void calibration(void)
{
    /*Apply 2 pulses to TCK.*/
    writePort( g_ucPinTCK, 0x00 );
    writePort( g_ucPinTCK, 0x01 );
    writePort( g_ucPinTCK, 0x00 );
    writePort( g_ucPinTCK, 0x01 );
    writePort( g_ucPinTCK, 0x00 );

    /*Delay for 1 millisecond. Pass on 1000 or 0x8001 both = 1ms delay.*/
    ispVMDelay(0x8001);

    /*Apply 2 pulses to TCK*/
    writePort( g_ucPinTCK, 0x01 );
    writePort( g_ucPinTCK, 0x00 );
    writePort( g_ucPinTCK, 0x01 );
    writePort( g_ucPinTCK, 0x00 );
}

static void _outp(unsigned char a_ucPins, unsigned char a_ucValue)
{
    int fd;
    char * gpio_value_path = NULL;

    if (a_ucPins == g_ucPinTCK)
        gpio_value_path = "/sys/class/gpio/gpio18/value";
    else if (a_ucPins == g_ucPinTMS)
        gpio_value_path = "/sys/class/gpio/gpio11/value";
    else if (a_ucPins == g_ucPinTDI)
        gpio_value_path = "/sys/class/gpio/gpio8/value";
    else if ((a_ucPins == g_ucPinENABLE) || (a_ucPins == g_ucPinTRST))
        return;
    else
    {
        printf("Unknown Pin = 0x%0x\n", a_ucPins);
        return;
    }

    if ((fd = open(gpio_value_path, O_WRONLY)) < 0)
    {
        printf("Cannot open %s\n", gpio_value_path);
        return;
    }

    if (a_ucValue)
    {
        /* Set GPIO high status */
        write(fd, "1", 1);
    }
    else
    {
        /* Set GPIO low status */
        write(fd, "0", 1);
    }

    close(fd);
}
