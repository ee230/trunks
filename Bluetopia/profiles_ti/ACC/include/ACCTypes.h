/*****< acctypes.h >***********************************************************/
/*      Copyright 2005 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ACCTypes - Stonestreet One Bluetooth Stack Accelerometer (TI Proprietary) */
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
#ifndef __ACCTYPESH__
#define __ACCTYPESH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.  */

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the TI Accelerometer UUID Constant    */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_ACC_SERVICE_UUID_16(_x)            ASSIGN_UUID_16((_x), 0xA0, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service UUID in UUID16    */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the TI Accelerometer Service UUID (MACRO returns boolean */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the TI Accelerometer Service     */
   /* UUID.                                                             */
#define TI_ACC_COMPARE_ACC_SERVICE_UUID_TO_UUID_16(_x)   COMPARE_UUID_16_TO_CONSTANT((_x), 0xA0, 0xFF)

   /* The following defines the TI Accelerometer Service UUID that is   */
   /* used when building the TI Accelerometer Service Table.            */
#define TI_ACC_SERVICE_UUID_CONSTANT                     { 0xA0, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service Enable Characteristic 16 bit UUID to the    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the KFS Enable */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_ENABLE_UUID_16(_x)                 ASSIGN_UUID_16((_x), 0xA1, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service Enable            */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the TI Accelerometer   */
   /* Service Enable Characteristic UUID (MACRO returns boolean result) */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the TI Accelerometer Service Enable        */
   /* Characteristic UUID.                                              */
#define TI_ACC_COMPARE_ACC_ENABLE_UUID_TO_UUID_16(_x)    COMPARE_UUID_16_TO_CONSTANT((_x), 0xA1, 0xFF)

   /* The following defines the TI Accelerometer Service Enable         */
   /* Characteristic UUID that is used when building the TI             */
   /* Accelerometer Service Table.                                      */
#define TI_ACC_ENABLE_CHARACTERISTIC_UUID_CONSTANT       { 0xA1, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service Range Characteristic 16 bit UUID to the     */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the KFS Range  */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_RANGE_UUID_16(_x)                  ASSIGN_UUID_16((_x), 0xA2, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service Range             */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the TI Accelerometer   */
   /* Service Range Characteristic UUID (MACRO returns boolean result)  */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the TI Accelerometer Service Range         */
   /* Characteristic UUID.                                              */
#define TI_ACC_COMPARE_ACC_RANGE_UUID_TO_UUID_16(_x)     COMPARE_UUID_16_TO_CONSTANT((_x), 0xA2, 0xFF)

   /* The following defines the TI Accelerometer Service Range          */
   /* Characteristic UUID that is used when building the TI             */
   /* Accelerometer Service Table.                                      */
#define TI_ACC_RANGE_CHARACTERISTIC_UUID_CONSTANT        { 0xA2, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service X-Axis Characteristic 16 bit UUID to the    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the KFS X-Axis */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_X_AXIS_UUID_16(_x)                 ASSIGN_UUID_16((_x), 0xA3, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service X-Axis            */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the TI Accelerometer   */
   /* Service X-Axis Characteristic UUID (MACRO returns boolean result) */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the TI Accelerometer Service X-Axis        */
   /* Characteristic UUID.                                              */
#define TI_ACC_COMPARE_ACC_X_AXIS_UUID_TO_UUID_16(_x)   COMPARE_UUID_16_TO_CONSTANT((_x), 0xA3, 0xFF)

   /* The following defines the TI Accelerometer Service X-Axis         */
   /* Characteristic UUID that is used when building the TI             */
   /* Accelerometer Service Table.                                      */
#define TI_ACC_X_AXIS_CHARACTERISTIC_UUID_CONSTANT      { 0xA3, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service Y-Axis Characteristic 16 bit UUID to the    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the KFS Y-Axis */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_Y_AXIS_UUID_16(_x)                ASSIGN_UUID_16((_x), 0xA4, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service Y-Axis            */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the TI Accelerometer   */
   /* Service Y-Axis Characteristic UUID (MACRO returns boolean result) */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the TI Accelerometer Service Y-Axis        */
   /* Characteristic UUID.                                              */
#define TI_ACC_COMPARE_ACC_Y_AXIS_UUID_TO_UUID_16(_x)   COMPARE_UUID_16_TO_CONSTANT((_x), 0xA4, 0xFF)

   /* The following defines the TI Accelerometer Service Y-Axis         */
   /* Characteristic UUID that is used when building the TI             */
   /* Accelerometer Service Table.                                      */
#define TI_ACC_Y_AXIS_CHARACTERISTIC_UUID_CONSTANT      { 0xA4, 0xFF }

   /* The following MACRO is a utility MACRO that assigns the TI        */
   /* Accelerometer Service Z-Axis Characteristic 16 bit UUID to the    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the KFS Z-Axis */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define TI_ACC_ASSIGN_Z_AXIS_UUID_16(_x)                ASSIGN_UUID_16((_x), 0xA5, 0xFF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined TI Accelerometer Service Z-Axis            */
   /* Characteristic UUID in UUID16 form.  This MACRO only returns      */
   /* whether the UUID_16_t variable is equal to the TI Accelerometer   */
   /* Service Z-Axis Characteristic UUID (MACRO returns boolean result) */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the TI Accelerometer Service Z-Axis        */
   /* Characteristic UUID.                                              */
#define TI_ACC_COMPARE_ACC_Z_AXIS_UUID_TO_UUID_16(_x)   COMPARE_UUID_16_TO_CONSTANT((_x), 0xA5, 0xFF)

   /* The following defines the TI Accelerometer Service Z-Axis         */
   /* Characteristic UUID that is used when building the TI             */
   /* Accelerometer Service Table.                                      */
#define TI_ACC_Z_AXIS_CHARACTERISTIC_UUID_CONSTANT       { 0xA5, 0xFF }

   /* The following define the valid Enable Characteristic Values.      */
#define TI_ACC_ENABLE_ACCELEROMETER_DISABLED             0x00
#define TI_ACC_ENABLE_ACCELEROMETER_ENABLE               0x01

   /* The following defines the TI Accelerometer Service Enable         */
   /* Characterisitic Value Length.                                     */
#define TI_ACC_ENABLE_VALUE_LENGTH                       (BYTE_SIZE)

   /* The following define the valid Range Characteristic Values.       */
#define TI_ACC_RANGE_2G                                  20
#define TI_ACC_RANGE_8G                                  80

   /* The following defines the TI Accelerometer Service Range          */
   /* Characterisitic Value Length.                                     */
#define TI_ACC_RANGE_VALUE_LENGTH                       (WORD_SIZE)

   /* The following defines the TI Accelerometer Service Axis (X,Y or Z)*/
   /* Characteristic Value Length.                                      */
#define TI_ACC_AXIS_VALUE_LENGTH                        (BYTE_SIZE)

   /* The following defines the KFS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the KFS Service is      */
   /* registered.                                                       */
#define TI_ACC_SERVICE_FLAGS                            (GATT_SERVICE_FLAGS_LE_SERVICE|GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
