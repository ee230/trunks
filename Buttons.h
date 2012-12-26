/*****< buttons.h >************************************************************/
/*      Copyright 2000 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BUTTONS - Button Processing Definitions and Declarations.                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/24/12  T. Cook        Initial Creation                                */
/******************************************************************************/
#ifndef __BUTTONSH__
#define __BUTTONSH__

   /* The following definde the error code values that may be returned  */
   /* by the function in this module.                                   */
#define BUTTONS_ERROR_SUCCESS                             (0)
#define BUTTONS_ERROR_INVALID_PARAMETER                  (-1)
#define BUTTONS_ERROR_THREAD_CREATION_FAILURE            (-2)
   
   /* Defines the amount of time that a button must be pressed to be    */
   /* considered debounced.                                             */
#define BUTTON_SAMPLE_RATE_MILLISECONDS                  30

   /* The following represents all of the buttons that may be processed */
   /* by this module.                                                   */
   /* * NOTE * For the ez430 only btSwitch1 and btSwitch2 are provided. */
typedef enum
{
   btUp,
   btDown,
   btSelect,
   btSwitch1,
   btSwitch2
} Button_t;

   /* The following defines the values that can be passed in the        */
   /* ButtonState parameter of the UserIOCallback.                      */
#define BUTTON_STATE_BUTTON_RELEASED                     0
#define BUTTON_STATE_BUTTON_PRESSED                      1

   /* The folloeing defines the format of the User IO Callback function.*/
typedef void (*ButtonCallback_t)(Button_t Button, unsigned char ButtonState);

   /* The following is the Main Application Thread.  It will Initialize */
   /* the Bluetooth Stack and all used profiles.                        */
int ButtonInit(ButtonCallback_t ButtonCallbackFunction);
   
   /* The following function is provided to allow a way of shutting down*/
   /* the button module.                                                */
void ButtonShutdown(void);

#endif

