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
static char *h_devid = "Device_ID\n",
	    *h_pll = "Core clock frequency [MHz]\n"
		     "\t0 = 450(internal)\n"
		     "\t1 = 487.5 (internal)\n"
		     "\t2 = 525 (default)\n"
		     "\t3 = 583.334 (internal)\n"
		     "\t4 = 600\n"
		     "\t5 = 625 (internal)\n"
		     "\t6 = 556.250 (internal)\n"
		     "\t7 = 250 (internal)\n"
		     "\t8 = 572.917\n"
		     "\t9 = 577.083 (internal)\n"
		     "\t10 = 585.417 (internal)\n"
		     "\t11 = 400 (internal)\n"
		     "\t12 = 535.416 (internal)\n"
		     "\t13 = 545.8335 (internal)\n"
		     "\t14 = 593.75 (internal)\n"
		     "\t15 = PLL bypass Reserved\n",
	    *h_pllmppm = "MPPM clock frequency [MHz]\n"
			 "\t0 = 650 (internal)\n"
			 "\t1 = 700 (internal)\n"
			 "\t2 = 750 (internal)\n"
			 "\t3 = 800 (internal)\n"
			 "\t4 = 850 (default)\n"
			 "\t5 = 875 (internal)\n"
			 "\t6 = 900 (internal)\n"
			 "\t7 = PLL bypass. Reserved\n",
	    *h_pllclk = "PTP clock frequency\n"
			"\t0 = PTP Clock = 500MHz (default)\n"
			"\t1 = PLL Bypass. Reserved.\n",
	    *h_port_cg_clk = "Port (CG MAC) clock frequency [MHz]\n"
			"\t0 = 800 (default)\n"
			"\t1 = 725 (internal)\n"
			"\t2 = 700 (internal)r\n"
			"\t3 = PLL bypass. Reserved.\n",
	    *h_boardid = "BoardID\n"
			 "\t0 - BC3 DB Board\n"
			 "\t1 - BC3 ETP Board\n";

/* PCA9560PW	is used for all SatRs configurations (0x4c, 0x4d, 0x4f, 0x4e)
 * PCA9555	is used for all Serdes configurations (0x20)
 */
struct satr_info bc3_satr_info[] = {
/*	  name		    twsi_addr twsi_reg field_of bit_mask moreThen256 default  help		pca9555*/
	{"devid"       ,	0x4c,	0,		0,		0xf,	MV_FALSE,	0xf,	&h_devid,	MV_FALSE},
	{"corepll"     ,	0x4d,	0,		0,		0xf,	MV_FALSE,	0x2,	&h_pll,		MV_FALSE},
	{"pll-mppm-cnf", 	0x4e,	0,		0,		0x7,	MV_FALSE,	0x4,	&h_pllmppm,	MV_FALSE},
	{"ptp-pll-frq" , 	0x4e,	0,		3,		0x1,	MV_FALSE,	0x0,	&h_pllclk,	MV_FALSE},
	{"port-CG-frq" ,	0x4f,	0,		0,		0x3,	MV_FALSE,	0x0,	&h_port_cg_clk,	MV_FALSE},
	{"boardid"     ,	0x53,	7,		0,		0x7,	MV_TRUE,	0x0,	&h_boardid,	MV_FALSE},
	/* the "LAST entry should be always last - it is used for SatR max options calculation */
	{"LAST"        ,	0x0,	0,		0,		0x0,	MV_FALSE,	0x0,	NULL,		MV_FALSE},
};

