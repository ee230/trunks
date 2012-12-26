/*****< buttons.c >************************************************************/
/*      Copyright 2008 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BUTTONS - Stonestreet One Button Processing Implementation.               */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/24/12  T. Cook        Initial implementation.                         */
/******************************************************************************/
#include <string.h>
#include <msp430.h>
#include "Buttons.h"
#include "HAL.h"
#include "BTPSKRNL.h"

#define NUMBER_OF_BUTTONS 2
#define BUTTON_S1         BIT2 
#define BUTTON_S2         BIT3
 
typedef struct _tagButtonState_t
{
   unsigned long   StateChangeMSCount;
   volatile Byte_t UpCount;
   volatile Byte_t DownCount;
} ButtonState_t;

   /* Internal local static variables.                                  */
static ButtonCallback_t CallbackFunction = NULL;
static ButtonState_t    ButtonState[NUMBER_OF_BUTTONS];

   /* Internal static function prototypes.                              */
static void ButtonInterruptHandler(Byte_t BitNumber, Byte_t ArrayIndex);
static void ProcessButtons(void *SchedulerParameter);

   /* The following function is the button press interrupt handler.     */
#pragma vector=PORT2_VECTOR
__interrupt void ButtonInterrupt(void)
{
   switch(__even_in_range(P2IV, 10))
   {
      case P2IV_P2IFG2: 
         ButtonInterruptHandler(BUTTON_S1, 0);
         break;
      case P2IV_P2IFG3:
         ButtonInterruptHandler(BUTTON_S2, 1);
         break;
   }

   /* Exit from low power mode 3.                                       */
   LPM3_EXIT;
}

static void ButtonInterruptHandler(Byte_t BitNumber, Byte_t ArrayIndex)
{
   unsigned long CurrentTickCount = HAL_GetTickCount();
   unsigned long ElapsedTicks;

   ElapsedTicks = CurrentTickCount - ButtonState[ArrayIndex].StateChangeMSCount;

   if(ElapsedTicks & 0x80000000)
      ElapsedTicks = CurrentTickCount + (0xFFFFFFFF - ButtonState[ArrayIndex].StateChangeMSCount) + 1;

   /* Do not allow a button change unless pressed for between 150 and   */
   /* 1600 ms.                                                          */
   if(ElapsedTicks >= BUTTON_SAMPLE_RATE_MILLISECONDS)
   {
      if(P2IES & BitNumber)
         ++(ButtonState[ArrayIndex].UpCount);
      else
         ++(ButtonState[ArrayIndex].DownCount);

      ButtonState[ArrayIndex].StateChangeMSCount = CurrentTickCount;
   }

   /* Toggle the interrupt edge select.                                 */
   if(P2IES & BitNumber)
      P2IES &= ~BitNumber;
   else
      P2IES |= BitNumber;
}

   /* The following function is used to process any buttons that are    */
   /* pressed.                                                          */
static void ProcessButtons(void *SchedulerParameter)
{
   Byte_t        UpCount;
   Byte_t        DownCount;
   Byte_t        BitNumber[]   = {BUTTON_S1, BUTTON_S2};
   Button_t      ButtonEnums[] = {btSwitch1, btSwitch2};
   unsigned int  Index;

   /* If we have been initialized.                                      */
   if(CallbackFunction)
   {
      /* Loop through all of the buttons we are debouncing.             */
      for(Index = 0; Index < NUMBER_OF_BUTTONS; Index++)
      {
         /* Disable the corresponding interrupt while we check the      */
         /* state.                                                      */
         P2IE &= ~BitNumber[Index];
      
         UpCount                      = ButtonState[Index].UpCount;
         ButtonState[Index].UpCount   = 0;

         DownCount                    = ButtonState[Index].DownCount;
         ButtonState[Index].DownCount = 0;

         /* Re-enable the corresponding interrupt.                      */
         P2IE |= BitNumber[Index];
   
         while((UpCount) || (DownCount))
         {
            /* Make the callbacks in the correct order.                 */
            if(UpCount > DownCount)
            {
              (*CallbackFunction)(ButtonEnums[Index], BUTTON_STATE_BUTTON_PRESSED);
              --UpCount;

              if(DownCount)
              {
                 (*CallbackFunction)(ButtonEnums[Index], BUTTON_STATE_BUTTON_RELEASED);
                 --DownCount;
              }
            }
            else
            {
               if(DownCount)
               {
                  (*CallbackFunction)(ButtonEnums[Index], BUTTON_STATE_BUTTON_RELEASED);
                  --DownCount;
               }
               
               if(UpCount)
               {
                  (*CallbackFunction)(ButtonEnums[Index], BUTTON_STATE_BUTTON_PRESSED);
                  --UpCount;
               }
            }
         }
      }
   }
}

   /* The following is the Main Application Thread.  It will Initialize */
   /* the Bluetooth Stack and all used profiles.                        */
int ButtonInit(ButtonCallback_t ButtonCallback)
{
   int ret_val;

   /* Verify that the callback function that was passed in appears      */
   /* valid.                                                            */
   if(ButtonCallback)
   {
      /* Save the pointer to the Callback Function.                     */
      CallbackFunction = ButtonCallback;

      /* Configure the GPIO Registers.                                  */
      P2SEL &= ~(BUTTON_S1 | BUTTON_S2);
      P2REN |= (BUTTON_S1 | BUTTON_S2);
      P2DIR &= ~(BUTTON_S1 | BUTTON_S2);
      P2OUT |= (BUTTON_S1 | BUTTON_S2);
      P2IES |= (BUTTON_S1 | BUTTON_S2);
      P2IFG &= ~(BUTTON_S1 | BUTTON_S2);
      P2IE  |= (BUTTON_S1 | BUTTON_S2);

      /* Add the processing function to the scheduler.                  */
      if(BTPS_AddFunctionToScheduler(ProcessButtons, NULL, 1))
      {
         /* Return success to the caller.                               */
         ret_val = 0;
      }
      else
         ret_val = BUTTONS_ERROR_INVALID_PARAMETER;
   }
   else
      ret_val = BUTTONS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is provided to allow a way of shutting down*/
   /* the button module.                                                */
void ButtonShutdown(void)
{
   /* Delete the button processing function.                            */
   BTPS_DeleteFunctionFromScheduler(ProcessButtons, NULL);
   
   /* Disable the interrupts for the buttons and configure buttons to be*/
   /* inputs with pull down resistors.                                  */
   P2IE  &= ~(BUTTON_S1 | BUTTON_S2);
   P2SEL &= ~(BUTTON_S1 | BUTTON_S2);
   P2OUT &= ~(BUTTON_S1 | BUTTON_S2);
   P2REN |= (BUTTON_S1 | BUTTON_S2);
   P2DIR &= ~(BUTTON_S1 | BUTTON_S2);
}

