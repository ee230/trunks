/*****< KeyFobDemo.c >*********************************************************/
/*      Copyright 2001 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  KeyFobDemo - Embedded Bluetooth Key Fob (TI) Sample Application (SPP +    */
/*               GATT).                                                       */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/25/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __KEYFOBDEMO_H__
#define __KEYFOBDEMO_H__

   /* The following defines the SPP Packet Types that are sent by this  */
   /* application.  Current only packets containing both Button and     */
   /* Accelerometer Data are sent.                                      */
#define KEY_FOB_SPP_PACKET_TYPE_BUTTON_ACCEL_DATA           0x01

   /* The following structure defines the packet header that is placed  */
   /* on SPP Data that is sent by this application.                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagKey_Fob_SPP_Header_t
{
   NonAlignedByte_t PacketType;
   NonAlignedByte_t PacketLength;
} __PACKED_STRUCT_END__ Key_Fob_SPP_Header_t;

#define KEY_FOB_SPP_HEADER_DATA_SIZE                     (sizeof(Key_Fob_SPP_Header_t))

   /* The following packet corresponds to the Packet Type               */
   /* KEY_FOB_SPP_PACKET_TYPE_BUTTON_ACCEL_DATA.                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagKey_Fob_SPP_Button_Accel_Data_t
{
   Key_Fob_SPP_Header_t Header;
   NonAlignedByte_t     KeyPressData;
   NonAlignedByte_t     X_Axis;
   NonAlignedByte_t     Y_Axis;
   NonAlignedByte_t     Z_Axis;
} Key_Fob_SPP_Button_Accel_Data_t;

#define KEY_FOB_SPP_BUTTON_ACCEL_DATA_SIZE               (sizeof(Key_Fob_SPP_Button_Accel_Data_t))

#endif

