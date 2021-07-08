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

#include "sar_sw_lib.h"

/* help descriptions */
static char *h_devmode = "Device_Mode\n"
			 "Device Id field setting\n",
	    *h_pllptpcgf = "PLL PTP Configuration\n"
			   "\t0 = 500 MHz (default)\n"
			   "\t1 = reserved\n",
	    *h_coreclkfreq = "Core Clock Frequency\n"
		     	     "\t0 = 450MHz (internal)\n"
		    	     "\t1 = 475MHz (internal)\n"
		    	     "\t2 = 600MHz (default)\n"
		     	     "\t3 = 525MHz (internal)\n"
		     	     "\t4 = 625MHz (internal)\n"
		    	     "\t5 = 650MHz (internal)\n"
		     	     "\t6 = 575MHz (internal)\n"
		     	     "\t7 = PLL bypass (Reserved)\n",
	    *h_efusebypass = "efuse bypass\n"
		     	     "\t0 = Bypass\n"
		     	     "\t1 = default\n",
	    *h_mppmclkfreq = "MPPM Clock Frequency\n"
		     	     "\t0 = 650MHz (internal)\n"
		    	     "\t1 = 900MHz (internal)\n"
		    	     "\t2 = 850MHz (default)\n"
		     	     "\t3 = PLL bypass (Reserved)\n",
	    *h_mbusfreq = "MBUS Frequency\n"
			  "\t0 = 250 (default)\n"
			  "\t1 = PLL bypass (Reserved)\n",
	    *h_portCGMacClk = "Port (CG MAC) Clock Frequency\n"
			      "\t0 = 833.33 (default)\n"
			      "\t1 = 800\n"
			      "\t2 = 700 (internal)\n"
			      "\t3 = PLL bypass (Reserved)\n";

/* PCA9560PW	is used for all SatRs configurations (0x4c, 0x4d, 0x4e) */

struct satr_info aldrin2_satr_info[] = {
/*	name		   twsi_addr  twsi_reg field_off bit_mask moreThen256 default  help	    pca9560*/
	{"devmode"          ,	0x4c,	0,	    0,	    0x1f,	MV_FALSE,	0xf,	&h_devmode,	MV_FALSE},
	{"pllptpcfg"        ,	0x4d,	0,	    0,	    0x1,	MV_FALSE,	0x0,	&h_pllptpcgf,	MV_FALSE},
	{"corepll"          ,	0x4d,	0,	    1,	    0x7,	MV_FALSE,	0x2,	&h_coreclkfreq,	MV_FALSE},
	{"efusebypass"      ,	0x4d,	0,	    4,	    0x1,	MV_FALSE,	0x1,	&h_efusebypass,	MV_FALSE},
	{"mppmclockfreq"    ,	0x4e,	0,	    0,	    0x3,	MV_FALSE,	0x2,	&h_mppmclkfreq,	MV_FALSE},
	{"mbusfreq"         ,	0x4e,	0,	    2,	    0x1,	MV_FALSE,	0x0,	&h_mbusfreq,	MV_FALSE},
	{"portCGMacClk"     ,	0x4e,	0,	    3,	    0x3,	MV_FALSE,	0x0,	&h_portCGMacClk,MV_FALSE},
	/* the "LAST entry should be always last     - it is used for SatR max options calculation */
	{"LAST"           ,	0x0,	0,	    0,	    0x0,	MV_FALSE,	0x0,	NULL,		MV_FALSE},
};

