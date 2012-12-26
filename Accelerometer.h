/*****< accelerometer.h >******************************************************/
/*      Copyright 2000 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ACCELEROMETER - Accelerometer Processing Definitions and Declarations.    */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/24/12  T. Cook        Initial Creation                                */
/******************************************************************************/
#ifndef __ACCELEROMETERH__
#define __ACCELEROMETERH__

   /* The following structure is a container for the information that is*/
   /* provided by the accelerometer.                                    */
typedef struct _tagAccelerometer_Data_t
{
   unsigned char X_Axis;
   unsigned char Y_Axis;
   unsigned char Z_Axis;
} Accelerometer_Data_t;

   /* The following function is provided to allow a mechanism of        */
   /* enabling the accelerometer.                                       */
void Acceleterometer_Enable(unsigned long SystemClockMHz);
   
   /* The following function is used to determine if accelerometer data */
   /* is ready to be read.                                              */
unsigned char Accelerometer_Ready(void);

   /* The following function is used to read accelerometer data from the*/
   /* accelerometer.                                                    */
void Accelerometer_Read(Accelerometer_Data_t *AccelData);

   /* The following function is provided to allow a mechanism of        */
   /* shutting down the accelerometer.                                  */
void Accelerometer_Shutdown(void);

   /* The following function is a utility function that is used to      */
   /* process the ADC.                                                  */
void ProcessADC(void *Parameter);

#endif

