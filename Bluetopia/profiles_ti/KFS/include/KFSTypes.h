/*****< kfstypes.h >***********************************************************/
/*      Copyright 2005 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  KFSTypes - Stonestreet One Bluetooth Stack Key Fob (TI Proprietary)       */
/*             Service Types.                                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/03/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __KFSTYPESH__
#define __KFSTYPESH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.  */

   /* The following MACRO is a utility MACRO that assigns the TI Key Fob*/
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is the UUID_16_t variable that  */
   /* is to receive the KFS UUID Constant value.                        */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_KFS_ASSIGN_KFS_SERVICE_UUID_16(_x)               ASSIGN_UUID_16((_x), 0xE0, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Key Fob Service UUID in UUID16 form.    */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the TI Key Fob Service UUID (MACRO returns boolean result) NOT    */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the TI Key Fob Service UUID.               */
#define TI_KFS_COMPARE_KFS_SERVICE_UUID_TO_UUID_16(_x)      COMPARE_UUID_16_TO_CONSTANT((_x), 0xE0, 0xFF)

   /* The following defines the TI Key Fob Service UUID that is used    */
   /* when building the TI Key Fob Service Table.                       */
#define TI_KFS_SERVICE_UUID_CONSTANT                        { 0xE0, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI Key Fob*/
   /* Service KeyPressed Characteristic 16 bit UUID to the specified    */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the KFS KeyPressed UUID     */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_KFS_ASSIGN_KEYPRESSED_UUID_16(_x)                ASSIGN_UUID_16((_x), 0xE1, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Key Fob KeyPressed Characteristic UUID  */
   /* in UUID16 form.  This MACRO only returns whether the UUID_16_t    */
   /* variable is equal to the TI Key Fob KeyPressed Characteristic UUID*/
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the TI Key*/
   /* Fob KeyPressed Characteristic UUID.                               */
#define TI_KFS_COMPARE_KFS_KEYPRESSED_UUID_TO_UUID_16(_x)   COMPARE_UUID_16_TO_CONSTANT((_x), 0xE1, 0xFF)

   /* The following defines the Key Pressed Characteristic UUID that is */
   /* used when building the Key Pressed Characteristic Service Table.  */
#define TI_KFS_KEYPRESSED_CHARACTERISTIC_UUID_CONSTANT      { 0xE1, 0xFF }

   /* The following define the Key Pressed Characteristic bit masks.    */
#define TI_KFS_KEY_PRESSED_BUTTON_ONE_PRESSED               0x01
#define TI_KFS_KEY_PRESSED_BUTTON_TWO_PRESSED               0x02

   /* The following defines the Key Pressed Value Length.               */
#define TI_KFS_KEY_PRESSED_VALUE_LENGTH                     (BYTE_SIZE)

#endif
