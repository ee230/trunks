/*****< accelerometer_ez430.c >************************************************/
/*      Copyright 2008 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ACCELEROMETER - Stonestreet One Accelerometer (ez430) Implementation.     */
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
#include "Accelerometer.h"
#include "HAL.h"
#include "BTPSKRNL.h"

   /* Accelerometer Definitions.                                        */
#define ACCEL_SLAVE_ADDRESS                    0x1C    
#define ACCEL_IDENTIFICATION_REGISTER          0x00    
#define ACCEL_REVISION_ID_REGISTER             0x01    
#define ACCEL_CONTROL_REGISTER                 0x02    
#define ACCEL_STATUS_REGISTER                  0x03    
#define ACCEL_RESET_REGISTER                   0x04    
#define ACCEL_INTERRUPT_STATUS_REGISTER        0x05  
#define ACCEL_XAXIS_REGISTER                   0x06    
#define ACCEL_YAXIS_REGISTER                   0x07    
#define ACCEL_ZAXIS_REGISTER                   0x08    
#define ACCEL_MOTION_THRESH_REGISTER           0x09    
#define ACCEL_MOTION_DETECT_TIME_REGISTER      0x0A    
#define ACCEL_FREE_FALL_THRESHOLD_REGISTER     0x0B    

#define ACCEL_RANGE_2G                         0x80    
#define ACCEL_MEASUREMENT_MODE_100HZ           0x02    
#define ACCEL_MEASUREMENT_MODE_400HZ           0x04    
#define ACCEL_MEASUREMENT_MODE_40HZ            0x06    
#define ACCEL_MOTION_DETECT_MODE_10HZ          0x08    
#define ACCEL_FREE_FALL_MODE_100HZ             0x0A    
#define ACCEL_FREE_FALL_MODE_400HZ             0x0C    

   /* Internal Function Prototypes.                      */
static void InitializeI2CModule(unsigned long SystemClockMHz);
static void I2CSetSlaveAddress(unsigned char SlaveAddress);
static void I2CWriteRegister(unsigned char Register, unsigned char Data);
static unsigned char I2CReadRegister(unsigned char Register);
static void I2CShutdown(void);

   /* The following function is provided to provide a way of            */
   /* initializing the I2C Module that is used to communicate with the  */
   /* Accelerometer.                                                    */
static void InitializeI2CModule(unsigned long SystemClockMHz)
{
   /* Initialize the pins used by the I2C Module.                       */
   P10SEL |= BIT2 + BIT1;  

   /* Put the I2C Module in reset while it is being configured.         */
   UCB3CTL1 |= UCSWRST;       

   /* Select the SMCLK as the clock source.                             */
   UCB3CTL1 |= UCSSEL_2;      

   /* Select I2C Master Mode.                                           */
   UCB3CTL0 |= UCMST + UCSYNC + UCMODE_3; 

   /* Set the High Baud Rate divider to 0.                              */
   UCB3BR1 = 0;            

   /* Calculate the divider needed to generate a 100KHz clock signal.   */
   UCB3BR0 = (SystemClockMHz / 100000L);

   /* Bring the I2C Module out of reset.                                */
   UCB3CTL1 &= ~UCSWRST;      
}
   
   /* The following function is used to set the Slave Address.          */
static void I2CSetSlaveAddress(unsigned char SlaveAddress)
{
   /* Reset the I2C Module while we set the Slave Address.              */
   UCB3CTL1 |= UCSWRST;       

   /* Configure the Slave Address.                                      */
   UCB3I2CSA = SlaveAddress;       

   /* Bring the I2C Module out of reset.                                */
   UCB3CTL1 &= ~UCSWRST;      
}

   /* The following function is used to write to a specified I2C        */
   /* Register.                                                         */
static void I2CWriteRegister(unsigned char Register, unsigned char Data)
{
   /* Start the I2C Transaction by sending the Slave Address.           */
   UCB3CTL1 |= UCTR + UCTXSTT;    

   /* Wait for the transmission to be completed.                        */
   while (!(UCB3IFG & UCTXIFG))
      ;    

   /* Send the Register address to the slave.                           */
   UCB3TXBUF = Register;        

   /* Wait for the transmission to be completed.                        */
   while (!(UCB3IFG & UCTXIFG))
      ;    

   /* Send the requested data.                                          */
   UCB3TXBUF = Data;          

   /* Wait for the transmission to be completed.                        */
   while (!(UCB3IFG & UCTXIFG))
      ;    

   /* Send the stop.                                                    */
   UCB3CTL1 |= UCTXSTP;       

   /* Wait for the Stop Procedure to be completed.                      */
   while (UCB3CTL1 & UCTXSTP)
      ; 

   /* Clear the Tx Interrupt Flag.                                      */
   UCB3IFG &= ~UCTXIFG;         
}
   
   /* The following function is used to read a specified register from  */
   /* an I2C Slave.                                                     */
static unsigned char I2CReadRegister(unsigned char Register)
{
   unsigned char Result;

   /* Start the I2C Transaction by sending the Slave Address.           */
   UCB3CTL1 |= UCTR + UCTXSTT;    

   /* Wait for the transmission to be completed.                        */
   while (!(UCB3IFG & UCTXIFG))
      ;    

   /* Send the Register address to the slave.                           */
   UCB3TXBUF = Register;        

   /* Wait for the transmission to be completed.                        */
   while (!(UCB3IFG & UCTXIFG))
      ;    

   /* Clear the Tx Interrupt Flag.                                      */
   UCB3IFG &= ~UCTXIFG;   
   
   /* Go to I2C Receiver Mode.                                          */
   UCB3CTL1 &= ~UCTR; 

   /* Restart the I2C Transaction by sending the Slave Address.         */
   UCB3CTL1 |= UCTXSTT;    

   /* Wait for the transmission to be completed.                        */
   while (UCB3CTL1 & UCTXSTT)
      ;  

   /* Set the I2C Stop Condition.                                       */
   UCB3CTL1 |= UCTXSTP;       

   /* Wait for the data to be received.                                 */
   while (!(UCB3IFG & UCRXIFG));    

   /* Read the Data.                                                    */
   Result = UCB3RXBUF;         

   /* Terminate the Rx Communication and wait for the stop to complete. */
   while (UCB3CTL1 & UCTXSTP);    

   /* Clear the Rx Interrupt flag.                                      */
   UCB3IFG &= ~UCRXIFG;         

   return(Result);
}

   /* The following function is used to shutdown the I2C Port.          */
static void I2CShutdown(void)
{
   /* Reset the I2C Module.                                             */
   UCB3CTL1 |= UCSWRST;       

   /* Set the I2C Pins as output at logic level 0.                      */
   P10OUT &= ~(BIT2 | BIT1);   
   P10SEL &= ~(BIT2 | BIT1);   
   P10DIR |= BIT2 | BIT1;  
}
   
   /* The following function is provided to allow a mechanism of        */
   /* enabling the accelerometer.                                       */
void Acceleterometer_Enable(unsigned long SystemClockMHz)
{
   /* Initialize the I2C Module.                                        */
   InitializeI2CModule(SystemClockMHz);

   /* Configure the Acceleterometer Interrupt Pin as an input with      */
   /* pull-down resistors.  The interrupt for the pin is configured to  */
   /* fire on a low to high transition.                                 */
   P1SEL &= ~BIT2;    
   P1DIR &= ~BIT2;    
   P1REN |= BIT2;
   P1OUT &= ~BIT2;
   P1IES &= ~BIT2;
   P1IFG &= ~BIT2;
   P1IE  |= BIT2;

   /* Configure the I2C Address of the Accelerometer.                   */
   I2CSetSlaveAddress(ACCEL_SLAVE_ADDRESS);   

   /* Configure the Accelerometer for 2G Mode with 40Hz measurement     */
   /* rate.                                                             */
   I2CWriteRegister(ACCEL_CONTROL_REGISTER, ACCEL_RANGE_2G | ACCEL_MEASUREMENT_MODE_40HZ);
}

   /* The following function is used to determine if accelerometer data */
   /* is ready to be read.                                              */
unsigned char Accelerometer_Ready(void)
{
   return((unsigned char)(P1IN & BIT2));
}

   /* The following function is used to read accelerometer data from the*/
   /* accelerometer.                                                    */
void Accelerometer_Read(Accelerometer_Data_t *AccelData)
{
   if(AccelData)
   {
      /* Set the slave address for the accelerometer.                   */
      I2CSetSlaveAddress(ACCEL_SLAVE_ADDRESS);    
   
      /* Wait until the data is valid.                                  */
      while (!(P1IN & BIT2)) 
         ;    
      AccelData->X_Axis = I2CReadRegister(ACCEL_XAXIS_REGISTER);  
   
      /* Wait until the data is valid.                                  */
      while (!(P1IN & BIT2))
         ;    
      AccelData->Y_Axis = I2CReadRegister(ACCEL_YAXIS_REGISTER);  
   
      /* Wait until the data is valid.                                  */
      while (!(P1IN & BIT2))
         ;    

      AccelData->Z_Axis = I2CReadRegister(ACCEL_ZAXIS_REGISTER);  
   }
}
   
   /* The following function is provided to allow a mechanism of        */
   /* shutting down the accelerometer.                                  */
void Accelerometer_Shutdown(void)
{
   /* Set the slave address for the accelerometer.                      */
   I2CSetSlaveAddress(ACCEL_SLAVE_ADDRESS);    

   /* Reset the Accelerometer.                                          */
   I2CWriteRegister(ACCEL_RESET_REGISTER, 0x02);
   I2CWriteRegister(ACCEL_RESET_REGISTER, 0x0A);
   I2CWriteRegister(ACCEL_RESET_REGISTER, 0x04);

   /* Shutdown the I2C Module.                                          */
   I2CShutdown();

   /* Configure the Accelerometer to be an input with pull-down resistor*/
   /* enabled.                                                          */
   P1SEL &= ~BIT2;
   P1DIR &= ~BIT2;  
   P1OUT &= ~BIT2;   
   P1REN |= BIT2; 
   P1IE  &= ~BIT2;
}

   /* The following function is a utility function that is used to      */
   /* process the ADC.                                                  */
void ProcessADC(void *Parameter)
{
   //xxx nothing to do here.
}
