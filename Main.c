/*****< main.c >***************************************************************/
/*      Copyright 2001 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAIN - Main application implementation.                                   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/28/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "HAL.h"                 /* Function for Hardware Abstraction.        */
#include "Main.h"                /* Main application header.                  */
#include "EHCILL.h"              /* eHCILL Implementation Header.             */
#include "Accelerometer.h"       /* Accelerometer Definitions.                */
#include "HRDWCFG.h"

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define LED_TOGGLE_RATE_SUCCESS                    (500) /* The LED Toggle    */
                                                         /* rate when the demo*/
                                                         /* successfully      */
                                                         /* starts up.        */

   /* The following parameters are used when configuring HCILL Mode.    */
#define HCILL_MODE_INACTIVITY_TIMEOUT              (500)
#define HCILL_MODE_RETRANSMIT_TIMEOUT              (100)

   /* The following is used as a printf replacement.                    */
#define Display(_x)                 do { BTPS_OutputMessage _x; } while(0)

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int BluetoothStackID;

   /* Application Tasks.                                                */
static void DisplayCallback(char Character);
static unsigned long GetTickCallback(void);
static void MainThread(void);

   /* The following function is registered with the application so that */
   /* it can display strings to the debug UART.                         */
static void DisplayCallback(char Character)
{
   HAL_ConsoleWrite(1, &Character);
}

   /* The following function is registered with the application so that */
   /* it can get the current System Tick Count.                         */
static unsigned long GetTickCallback(void)
{
   return(HAL_GetTickCount());
}

   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
static void MainThread(void)
{
   int                     Result;
   BTPS_Initialization_t   BTPS_Initialization;
   HCI_DriverInformation_t HCI_DriverInformation;

   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, cpUART);
   HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay = 100;

   /* Set up the application callbacks.                                 */
   BTPS_Initialization.GetTickCountCallback  = GetTickCallback;
   BTPS_Initialization.MessageOutputCallback = DisplayCallback;

   /* Initialize the application.                                       */
   if((Result = InitializeApplication(&HCI_DriverInformation, &BTPS_Initialization)) > 0)
   {
      /* Save the Bluetooth Stack ID.                                   */
      BluetoothStackID = (unsigned int)Result;

      /* Go ahead an enable HCILL Mode.                                 */
      HCILL_Init();
      HCILL_Configure(BluetoothStackID, HCILL_MODE_INACTIVITY_TIMEOUT, HCILL_MODE_RETRANSMIT_TIMEOUT, TRUE);

      /* Call the main application state machine.                       */
      while(1)
         ApplicationMain();
   }
}

   /* The following is the Main application entry point.  This function */
   /* will configure the hardware and initialize the OS Abstraction     */
   /* layer, create the Main application thread and start the scheduler.*/
int main(void)
{
   /* Turn off the watchdog timer                                       */
   WDTCTL = WDTPW | WDTHOLD;

   /* Configure the hardware for its intended use.                      */
   HAL_ConfigureHardware();

   /* Enable interrupts and call the main application thread.           */
   __enable_interrupt();
   MainThread();

   /* MainThread should run continously, if it exits an error occured.  */
   while(1)
   {
      HAL_LedToggle(0);
      BTPS_Delay(100);
   }
}


