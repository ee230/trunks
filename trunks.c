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
/*   11/15/12  Z. Haq         Minor Mods for SPP data format and Device Name  */
/******************************************************************************/
#include <stdio.h>               /* Included for sscanf.                      */
#include <stdlib.h>                    /*included for itoa() call             */
#include "Main.h"                /* Application Interface Abstraction.        */
#include "EHCILL.h"              /* HCILL Implementation Header.              */
#include "HAL.h"                 /* Function for Hardware Abstraction.        */
#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "BTPSKRNL.h"            /* BTPS Kernel Header.                       */
#include "HCITRANS.h"            /* HCI Transport Layer Header.               */
#include "HRDWCFG.h"             /* Hardware Configuration.                   */

#define MAX_SUPPORTED_LINK_KEYS                    (1)   /* Max supported Link*/
/* keys.             */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
/* error occurred in */
/* execution of the  */
/* Command Function. */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
/* error occurred due*/
/* to the fact that  */
/* one or more of the*/
/* required          */
/* parameters were   */
/* invalid.          */

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
/* error occurred    */
/* while Initializing*/
/* the Bluetooth     */
/* Protocol Stack.   */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
/* occurred due to   */
/* attempted         */
/* execution of a    */
/* Command when a    */
/* Bluetooth Protocol*/
/* Stack has not been*/
/* opened.           */

#define CLASSIC_PIN_CODE                          "0000" /* PIN Code used for */
/* Classic Pairing.  */

#define SPP_PORT_NUMBER                           1      /* Default SPP Port  */
/* Number.           */

#define SPP_BUFFER_SIZE                           512    /* The following     */
/* controls the SPP  */
/* Buffer Size.      */

/* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "Keyfob"
#define CB_DEMO_DEVICE_NAME                        "Keyfob"

/* The following is used as a printf replacement.                    */
#define Display(_x)                                do { BTPS_OutputMessage _x; } while(0)

/* The following type definition represents the container type which */
/* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
/* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t {
	BD_ADDR_t BD_ADDR;
	Link_Key_t LinkKey;
} LinkKeyInfo_t;

/* Structure used to hold all of the GAP LE Parameters.              */
typedef struct _tagGAPLE_Parameters_t {
	GAP_LE_IO_Capability_t IOCapability;
	Boolean_t MITMProtection;
	Boolean_t OOBDataPresent;
} GAPLE_Parameters_t;

#define GAPLE_PARAMETERS_DATA_SIZE                       (sizeof(GAPLE_Parameters_t))

/* The following structure is a container for information on         */
/* connected devices.                                                */
typedef struct _tagConnectionInfo_t {
	unsigned int ConnectionIndex;
	BD_ADDR_t BD_ADDR;
} ConnectionInfo_t;

/* The following structure is used to hold all of the application    */
/* state information.                                                */
typedef struct _tagApplicationStateInfo_t {
	unsigned int BluetoothStackID;
	Byte_t Flags;
	Mailbox_t Mailbox;
	unsigned int GAPSInstanceID;
	unsigned int HCIEventCallbackHandle;
	ConnectionInfo_t LEConnectionInfo;
	ConnectionInfo_t CBConnectionInfo;
	unsigned int SPPServerPortID;
	DWord_t SPPServerSDPHandle;
	unsigned int SPPBufferLength;
	Byte_t SPPBuffer[SPP_BUFFER_SIZE];
	Byte_t AccelEnableCount;
} ApplicationStateInfo_t;

#define APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED        0x01
#define APPLICATION_STATE_INFO_FLAGS_CB_CONNECTED        0x02
#define APPLICATION_STATE_INFO_FLAGS_SPP_BUFFER_FULL     0x04
#define APPLICATION_STATE_INFO_SNIFF_MODE_ACTIVE         0x08

/* The following defines are used with the application mailbox.      */
#define APPLICATION_MAILBOX_DEPTH                        8
#define APPLICATION_MAILBOX_SIZE                         BYTE_SIZE

#define APPLICATION_MAILBOX_MESSAGE_ID_LE_DISCONNECTED   0x01
#define APPLICATION_MAILBOX_MESSAGE_ID_LE_CONNECTED      0x02
#define APPLICATION_MAILBOX_MESSAGE_ID_CB_DISCONNECTED   0x03
#define APPLICATION_MAILBOX_MESSAGE_ID_CB_CONNECTED      0x04
#define APPLICATION_MAILBOX_MESSAGE_ID_SPP_BUFFER_EMPTY  0x05

/* The following structure for a Master is used to hold a list of    */
/* information on all paired devices. For slave we will not use this */
/* structure.                                                        */
typedef struct _tagDeviceInfo_t {
	Byte_t Flags;
	Byte_t EncryptionKeySize;
	GAP_LE_Address_Type_t ConnectionAddressType;
	BD_ADDR_t ConnectionBD_ADDR;
	Long_Term_Key_t LTK;
	Random_Number_t Rand;
	Word_t EDIV;
	struct _tagDeviceInfoInfo_t *NextDeviceInfoInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

/* Defines the bit mask flags that may be set in the DeviceInfo_t    */
/* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                      0x01

/* User to represent a structure to hold a BD_ADDR return from       */
/* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[16];

/* The Encryption Root Key should be generated  */
/* in such a way as to guarantee 128 bits of    */
/* entropy.                                     */
static BTPSCONST Encryption_Key_t ER = { 0x28, 0xBA, 0xE1, 0x35, 0x13, 0xB2,
		0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51 };

/* The Identity Root Key should be generated    */
/* in such a way as to guarantee 128 bits of    */
/* entropy.                                     */
static BTPSCONST Encryption_Key_t IR = { 0x41, 0x09, 0xA2, 0x88, 0x09, 0x6B,
		0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE };

/* The following keys can be regerenated on the */
/* fly using the constant IR and ER keys and    */
/* are used globally, for all devices.          */
static Encryption_Key_t DHK;
static Encryption_Key_t IRK;

/* Internal Variables to this Module (Remember that all variables    */
/* declared static are initialized to 0 automatically by the         */
/* compiler as part of standard C/C++).                              */

static ApplicationStateInfo_t ApplicationStateInfo; /* Container for all of the        */
/* Application State Information.  */

static GAPLE_Parameters_t LE_Parameters; /* Holds GAP Parameters like       */
/* Discoverability, Connectability */
/* Modes.                          */

static DeviceInfo_t *DeviceInfoList; /* Holds the list head for the     */
/* device info list.               */

static LinkKeyInfo_t LinkKeyInfo[MAX_SUPPORTED_LINK_KEYS]; /* Variable holds     */
/* BD_ADDR <-> Link Keys for       */
/* pairing.                        */

static GAP_IO_Capability_t IOCapability; /* Variable which holds the        */
/* current I/O Capabilities that   */
/* are to be used for Secure Simple*/
/* Pairing.                        */

static Boolean_t OOBSupport; /* Variable which flags whether    */
/* or not Out of Band Secure Simple*/
/* Pairing exchange is supported.  */

static Boolean_t MITMProtection; /* Variable which flags whether or */
/* not Man in the Middle (MITM)    */
/* protection is to be requested   */
/* during a Secure Simple Pairing  */
/* procedure.                      */

/* The following string table is used to map HCI Version information */
/* to an easily displayable version string.                          */
static BTPSCONST char *HCIVersionStrings[] = { "1.0b", "1.1", "1.2", "2.0",
		"2.1", "3.0", "4.0", "Unknown (greater 4.0)" };

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

/* The following string table is used to map the API I/O Capabilities*/
/* values to an easily displayable string.                           */
static BTPSCONST char *IOCapabilitiesStrings[] =
		{ "Display Only", "Display Yes/No", "Keyboard Only", "No Input/Output",
				"Keyboard/Display" };

/* Internal function prototypes.                                     */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead,
		GAP_LE_Address_Type_t ConnectionAddressType,
		BD_ADDR_t ConnectionBD_ADDR);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead,
		BD_ADDR_t BD_ADDR);
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead,
		BD_ADDR_t BD_ADDR);
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree);
static void FreeDeviceInfoList(DeviceInfo_t **ListHead);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr);

static void DisplayPairingInformation(
		GAP_LE_Pairing_Capabilities_t Pairing_Capabilities);
static void DisplayFunctionError(char *Function, int Status);
static void DisplayFunctionSuccess(char *Function);

static int SPPOpenServer(unsigned int BluetoothStackID);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation,
		BTPS_Initialization_t *BTPS_Initialization);
static int CloseStack(void);

static int SetPairable(void);

static void PostApplicationMailbox(Byte_t MessageID);

static void ConfigureCapabilities(GAP_LE_Pairing_Capabilities_t *Capabilities);
static int SlavePairingRequestResponse(unsigned int BluetoothStackID,
		BD_ADDR_t BD_ADDR);
static int EncryptionInformationRequestResponse(unsigned int BluetoothStackID,
		BD_ADDR_t BD_ADDR, Byte_t KeySize,
		GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);
static int DeleteLinkKey(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

static void FormatEIRData(unsigned int BluetoothStackID);
static void FormatAdvertisingData(unsigned int BluetoothStackID,
		Boolean_t SupportBR_EDR);

static int StartAdvertising(unsigned int BluetoothStackID);

static void IdleFunction(unsigned int BluetoothStackID);

static unsigned int FormatSPPDataPacket(unsigned int PacketBufferLength,
		Byte_t *PacketBuffer);
static void ProcessSendSPPData(Boolean_t PacketizeCurrentData);

/* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID,
		GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Connection_Event_Callback(
		unsigned int BluetoothStackID,
		GATT_Connection_Event_Data_t *GATT_Connection_Event_Data,
		unsigned long CallbackParameter);
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID,
		GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SPP_Event_Callback(unsigned int BluetoothStackID,
		SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter);

/* The following function adds the specified Entry to the specified  */
/* List.  This function allocates and adds an entry to the list that */
/* has the same attributes as parameters to this function.  This     */
/* function will return FALSE if NO Entry was added.  This can occur */
/* if the element passed in was deemed invalid or the actual List    */
/* Head was invalid.                                                 */
/* ** NOTE ** This function does not insert duplicate entries into   */
/*            the list.  An element is considered a duplicate if the */
/*            Connection BD_ADDR.  When this occurs, this function   */
/*            returns NULL.                                          */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead,
		GAP_LE_Address_Type_t ConnectionAddressType,
		BD_ADDR_t ConnectionBD_ADDR) {
	Boolean_t ret_val = FALSE;
	DeviceInfo_t *DeviceInfoPtr;

	/* Verify that the passed in parameters seem semi-valid.             */
	if ((ListHead) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))) {
		/* Allocate the memory for the entry.                             */
		if ((DeviceInfoPtr = BTPS_AllocateMemory(sizeof(DeviceInfo_t))) != NULL) {
			/* Initialize the entry.                                       */
			BTPS_MemInitialize(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
			DeviceInfoPtr->ConnectionAddressType = ConnectionAddressType;
			DeviceInfoPtr->ConnectionBD_ADDR = ConnectionBD_ADDR;

			ret_val = BSC_AddGenericListEntry_Actual(ekBD_ADDR_t,
					BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR),
					BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr),
					(void **) (ListHead), (void *) (DeviceInfoPtr));
			if (!ret_val) {
				/* Failed to add to list so we should free the memory that  */
				/* we allocated for the entry.                              */
				BTPS_FreeMemory(DeviceInfoPtr);
			}
		}
	}

	return (ret_val);
}

/* The following function searches the specified List for the        */
/* specified Connection BD_ADDR.  This function returns NULL if      */
/* either the List Head is invalid, the BD_ADDR is invalid, or the   */
/* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead,
		BD_ADDR_t BD_ADDR) {
	return (BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *) (&BD_ADDR),
			BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR),
			BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr),
			(void **) (ListHead)));
}

/* The following function searches the specified Key Info List for   */
/* the specified BD_ADDR and removes it from the List.  This function*/
/* returns NULL if either the List Head is invalid, the BD_ADDR is   */
/* invalid, or the specified Entry was NOT present in the list.  The */
/* entry returned will have the Next Entry field set to NULL, and    */
/* the caller is responsible for deleting the memory associated with */
/* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead,
		BD_ADDR_t BD_ADDR) {
	return (BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *) (&BD_ADDR),
			BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR),
			BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr),
			(void **) (ListHead)));
}

/* This function frees the specified Key Info Information member     */
/* memory.                                                           */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree) {
	BSC_FreeGenericListEntryMemory((void *) (EntryToFree));
}

/* The following function deletes (and free's all memory) every      */
/* element of the specified Key Info List. Upon return of this       */
/* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead) {
	BSC_FreeGenericListEntryList((void **) (ListHead),
			BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr) );
}

/* The following function is responsible for converting data of type */
/* BD_ADDR to a string.  The first parameter of this function is the */
/* BD_ADDR to be converted to a string.  The second parameter of this*/
/* function is a pointer to the string in which the converted BD_ADDR*/
/* is to be stored.                                                  */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr) {
	BTPS_SprintF((char *) BoardStr, "0x%02X%02X%02X%02X%02X%02X",
			Board_Address.BD_ADDR5, Board_Address.BD_ADDR4,
			Board_Address.BD_ADDR3, Board_Address.BD_ADDR2,
			Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

/* The following function displays the pairing capabalities that is  */
/* passed into this function.                                        */
static void DisplayPairingInformation(
		GAP_LE_Pairing_Capabilities_t Pairing_Capabilities) {
	/* Display the IO Capability.                                        */
	switch (Pairing_Capabilities.IO_Capability) {
	case licDisplayOnly:
		Display(("   IO Capability:       lcDisplayOnly.\r\n"));
		break;
	case licDisplayYesNo:
		Display(("   IO Capability:       lcDisplayYesNo.\r\n"));
		break;
	case licKeyboardOnly:
		Display(("   IO Capability:       lcKeyboardOnly.\r\n"));
		break;
	case licNoInputNoOutput:
		Display(("   IO Capability:       lcNoInputNoOutput.\r\n"));
		break;
	case licKeyboardDisplay:
		Display(("   IO Capability:       lcKeyboardDisplay.\r\n"));
		break;
	}

	Display(
			("   MITM:                %s.\r\n", (Pairing_Capabilities.MITM == TRUE)?"TRUE":"FALSE"));
	Display(
			("   Bonding Type:        %s.\r\n", (Pairing_Capabilities.Bonding_Type == lbtBonding)?"Bonding":"No Bonding"));
	Display(
			("   OOB:                 %s.\r\n", (Pairing_Capabilities.OOB_Present == TRUE)?"OOB":"OOB Not Present"));
	Display(
			("   Encryption Key Size: %d.\r\n", Pairing_Capabilities.Maximum_Encryption_Key_Size));
	Display(("   Sending Keys: \r\n"));
	Display(
			("      LTK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Encryption_Key == TRUE)?"YES":"NO")));
	Display(
			("      IRK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Identification_Key == TRUE)?"YES":"NO")));
	Display(
			("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Signing_Key == TRUE)?"YES":"NO")));
	Display(("   Receiving Keys: \r\n"));
	Display(
			("      LTK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Encryption_Key == TRUE)?"YES":"NO")));
	Display(
			("      IRK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Identification_Key == TRUE)?"YES":"NO")));
	Display(
			("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Signing_Key == TRUE)?"YES":"NO")));
}

/* Displays a function error message.                                */
static void DisplayFunctionError(char *Function, int Status) {
	Display(("%s Failed: %d.\r\n", Function, Status));
}

/* Displays a function success message.                              */
static void DisplayFunctionSuccess(char *Function) {
	Display(("%s success.\r\n",Function));
}

/* The following function is responsible for opening a Serial Port   */
/* Server on the Local Device.  This function opens the Serial Port  */
/* Server on the specified RFCOMM Channel.  This function returns    */
/* zero if successful, or a negative return value if an error        */
/* occurred.                                                         */
static int SPPOpenServer(unsigned int BluetoothStackID) {
	int ret_val;
	char *ServiceName;

	/* First check to see if a valid Bluetooth Stack ID exists.          */
	if (BluetoothStackID) {
		/* Simply attempt to open an Serial Server, on RFCOMM Server Port */
		/* 1.                                                             */
		ret_val = SPP_Open_Server_Port(BluetoothStackID, SPP_PORT_NUMBER,
				SPP_Event_Callback, (unsigned long) 0);

		/* If the Open was successful, then note the Serial Port Server   */
		/* ID.                                                            */
		if (ret_val > 0) {
			/* Note the Serial Port Server ID of the opened Serial Port    */
			/* Server.                                                     */
			ApplicationStateInfo.SPPServerPortID = (unsigned int) ret_val;

			/* Create a Buffer to hold the Service Name.                   */
			if ((ServiceName = BTPS_AllocateMemory(64)) != NULL) {
				/* The Server was opened successfully, now register a SDP   */
				/* Record indicating that an Serial Port Server exists.  Do */
				/* this by first creating a Service Name.                   */
				BTPS_SprintF(ServiceName, "Serial Port Server Port %d",
						SPP_PORT_NUMBER);

				/* Now that a Service Name has been created try to Register */
				/* the SDP Record.                                          */
				ret_val = SPP_Register_SDP_Record(BluetoothStackID,
						ApplicationStateInfo.SPPServerPortID, NULL, ServiceName,
						&(ApplicationStateInfo.SPPServerSDPHandle));

				/* If there was an error creating the Serial Port Server's  */
				/* SDP Service Record then go ahead an close down the server*/
				/* an flag an error.                                        */
				if (ret_val < 0) {
					Display(
							("Unable to Register Server SDP Record, Error = %d.\r\n", ret_val));

					SPP_Close_Server_Port(BluetoothStackID,
							ApplicationStateInfo.SPPServerPortID);

					/* Flag that there is no longer an Serial Port Server    */
					/* Open.                                                 */
					ApplicationStateInfo.SPPServerPortID = 0;

					ret_val = FUNCTION_ERROR;
				} else {
					/* Simply flag to the user that everything initialized   */
					/* correctly.                                            */
					Display(("Server Opened: %d.\r\n", SPP_PORT_NUMBER));

					/* Flag success to the caller.                           */
					ret_val = 0;
				}

				/* Free the Service Name buffer.                            */
				BTPS_FreeMemory(ServiceName);
			} else {
				Display(
						("Failed to allocate buffer to hold Service Name in SDP Record.\r\n"));
			}
		} else {
			Display(
					("Unable to Open Server on: %d, Error = %d.\r\n", SPP_PORT_NUMBER, ret_val));

			ret_val = FUNCTION_ERROR;
		}
	} else {
		/* No valid Bluetooth Stack ID exists.                            */
		ret_val = INVALID_STACK_ID_ERROR;
	}

	return (ret_val);
}

/* The following function is responsible for opening the SS1         */
/* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
/* HCI Driver Information structure that contains the HCI Driver     */
/* Transport Information.  This function returns zero on successful  */
/* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation,
		BTPS_Initialization_t *BTPS_Initialization) {
	int Result;
	int ret_val = 0;
	char BluetoothAddress[16];
	Byte_t Status;
	BD_ADDR_t BD_ADDR;
	unsigned int ServiceID;
	HCI_Version_t HCIVersion;
	L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;

	/* Next, makes sure that the Driver Information passed appears to be */
	/* semi-valid.                                                       */
	if (HCI_DriverInformation) {
		Display(("\r\n"));

		/* Initialize BTPSKNRl.                                           */
		BTPS_Init((void *) BTPS_Initialization);

		Display(("OpenStack().\r\n"));

		/* Clear the application state information.                       */
		BTPS_MemInitialize(&ApplicationStateInfo, 0,
				sizeof(ApplicationStateInfo));

		/* Initialize the Stack                                           */
		Result = BSC_Initialize(HCI_DriverInformation, 0);

		/* Next, check the return value of the initialization to see if it*/
		/* was successful.                                                */
		if (Result > 0) {
			/* The Stack was initialized successfully, inform the user and */
			/* set the return value of the initialization function to the  */
			/* Bluetooth Stack ID.                                         */
			ApplicationStateInfo.BluetoothStackID = Result;
			Display(
					("Bluetooth Stack ID: %d.\r\n", ApplicationStateInfo.BluetoothStackID));

			/* Initialize the Default Pairing Parameters.                  */
			LE_Parameters.IOCapability = licNoInputNoOutput;
			LE_Parameters.MITMProtection = FALSE;
			LE_Parameters.OOBDataPresent = FALSE;

			/* Initialize the default Secure Simple Pairing parameters.    */
			IOCapability = icNoInputNoOutput;
			OOBSupport = FALSE;
			MITMProtection = FALSE;

			if (!HCI_Version_Supported(ApplicationStateInfo.BluetoothStackID,
					&HCIVersion))
				Display(
						("Device Chipset: %s.\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]));

			/* Let's output the Bluetooth Device Address so that the user  */
			/* knows what the Device Address is.                           */
			if (!GAP_Query_Local_BD_ADDR(ApplicationStateInfo.BluetoothStackID,
					&BD_ADDR)) {
				BD_ADDRToStr(BD_ADDR, BluetoothAddress);

				Display(("BD_ADDR: %s\r\n", BluetoothAddress));
			}

			/* Go ahead and allow Master/Slave Role Switch.                */
			L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config =
					cqAllowRoleSwitch;
			L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config =
					csMaintainCurrentRole;

			L2CA_Set_Link_Connection_Configuration(
					ApplicationStateInfo.BluetoothStackID,
					&L2CA_Link_Connect_Params);

			if (HCI_Command_Supported(ApplicationStateInfo.BluetoothStackID,
					HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER)
					> 0)
				HCI_Write_Default_Link_Policy_Settings(
						ApplicationStateInfo.BluetoothStackID,
						(HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH
								| HCI_LINK_POLICY_SETTINGS_ENABLE_SNIFF_MODE),
						&Status);

			/* Write the Inquiry/Page Scan Modes that we will be using     */
			/* (Interlaced scanning).                                      */
			HCI_Write_Inquiry_Scan_Type(ApplicationStateInfo.BluetoothStackID,
					HCI_INQUIRY_SCAN_TYPE_OPTIONAL_INTERLACED_SCAN, &Status);
			HCI_Write_Page_Scan_Type(ApplicationStateInfo.BluetoothStackID,
					HCI_PAGE_SCAN_TYPE_OPTIONAL_INTERLACED_SCAN, &Status);

			/* Delete all Stored Link Keys.                                */
			ASSIGN_BD_ADDR(BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

			DeleteLinkKey(ApplicationStateInfo.BluetoothStackID, BD_ADDR);

			/* Regenerate IRK and DHK from the constant Identity Root Key. */
			GAP_LE_Diversify_Function(ApplicationStateInfo.BluetoothStackID,
					(Encryption_Key_t *) (&IR), 1, 0, &IRK);
			GAP_LE_Diversify_Function(ApplicationStateInfo.BluetoothStackID,
					(Encryption_Key_t *) (&IR), 3, 0, &DHK);

			/* Flag that we have no Key Information in the Key List.       */
			DeviceInfoList = NULL;

			/* Attempt to open an SPP Server.                              */
			if (!(Result = SPPOpenServer(ApplicationStateInfo.BluetoothStackID))) {
				/* Initialize the GATT Service.                             */
				if (!(Result = GATT_Initialize(
						ApplicationStateInfo.BluetoothStackID,
						GATT_INITIALIZATION_FLAGS_SUPPORT_LE,
						GATT_Connection_Event_Callback, 0))) {
					/* Initialize the GAPS Service.                          */
					Result = GAPS_Initialize_Service(
							ApplicationStateInfo.BluetoothStackID, &ServiceID);
					if (Result > 0) {
						/* Save the Instance ID of the GAP Service.           */
						ApplicationStateInfo.GAPSInstanceID =
								(unsigned int) Result;

						/* Set the GAP Device Name and Device Appearance.     */
						GAPS_Set_Device_Name(
								ApplicationStateInfo.BluetoothStackID,
								ApplicationStateInfo.GAPSInstanceID,
								LE_DEMO_DEVICE_NAME);
						GAPS_Set_Device_Appearance(
								ApplicationStateInfo.BluetoothStackID,
								ApplicationStateInfo.GAPSInstanceID,
								GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER);

						/* Save the callback handle.                    */
						ApplicationStateInfo.HCIEventCallbackHandle =
								(unsigned int) ret_val;

						/* Format the EIR Data.                         */
						FormatEIRData(ApplicationStateInfo.BluetoothStackID);

						/* Format the Advertising Data.                 */
						FormatAdvertisingData(
								ApplicationStateInfo.BluetoothStackID, TRUE);

						/* Return success to the caller.                */
						ret_val = 0;
					} else {
						/* The Stack was NOT initialized successfully, inform */
						/* the user and set the return value of the           */
						/* initialization function to an error.               */
						DisplayFunctionError("GAPS_Initialize_Service", Result);

						ret_val = UNABLE_TO_INITIALIZE_STACK;
					}

					/* Shutdown the stack if an error occurred.              */
					if (ret_val < 0)
						CloseStack();
				} else {
					/* The Stack was NOT initialized successfully, inform the*/
					/* user and set the return value of the initialization   */
					/* function to an error.                                 */
					DisplayFunctionError("GATT_Initialize", Result);

					/* Shutdown the stack.                                   */
					CloseStack();

					ret_val = UNABLE_TO_INITIALIZE_STACK;
				}
			} else {
				/* The Stack was NOT initialized successfully, inform the   */
				/* user and set the return value of the initialization      */
				/* function to an error.                                    */
				DisplayFunctionError("OpenServer", Result);

				/* Shutdown the stack.                                      */
				CloseStack();

				ret_val = UNABLE_TO_INITIALIZE_STACK;
			}
		} else {
			/* The Stack was NOT initialized successfully, inform the user */
			/* and set the return value of the initialization function to  */
			/* an error.                                                   */
			DisplayFunctionError("BSC_Initialize", Result);

			ApplicationStateInfo.BluetoothStackID = 0;

			ret_val = UNABLE_TO_INITIALIZE_STACK;
		}
	} else {
		/* One or more of the necessary parameters are invalid.           */
		ret_val = INVALID_PARAMETERS_ERROR;
	}

	return (ret_val);
}

/* The following function is responsible for closing the SS1         */
/* Bluetooth Protocol Stack.  This function requires that the        */
/* Bluetooth Protocol stack previously have been initialized via the */
/* OpenStack() function.  This function returns zero on successful   */
/* execution and a negative value on all errors.                     */
static int CloseStack(void) {
	int ret_val = 0;

	/* First check to see if the Stack has been opened.                  */
	if (ApplicationStateInfo.BluetoothStackID) {
		/* Cleanup GAP Service Module.                                    */
		if (ApplicationStateInfo.GAPSInstanceID)
			GAPS_Cleanup_Service(ApplicationStateInfo.BluetoothStackID,
					ApplicationStateInfo.GAPSInstanceID);

		if (ApplicationStateInfo.SPPServerPortID) {
			SPP_Un_Register_SDP_Record( ApplicationStateInfo.BluetoothStackID,
					ApplicationStateInfo.SPPServerPortID,
					ApplicationStateInfo.SPPServerSDPHandle);

			SPP_Close_Server_Port(ApplicationStateInfo.BluetoothStackID,
					ApplicationStateInfo.SPPServerPortID);
		}

		if (ApplicationStateInfo.HCIEventCallbackHandle)
			HCI_Un_Register_Callback(ApplicationStateInfo.BluetoothStackID,
					ApplicationStateInfo.HCIEventCallbackHandle);

		/* Cleanup GATT Module.                                           */
		GATT_Cleanup(ApplicationStateInfo.BluetoothStackID);

		/* Simply close the Stack                                         */
		BSC_Shutdown(ApplicationStateInfo.BluetoothStackID);

		/* Free BTPSKRNL allocated memory.                                */
		BTPS_DeInit();

		Display(("Stack Shutdown.\r\n"));

		/* Free the Key List.                                             */
		FreeDeviceInfoList(&DeviceInfoList);

		/* Flag that the Stack is no longer initialized.                  */
		BTPS_MemInitialize(&ApplicationStateInfo, 0,
				sizeof(ApplicationStateInfo));

		/* Flag success to the caller.                                    */
		ret_val = 0;
	} else {
		/* A valid Stack ID does not exist, inform to user.               */
		ret_val = UNABLE_TO_INITIALIZE_STACK;
	}

	return (ret_val);
}

/* The following function is responsible for placing the local       */
/* Bluetooth device into Pairable mode.  Once in this mode the device*/
/* will response to pairing requests from other Bluetooth devices.   */
/* This function returns zero on successful execution and a negative */
/* value on all errors.                                              */
static int SetPairable(void) {
	int Result;
	int ret_val = 0;

	/* First, check that a valid Bluetooth Stack ID exists.              */
	if (ApplicationStateInfo.BluetoothStackID) {
		/* Attempt to set the attached device to be pairable with SSP.    */
		Result = GAP_Set_Pairability_Mode(ApplicationStateInfo.BluetoothStackID,
				pmPairableMode);

		/* Next, check the return value of the GAP Set Pairability mode   */
		/* command for successful execution.                              */
		if (!Result) {
			/* The device has been set to pairable mode, now register an   */
			/* Authentication Callback to handle the Authentication events */
			/* if required.                                                */
			Result = GAP_Register_Remote_Authentication(
					ApplicationStateInfo.BluetoothStackID, GAP_Event_Callback,
					(unsigned long) 0);

			/* Next, check the return value of the GAP Register Remote     */
			/* Authentication command for successful execution.            */
			if (!Result) {
				/* Now Set the LE Pairability.                              */

				/* Attempt to set the attached device to be pairable.       */
				Result = GAP_LE_Set_Pairability_Mode(
						ApplicationStateInfo.BluetoothStackID, lpmPairableMode);

				/* Next, check the return value of the GAP Set Pairability  */
				/* mode command for successful execution.                   */
				if (!Result) {
					/* The device has been set to pairable mode, now register*/
					/* an Authentication Callback to handle the              */
					/* Authentication events if required.                    */
					Result = GAP_LE_Register_Remote_Authentication(
							ApplicationStateInfo.BluetoothStackID,
							GAP_LE_Event_Callback, (unsigned long) 0);

					/* Next, check the return value of the GAP Register      */
					/* Remote Authentication command for successful          */
					/* execution.                                            */
					if (Result) {
						/* An error occurred while trying to execute this     */
						/* function.                                          */
						DisplayFunctionError(
								"GAP_LE_Register_Remote_Authentication",
								Result);

						ret_val = Result;
					}
				} else {
					/* An error occurred while trying to make the device     */
					/* pairable.                                             */
					DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

					ret_val = Result;
				}
			} else {
				/* An error occurred while trying to execute this function. */
				DisplayFunctionError("GAP_Register_Remote_Authentication",
						Result);

				ret_val = Result;
			}
		} else {
			/* An error occurred while trying to make the device pairable. */
			DisplayFunctionError("GAP_Set_Pairability_Mode", Result);

			ret_val = Result;
		}
	} else {
		/* No valid Bluetooth Stack ID exists.                            */
		ret_val = INVALID_STACK_ID_ERROR;
	}

	return (ret_val);
}

/* The following function is a utility function that is provided to  */
/* allow a mechanism of posting into the application mailbox.        */
static void PostApplicationMailbox(Byte_t MessageID) {
	/* Post to the application mailbox.                                  */
	BTPS_AddMailbox(ApplicationStateInfo.Mailbox, (void *) &MessageID);
}

/* The following function provides a mechanism to configure a        */
/* Pairing Capabilities structure with the application's pairing     */
/* parameters.                                                       */
static void ConfigureCapabilities(GAP_LE_Pairing_Capabilities_t *Capabilities) {
	/* Make sure the Capabilities pointer is semi-valid.                 */
	if (Capabilities) {
		/* Configure the Pairing Cabilities structure.                    */
		Capabilities->Bonding_Type = lbtBonding;
		Capabilities->IO_Capability = LE_Parameters.IOCapability;
		Capabilities->MITM = LE_Parameters.MITMProtection;
		Capabilities->OOB_Present = LE_Parameters.OOBDataPresent;

		/* ** NOTE ** This application always requests that we use the    */
		/*            maximum encryption because this feature is not a    */
		/*            very good one, if we set less than the maximum we   */
		/*            will internally in GAP generate a key of the        */
		/*            maximum size (we have to do it this way) and then   */
		/*            we will zero out how ever many of the MSBs          */
		/*            necessary to get the maximum size.  Also as a slave */
		/*            we will have to use Non-Volatile Memory (per device */
		/*            we are paired to) to store the negotiated Key Size. */
		/*            By requesting the maximum (and by not storing the   */
		/*            negotiated key size if less than the maximum) we    */
		/*            allow the slave to power cycle and regenerate the   */
		/*            LTK for each device it is paired to WITHOUT storing */
		/*            any information on the individual devices we are    */
		/*            paired to.                                          */
		Capabilities->Maximum_Encryption_Key_Size =
				GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

		/* This application only demostrates using Long Term Key's (LTK)  */
		/* for encryption of a LE Link, however we could request and send */
		/* all possible keys here if we wanted to.                        */
		Capabilities->Receiving_Keys.Encryption_Key = FALSE;
		Capabilities->Receiving_Keys.Identification_Key = FALSE;
		Capabilities->Receiving_Keys.Signing_Key = FALSE;

		Capabilities->Sending_Keys.Encryption_Key = TRUE;
		Capabilities->Sending_Keys.Identification_Key = FALSE;
		Capabilities->Sending_Keys.Signing_Key = FALSE;
	}
}

/* The following function provides a mechanism of sending a Slave    */
/* Pairing Response to a Master's Pairing Request.                   */
static int SlavePairingRequestResponse(unsigned int BluetoothStackID,
		BD_ADDR_t BD_ADDR) {
	int ret_val;
	BoardStr_t BoardStr;
	GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

	/* Make sure a Bluetooth Stack is open.                              */
	if (BluetoothStackID) {
		BD_ADDRToStr(BD_ADDR, BoardStr);
		Display(("Sending Pairing Response to %s.\r\n", BoardStr));

		/* We must be the slave if we have received a Pairing Request     */
		/* thus we will respond with our capabilities.                    */
		AuthenticationResponseData.GAP_LE_Authentication_Type =
				larPairingCapabilities;
		AuthenticationResponseData.Authentication_Data_Length =
				GAP_LE_PAIRING_CAPABILITIES_SIZE;

		/* Configure the Application Pairing Parameters.                  */
		ConfigureCapabilities(
				&(AuthenticationResponseData.Authentication_Data.Pairing_Capabilities));

		/* Attempt to pair to the remote device.                          */
		ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR,
				&AuthenticationResponseData);

		Display( ("GAP_LE_Authentication_Response returned %d.\r\n", ret_val));
	} else {
		Display(("Stack ID Invalid.\r\n"));

		ret_val = INVALID_STACK_ID_ERROR;
	}

	return (ret_val);
}

/* The following function is provided to allow a mechanism of        */
/* responding to a request for Encryption Information to send to a   */
/* remote device.                                                    */
static int EncryptionInformationRequestResponse(unsigned int BluetoothStackID,
		BD_ADDR_t BD_ADDR, Byte_t KeySize,
		GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information) {
	int ret_val;
	Word_t LocalDiv;

	/* Make sure a Bluetooth Stack is open.                              */
	if (BluetoothStackID) {
		/* Make sure the input parameters are semi-valid.                 */
		if ((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information)){
		Display(("   Calling GAP_LE_Generate_Long_Term_Key.\r\n"));

		/* Generate a new LTK, EDIV and Rand tuple.                    */
		ret_val = GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
		if(!ret_val)
		{
			Display(("   Encryption Information Request Response.\r\n"));

			/* Response to the request with the LTK, EDIV and Rand      */
			/* values.                                                  */
			GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type = larEncryptionInformation;
			GAP_LE_Authentication_Response_Information->Authentication_Data_Length = GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
			GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

			ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
			if(!ret_val)
			{
				Display(("   GAP_LE_Authentication_Response (larEncryptionInformation) success.\r\n", ret_val));
			}
			else
			{
				Display(("   Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val));
			}
		}
		else
		{
			Display(("   Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val));
		}
	}
	else
	{
		Display(("Invalid Parameters.\r\n"));

		ret_val = INVALID_PARAMETERS_ERROR;
	}
}
else
{
	Display(("Stack ID Invalid.\r\n"));

	ret_val = INVALID_STACK_ID_ERROR;
}

	return (ret_val);
}

/* The following function is a utility function that exists to delete*/
/* the specified Link Key from the Local Bluetooth Device.  If a NULL*/
/* Bluetooth Device Address is specified, then all Link Keys will be */
/* deleted.                                                          */
static int DeleteLinkKey(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR) {
	int Result;
	Byte_t Status_Result;
	Word_t Num_Keys_Deleted = 0;
	BD_ADDR_t NULL_BD_ADDR;

	Result = HCI_Delete_Stored_Link_Key(BluetoothStackID, BD_ADDR, TRUE,
			&Status_Result, &Num_Keys_Deleted);

	/* Any stored link keys for the specified address (or all) have been */
	/* deleted from the chip.  Now, let's make sure that our stored Link */
	/* Key Array is in sync with these changes.                          */

	/* First check to see all Link Keys were deleted.                    */
	ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

	if (COMPARE_BD_ADDR(BD_ADDR, NULL_BD_ADDR))
		BTPS_MemInitialize(LinkKeyInfo, 0, sizeof(LinkKeyInfo));
	else {
		/* Individual Link Key.  Go ahead and see if know about the entry */
		/* in the list.                                                   */
		for (Result = 0; (Result < sizeof(LinkKeyInfo) / sizeof(LinkKeyInfo_t));
				Result++) {
			if (COMPARE_BD_ADDR(BD_ADDR, LinkKeyInfo[Result].BD_ADDR)) {
				LinkKeyInfo[Result].BD_ADDR = NULL_BD_ADDR;

				break;
			}
		}
	}

	return (Result);
}

/* The following function is a utility function which exists to      */
/* format the EIR Data that is used by this application.             */
static void FormatEIRData(unsigned int BluetoothStackID) {
	Byte_t *TempPtr;
	Byte_t Status;
	SByte_t TxPower;
	unsigned int Length;
	unsigned int StringLength;
	Extended_Inquiry_Response_Data_t EIRData;

	/* First, check that valid Bluetooth Stack ID exists.                */
	if (BluetoothStackID) {
		/* Begin formatting the EIR Data.                                 */
		Length = 0;
		TempPtr = EIRData.Extended_Inquiry_Response_Data;

		BTPS_MemInitialize(TempPtr, 0, EXTENDED_INQUIRY_RESPONSE_DATA_SIZE);

		/* Format the Inquiry Response Transmit Power Level.              */
		*TempPtr++ = 2;
		*TempPtr++ = HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_TX_POWER_LEVEL;

		if ((HCI_Read_Inquiry_Response_Transmit_Power_Level(BluetoothStackID,
				&Status, &TxPower)) || (Status))
			*TempPtr++ = 0;
		else
			*TempPtr++ = (Byte_t) TxPower;

		Length += 3;

		/* Format the Service Class List.                                 */
		*TempPtr++ = 1 + (UUID_16_SIZE<<2);
		*TempPtr++ =
				HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_COMPLETE;

		/* Assign the SDP Server UUID.                                    */
		SDP_ASSIGN_SDP_SERVER_UUID_16(*((UUID_16_t *)TempPtr));

		/* Swap the Endian-ness (above macro assigns in big endian        */
		/* format).                                                       */
		Status = TempPtr[1];
		TempPtr[1] = TempPtr[0];
		TempPtr[0] = Status;

		TempPtr += UUID_16_SIZE;

		/* Assign the SPP UUID.                                           */
		SDP_ASSIGN_SERIAL_PORT_PROFILE_UUID_16(*((UUID_16_t *)TempPtr));

		/* Swap the Endian-ness (above macro assigns in big endian        */
		/* format).                                                       */
		Status = TempPtr[1];
		TempPtr[1] = TempPtr[0];
		TempPtr[0] = Status;

		TempPtr += UUID_16_SIZE;

		/* Assign the Key Fob UUID.                                       */
		//TI_KFS_ASSIGN_KFS_SERVICE_UUID_16(*((UUID_16_t *)TempPtr)); FIXME
		TempPtr += UUID_16_SIZE;

		/* Assign the Accelerometer UUID.                                 */
		//TI_ACC_ASSIGN_ACC_SERVICE_UUID_16(*((UUID_16_t *) TempPtr)); FIXME
		TempPtr += UUID_16_SIZE;

		Length += 2 + (UUID_16_SIZE<<2);

		/* Format the local device name.                                  */
		StringLength = BTPS_StringLength(CB_DEMO_DEVICE_NAME);
		if (StringLength
				<= (EXTENDED_INQUIRY_RESPONSE_DATA_MAXIMUM_SIZE - Length - 2)) {
			*TempPtr++ = StringLength + 1;
			*TempPtr++ =
					HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_COMPLETE;
		} else {
			StringLength = 64;
			*TempPtr++ = StringLength + 1;
			*TempPtr++ =
					HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_LOCAL_NAME_SHORTENED;
		}

		BTPS_MemCopy(TempPtr, CB_DEMO_DEVICE_NAME, StringLength);

		TempPtr += StringLength;
		Length += StringLength + 2;

		/* Write the Extended Inquiry Response Data.                      */
		if (!GAP_Write_Extended_Inquiry_Information(BluetoothStackID,
				HCI_EXTENDED_INQUIRY_RESPONSE_FEC_REQUIRED, &EIRData))
			Display(("EIR Data Configured Successfully.\r\n"));
	}
}

/* The following function is a utility function that formats the     */
/* advertising data.                                                 */
static void FormatAdvertisingData(unsigned int BluetoothStackID,
		Boolean_t SupportBR_EDR) {
	int Result;
	unsigned int Length;
	unsigned int StringLength;
	union {
		Advertising_Data_t AdvertisingData;
		Scan_Response_Data_t ScanResponseData;
	} Advertisement_Data_Buffer;

	/* First, check that valid Bluetooth Stack ID exists.                */
	if (BluetoothStackID) {
		BTPS_MemInitialize(&(Advertisement_Data_Buffer.AdvertisingData), 0,
				sizeof(Advertising_Data_t));

		Length = 0;

		/* Set the Flags A/D Field (1 byte type and 1 byte Flags.         */
		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] = 2;
		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[1] =
				HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] =
				HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;

		/* Determine if we are going to say that we support BR/EDR.       */
		if (!SupportBR_EDR)
			Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] |=
					HCI_LE_ADVERTISING_FLAGS_BR_EDR_NOT_SUPPORTED_FLAGS_BIT_MASK;

		Length += Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0]
				+ 1;

		/* Configure the services that we say we support.                 */
		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] = 1
				+ (UUID_16_SIZE<<1);
		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length + 1] =
				HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;

		Length +=
				Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length]
						+ 1;

		/* Set the Device Name String.                                    */
		StringLength = BTPS_StringLength(LE_DEMO_DEVICE_NAME);
		if (StringLength < (ADVERTISING_DATA_MAXIMUM_SIZE - Length - 2))
			Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length
					+ 1] =
					HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
		else {
			Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length
					+ 1] =
					HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
			StringLength = (ADVERTISING_DATA_MAXIMUM_SIZE - Length - 2);
		}

		Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] =
				StringLength + 1;

		BTPS_MemCopy(
				&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length
						+ 2]), LE_DEMO_DEVICE_NAME, StringLength);

		Length +=
				Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length]
						+ 1;

		/* Write thee advertising data to the chip.                       */
		Result = GAP_LE_Set_Advertising_Data(BluetoothStackID, Length,
				&(Advertisement_Data_Buffer.AdvertisingData));
		if (!Result)
			Display(("Advertising Data Configured Successfully.\r\n"));
		else
			Display(
					("GAP_LE_Set_Advertising_Data(dtAdvertising) returned %d.\r\n", Result));
	}
}

/* The following function is a utility function that starts an       */
/* advertising process.                                              */
static int StartAdvertising(unsigned int BluetoothStackID) {
	int ret_val;
	GAP_LE_Advertising_Parameters_t AdvertisingParameters;
	GAP_LE_Connectability_Parameters_t ConnectabilityParameters;

	/* First, check that valid Bluetooth Stack ID exists.                */
	if (BluetoothStackID) {
		/* Set up the advertising parameters.                             */
		AdvertisingParameters.Advertising_Channel_Map =
				HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
		AdvertisingParameters.Scan_Request_Filter = fpNoFilter;
		AdvertisingParameters.Connect_Request_Filter = fpNoFilter;
		AdvertisingParameters.Advertising_Interval_Min = 50;
		AdvertisingParameters.Advertising_Interval_Max = 100;

		/* Configure the Connectability Parameters.                       */
		/* * NOTE * Since we do not ever put ourselves to be direct       */
		/*          connectable then we will set the DirectAddress to all */
		/*          0s.                                                   */
		ConnectabilityParameters.Connectability_Mode = lcmConnectable;
		ConnectabilityParameters.Own_Address_Type = latPublic;
		ConnectabilityParameters.Direct_Address_Type = latPublic;
		ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0,
				0);

		/* Now enable advertising.                                        */
		ret_val = GAP_LE_Advertising_Enable(BluetoothStackID, TRUE,
				&AdvertisingParameters, &ConnectabilityParameters,
				GAP_LE_Event_Callback, 0);
		if (!ret_val)
			Display(("GAP_LE_Advertising_Enable success.\r\n"));
		else {
			Display( ("GAP_LE_Advertising_Enable returned %d.\r\n", ret_val));

			ret_val = FUNCTION_ERROR;
		}
	} else {
		/* No valid Bluetooth Stack ID exists.                            */
		ret_val = INVALID_STACK_ID_ERROR;
	}

	return (ret_val);
}

/* The following function is responsible for checking the idle state */
/* and possibly entering LPM3 mode.                                  */
static void IdleFunction(unsigned int BluetoothStackID) {
	HCILL_State_t HCILL_State;

	/* Determine the HCILL State.                                        */
	HCILL_State = HCILL_GetState();

	/* If the stack is Idle and we are in HCILL Sleep, then we may enter */
	/* LPM3 mode (with Timer Interrupts disabled).                       */
	if ((BSC_QueryStackIdle(BluetoothStackID)) && (HCILL_State == hsSleep)
			&& (!HCILL_Get_Power_Lock_Count())) {
		/* Enter MSP430 LPM3.                                             */
		LPM3;
	} else {
		/* Enter Low Power Mode 0 if no HCI UART data is ready to be      */
		/* process.                                                       */
		if (!HCITR_RxBytesReady(0))
			LPM0;
	}
}

/* The following function is a utility function that is used to      */
/* format a SPP Data Packet.  This function returns the number of    */
/* bytes formatted into PacketBuffer.                                */
static unsigned int FormatSPPDataPacket(unsigned int PacketBufferLength,
		Byte_t *PacketBuffer) {
	static int i = 0;

	BTPS_SprintF((char *) PacketBuffer, "hello #%i\n", i++);

	return BTPS_StringLength((char *) PacketBuffer);
}

/* The following function is a utility function which is used to     */
/* packetize and send SPP Data to a connected device.                */
static void ProcessSendSPPData(Boolean_t PacketizeCurrentData) {
	int Result;

	/* Only continue if we are current connected to a BR/EDR Device AND  */
	/* the SPP Buffer is not Empty.                                      */
	if ((ApplicationStateInfo.Flags
			& (APPLICATION_STATE_INFO_FLAGS_CB_CONNECTED
					| APPLICATION_STATE_INFO_FLAGS_SPP_BUFFER_FULL))
			== APPLICATION_STATE_INFO_FLAGS_CB_CONNECTED) {
		/* If requested packetize the current Button and Accelerometer    */
		/* Data.                                                          */
		if (PacketizeCurrentData) {
			/* Packetize the current data to the end of the SPP Buffer and */
			/* increment the length of the SPP Buffer.                     */
			ApplicationStateInfo.SPPBufferLength +=
					FormatSPPDataPacket(
							SPP_BUFFER_SIZE
									- ApplicationStateInfo.SPPBufferLength,
							&(ApplicationStateInfo.SPPBuffer[ApplicationStateInfo.SPPBufferLength]));
		}

		/* Send the packetized SPP data to the remote device.             */
		Result = SPP_Data_Write(ApplicationStateInfo.BluetoothStackID,
				ApplicationStateInfo.SPPServerPortID,
				ApplicationStateInfo.SPPBufferLength,
				(unsigned char *) ApplicationStateInfo.SPPBuffer);
		//Result = SPP_Data_Write(ApplicationStateInfo.BluetoothStackID, ApplicationStateInfo.SPPServerPortID, 6, "hello!");

		if (Result > 0) {
			/* If we wrote less than the requested number of bytes move the*/
			/* data forward in the buffer and flag that the SPP Buffer is  */
			/* FULL.                                                       */
			if ((unsigned int) Result < ApplicationStateInfo.SPPBufferLength) {
				BTPS_MemMove(ApplicationStateInfo.SPPBuffer,
						&(ApplicationStateInfo.SPPBuffer[(unsigned int) Result]),
						(ApplicationStateInfo.SPPBufferLength - Result));

				ApplicationStateInfo.Flags |=
						APPLICATION_STATE_INFO_FLAGS_SPP_BUFFER_FULL;
			}

			ApplicationStateInfo.SPPBufferLength -= (unsigned int) Result;
		} else
			Display(("Error - SPP_Data_Write returned %d.\r\n", Result));
	}
}

/* ***************************************************************** */
/*                         Event Callbacks                           */
/* ***************************************************************** */

/* The following function is for the GAP LE Event Receive Data       */
/* Callback.  This function will be called whenever a Callback has   */
/* been registered for the specified GAP LE Action that is associated*/
/* with the Bluetooth Stack.  This function passes to the caller the */
/* GAP LE Event Data of the specified Event and the GAP LE Event     */
/* Callback Parameter that was specified when this Callback was      */
/* installed.  The caller is free to use the contents of the GAP LE  */
/* Event Data ONLY in the context of this callback.  If the caller   */
/* requires the Data for a longer period of time, then the callback  */
/* function MUST copy the data into another Data Buffer.  This       */
/* function is guaranteed NOT to be invoked more than once           */
/* simultaneously for the specified installed callback (i.e.  this   */
/* function DOES NOT have be reentrant).  It Needs to be noted       */
/* however, that if the same Callback is installed more than once,   */
/* then the callbacks will be called serially.  Because of this, the */
/* processing in this function should be as efficient as possible.   */
/* It should also be noted that this function is called in the Thread*/
/* Context of a Thread that the User does NOT own.  Therefore,       */
/* processing in this function should be as efficient as possible    */
/* (this argument holds anyway because other GAP Events will not be  */
/* processed while this function call is outstanding).               */
/* * NOTE * This function MUST NOT Block and wait for Events that can*/
/*          only be satisfied by Receiving a Bluetooth Event         */
/*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
/*          Callbacks will be issued while this function is currently*/
/*          outstanding.                                             */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID,
		GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter) {
	int Result;
	BoardStr_t BoardStr;
	DeviceInfo_t *DeviceInfo;
	Long_Term_Key_t GeneratedLTK;
	GAP_Encryption_Mode_t GAP_Encryption_Mode;
	GAP_LE_Authentication_Event_Data_t *Authentication_Event_Data;
	GAP_LE_Authentication_Response_Information_t GAP_LE_Authentication_Response_Information;

	/* Verify that all parameters to this callback are Semi-Valid.       */
	if ((BluetoothStackID) && (GAP_LE_Event_Data)) {
		switch (GAP_LE_Event_Data->Event_Data_Type) {
		case etLE_Connection_Complete:
			Display(
					("etLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));

			if (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data) {
				BD_ADDRToStr(
						GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address,
						BoardStr);

				Display(
						("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status));
				Display(
						("   Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave"));
				Display(
						("   Address Type: %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic)?"Public":"Random"));
				Display(("   BD_ADDR:      %s.\r\n", BoardStr));

				if (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status
						== HCI_ERROR_CODE_NO_ERROR) {
					/* Make sure that no entry already exists.            */
					if ((DeviceInfo =
							SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,
									GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address))
							== NULL) {
						/* No entry exists so create one.                  */
						if (!CreateNewDeviceInfoEntry(&DeviceInfoList,
								GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type,
								GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address))
							Display(
									("Failed to add device to Device Info List.\r\n"));
					}
				}
			}
			break;
		case etLE_Disconnection_Complete:
			Display(
					("etLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

			if (GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data) {
				Display(
						("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status));
				Display(
						("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason));

				BD_ADDRToStr(
						GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address,
						BoardStr);
				Display(("   BD_ADDR: %s.\r\n", BoardStr));

				/* Check to see if the device info is present in the     */
				/* list.                                                 */
				if ((DeviceInfo =
						SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,
								GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address))
						!= NULL) {
					/* Check to see if the link is encrypted.  If it isn't*/
					/* we will delete the device structure.               */
					Result =
							GAP_LE_Query_Encryption_Mode(BluetoothStackID,
									GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address,
									&GAP_Encryption_Mode);
					if ((Result) || (GAP_Encryption_Mode == emDisabled)) {
						/* Connection is not encrypted so delete the device*/
						/* structure.                                      */
						DeviceInfo =
								DeleteDeviceInfoEntry(&DeviceInfoList,
										GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address);
						if (DeviceInfo)
							FreeDeviceInfoEntryMemory(DeviceInfo);
					}
				}

				/* Inform the event handler of the LE Disconnection.     */
				PostApplicationMailbox(
						APPLICATION_MAILBOX_MESSAGE_ID_LE_DISCONNECTED);
			}
			break;
		case etLE_Encryption_Change:
			Display(
					("etLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));
			break;
		case etLE_Encryption_Refresh_Complete:
			Display(
					("etLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));
			break;
		case etLE_Authentication:
			Display(
					("etLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

			/* Make sure the authentication event data is valid before  */
			/* continueing.                                             */
			if ((Authentication_Event_Data =
					GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data)
					!= NULL) {
				BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

				switch (Authentication_Event_Data->GAP_LE_Authentication_Event_Type) {
				case latLongTermKeyRequest:
					Display(("    latKeyRequest: \r\n"));
					Display(("      BD_ADDR: %s.\r\n", BoardStr));

					/* The other side of a connection is requesting    */
					/* that we start encryption. Thus we should        */
					/* regenerate LTK for this connection and send it  */
					/* to the chip.                                    */
					Result =
							GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID,
									(Encryption_Key_t *) (&DHK),
									(Encryption_Key_t *) (&ER),
									Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV,
									&(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand),
									&GeneratedLTK);
					if (!Result) {
						Display(
								("      GAP_LE_Regenerate_Long_Term_Key Success.\r\n"));

						/* Respond with the Re-Generated Long Term Key. */
						GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type =
								larLongTermKey;
						GAP_LE_Authentication_Response_Information.Authentication_Data_Length =
								GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
						GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size =
								GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
						GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key =
								GeneratedLTK;
					} else {
						Display(
								("      GAP_LE_Regenerate_Long_Term_Key returned %d.\r\n",Result));

						/* Since we failed to generate the requested key*/
						/* we should respond with a negative response.  */
						GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type =
								larLongTermKey;
						GAP_LE_Authentication_Response_Information.Authentication_Data_Length =
								0;
					}

					/* Send the Authentication Response.               */
					Result = GAP_LE_Authentication_Response(BluetoothStackID,
							Authentication_Event_Data->BD_ADDR,
							&GAP_LE_Authentication_Response_Information);
					if (Result) {
						Display(
								("      GAP_LE_Authentication_Response returned %d.\r\n",Result));
					}
					break;
				case latPairingRequest:
					Display(("Pairing Request: %s.\r\n",BoardStr));
					DisplayPairingInformation(
							Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);

					/* This is a pairing request. Respond with a       */
					/* Pairing Response.                               */
					/* * NOTE * This is only sent from Master to Slave.*/
					/*          Thus we must be the Slave in this      */
					/*          connection.                            */

					/* Send the Pairing Response.                      */
					SlavePairingRequestResponse(BluetoothStackID,
							Authentication_Event_Data->BD_ADDR);
					break;
				case latConfirmationRequest:
					Display(("latConfirmationRequest.\r\n"));

					if (Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type
							== crtNone) {
						Display(("Invoking Just Works.\r\n"));

						/* Just Accept Just Works Pairing.              */
						GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type =
								larConfirmation;

						/* By setting the Authentication_Data_Length to */
						/* any NON-ZERO value we are informing the GAP  */
						/* LE Layer that we are accepting Just Works    */
						/* Pairing.                                     */
						GAP_LE_Authentication_Response_Information.Authentication_Data_Length =
								DWORD_SIZE;

						Result = GAP_LE_Authentication_Response(
								BluetoothStackID,
								Authentication_Event_Data->BD_ADDR,
								&GAP_LE_Authentication_Response_Information);
						if (Result) {
							Display(
									("GAP_LE_Authentication_Response returned %d.\r\n",Result));
						}
					} else {
						if (Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type
								== crtPasskey) {
							Display(
									("Call LEPasskeyResponse [PASSCODE].\r\n"));
						} else {
							if (Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type
									== crtDisplay) {
								Display(
										("Passkey: %06l.\r\n", Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
							}
						}
					}
					break;
				case latSecurityEstablishmentComplete:
					Display(
							("Security Re-Establishment Complete: %s.\r\n", BoardStr));
					Display(
							("                            Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status));
					break;
				case latPairingStatus:
					Display(("Pairing Status: %s.\r\n", BoardStr));
					Display(
							("        Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status));

					if (Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status
							== GAP_LE_PAIRING_STATUS_NO_ERROR) {
						Display(
								("        Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size));
					} else {
						/* Failed to pair so delete the key entry for   */
						/* this device and disconnect the link.         */
						if ((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList,
								Authentication_Event_Data->BD_ADDR)) != NULL)
							FreeDeviceInfoEntryMemory(DeviceInfo);

						/* Disconnect the Link.                         */
						GAP_LE_Disconnect(BluetoothStackID,
								Authentication_Event_Data->BD_ADDR);
					}
					break;
				case latEncryptionInformationRequest:
					Display(
							("Encryption Information Request %s.\r\n", BoardStr));

					/* Generate new LTK,EDIV and Rand and respond with */
					/* them.                                           */
					EncryptionInformationRequestResponse(BluetoothStackID,
							Authentication_Event_Data->BD_ADDR,
							Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size,
							&GAP_LE_Authentication_Response_Information);
					break;
				case latEncryptionInformation:
					/* Display the information from the event.         */
					Display(
							(" Encryption Information from RemoteDevice: %s.\r\n", BoardStr));
					Display(
							("                             Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size));
					break;
				}
			}
			break;
		}
	}
}

/* The following function is for an GATT Connection Event Callback.  */
/* This function is called for GATT Connection Events that occur on  */
/* the specified Bluetooth Stack.  This function passes to the caller*/
/* the GATT Connection Event Data that occurred and the GATT         */
/* Connection Event Callback Parameter that was specified when this  */
/* Callback was installed.  The caller is free to use the contents of*/
/* the GATT Client Event Data ONLY in the context of this callback.  */
/* If the caller requires the Data for a longer period of time, then */
/* the callback function MUST copy the data into another Data Buffer.*/
/* This function is guaranteed NOT to be invoked more than once      */
/* simultaneously for the specified installed callback (i.e.  this   */
/* function DOES NOT have be reentrant).  It Needs to be noted       */
/* however, that if the same Callback is installed more than once,   */
/* then the callbacks will be called serially.  Because of this, the */
/* processing in this function should be as efficient as possible.   */
/* It should also be noted that this function is called in the Thread*/
/* Context of a Thread that the User does NOT own.  Therefore,       */
/* processing in this function should be as efficient as possible    */
/* (this argument holds anyway because another GATT Event            */
/* (Server/Client or Connection) will not be processed while this    */
/* function call is outstanding).                                    */
/* * NOTE * This function MUST NOT Block and wait for Events that can*/
/*          only be satisfied by Receiving a Bluetooth Event         */
/*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
/*          Callbacks will be issued while this function is currently*/
/*          outstanding.                                             */
static void BTPSAPI GATT_Connection_Event_Callback(
		unsigned int BluetoothStackID,
		GATT_Connection_Event_Data_t *GATT_Connection_Event_Data,
		unsigned long CallbackParameter) {
	BoardStr_t BoardStr;

	/* Verify that all parameters to this callback are Semi-Valid.       */
	if ((BluetoothStackID) && (GATT_Connection_Event_Data)) {
		/* Determine the Connection Event that occurred.                  */
		switch (GATT_Connection_Event_Data->Event_Data_Type) {
		case etGATT_Connection_Device_Connection:
			if (GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data) {
				Display(
						("\r\netGATT_Connection_Device_Connection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
				BD_ADDRToStr(
						GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice,
						BoardStr);
				Display(
						("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID));
				Display(
						("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
				Display(("   Remote Device:   %s.\r\n", BoardStr));
				Display(
						("   Connection MTU:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU));

				/* Store the Connection Information for this connection. */
				ApplicationStateInfo.LEConnectionInfo.ConnectionIndex =
						GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
				ApplicationStateInfo.LEConnectionInfo.BD_ADDR =
						GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;

				/* Notify the event handler of the connection.           */
				PostApplicationMailbox(
						APPLICATION_MAILBOX_MESSAGE_ID_LE_CONNECTED);
			} else
				Display(("Error - Null Connection Data.\r\n"));
			break;
		}
	} else {
		/* There was an error with one or more of the input parameters.   */
		Display(("\r\n"));

		Display(("GATT Connection Callback Data: Event_Data = NULL.\r\n"));
	}
}

/* The following function is for the GAP Event Receive Data Callback.*/
/* This function will be called whenever a Callback has been         */
/* registered for the specified GAP Action that is associated with   */
/* the Bluetooth Stack.  This function passes to the caller the GAP  */
/* Event Data of the specified Event and the GAP Event Callback      */
/* Parameter that was specified when this Callback was installed.    */
/* The caller is free to use the contents of the GAP Event Data ONLY */
/* in the context of this callback.  If the caller requires the Data */
/* for a longer period of time, then the callback function MUST copy */
/* the data into another Data Buffer.  This function is guaranteed   */
/* NOT to be invoked more than once simultaneously for the specified */
/* installed callback (i.e.  this function DOES NOT have be          */
/* reentrant).  It Needs to be noted however, that if the same       */
/* Callback is installed more than once, then the callbacks will be  */
/* called serially.  Because of this, the processing in this function*/
/* should be as efficient as possible.  It should also be noted that */
/* this function is called in the Thread Context of a Thread that the*/
/* User does NOT own.  Therefore, processing in this function should */
/* be as efficient as possible (this argument holds anyway because   */
/* other GAP Events will not be processed while this function call is*/
/* outstanding).                                                     */
/* * NOTE * This function MUST NOT Block and wait for events that    */
/*          can only be satisfied by Receiving other GAP Events.  A  */
/*          Deadlock WILL occur because NO GAP Event Callbacks will  */
/*          be issued while this function is currently outstanding.  */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID,
		GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter) {
	int Result;
	int Index;
	BD_ADDR_t NULL_BD_ADDR;
	Boolean_t OOB_Data;
	Boolean_t MITM;
	PIN_Code_t PINCode;
	BoardStr_t Callback_BoardStr;
	GAP_IO_Capability_t RemoteIOCapability;
	GAP_Remote_Name_Event_Data_t *GAP_Remote_Name_Event_Data;
	GAP_Authentication_Information_t GAP_Authentication_Information;

	/* First, check to see if the required parameters appear to be       */
	/* semi-valid.                                                       */
	if ((BluetoothStackID) && (GAP_Event_Data)) {
		/* The parameters appear to be semi-valid, now check to see what  */
		/* type the incoming event is.                                    */
		switch (GAP_Event_Data->Event_Data_Type) {
		case etAuthentication:
			/* An authentication event occurred, determine which type of*/
			/* authentication event occurred.                           */
			switch (GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->GAP_Authentication_Event_Type) {
			case atLinkKeyRequest:
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display(("atLinkKeyRequest: %s\r\n", Callback_BoardStr));

				/* Setup the authentication information response      */
				/* structure.                                         */
				GAP_Authentication_Information.GAP_Authentication_Type =
						atLinkKey;
				GAP_Authentication_Information.Authentication_Data_Length = 0;

				/* See if we have stored a Link Key for the specified */
				/* device.                                            */
				for (Index = 0;
						Index < (sizeof(LinkKeyInfo) / sizeof(LinkKeyInfo_t));
						Index++) {
					if (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device)) {
						/* Link Key information stored, go ahead and    */
						/* respond with the stored Link Key.            */
						GAP_Authentication_Information.Authentication_Data_Length =
								sizeof(Link_Key_t);
						GAP_Authentication_Information.Authentication_Data.Link_Key =
								LinkKeyInfo[Index].LinkKey;

						break;
					}
				}

				/* Submit the authentication response.                */
				Result =
						GAP_Authentication_Response(BluetoothStackID,
								GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
								&GAP_Authentication_Information);

				/* Check the result of the submitted command.         */
				if (!Result)
					DisplayFunctionSuccess("GAP_Authentication_Response");
				else
					DisplayFunctionError("GAP_Authentication_Response", Result);
				break;
			case atPINCodeRequest:
				/* A pin code request event occurred, first display   */
				/* the BD_ADD of the remote device requesting the pin.*/
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display(("atPINCodeRequest: %s\r\n", Callback_BoardStr));

				/* Initialize the PIN code.                           */
				ASSIGN_PIN_CODE(PINCode, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0);

				BTPS_MemCopy(&PINCode, CLASSIC_PIN_CODE,
						BTPS_StringLength(CLASSIC_PIN_CODE));

				/* Populate the response structure.                   */
				GAP_Authentication_Information.GAP_Authentication_Type =
						atPINCode;
				GAP_Authentication_Information.Authentication_Data_Length =
						(Byte_t) (BTPS_StringLength(CLASSIC_PIN_CODE));
				GAP_Authentication_Information.Authentication_Data.PIN_Code =
						PINCode;

				/* Submit the Authentication Response.                */
				Result =
						GAP_Authentication_Response(BluetoothStackID,
								GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
								&GAP_Authentication_Information);

				/* Check the return value for the submitted command   */
				/* for success.                                       */
				if (!Result) {
					/* Operation was successful, inform the user.      */
					Display(
							("GAP_Authentication_Response(), Pin Code Response Success.\r\n"));
				} else {
					/* Inform the user that the Authentication Response*/
					/* was not successful.                             */
					Display(
							("GAP_Authentication_Response() Failure: %d.\r\n", Result));
				}

				/* Inform the user that they will need to respond with*/
				/* a PIN Code Response.                               */
				Display(("Respond with: PINCodeResponse\r\n"));
				break;
			case atAuthenticationStatus:
				/* An authentication status event occurred, display   */
				/* all relevant information.                          */
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display(
						("atAuthenticationStatus: %d for %s\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status, Callback_BoardStr));

				if (GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status) {
					/* If we failed to authenticate the device delete  */
					/* any stored link key.                            */
					DeleteLinkKey(BluetoothStackID,
							GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device);
				}
				break;
			case atLinkKeyCreation:
				/* A link key creation event occurred, first display  */
				/* the remote device that caused this event.          */
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display(("atLinkKeyCreation: %s\r\n", Callback_BoardStr));

				/* Now store the link Key in either a free location OR*/
				/* over the old key location.                         */
				ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00);

				for (Index = 0, Result = -1;
						Index < (sizeof(LinkKeyInfo) / sizeof(LinkKeyInfo_t));
						Index++) {
					if (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
						break;
					else {
						if ((Result == (-1))
								&& (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, NULL_BD_ADDR)))
							Result = Index;
					}
				}

				/* If we didn't find a match, see if we found an empty*/
				/* location.                                          */
				if (Index == (sizeof(LinkKeyInfo) / sizeof(LinkKeyInfo_t)))
					Index = Result;

				/* Check to see if we found a location to store the   */
				/* Link Key information into.                         */
				if (Index != (-1)) {
					LinkKeyInfo[Index].BD_ADDR =
							GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;
					LinkKeyInfo[Index].LinkKey =
							GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

					Display(("Link Key Stored.\r\n"));
				} else
					Display(("Link Key array full.\r\n"));
				break;
			case atIOCapabilityRequest:
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display( ("atIOCapabilityRequest: %s\r\n", Callback_BoardStr));

				/* Setup the Authentication Information Response      */
				/* structure.                                         */
				GAP_Authentication_Information.GAP_Authentication_Type =
						atIOCapabilities;
				GAP_Authentication_Information.Authentication_Data_Length =
						sizeof(GAP_IO_Capabilities_t);
				GAP_Authentication_Information.Authentication_Data.IO_Capabilities.IO_Capability =
						(GAP_IO_Capability_t) IOCapability;
				GAP_Authentication_Information.Authentication_Data.IO_Capabilities.MITM_Protection_Required =
						MITMProtection;
				GAP_Authentication_Information.Authentication_Data.IO_Capabilities.OOB_Data_Present =
						OOBSupport;

				/* Submit the Authentication Response.                */
				Result =
						GAP_Authentication_Response(BluetoothStackID,
								GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
								&GAP_Authentication_Information);

				/* Check the result of the submitted command.         */
				/* Check the result of the submitted command.         */
				if (!Result)
					DisplayFunctionSuccess("Auth");
				else
					DisplayFunctionError("Auth", Result);
				break;
			case atIOCapabilityResponse:
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display( ("atIOCapabilityResponse: %s\r\n", Callback_BoardStr));

				RemoteIOCapability =
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.IO_Capability;
				MITM =
						(Boolean_t) GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.MITM_Protection_Required;
				OOB_Data =
						(Boolean_t) GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.OOB_Data_Present;

				Display(
						("Capabilities: %s%s%s\r\n", IOCapabilitiesStrings[RemoteIOCapability], ((MITM)?", MITM":""), ((OOB_Data)?", OOB Data":"")));
				break;
			case atUserConfirmationRequest:
				BD_ADDRToStr(
						GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
						Callback_BoardStr);
				Display(("\r\n"));
				Display(
						("atUserConfirmationRequest: %s\r\n", Callback_BoardStr));

				/* Invoke JUST Works Process...                       */
				GAP_Authentication_Information.GAP_Authentication_Type =
						atUserConfirmation;
				GAP_Authentication_Information.Authentication_Data_Length =
						(Byte_t) sizeof(Byte_t);
				GAP_Authentication_Information.Authentication_Data.Confirmation =
						TRUE;

				/* Submit the Authentication Response.                */
				Display(
						("\r\nAuto Accepting: %l\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));

				Result =
						GAP_Authentication_Response(BluetoothStackID,
								GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device,
								&GAP_Authentication_Information);

				if (!Result)
					DisplayFunctionSuccess("GAP_Authentication_Response");
				else
					DisplayFunctionError("GAP_Authentication_Response", Result);
				break;
			default:
				Display(("Un-handled Auth. Event.\r\n"));
				break;
			}
			break;
		case etRemote_Name_Result:
			/* Bluetooth Stack has responded to a previously issued     */
			/* Remote Name Request that was issued.                     */
			GAP_Remote_Name_Event_Data =
					GAP_Event_Data->Event_Data.GAP_Remote_Name_Event_Data;
			if (GAP_Remote_Name_Event_Data) {
				/* Inform the user of the Result.                        */
				BD_ADDRToStr(GAP_Remote_Name_Event_Data->Remote_Device,
						Callback_BoardStr);

				Display(("\r\n"));
				Display(("BD_ADDR: %s.\r\n", Callback_BoardStr));

				if (GAP_Remote_Name_Event_Data->Remote_Name)
					Display(
							("Name: %s.\r\n", GAP_Remote_Name_Event_Data->Remote_Name));
				else
					Display(("Name: NULL.\r\n"));
			}
			break;
		case etEncryption_Change_Result:
			BD_ADDRToStr(
					GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Remote_Device,
					Callback_BoardStr);
			Display(
					("\r\netEncryption_Change_Result for %s, Status: 0x%02X, Mode: %s.\r\n", Callback_BoardStr, GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Change_Status, ((GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Mode == emDisabled)?"Disabled": "Enabled")));
			break;
		default:
			/* An unknown/unexpected GAP event was received.            */
			Display(
					("\r\nUnknown Event: %d.\r\n", GAP_Event_Data->Event_Data_Type));
			break;
		}
	} else {
		/* There was an error with one or more of the input parameters.   */
		Display(("\r\n"));
		Display(("Null Event\r\n"));
	}
}

/* The following function is for an SPP Event Callback.  This        */
/* function will be called whenever a SPP Event occurs that is       */
/* associated with the Bluetooth Stack.  This function passes to the */
/* caller the SPP Event Data that occurred and the SPP Event Callback*/
/* Parameter that was specified when this Callback was installed.    */
/* The caller is free to use the contents of the SPP SPP Event Data  */
/* ONLY in the context of this callback.  If the caller requires the */
/* Data for a longer period of time, then the callback function MUST */
/* copy the data into another Data Buffer.  This function is         */
/* guaranteed NOT to be invoked more than once simultaneously for the*/
/* specified installed callback (i.e.  this function DOES NOT have be*/
/* reentrant).  It Needs to be noted however, that if the same       */
/* Callback is installed more than once, then the callbacks will be  */
/* called serially.  Because of this, the processing in this function*/
/* should be as efficient as possible.  It should also be noted that */
/* this function is called in the Thread Context of a Thread that the*/
/* User does NOT own.  Therefore, processing in this function should */
/* be as efficient as possible (this argument holds anyway because   */
/* another SPP Event will not be processed while this function call  */
/* is outstanding).                                                  */
/* * NOTE * This function MUST NOT Block and wait for Events that    */
/*          can only be satisfied by Receiving SPP Event Packets.  A */
/*          Deadlock WILL occur because NO SPP Event Callbacks will  */
/*          be issued while this function is currently outstanding.  */
static void BTPSAPI SPP_Event_Callback(unsigned int BluetoothStackID,
		SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter) {
	int ret_val = 0;
	BoardStr_t Callback_BoardStr;

	/* **** SEE SPPAPI.H for a list of all possible event types.  This   */
	/* program only services its required events.                   **** */

	/* First, check to see if the required parameters appear to be       */
	/* semi-valid.                                                       */
	if ((SPP_Event_Data) && (BluetoothStackID)) {
		/* The parameters appear to be semi-valid, now check to see what  */
		/* type the incoming event is.                                    */
		switch (SPP_Event_Data->Event_Data_Type) {
		case etPort_Open_Indication:
			/* A remote port is requesting a connection.                */
			BD_ADDRToStr(
					SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->BD_ADDR,
					Callback_BoardStr);

			Display(("\r\n"));
			Display(
					("SPP Open Indication, ID: 0x%04X, Board: %s.\r\n", SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->SerialPortID, Callback_BoardStr));

			/* Store the Connection Information for this connection.    */
			ApplicationStateInfo.CBConnectionInfo.BD_ADDR =
					SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->BD_ADDR;

			/* Notify the event handler of the connection.              */
			PostApplicationMailbox(APPLICATION_MAILBOX_MESSAGE_ID_CB_CONNECTED);
			break;
		case etPort_Close_Port_Indication:
			/* The Remote Port was Disconnected.                        */
			Display(("\r\n"));
			Display(
					("SPP Close Port, ID: 0x%04X\r\n", SPP_Event_Data->Event_Data.SPP_Close_Port_Indication_Data->SerialPortID));

			/* Notify the event handler of the disconnection.           */
			PostApplicationMailbox(
					APPLICATION_MAILBOX_MESSAGE_ID_CB_DISCONNECTED);
			break;
		case etPort_Status_Indication:
			/* Display Information about the new Port Status.           */
			Display(("\r\n"));
			Display(
					("SPP Port Status Indication: 0x%04X, Status: 0x%04X, Break Status: 0x%04X, Length: 0x%04X.\r\n", SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->SerialPortID, SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->PortStatus, SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->BreakStatus, SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->BreakTimeout));

			break;
		case etPort_Data_Indication:
			/* Simply inform the user that data has arrived.            */
			Display(("\r\n"));
			Display(
					("SPP Data Indication, ID: 0x%04X, Length: 0x%04X.\r\n", SPP_Event_Data->Event_Data.SPP_Data_Indication_Data->SerialPortID, SPP_Event_Data->Event_Data.SPP_Data_Indication_Data->DataLength));
			break;
		case etPort_Send_Port_Information_Indication:
			/* Simply Respond with the information that was sent to us. */
			ret_val =
					SPP_Respond_Port_Information(BluetoothStackID,
							SPP_Event_Data->Event_Data.SPP_Send_Port_Information_Indication_Data->SerialPortID,
							&SPP_Event_Data->Event_Data.SPP_Send_Port_Information_Indication_Data->SPPPortInformation);
			break;
		case etPort_Transmit_Buffer_Empty_Indication:
			/* Flag that the SPP Buffer is not full.                    */
			ApplicationStateInfo.Flags &=
					~APPLICATION_STATE_INFO_FLAGS_SPP_BUFFER_FULL;

			/* Post the message to the application handler.             */
			PostApplicationMailbox(
					APPLICATION_MAILBOX_MESSAGE_ID_SPP_BUFFER_EMPTY);
			break;
		default:
			/* An unknown/unexpected SPP event was received.            */
			Display(("\r\n"));
			Display(("Unknown Handled SPP Event.\r\n"));
			break;
		}

		/* Check the return value of any function that might have been    */
		/* executed in the callback.                                      */
		if (ret_val) {
			/* An error occurred, so output an error message.              */
			Display(("\r\n"));
			Display(("Error %d.\r\n", ret_val));
		}
	} else {
		/* There was an error with one or more of the input parameters.   */
		Display(("Null Event\r\n"));
	}
}

/* ***************************************************************** */
/*                    End of Event Callbacks.                        */
/* ***************************************************************** */

/* The following function is used to initialize the application      */
/* instance.  This function should open the stack and prepare to     */
/* execute commands based on user input.  The first parameter passed */
/* to this function is the HCI Driver Information that will be used  */
/* when opening the stack and the second parameter is used to pass   */
/* parameters to BTPS_Init.  This function returns the               */
/* BluetoothStackID returned from BSC_Initialize on success or a     */
/* negative error code (of the form APPLICATION_ERROR_XXX).          */
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation,
		BTPS_Initialization_t *BTPS_Initialization) {
	int ret_val = APPLICATION_ERROR_UNABLE_TO_OPEN_STACK;

	/* Next, makes sure that the Driver Information passed appears to be */
	/* semi-valid.                                                       */
	if ((HCI_DriverInformation) && (BTPS_Initialization)) {
		/* Try to Open the stack and check if it was successful.          */
		if (!OpenStack(HCI_DriverInformation, BTPS_Initialization)) {
			/* Now that the device is discoverable attempt to make it      */
			/* pairable.                                                   */
			ret_val = SetPairable();
			if (!ret_val) {
				/* Create the Application Mailbox.                          */
				if ((ApplicationStateInfo.Mailbox = BTPS_CreateMailbox(
						APPLICATION_MAILBOX_DEPTH, APPLICATION_MAILBOX_SIZE)) != NULL) {
					/* Post some messages to the application to kick start   */
					/* the application.                                      */
					PostApplicationMailbox(
							APPLICATION_MAILBOX_MESSAGE_ID_CB_DISCONNECTED);
					PostApplicationMailbox(
							APPLICATION_MAILBOX_MESSAGE_ID_LE_DISCONNECTED);

					/* Return success to the caller.                         */
					ret_val = (int) ApplicationStateInfo.BluetoothStackID;
				} else {
					Display(("Failed to create application mailbox.\r\n"));

					ret_val = UNABLE_TO_INITIALIZE_STACK;
				}
			} else
				DisplayFunctionError("SetPairable", ret_val);

			/* In some error occurred then close the stack.                */
			if (ret_val < 0) {
				/* Close the Bluetooth Stack.                               */
				CloseStack();
			}
		} else {
			/* There was an error while attempting to open the Stack.      */
			Display(("Unable to open the stack.\r\n"));
		}
	} else
		ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;

	return (ret_val);
}

/* The following function is the main application state machine which*/
/* is used to process all application events.                        */
void ApplicationMain(void) {
	Byte_t MessageID;

	/* Verify that the application mailbox has been created.             */
	if (ApplicationStateInfo.Mailbox) {
		/* Loop forever and process application messages.                 */
		while (1) {
			/* Process the scheduler.                                      */
			BTPS_ProcessScheduler();

			/* Wait on the application mailbox.                            */
			if (BTPS_WaitMailbox(ApplicationStateInfo.Mailbox, &MessageID)) {
				switch (MessageID) {
				case APPLICATION_MAILBOX_MESSAGE_ID_SPP_BUFFER_EMPTY:
					/* Since the SPP Buffer is empty go ahead and send all*/
					/* of the queued data.                                */
					ProcessSendSPPData(FALSE);
					break;
				case APPLICATION_MAILBOX_MESSAGE_ID_LE_CONNECTED:
					/* Set the LE Connection Flag.                        */
					ApplicationStateInfo.Flags |=
							APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;

					/* Set the LE LED.                                    */
					HAL_SetLED(1, 1);
					break;
				case APPLICATION_MAILBOX_MESSAGE_ID_LE_DISCONNECTED:
					/* Start an advertising process.                      */
					StartAdvertising(ApplicationStateInfo.BluetoothStackID);

					/* Clear the LE Connection Information.               */
					BTPS_MemInitialize(&(ApplicationStateInfo.LEConnectionInfo),
							0, sizeof(ApplicationStateInfo.CBConnectionInfo));

					/* Clear the LE Connection Flag.                      */
					ApplicationStateInfo.Flags &=
							~APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;

					/* Clear the LE LED.                                  */
					HAL_SetLED(1, 0);
					break;
				case APPLICATION_MAILBOX_MESSAGE_ID_CB_CONNECTED:
					/* Format the Advertising Data to say that LE is not  */
					/* supported so that the MSP430 Exp Data Collector    */
					/* will connect over LE if the SPP port is connected. */
					FormatAdvertisingData(ApplicationStateInfo.BluetoothStackID,
							FALSE);

					/* Set stack to be non-discoverable/non-connectable as*/
					/* we do only allow 1 BR/EDR Connection.              */
					GAP_Set_Discoverability_Mode(
							ApplicationStateInfo.BluetoothStackID,
							dmNonDiscoverableMode, 0);
					GAP_Set_Connectability_Mode(
							ApplicationStateInfo.BluetoothStackID,
							cmNonConnectableMode);

					/* Set the BR/EDR Connection Flag.                    */
					ApplicationStateInfo.Flags |=
							APPLICATION_STATE_INFO_FLAGS_CB_CONNECTED;

					/* Set the BR/EDR LED.                                */
					HAL_SetLED(0, 1);

					/* "Hello" once FIXME */
					ProcessSendSPPData(TRUE);
					break;
				case APPLICATION_MAILBOX_MESSAGE_ID_CB_DISCONNECTED:
					/* Format the advertising data to say that BR/EDR is  */
					/* supported so that the MSP430 Exp Data Collector    */
					/* will connect over SPP if no device is connected    */
					/* over SPP to use locally first.                     */
					FormatAdvertisingData(ApplicationStateInfo.BluetoothStackID,
							TRUE);

					/* Set the stack to be connectable and discoverable.  */
					GAP_Set_Discoverability_Mode(
							ApplicationStateInfo.BluetoothStackID,
							dmGeneralDiscoverableMode, 0);
					GAP_Set_Connectability_Mode(
							ApplicationStateInfo.BluetoothStackID,
							cmConnectableMode);

					/* Clear the BR/EDR Connection Information.           */
					BTPS_MemInitialize(&(ApplicationStateInfo.CBConnectionInfo),
							0, sizeof(ApplicationStateInfo.CBConnectionInfo));

					/* Clear the BR/EDR Connection Flag and the SPP Buffer*/
					/* Full Flag.                                         */
					ApplicationStateInfo.Flags &=
							~(APPLICATION_STATE_INFO_FLAGS_CB_CONNECTED
									| APPLICATION_STATE_INFO_FLAGS_SPP_BUFFER_FULL);

					/* Since we are disconnected we will discard any SPP  */
					/* data that was queued for transmission to the       */
					/* device.                                            */
					ApplicationStateInfo.SPPBufferLength = 0;

					/* Clear the BR/EDR LED.                              */
					HAL_SetLED(0, 0);
					break;
				}
			} else {
				/* Call the idle function.                                  */
				IdleFunction(ApplicationStateInfo.BluetoothStackID);
			}
		}
	}
}

