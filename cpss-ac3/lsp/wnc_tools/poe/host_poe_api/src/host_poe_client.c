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
*  Description: contain the implementation of host.
*
*************************************************************************/


/*=========================================================================
/ Define here the Arcitecture
/========================================================================*/




#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#include "../inc/host_poe_api.h"



/*---------------------------------------------------------------------
 *    description:     Main function
 *
 *    input :
 *    output:   none
 *
 *    return:
 *---------------------------------------------------------------------*/
char buffer[POE_MESSAGE_SIZE*4];
PoE_RC_e rc;
P_POE_BASE_MESSAGE pbmsg;
P_POE_REQUSET_MESSAGE send;

enum ACTION {
	SET_PORT_MATRIX,
	ENABLE_PORT,
	DISABLE_PORT,
	DISPLAY_TOTAL_POWER,
	DISPLAY_PORT_STATUS,
	DISPLAY_DEVICE_STATUS,
	SW_DOWNLOAD,
	SET_INTERRUPT
};

P_POE_SET_TEMP_MAT setTempMat;
P_POE_SET_BT_EVENTS_INTERRUPT_MASK setEventsIntMask;
P_POE_GET_TOTAL_POWER_REP getTotalPower;
P_POE_GET_SW_VERSION_REP getSWVersion;
P_POE_SET_BT_PORT_PARAMETERS setBtPortParameters;
P_POE_GET_BT_PORT_MEASUREMENTS getPortMeasurements;
P_POE_GET_BT_PORT_MEASUREMENTS_REP getPortMeasurementsRep;
P_POE_GET_BT_PORT_STATUS getBTPortStatus;
P_POE_GET_BT_PORT_STATUS_REP getBTPortStatusRep;
P_POE_GET_BT_DEV_STATUS getDeviceStatus;
P_POE_GET_BT_DEV_STATUS_REP getDeviceStatusRep;

PoE_RC_e set_port_matrix(int group) {
	char project_name[30];
	int i, port;
	int start, end, max_port;

	FILE *fp;
	fp = fopen("/usr/local/sku", "r");
	if (fp == NULL) {
		printf("Can't read /usr/local/sku\n");
		return -1;
	}

	fgets(project_name, 30, fp);
	fclose(fp);

	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	if (strncmp("OS6360-P10", project_name, strlen(project_name)-1) == 0) {
		port = 8;
		max_port = port;
	} else if (strncmp("OS6360-P24X", project_name, strlen(project_name)-1) == 0 ||
	           strncmp("OS6360-PH24", project_name, strlen(project_name)-1) == 0) {
		port = 24;
		max_port = port;
	} else if (strncmp("OS6360-P48X", project_name, strlen(project_name)-1) == 0) {
		port = 46;
		max_port = 48;
	} else {
		printf("Unknown project\n");
		return -2;
	}

	switch (group) {
		case 0:
			start = 0;
			end = port;
			break;

		case 1:
		case 2:
			start = max_port - (max_port / group);
			end = (max_port / (3 - group));

			if (group == 2) {
				end = end - (max_port - port);
			}
			break;

		case 3:
		default:
			start = max_port;
			end = port;
			break;
	}

	printf("Project: %s", project_name);
	for (i = 0; i < port; i++) {
		/* 4.3.1 Set Temporary Matrix */
		setTempMat = (P_POE_SET_TEMP_MAT)pbmsg->data;
		setTempMat->CHNum = i;
		if (i >= start && i < end) {
			setTempMat->physNumA = i;
		} else {
			setTempMat->physNumA = 0xFF;
		}
		setTempMat->physNumB = 0xFF;
		INIT_BASE_MSG(pbmsg, POE_CODE_SET_TEMP_MATRIX, sizeof(*setTempMat));
		rc = POEAPITranslateMessage(pbmsg);
		if (rc != PoE_RC_SUCSESS) {
			printf("Fail to set port matrix\n");
			return rc;
		}
	}

	/* For BT Setting */
	if (strncmp("OS6360-P48X", project_name, strlen(project_name)-1) == 0) {
		/* Front port 47 mapping to 48(0x30) and 49(0x31) */
		/* Front port 48 mapping to 50(0x32) and 51(0x33) */
		for (port = 46, i = 48; i < 52; i+=2, port++) {
			/* 4.3.1 Set Temporary Matrix */
			setTempMat = (P_POE_SET_TEMP_MAT)pbmsg->data;
			setTempMat->CHNum = port;
			if (group == 0 || group == 3) {
				setTempMat->physNumA = i;
				setTempMat->physNumB = i+1;
			} else {
				setTempMat->physNumA = 0xFF;
				setTempMat->physNumB = 0xFF;
			}
			INIT_BASE_MSG(pbmsg, POE_CODE_SET_TEMP_MATRIX, sizeof(*setTempMat));
			rc = POEAPITranslateMessage(pbmsg);
			if (rc != PoE_RC_SUCSESS) {
				printf("Fail to set BT matrix\n");
				return rc;
			}
		}
	}

	/* 4.3.3 Program Global Matrix */
	INIT_BASE_MSG(pbmsg, POE_CODE_PROGRAM_GLOBAL_MATRIX, sizeof(*send));
	rc = POEAPITranslateMessage(pbmsg);
	if (rc != PoE_RC_SUCSESS) {
		printf("Fail to initial matrix\n");
		return rc;
	}

	/* 4.1.3 Save System Settings */
	INIT_BASE_MSG(pbmsg, POE_CODE_SAVE_SYS_SETTING, sizeof(*send));
	rc = POEAPITranslateMessage(pbmsg);
	if (rc == PoE_RC_SUCSESS)
		printf("Success\n");
	else
		printf("Fail\n");

	return rc;
}

PoE_RC_e get_total_power(void) {
	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* 4.4.1 Get Total Power */
	INIT_BASE_MSG(pbmsg, POE_CODE_GET_TOTAL_POWER, sizeof(*getTotalPower));
	rc = POEAPITranslateMessage(pbmsg);
	getTotalPower = (P_POE_GET_TOTAL_POWER_REP)pbmsg->data;
	printf("PowerConsumption(W)=%d\n", getTotalPower->powerConsumption);
	printf("CalculatedPower(W)=%d\n", getTotalPower->calcPower);
	printf("AvailablePower(W)=%d\n", getTotalPower->availablePower);
	printf("PowerLimit(W)=%d\n", getTotalPower->powerLim);
	printf("PowerBank=%d\n", getTotalPower->powerBank);
	printf("VmainVoltage(V)=%f\n", getTotalPower->vmainVoltage * 0.1);

	/* 4.1.19 Get Software Version */
	INIT_BASE_MSG(pbmsg, POE_CODE_GET_SW_VERSION, sizeof(*getSWVersion));
	rc = POEAPITranslateMessage(pbmsg);
	getSWVersion = (P_POE_GET_SW_VERSION_REP)pbmsg->data;
	printf("SWVersion=%d.%d.%d\n", getSWVersion->swMajorNum, getSWVersion->swMinorVerNum ,getSWVersion->swPatchVerNum);
	return rc;
}

PoE_RC_e set_endis_channels(int port, int endis, ...) {
	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* 4.3.6 Set BT Port Parameters */
	setBtPortParameters = (P_POE_SET_BT_PORT_PARAMETERS)pbmsg->data;
	setBtPortParameters->portNum = port;
	setBtPortParameters->portModeCFG1 = endis;
	setBtPortParameters->portModeCFG2PmMode = POE_PORT_PM_MODE_DYNAMIC;
	setBtPortParameters->portModeCFG2ClassErr = POE_CLASS_ERROR_DO_NOT_CHANGE;
	setBtPortParameters->portOperMode = POE_PORT_PM_MODE_4P_90_2P_30_NO_LEGACY;
	setBtPortParameters->addPwrForPortMode = 0x00;
	setBtPortParameters->priority = POE_PORT_PRIORITY_DO_NOT_CHANGE_SETTINGS;
	INIT_BASE_MSG(pbmsg, POE_CODE_SET_BT_PORT_PARAMETERS, sizeof(*setBtPortParameters));
	rc = POEAPITranslateMessage(pbmsg);
	if (rc == PoE_RC_SUCSESS)
		printf("Success\n");
	else
		printf("Fail\n");

	return rc;
}

PoE_RC_e get_port_status(int port, ...) {
	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* 4.4.5 Get BT Port Measurements */
	getPortMeasurements = (P_POE_GET_BT_PORT_MEASUREMENTS)pbmsg->data;
	getPortMeasurements->CHNum = port;
	INIT_BASE_MSG(pbmsg, POE_CODE_GET_BT_PORT_MEAS, sizeof(*getPortMeasurements));
	rc = POEAPITranslateMessage(pbmsg);
	getPortMeasurementsRep = (P_POE_GET_BT_PORT_MEASUREMENTS_REP)pbmsg->data;
	printf("VmainVoltage(V)=%f\n", getPortMeasurementsRep->vmainVoltage * 0.1);
	printf("CalculatedCurrent(mA)=%d\n", getPortMeasurementsRep->calculatedCurrent);
	printf("MeasuredPortPowerByLogical(W)=%f\n", getPortMeasurementsRep->measuredPortPower * 0.1);
	printf("PortVoltage(V)=%f\n", getPortMeasurementsRep->portVoltage * 0.1);

	/* 4.3.8 Get BT Port Status */
	getBTPortStatus = (P_POE_GET_BT_PORT_STATUS)pbmsg->data;
	getBTPortStatus->portNum = port;
	INIT_BASE_MSG(pbmsg, POE_CODE_GET_BT_PORT_STATUS, sizeof(*getBTPortStatus));
	rc = POEAPITranslateMessage(pbmsg);
	getBTPortStatusRep = (P_POE_GET_BT_PORT_STATUS_REP)pbmsg->data;
	printf("PortStatus=%d\n", getBTPortStatusRep->portStatus);
	printf("PortModeCFG1=%d\n", getBTPortStatusRep->portModeCFG1);
	printf("AssignedClassPrimary=%d\n", getBTPortStatusRep->assignedClassPrimary);
	printf("AssignedClassSecondary=%d\n", getBTPortStatusRep->assignedClassSecondary);
	printf("MeasuredPortPowerByFormula(W)=%f\n", getBTPortStatusRep->measuredPortPower * 0.1);

	return rc;
}

PoE_RC_e get_poe_device_status(int device, ...) {
	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* 4.2.2 Get BT PoE Device Status */
	getDeviceStatus = (P_POE_GET_BT_DEV_STATUS)pbmsg->data;
	getDeviceStatus->chipSetNum = device;
	INIT_BASE_MSG(pbmsg, POE_CODE_GET_BT_DEV_STATUS, sizeof(*getDeviceStatus));
	rc = POEAPITranslateMessage(pbmsg);
	getDeviceStatusRep = (P_POE_GET_BT_DEV_STATUS_REP)pbmsg->data;
	printf("DeviceStatus=%d\n", getDeviceStatusRep->devStatus);
	printf("Temperature(C)=%d\n", getDeviceStatusRep->temperature);

	return rc;
}

PoE_RC_e set_poe_interrupt(int endis, ...) {
	pbmsg = (P_POE_BASE_MESSAGE) buffer;

	/* 4.1.9 Set BT Events Interrupt Mask */
	setEventsIntMask = (P_POE_SET_BT_EVENTS_INTERRUPT_MASK)pbmsg->data;
	setEventsIntMask->systemEventMaskRegister = 0;
	setEventsIntMask->deviceEventMaskRegister = 0;
	setEventsIntMask->portEventMaskRegister = endis;
	INIT_BASE_MSG(pbmsg, POE_CODE_SET_BT_EVENTS_INTERRUPT_MASK, sizeof(*setEventsIntMask));
	rc = POEAPITranslateMessage(pbmsg);
	if (rc == PoE_RC_SUCSESS)
		printf("Success\n");
	else
		printf("Fail\n");

	return rc;
}

void usage(void) {
	printf("Usage:\n");
	printf("NAME:\n");
	printf("       poe - PoE utility\n");
	printf("\n");
	printf("SYNOPSIS\n");
	printf("       poe [-i [group]]\n");
	printf("           [-e|-d port]\n");
	printf("           [-t]\n");
	printf("           [-p port]\n");
	printf("           [-s device]\n");
	printf("           [-r enable|disable]\n");
	printf("           [-u file_name]\n");
	printf("           [-h]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("       The command is to do some PoE commands including initial\n");
	printf("       function, enable/disable port, enable/disable interrupt,\n");
	printf("       display power, port's information and SW download. A command\n");
	printf("       only do one action. If you input more actions, only the last one\n");
	printf("       would be execute.\n");
	printf("\n");
	printf("MANDATORY OPTIONS\n");
	printf("       At least one optional option is needed.\n");
	printf("\n");
	printf("OPTIONAL OPTIONS\n");
	printf("       -i    Initialize the deivce.\n");
	printf("             Group: decimal, 1-3, default is 0.\n");
	printf("\n");
	printf("       -e    Enable PoE port.\n");
	printf("             Port: decimal, start from 1.\n");
	printf("\n");
	printf("       -d    Disable PoE port.\n");
	printf("             Port: decimal, start from 1.\n");
	printf("\n");
	printf("       -t    Display total power information.\n");
	printf("\n");
	printf("       -p    Display port's Status.\n");
	printf("             Port: decimal, start from 1.\n");
	printf("\n");
	printf("       -s    Display device's Status.\n");
	printf("             Device: decimal, start from 0.\n");
	printf("\n");
	printf("       -u    Upgrade MCU firmware.\n");
	printf("\n");
	printf("       -r    Enable/Disable interrupt.\n");
	printf("             1: Enable; 0: Disable.\n");
	printf("\n");
	printf("       -h    Help message\n");
	printf("\n");
	printf("PASS CRITERIA\n");
	printf("       - Initial function (-i):\n");
	printf("           SUCCESS: Completed to do initial function.\n");
	printf("             Display: \"Success\"\n");
	printf("             Return code : 0\n");
	printf("\n");
	printf("           ERROR: Fail to do initial function.\n");
	printf("             Display: \"Fail\"\n");
	printf("             Return code : Not 0\n");
	printf("\n");
	printf("       - Enable and disable PoE port (-e and -d):\n");
	printf("           SUCCESS: PoE controller returns success\n");
	printf("             Display: \"Success\" \n");
	printf("             Return code : 0\n");
	printf("\n");
	printf("           ERROR: PoE controller returns fail\n");
	printf("             Display: \"Fail\" \n");
	printf("             Return code : Not 0\n");
	printf("\n");
	printf("       - Software download (-u):\n");
	printf("           SUCCESS: Completed to do this function\n");
	printf("             Display: \"Success\"\n");
	printf("             Return code : 0\n");
	printf("\n");
	printf("           ERROR: Fail to do this function\n");
	printf("             Display: \"Fail\"\n");
	printf("             Return code : Not 0\n");
	printf("\n");
	printf("       - Enable and disable PoE interrupt(-r):\n");
	printf("           SUCCESS: Enable/Disable PoE interrupt\n");
	printf("             Display: \"Success\"\n");
	printf("             Return code : 0\n");
	printf("\n");
	printf("           ERROR: PoE returns fail\n");
	printf("             Display: \"Fail\"\n");
	printf("             Return code : Not 0\n");
	printf("\n");
	printf("EXAMPLE\n");
	printf("       poe -i\n");
	printf("       poe -i 1\n");
	printf("       poe -e 1\n");
	printf("       poe -d 1\n");
	printf("       poe -t\n");
	printf("       poe -p 1\n");
	printf("       poe -s 0\n");
	printf("       por -r 1\n");
	printf("       poe -u 24034600_1000_023.s19\n");
	printf("\n");
}

/*=========================================================================
// MAIN PROGRAM
//=========================================================================*/
int main(int argc, char *argv[])
{
	int ch, result = 0, i2cbus = 8, i2caddr = 0x28, action = -1, port = 0, device = -1, endis = 0;
	char deviceName[MAX_DEVICE_NAME_SIZE] = "NULL", *fileName = NULL;
	int group = 0;
	UINT16 deviceCommDelay = 50;
	PoE_Comm_Dev_e devType = POE_COMM_I2C;

	while ((ch = getopt(argc, argv, ":e:d:tp:s:iu:r:")) != -1) {
		switch (ch) {
			case 'i': /* Set Port Matrix*/
				action = SET_PORT_MATRIX;
				if (argc == 3)
					group = atoi(argv[optind]);
				break;
			case 'e': /* Enable Port */
				action = ENABLE_PORT;
				port = atoi(optarg) - 1;
				break;
			case 'd': /* Disable Port */
				action = DISABLE_PORT;
				port = atoi(optarg) - 1;
				break;
			case 't': /* Disaplay total power */
				action = DISPLAY_TOTAL_POWER;
				break;
			case 'p': /* Display port status */
				action = DISPLAY_PORT_STATUS;
				port = atoi(optarg) - 1;
				break;
			case 's': /* Display device status */
				action = DISPLAY_DEVICE_STATUS;
				device = atoi(optarg);
				break;
			case 'u': /* SW Download*/
				action = SW_DOWNLOAD;
				fileName = optarg;
				break;
			case 'r': /* Enable/Disable PoE interrupt mask */
				action = SET_INTERRUPT;
				endis = atoi(optarg);
				break;
			default:
				usage();
				goto exit_label;
		}
	}

	POE_SET_I2C_PARAMS i2c_params = {i2caddr, i2cbus};

	rc = POEAPIOpenSession((const char *) deviceName, devType, deviceCommDelay, &i2c_params, HOST_POE_API_VER);
	if (rc != PoE_RC_SUCSESS) {
		return rc;
	}

	int i;
	for (i = 0; i < 3; i++)
		POECommWaitReadReply(); /* Read first for clean message*/

	switch (action) {
		case SET_PORT_MATRIX:
			result = set_port_matrix(group);
			break;
		case DISPLAY_TOTAL_POWER:
			result = get_total_power();
			break;
		case ENABLE_PORT:
			if (port != -1)
				result = set_endis_channels(port, 1);
			break;
		case DISABLE_PORT:
			if (port != -1)
				result = set_endis_channels(port, 0);
			break;
		case DISPLAY_DEVICE_STATUS:
			if (device != -1)
				result = get_poe_device_status(device);
			break;
		case DISPLAY_PORT_STATUS:
			if (port != -1)
				result = get_port_status(port);
			break;
		case SW_DOWNLOAD:
			eCmnd_do_poe_firmware_update(fileName);
			break;
		case SET_INTERRUPT:
			result = set_poe_interrupt(endis);
			break;
		default:
			break;
	}

exit_label:
	return result;
}


