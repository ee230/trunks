/*****< main.h >***************************************************************/
/*                                                                            */
/*  Based on KeyFobDemo                                                       */
/*                                                                            */
/*      Copyright 2001 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  Author:  Tim Cook, Clément Guérin                                         */
/*                                                                            */
/******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "SS1BTPS.h"             /* Main SS1 Bluetooth Stack Header.          */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define APPLICATION_ERROR_INVALID_PARAMETERS             (-1000)
#define APPLICATION_ERROR_UNABLE_TO_OPEN_STACK           (-1001)

   /* The following function is used to initialize the application      */
   /* instance.  This function should open the stack and prepare to     */
   /* execute commands based on user input.  The first parameter passed */
   /* to this function is the HCI Driver Information that will be used  */
   /* when opening the stack and the second parameter is used to pass   */
   /* parameters to BTPS_Init.  This function returns the               */
   /* BluetoothStackID returned from BSC_Initialize on success or a     */
   /* negative error code (of the form APPLICATION_ERROR_XXX).          */
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);

   /* The following function is the main application state machine which*/
   /* is used to process all application events.                        */
void ApplicationMain(void);

   /* The following function is used to notify the application that data*/
   /* must be sent.                                                     */
void DataSendCallback(void *param);

#endif

