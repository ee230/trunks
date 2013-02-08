/*****< main.c >***************************************************************/
/*                                                                            */
/*  Based on KeyFobDemo                                                       */
/*                                                                            */
/*      Copyright 2001 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  Author:  Tim Cook, Clément Guérin                                         */
/*                                                                            */
/******************************************************************************/

#include "HAL.h"
#include "Main.h"
#include "EHCILL.h"
#include "HRDWCFG.h"

#define HCILL_MODE_INACTIVITY_TIMEOUT              (500)
#define HCILL_MODE_RETRANSMIT_TIMEOUT              (100)

static void DisplayCallback(char Character) {
	HAL_ConsoleWrite(1, &Character);
}

static unsigned long GetTickCallback(void) {
	return (HAL_GetTickCount());
}

static void MainThread(void) {
	unsigned int BluetoothStackID;
	int Result;
	BTPS_Initialization_t BTPS_Initialization;
	HCI_DriverInformation_t HCI_DriverInformation;

	/* Configure the UART Parameters.                                    */
	HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, cpUART);
	HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay =
			100;

	/* Set up the application callbacks.                                 */
	BTPS_Initialization.GetTickCountCallback = GetTickCallback;
	BTPS_Initialization.MessageOutputCallback = DisplayCallback;

	/* Initialize the application.                                       */
	if ((Result = InitializeApplication(&HCI_DriverInformation,
			&BTPS_Initialization)) > 0) {
		/* Save the Bluetooth Stack ID.                                   */
		BluetoothStackID = (unsigned int) Result;

		/* Go ahead an enable HCILL Mode.                                 */
		HCILL_Init();
		HCILL_Configure(BluetoothStackID, HCILL_MODE_INACTIVITY_TIMEOUT,
				HCILL_MODE_RETRANSMIT_TIMEOUT, TRUE);

		/* Call the main application state machine.                       */
		while (1) {
			ApplicationMain();
		}
	}
}

int main(void) {
	/* Turn off the watchdog timer                                       */
	WDTCTL = WDTPW | WDTHOLD;

	/* Configure the hardware for its intended use.                      */
	HAL_ConfigureHardware();

	/* Enable interrupts and call the main application thread.           */
	__enable_interrupt();
	MainThread();

	/* MainThread should run continously, if it exits an error occured.  */
	while (1) {
		HAL_LedToggle(0);
		BTPS_Delay(100);
	}
}

