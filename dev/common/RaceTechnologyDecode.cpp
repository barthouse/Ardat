/******************************************************************************************************
*
* Project		: Serial Decoder
* File			: tskSerialDecode.c
* Description	: Serial data processing functions
* Author		: Dr. Andrew J. Durrant
* Copyright		: Race Technology Ltd, UK
*
*
* Version	Date		Author					Change/Reason
* -------	----------	--------------------	-------------------------------------------------------
*	1.0		2006		Dr.Andrew Durrant		Originally created in Visual Basic
*	2.0		01-02-2007	Thisara Matharage		Converted to C language
*	3.0		06-06-2007	Sumudu Fernando			Optimizing the code
*	3.1		14-06-2007							Bug Fixing
*	3.2		23-08-2007	Thisara Matharage		Bug Fixing
*	3.3		07-09-2007	Thisara Matharage		DVR commading added based on the Serial Commands on DRAFT 1
*	3.4		25-09-2007	Thisara Matharage		RUN file write
*	3.5		29-09-2007	Thisara Matharage		Removal of pointers, Extract msg, Bug fixing and improvement
*	3.6		10-10-2007	Thisara Matharage		RUN file header signature (Ln 723)
												Save msg type 0x68 (video frame number) Ln 1252
*	3.7		10-11-2007	Thisara Matharage		Control Playback
*	3.8		12-04-2007	Nuwan Gajaweera			Optimize code by eliminating seperate fucntion for checksum calculation
												Added MessagesLengths[] array
												Added functions - func80_HARDC_GPSYAW(), func81_HARDC_PITCH_RATE (), func82_HARDC_PITCH (), 
												func83_HARDC_ROLL_RATE (), func86_HARDC_PULSE0 (), func90_HARDC_BASELINERTK ()
												Changed func20_ADC0HARDC - added channels ADC16HARDC to ADC31HARDC 
*	3.9		07-01-2008	Nuwan Gajaweera			Added function f_NoData. The following channels are pointed to new func 
												52~54, 63, 65~70, 76, 77, 91, 101, 102, 104
												and 103 (in Non DVR systems)
												Added function func83_HARDC_ROLL_RATE and related constants GYROROLLRATE_, GPSROLLRATELENGTH
												Added following functions (unused and untested) - func63_HARDC_START_OF_RUN, func65_HARDC_GEAR_SETUP_DATA
												func67_HARDC_DASHBOARD_SETUP_DATA, func68_HARDC_DASHBOARD_SETUP_DATA_2, func69_HARDC_NEW_TARGET_SECTOR_TIME
												func70_HARDC_NEW_TARGET_MARKER_TIME, func76_HARDC_NEW_LCD_DATA, func77_HARDC_NEW_LED_DATA
												func91_HARDC_UNIT_CONTROL, func0x65_HARDC_SECTOR_DEFINITION, func0x68_HARDC_VIDEO_FRAME_INDEX
*******************************************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
///////////////////////////////////////////////////////////////////////////////////////////
//									Header files
///////////////////////////////////////////////////////////////////////////////////////////
#include <string.h>	// strcpy, strcat, memmove
#include <math.h>	// pow
#include <stdio.h>
///////////////////////////////////////////////////////////////////////////////////////////

//#define C6000
//#define C2000
#define PC

#ifndef RTMSGDEC_H
#define RTMSGDEC_H

#define MAX_MSG_LEN		255
#define ARRAY_LEN		256

#define GRAVG			((F64)9.80665)			//gravitational contant g
#define PI				((F64)3.141592654)		//Pi!
#define TICKPERIOD		(1.66666666666667E-07)  // Add comment (###)

////////////////////////////////////////////////////////////////////////////////////////
//								Message Length Defs
////////////////////////////////////////////////////////////////////////////////////////

#define ENCSERIALHARDCLENGTH					9
#define GPSALTLENGTH							10
#define GPSHEADINGLENGTH						10
#define GPSDATELENGTH							10
#define MARKERCHANNELLENGTH						21
#define SECTORTIMECHANNELLENGTH					7
#define ADCCHANNELLENGTH						4
#define ACCELCHANNELLENGTH						6
#define ZACCELCHANNELLENGTH						4
#define GPSPOSCHANNELLENGTH						14
#define GPSSPEEDCHANNELLENGTH					10
#define DIGITALCHANNELLENGTH					3
#define U24CHANNELLENGTH						5
#define U32CHANNELLENGTH						6
#define FREQCHANNELLENGTH						5
#define EXTENDEDFREQCHANNELLENGTH				11
#define PROCESSEDSPEEDCHANNELLENGTH				5
#define LOGGERSERIALNUMBERCHANNELLENGTH			6
#define DEBUGVAR_CHANNELLENGTH					6

#define ECU_MODULETYPE_CHANNELLENGTH			3
#define ECU_TEMP_CHANNELLENGTH					5
#define ECU_FREQ_CHANNELLENGTH					5

#define ECU_PERC_CHANNELLENGTH					5
#define ECU_TIME_CHANNELLENGTH					6

#define ECU_ANGLE_CHANNELLENGTH					5
#define ECU_PRESSURE_CHANNELLENGTH				6
#define ECU_MISC_CHANNELLENGTH					5

#define STARTSTOPCHANNELLENGTH					11
#define GPSGRADIENTCHANNELLENGTH				10
#define PROCESSEDDISTANCECHANNELLENGTH			6
#define YAWGYROCHANNELLENGTH					4

#define GPSYAWLENGTH							4
#define GPSPITCHRATELENGTH						5
#define GPSPITCHLENGTH							5
#define GPSROLLRATELENGTH						5
#define PULSECOUNTLENGTH						5
#define BASELINERTKLENGTH						6

// DVR COMMAND LENGTH
#define DVR_CMD_CHANNELLENGTH					18

//###
#define CHANNELDATALENGTH						67
#define DISPLAYCHANNELLENGTH					11
#define REFLASHCHANNELLENGTH					6
#define STARTOFRUNLENGTH						3
#define GEARSETUPDATALENGTH						30
#define BARGRAPHSETUPDATALENGTH					11
#define DASHBOARD_SETUP_DATALENGTH				4
#define DASHBOARDSETUPDATA2LENGTH				4
#define NEWTARGETSECTORTIMELENGTH				42
#define NEWTARGETMARKERTIMELENGTH				42
#define NEWLCDDATA								24
#define NEWLEDDATA								3
#define ROLLANGLELENGTH							5
#define UNITCONTROLLENGTH						5
#define SECTORDEFINITIONLENGTH					19
#define VIDEOINDEXLENGTH						6

////////////////////////////////////////////////////////////////////////////////////////
//								End of Message Length Defs
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
//								InternalVariableReference
////////////////////////////////////////////////////////////////////////////////////////
#define MAXVARS			8000					//maximum number of calculated variables allowed

enum InternalVariableReference_{
	TIMES_ =					1,
	ACCELLONG_ =				2,
	ACCELLONGFLAG_ =			3,
	ACCELLAT_ =					4,
	ACCELVECT_ =				5,
	RPM_ =						6,
	GPSSPEED_ =					7,
	WHEELSPEED_ =				8,
	DISTANCE_ =					9,
	POWERATWHEELS_ =			10,
	TORQUEATWHEELS_ =			11,
	GEAR_ =						12,
	POSXM_ =					13,
	POSYM_ =					14,
	CTIME_ =					15,
	CTIMERATE_ =				16,
	TRACKHEADING_ =				17,
	CHANGEINHEADING_ =			18,
	CORNERRADIUS_ =				19,
	BICYCLELEANANGLE_ =			20,
	LAPTIMES_ =					21,
	SECTORTIMES_ =				22,
	TRACKBEACON_ =				23,
	GYROYAWRATE_ =				24,
	GYROYAW_ =					25,
	GYROSLIPANGLE_ =			26,
	BUFFERSIZE_ =				27,
	SERIALDATACOUNT_ =			28,
	ACCEL_Z_ =					29,
	GYROPITCHRATE_ =			30,
	GYROROLLRATE_ =				31,

	DL1_ANAL0_ =				200,
	DL1_ANAL1_ =				201,
	DL1_ANAL2_ =				202,
	DL1_ANAL3_ =				203,
	DL1_ANAL4_ =				204,
	DL1_ANAL5_ =				205,
	DL1_ANAL6_ =				206,
	DL1_ANAL7_ =				207,
	DL1_ANAL8_ =				208,
	DL1_ANAL9_ =				209,
	DL1_ANAL10_ =				210,
	DL1_ANAL11_ =				211,
	DL1_ANAL12_ =				212,
	DL1_ANAL13_ =				213,
	DL1_ANAL14_ =				214,
	DL1_ANAL15_ =				215,
	DL1_ANAL16_ =				216,
	DL1_ANAL17_ =				217,
	DL1_ANAL18_ =				218,
	DL1_ANAL19_ =				219,
	DL1_ANAL20_ =				220,
	DL1_ANAL21_ =				221,
	DL1_ANAL22_ =				222,
	DL1_ANAL23_ =				223,
	DL1_ANAL24_ =				224,
	DL1_ANAL25_ =				225,
	DL1_ANAL26_ =				226,
	DL1_ANAL27_ =				227,
	DL1_ANAL28_ =				228,
	DL1_ANAL29_ =				229,
	DL1_ANAL30_ =				230,
	DL1_ANAL31_ =				231,

	DL1_FREQ1_ =				300,
	DL1_FREQ2_ =				301,
	DL1_FREQ3_ =				302,
	DL1_FREQ4_ =				303,
	FREQ1_HIGHPERIOD_ =			304,
	FREQ1_LOWPERIOD_ =			305,
	FREQ1_PULSEPOSITION_ =		306,
	FREQ2_HIGHPERIOD_ =			307,
	FREQ2_LOWPERIOD_ =			308,
	FREQ2_PULSEPOSITION_ =		309,
	FREQ3_HIGHPERIOD_ =			310,
	FREQ3_LOWPERIOD_ =			311,
	FREQ3_PULSEPOSITION_ =		312,
	FREQ4_HIGHPERIOD_ =			313,
	FREQ4_LOWPERIOD_ =			314,
	FREQ4_PULSEPOSITION_ =		315,
	FREQ1_PULSE_COUNT_ =		316,
	FREQ2_PULSE_COUNT_ =		317,
	FREQ3_PULSE_COUNT_ =		318,
	FREQ4_PULSE_COUNT_ =		319,

	RPM_HIGHPERIOD_ =			320, //Added by Nuwan
	RPM_LOWPERIOD_ =			321,
	RPM_PULSEPOSITION_ =		322,

	GPSALTITUDE_ =				400,
	GPSLATACCEL_ =				401,
	GPSLONGACCEL_ =				402,
	GPSHEADING_ =				403,
	GPSGRADIENT_ =				404,
	GPSMSTIME_ =				405,
	GPSPOSITIONACC_ =			406,
	GPSSPEEDACC_ =				407,
	GPSHEADINGACC_ =			408,
	GPSGRADIENTACC_ =			409,
	GPSALTACC_ =				410,
	ACTUALGPSSATCOUNT_ =		411,
	WEIGHTEDGPSSATCOUNT_ =		412,
	GPSVELSOURCE_ =				413,
	GPSRAWVEL_ =				414,

	GPSRTKPITCH_ =				430,
	GPSRTKYAW_ =				431,
	GPSROLL_ =					432,//Added by nuwan

	GPSRTKBASELINE_ =			450,
	GPSRTKBASELINEACC_ =		451,

	GPSDATACOUNT_ =				500,

	ECU_FIRST_TEMP_ =			1000,
	ECU_FIRST_FREQ_ =			1500,
	ECU_FIRST_PERC_ =			2000,
	ECU_FIRST_TIME_ =			2500,

	ECU_FIRST_ANGLE_ =			1500,
	ECU_FIRST_PRESSURE_ =		2000,
	ECU_FIRST_MISC_ =			2500,

	DEBUG_VAR_FP_0_ =			5050,
	DEBUG_VAR_FP_1_ =			5051,
	DEBUG_VAR_FP_2_ =			5052,
	DEBUG_VAR_FP_3_ =			5053,
	DEBUG_VAR_FP_4_ =			5054,
	DEBUG_VAR_FP_5_ =			5055,
	DEBUG_VAR_FP_6_ =			5056,
	DEBUG_VAR_FP_7_ =			5057,
	DEBUG_VAR_FP_8_ =			5058,
	DEBUG_VAR_FP_9_ =			5059,
	DEBUG_VAR_FP_A_ =			5060,
	DEBUG_VAR_FP_B_ =			5061,
	DEBUG_VAR_FP_C_ =			5062,
	DEBUG_VAR_FP_D_ =			5063,
	DEBUG_VAR_FP_E_ =			5064,
	DEBUG_VAR_FP_F_ =			5065,
	DEBUG_VAR_U32_0_ =			5066,
	DEBUG_VAR_U32_1_ =			5067,
	DEBUG_VAR_U32_2_ =			5068,
	DEBUG_VAR_U32_3_ =			5069,
	DEBUG_VAR_U32_4_ =			5070,
	DEBUG_VAR_U32_5_ =			5071,
	DEBUG_VAR_U32_6_ =			5072,
	DEBUG_VAR_U32_7_ =			5073,
	DEBUG_VAR_S32_0_ =			5074,
	DEBUG_VAR_S32_1_ =			5075,
	DEBUG_VAR_S32_2_ =			5076,
	DEBUG_VAR_S32_3_ =			5077,
	DEBUG_VAR_S32_4_ =			5078,
	DEBUG_VAR_S32_5_ =			5079,
	DEBUG_VAR_S32_6_ =			5080,
	DEBUG_VAR_S32_7_ =			5081,

	USER_VAR_1_ =				6001,
	USER_VAR_2_	=				6002,
	USER_VAR_3_ =				6003,
	USER_VAR_4_ =				6004,
	USER_VAR_5_ =				6005,
	USER_VAR_6_ =				6006,
	USER_VAR_7_ =				6007,
	USER_VAR_8_ =				6008,
	USER_VAR_9_ =				6009,
	USER_VAR_10_ =				6010,
	USER_VAR_11_ =				6011,
	USER_VAR_12_ =				6012,
	USER_VAR_13_ =				6013,
	USER_VAR_14_ =				6014,
	USER_VAR_15_ =				6015,
	USER_VAR_16_ =				6016,
	USER_VAR_17_ =				6017,
	USER_VAR_18_ =				6018,
	USER_VAR_19_ =				6019,
	USER_VAR_20_ =				6020,
	USER_VAR_21_ =				6021,
	USER_VAR_22_ =				6022,
	USER_VAR_23_ =				6023,
	USER_VAR_24_ =				6024,
	USER_VAR_25_ =				6025,
	NODATA_ =					MAXVARS
};

////////////////////////////////////////////////////////////////////////////////////////
//								InternalVariableReference
////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
//								Hardware Channels
////////////////////////////////////////////////////////////////////////////////////////
enum HardwareChannels_{
	ENCSERIALHARDC =				1,
	STARTSTOPHARDC =				2,
	GPSRAWDATA =					3,
	SECTORTIMEHARDC =				4,
	MARKERHARDC =					5,
	LOGGERSERIALNUMBERC =			6,
	GPSTIMEMSWEEKC =				7,
	ACCELSHARDC =					8,
	TIMEHARDC =						9,
	GPSPOSHARDC =					10,
	GPSSPEEDHARDC =					11,
	BEACONHARDC =					12,
	GPSPULSEHARDC =					13,

	FREQ0HARDC =					14,
	FREQ1HARDC =					15,
	FREQ2HARDC =					16,
	FREQ3HARDC =					17,
	RPMHARDC =						18,

	SERIALDATAHARDC =				19,

	ADC0HARDC =						20,
	ADC1HARDC =						21,
	ADC2HARDC =						22,
	ADC3HARDC =						23,
	ADC4HARDC =						24,
	ADC5HARDC =						25,
	ADC6HARDC =						26,
	ADC7HARDC =						27,
	ADC8HARDC =						28,
	ADC9HARDC =						29,
	ADC10HARDC =					30,	// PROCESSEDSPEEDHARDC__OLD = 30,
	ADC11HARDC =					31,
	ADC12HARDC =					32,
	ADC13HARDC =					33,
	ADC14HARDC =					34,
	ADC15HARDC =					35,
	ADC16HARDC =					36,
	ADC17HARDC =					37,
	ADC18HARDC =					38,
	ADC19HARDC =					39,
	ADC20HARDC =					40,
	ADC21HARDC =					41,
	ADC22HARDC =					42,
	ADC23HARDC =					43,
	ADC24HARDC =					44,
	ADC25HARDC =					45,
	ADC26HARDC =					46,
	ADC27HARDC =					47,
	ADC28HARDC =					48,
	ADC29HARDC =					49,
	ADC30HARDC =					50,
	ADC31HARDC =					51,

	CHANNELDATA =					52,
	DISPLAYCHANNEL =				53,
	REFLASHCHANNEL =				54,

	GPSDATEHARDC =					55,
	GPSHEADINGHARDC =				56,
	GPSALTHARDC =					57,

	EXTENDED_FREQ0HARDC =			58,
	EXTENDED_FREQ1HARDC =			59,
	EXTENDED_FREQ2HARDC =			60,
	EXTENDED_FREQ3HARDC =			61,
	EXTENDED_RPMHARDC =				62,

	PROCESSEDSPEEDHARDC =			64,

	HARDC_GEAR_SETUP_DATA =			65,
	HARDC_BARGRAPH_SETUP_DATA =		66,
	HARDC_DASHBOARD_SETUP_DATA =	67,
	HARDC_DASHBOARD_SETUP_DATA_2 =	68,

	HARDC_NEW_TARGET_SECTOR_TIME =	69,
	HARDC_NEW_TARGET_MARKER_TIME =	70,

	HARDC_ECU_MODULETYPE =			71,
	HARDC_ECU_TEMP =				72,
	HARDC_ECU_FREQ =				73,
	HARDC_ECU_PERC =				74,
	HARDC_ECU_TIME =				75,

	PROCESSEDDISTANCEHARDC =		78,

	YAWGYROHARDC =					79,

	HARDC_GPSYAW =					80,
	HARDC_PITCH_RATE =				81,
	HARDC_PITCH =					82,
	HARDC_ROLL_RATE =				83,
	HARDC_ROLL =					84,

	GPSGRADIENTHARDC =				85,

	HARDC_PULSE0 =					86,
	HARDC_PULSE1 =					87,
	HARDC_PULSE2 =					88,
	HARDC_PULSE3 =					89,
	HARDC_BASELINERTK =				90,

	HARDC_UNIT_CONTROL =			91,

	HARDC_Z_ACCEL =					92,
	HARDC_ECU_ANGLE =				93,
	HARDC_ECU_PRESSURE =			94,
	HARDC_ECU_MISC =				95,

	HARDC_SECTOR_DEFINITION =		0x65,
	HARDC_BREAKBOX_PC_COMM =		0x66,

	HARDC_DVR_CMD =					0x67,
	HARDC_VIDEO_INDEX =				0x68,

	HARDC_DEBUG_VAR_FP_0 =			0xD0,
	HARDC_DEBUG_VAR_FP_1 =			0xD1,
	HARDC_DEBUG_VAR_FP_2 =			0xD2,
	HARDC_DEBUG_VAR_FP_3 =			0xD3,
	HARDC_DEBUG_VAR_FP_4 =			0xD4,
	HARDC_DEBUG_VAR_FP_5 =			0xD5,
	HARDC_DEBUG_VAR_FP_6 =			0xD6,
	HARDC_DEBUG_VAR_FP_7 =			0xD7,
	HARDC_DEBUG_VAR_FP_8 =			0xD8,
	HARDC_DEBUG_VAR_FP_9 =			0xD9,
	HARDC_DEBUG_VAR_FP_A =			0xDA,
	HARDC_DEBUG_VAR_FP_B =			0xDB,
	HARDC_DEBUG_VAR_FP_C =			0xDC,
	HARDC_DEBUG_VAR_FP_D =			0xDD,
	HARDC_DEBUG_VAR_FP_E =			0xDE,
	HARDC_DEBUG_VAR_FP_F =			0xDF,

	HARDC_DEBUG_VAR_U32_0 =			0xE0,
	HARDC_DEBUG_VAR_U32_1 =			0xE1,
	HARDC_DEBUG_VAR_U32_2 =			0xE2,
	HARDC_DEBUG_VAR_U32_3 =			0xE3,
	HARDC_DEBUG_VAR_U32_4 =			0xE4,
	HARDC_DEBUG_VAR_U32_5 =			0xE5,
	HARDC_DEBUG_VAR_U32_6 =			0xE6,
	HARDC_DEBUG_VAR_U32_7 =			0xE7,

	HARDC_DEBUG_VAR_S32_0 =			0xF0,
	HARDC_DEBUG_VAR_S32_1 =			0xF1,
	HARDC_DEBUG_VAR_S32_2 =			0xF2,
	HARDC_DEBUG_VAR_S32_3 =			0xF3,
	HARDC_DEBUG_VAR_S32_4 =			0xF4,
	HARDC_DEBUG_VAR_S32_5 =			0xF5,
	HARDC_DEBUG_VAR_S32_6 =			0xF6,
	HARDC_DEBUG_VAR_S32_7 =			0xF7,
};

////////////////////////////////////////////////////////////////////////////////////////
//								End of Hardware Channels
////////////////////////////////////////////////////////////////////////////////////////


#define TRUE	1
#define FALSE	0


#ifdef C2000
	typedef unsigned char	U8;
	typedef unsigned int	U16;
	typedef unsigned long	U32;

	typedef char			S8;
	typedef int				S16;
	typedef long			S32;

	typedef float			F32;
	typedef long double		F64;
#else
	typedef unsigned char		U8;	
	typedef unsigned short		U16;
	typedef unsigned int		U32;

	typedef char				S8;
	typedef short				S16;
	typedef int					S32;

	typedef float				F32;
	typedef double				F64;
#endif

typedef void (*CHPF) ();

#ifdef C2000
	extern const U8 MessagesLengths[];
	extern void serialDecode (U8 c);
	extern void decodeData(U8 decode);
	extern void initDecoder(void);
#endif

#ifdef PC
	extern void serialDecode (U8 c);
	void open_log(char *f_name);
	void close_log(void);
#endif

#endif

///////////////////////////////////////////////////////////////////////////////////////////
//									Common variables
///////////////////////////////////////////////////////////////////////////////////////////

U8 bDecoderRunning = FALSE;					//This gives the decoder state (TRUE - we're in the middle of decoding a message, FALSE - we're about to decode a new message)
U8 bReturnDataValues = TRUE;				//This specifies whether the date should be decoded(TRUE) or not(FALSE)

U8 iHardwareChannel;						// this is the hardware channel that the data was extracted from
U8 iLengthOfMsg;							// length of current message
//U16 iChecksum1;

U8 serial_Buf[MAX_MSG_LEN<<1];				// this is the main raw data input array
U16 iRawInput = 0;							// the next data byte recived will be stored at this index in the serial_Buf[]
U16 iRawSample = 0;							// the start of the current message, this is an index into the serial_Buf[] as well

U8 iNumReturnedFloatChannels;				// this is the number of valid results
S32 iArrReturnedChannels[21];				// these are the soft channels associated with the results array (internal variable reference)
F64 dArrReturnedFloatValues[21];			// these are the returned float values

U8 bBytesReturned;							// this specifies whether bytes we extracted from the current message
U8 iNumberOfBytesReturned;					// this is the number of bytes that are returned
U8 bArrReturnedByteValues[257];				// this is the returned byte values

U8 bDataValid;


#ifdef PC
	FILE *file_out;
#endif

//This array gives the message length for each message type. The index into the array is the hardware channel
const U8 MessagesLengths[ARRAY_LEN] = {
	0,	// channel #0(0)
	ENCSERIALHARDCLENGTH,	// channel #1(1)
	STARTSTOPCHANNELLENGTH,	// channel #2(2)
	MAX_MSG_LEN,	// channel #3(3)
	SECTORTIMECHANNELLENGTH,	// channel #4(4)
	MARKERCHANNELLENGTH,	// channel #5(5)
	LOGGERSERIALNUMBERCHANNELLENGTH,	// channel #6(6)
	U32CHANNELLENGTH,	// channel #7(7)
	ACCELCHANNELLENGTH,	// channel #8(8)
	U24CHANNELLENGTH,	// channel #9(9)
	GPSPOSCHANNELLENGTH,	// channel #10(A)
	GPSSPEEDCHANNELLENGTH,	// channel #11(B)
	DIGITALCHANNELLENGTH,	// channel #12(C)
	DIGITALCHANNELLENGTH,	// channel #13(D)
	FREQCHANNELLENGTH,	// channel #14(E)
	FREQCHANNELLENGTH,	// channel #15(F)
	FREQCHANNELLENGTH,	// channel #16(10)
	FREQCHANNELLENGTH,	// channel #17(11)
	FREQCHANNELLENGTH,	// channel #18(12)
	MAX_MSG_LEN,	// channel #19(13)
	ADCCHANNELLENGTH,	// channel #20(14)
	ADCCHANNELLENGTH,	// channel #21(15)
	ADCCHANNELLENGTH,	// channel #22(16)
	ADCCHANNELLENGTH,	// channel #23(17)
	ADCCHANNELLENGTH,	// channel #24(18)
	ADCCHANNELLENGTH,	// channel #25(19)
	ADCCHANNELLENGTH,	// channel #26(1A)
	ADCCHANNELLENGTH,	// channel #27(1B)
	ADCCHANNELLENGTH,	// channel #28(1C)
	ADCCHANNELLENGTH,	// channel #29(1D)
	ADCCHANNELLENGTH,	// channel #30(1E)
	ADCCHANNELLENGTH,	// channel #31(1F)
	ADCCHANNELLENGTH,	// channel #32(20)
	ADCCHANNELLENGTH,	// channel #33(21)
	ADCCHANNELLENGTH,	// channel #34(22)
	ADCCHANNELLENGTH,	// channel #35(23)
	ADCCHANNELLENGTH,	// channel #36(24)
	ADCCHANNELLENGTH,	// channel #37(25)
	ADCCHANNELLENGTH,	// channel #38(26)
	ADCCHANNELLENGTH,	// channel #39(27)
	ADCCHANNELLENGTH,	// channel #40(28)
	ADCCHANNELLENGTH,	// channel #41(29)
	ADCCHANNELLENGTH,	// channel #42(2A)
	ADCCHANNELLENGTH,	// channel #43(2B)
	ADCCHANNELLENGTH,	// channel #44(2C)
	ADCCHANNELLENGTH,	// channel #45(2D)
	ADCCHANNELLENGTH,	// channel #46(2E)
	ADCCHANNELLENGTH,	// channel #47(2F)
	ADCCHANNELLENGTH,	// channel #48(30)
	ADCCHANNELLENGTH,	// channel #49(31)
	ADCCHANNELLENGTH,	// channel #50(32)
	ADCCHANNELLENGTH,	// channel #51(33)
	CHANNELDATALENGTH,	// channel #52(34)
	DISPLAYCHANNELLENGTH,	// channel #53(35)
	REFLASHCHANNELLENGTH,	// channel #54(36)
	GPSDATELENGTH,	// channel #55(37)
	GPSHEADINGLENGTH,	// channel #56(38)
	GPSALTLENGTH,	// channel #57(39)
	EXTENDEDFREQCHANNELLENGTH,	// channel #58(3A)
	EXTENDEDFREQCHANNELLENGTH,	// channel #59(3B)
	EXTENDEDFREQCHANNELLENGTH,	// channel #60(3C)
	EXTENDEDFREQCHANNELLENGTH,	// channel #61(3D)
	EXTENDEDFREQCHANNELLENGTH,	// channel #62(3E)
	STARTOFRUNLENGTH,	// channel #63(3F)
	PROCESSEDSPEEDCHANNELLENGTH,	// channel #64(40)
	GEARSETUPDATALENGTH,	// channel #65(41)
	BARGRAPHSETUPDATALENGTH,	// channel #66(42)
	DASHBOARD_SETUP_DATALENGTH,	// channel #67(43)
	DASHBOARDSETUPDATA2LENGTH,	// channel #68(44)
	NEWTARGETSECTORTIMELENGTH,	// channel #69(45)
	NEWTARGETMARKERTIMELENGTH,	// channel #70(46)
	ECU_MODULETYPE_CHANNELLENGTH,	// channel #71(47)
	ECU_TEMP_CHANNELLENGTH,	// channel #72(48)
	ECU_FREQ_CHANNELLENGTH,	// channel #73(49)
	ECU_PERC_CHANNELLENGTH,	// channel #74(4A)
	ECU_TIME_CHANNELLENGTH,	// channel #75(4B)
	NEWLCDDATA,	// channel #76(4C)
	NEWLEDDATA,	// channel #77(4D)
	PROCESSEDDISTANCECHANNELLENGTH,	// channel #78(4E)
	YAWGYROCHANNELLENGTH,	// channel #79(4F)
	GPSYAWLENGTH,	// channel #80(50)
	GPSPITCHRATELENGTH,	// channel #81(51)
	GPSPITCHLENGTH,	// channel #82(52)
	GPSROLLRATELENGTH,	// channel #83(53)
	ROLLANGLELENGTH,	// channel #84(54)
	GPSGRADIENTCHANNELLENGTH,	// channel #85(55)
	PULSECOUNTLENGTH,	// channel #86(56)
	PULSECOUNTLENGTH,	// channel #87(57)
	PULSECOUNTLENGTH,	// channel #88(58)
	PULSECOUNTLENGTH,	// channel #89(59)
	BASELINERTKLENGTH,	// channel #90(5A)
	UNITCONTROLLENGTH,	// channel #91(5B)
	ZACCELCHANNELLENGTH,	// channel #92(5C)
	ECU_ANGLE_CHANNELLENGTH,	// channel #93(5D)
	ECU_PRESSURE_CHANNELLENGTH,	// channel #94(5E)
	ECU_MISC_CHANNELLENGTH,	// channel #95(5F)
	0,	// channel #96(60)
	0,	// channel #97(61)
	0,	// channel #98(62)
	0,	// channel #99(63)
	0,	// channel #100(64)
	SECTORDEFINITIONLENGTH,	// channel #101(65)
	MAX_MSG_LEN,	// channel #102(66)
	DVR_CMD_CHANNELLENGTH,	// channel #103(67)
	VIDEOINDEXLENGTH,	// channel #104(68)
	0,	// channel #105(69)
	0,	// channel #106(6A)
	0,	// channel #107(6B)
	0,	// channel #108(6C)
	0,	// channel #109(6D)
	0,	// channel #110(6E)
	0,	// channel #111(6F)
	0,	// channel #112(70)
	0,	// channel #113(71)
	0,	// channel #114(72)
	0,	// channel #115(73)
	0,	// channel #116(74)
	0,	// channel #117(75)
	0,	// channel #118(76)
	0,	// channel #119(77)
	0,	// channel #120(78)
	0,	// channel #121(79)
	0,	// channel #122(7A)
	0,	// channel #123(7B)
	0,	// channel #124(7C)
	0,	// channel #125(7D)
	0,	// channel #126(7E)
	0,	// channel #127(7F)
	0,	// channel #128(80)
	0,	// channel #129(81)
	0,	// channel #130(82)
	0,	// channel #131(83)
	0,	// channel #132(84)
	0,	// channel #133(85)
	0,	// channel #134(86)
	0,	// channel #135(87)
	0,	// channel #136(88)
	0,	// channel #137(89)
	0,	// channel #138(8A)
	0,	// channel #139(8B)
	0,	// channel #140(8C)
	0,	// channel #141(8D)
	0,	// channel #142(8E)
	0,	// channel #143(8F)
	0,	// channel #144(90)
	0,	// channel #145(91)
	0,	// channel #146(92)
	0,	// channel #147(93)
	0,	// channel #148(94)
	0,	// channel #149(95)
	0,	// channel #150(96)
	0,	// channel #151(97)
	0,	// channel #152(98)
	0,	// channel #153(99)
	0,	// channel #154(9A)
	0,	// channel #155(9B)
	0,	// channel #156(9C)
	0,	// channel #157(9D)
	0,	// channel #158(9E)
	0,	// channel #159(9F)
	0,	// channel #160(A0)
	0,	// channel #161(A1)
	0,	// channel #162(A2)
	0,	// channel #163(A3)
	0,	// channel #164(A4)
	0,	// channel #165(A5)
	0,	// channel #166(A6)
	0,	// channel #167(A7)
	0,	// channel #168(A8)
	0,	// channel #169(A9)
	0,	// channel #170(AA)
	0,	// channel #171(AB)
	0,	// channel #172(AC)
	0,	// channel #173(AD)
	0,	// channel #174(AE)
	0,	// channel #175(AF)
	0,	// channel #176(B0)
	0,	// channel #177(B1)
	0,	// channel #178(B2)
	0,	// channel #179(B3)
	0,	// channel #180(B4)
	0,	// channel #181(B5)
	0,	// channel #182(B6)
	0,	// channel #183(B7)
	0,	// channel #184(B8)
	0,	// channel #185(B9)
	0,	// channel #186(BA)
	0,	// channel #187(BB)
	0,	// channel #188(BC)
	0,	// channel #189(BD)
	0,	// channel #190(BE)
	0,	// channel #191(BF)
	0,	// channel #192(C0)
	0,	// channel #193(C1)
	0,	// channel #194(C2)
	0,	// channel #195(C3)
	0,	// channel #196(C4)
	0,	// channel #197(C5)
	0,	// channel #198(C6)
	0,	// channel #199(C7)
	0,	// channel #200(C8)
	0,	// channel #201(C9)
	0,	// channel #202(CA)
	0,	// channel #203(CB)
	0,	// channel #204(CC)
	0,	// channel #205(CD)
	0,	// channel #206(CE)
	0,	// channel #207(CF)
	DEBUGVAR_CHANNELLENGTH,	// channel #208(D0)
	DEBUGVAR_CHANNELLENGTH,	// channel #209(D1)
	DEBUGVAR_CHANNELLENGTH,	// channel #210(D2)
	DEBUGVAR_CHANNELLENGTH,	// channel #211(D3)
	DEBUGVAR_CHANNELLENGTH,	// channel #212(D4)
	DEBUGVAR_CHANNELLENGTH,	// channel #213(D5)
	DEBUGVAR_CHANNELLENGTH,	// channel #214(D6)
	DEBUGVAR_CHANNELLENGTH,	// channel #215(D7)
	DEBUGVAR_CHANNELLENGTH,	// channel #216(D8)
	DEBUGVAR_CHANNELLENGTH,	// channel #217(D9)
	DEBUGVAR_CHANNELLENGTH,	// channel #218(DA)
	DEBUGVAR_CHANNELLENGTH,	// channel #219(DB)
	DEBUGVAR_CHANNELLENGTH,	// channel #220(DC)
	DEBUGVAR_CHANNELLENGTH,	// channel #221(DD)
	DEBUGVAR_CHANNELLENGTH,	// channel #222(DE)
	DEBUGVAR_CHANNELLENGTH,	// channel #223(DF)
	DEBUGVAR_CHANNELLENGTH,	// channel #224(E0)
	DEBUGVAR_CHANNELLENGTH,	// channel #225(E1)
	DEBUGVAR_CHANNELLENGTH,	// channel #226(E2)
	DEBUGVAR_CHANNELLENGTH,	// channel #227(E3)
	DEBUGVAR_CHANNELLENGTH,	// channel #228(E4)
	DEBUGVAR_CHANNELLENGTH,	// channel #229(E5)
	DEBUGVAR_CHANNELLENGTH,	// channel #230(E6)
	DEBUGVAR_CHANNELLENGTH,	// channel #231(E7)
	0,	// channel #232(E8)
	0,	// channel #233(E9)
	0,	// channel #234(EA)
	0,	// channel #235(EB)
	0,	// channel #236(EC)
	0,	// channel #237(ED)
	0,	// channel #238(EE)
	0,	// channel #239(EF)
	DEBUGVAR_CHANNELLENGTH,	// channel #240(F0)
	DEBUGVAR_CHANNELLENGTH,	// channel #241(F1)
	DEBUGVAR_CHANNELLENGTH,	// channel #242(F2)
	DEBUGVAR_CHANNELLENGTH,	// channel #243(F3)
	DEBUGVAR_CHANNELLENGTH,	// channel #244(F4)
	DEBUGVAR_CHANNELLENGTH,	// channel #245(F5)
	DEBUGVAR_CHANNELLENGTH,	// channel #246(F6)
	DEBUGVAR_CHANNELLENGTH,	// channel #247(F7)
	0,	// channel #248(F8)
	0,	// channel #249(F9)
	0,	// channel #250(FA)
	0,	// channel #251(FB)
	0,	// channel #252(FC)
	0,	// channel #253(FD)
	0,	// channel #254(FE)
	0	// channel #255(FF)
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//										DVR SERIAL COMMAND
/////////////////////////////////////////////////////////////////////////////////////////////////////
void func0x67_HARDC_DVR_CMD (U16 iCheckSum)
{
	iNumReturnedFloatChannels = 0;
	iNumberOfBytesReturned = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//									Functions for Standard Serial Decoder
///////////////////////////////////////////////////////////////////////////////////////////

F64 ExtractI32MsbFirst ( U8 *RawData, S32 Offset)
{

	F64 ret;

#ifndef C6000
		ret = (F64) ( (S32) ( ((U32)(RawData[Offset + 0])      <<24) + ((S32)(RawData[Offset + 1])<<16) + ((S32)(RawData[Offset + 2])<<8) + RawData[Offset + 3] ) );
		return ret;

#else
		ret = (F64) ( ((S32)(RawData[Offset + 0] & 0x7F)<<24) + ((S32)(RawData[Offset + 1])<<16) + ((S32)(RawData[Offset + 2])<<8) + RawData[Offset + 3] );

		if ((RawData[Offset + 0] & 0x80) != 0)
			return (-(1L<<31) + ret);
		else
			return ret;
#endif
}


F64 ExtractU32MsbFirst ( U8 *RawData, S32 Offset){
	return (F64) ( ((S32)(RawData[Offset])<<24) + ((S32)(RawData[Offset + 1])<<16) + ((S32)(RawData[Offset + 2])<<8) + RawData[Offset + 3] );
}


// ***** Functions called through pFuncArray  *****//


void func1_ENCSERIALHARDC ()
{

	iNumReturnedFloatChannels = 0;
	bBytesReturned = 0;

}


void func2_STARTSTOPHARDC ()
{

	S8 str[160];

	str[0]='\0';

	//GetStartStopInfo
	switch (serial_Buf[iRawSample+1])
	{
		case 1:
			strcpy(str," Logging started by button press. ");
			break;

		case 2:
			strcpy(str," Logging started automatically. ");

			switch (serial_Buf[iRawSample + 6]){

				case 1:	case 2:	case 3:	case 4:	case 5:	case 6:	case 7:	case 8:
					strcat(str, "Autostart based on analogue voltage. ");
					break;
				case 15:
					strcat(str, "Autostart based on lateral acceleration.");
					break;
				case 16:
					strcat(str, "Autostart based on longitudinal acceleration. ");
					break;
				default:
					break;
			}
			break;

		case 3:
			strcpy(str, " Logging started by pretrigger. ");
			break;

		default:
			break;
	}

	switch (serial_Buf[iRawSample + 2])
	{

		case 1:
			strcat(str, "Logging stopped by button press. ");
			break;

		case 2:
			strcat(str, "Logging stopped automatically. ");

			switch(serial_Buf[iRawSample + 7])
			{
#ifdef C6000
				case 1 - 8:
#else
				case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:
#endif
					strcat(str, "Autostop based on analogue voltage. ");

				case 15:
					strcat(str, "Autostop based on lateral acceleration.");

				case 16:
					strcat(str, "Autostop based on longitudinal acceleration. ");
				default:
					break;
			}
			break;

		case 3:
			strcat(str, "Logging stopped by posttrigger. ");
			break;

		case 4:
			strcat(str, "Logging stopped by low battery. ");
			break;

		case 5:
			strcat(str, "Logging stopped by slow CF card. ");
			break;

		case 6:
			strcat(str, "Logging stopped by GSM command. ");
			break;

		case 7:
			strcat(str, "Logging stopped by full CF card. ");
			break;

		default:
			break;
	}

	bBytesReturned = 1;
	iNumberOfBytesReturned = (U8) strlen(str) - 1;
	strcpy ((S8 *)bArrReturnedByteValues, str);
	iNumReturnedFloatChannels = 1;
	dArrReturnedFloatValues[0] = ((S32)(serial_Buf[iRawSample + 8])<<8) + serial_Buf[iRawSample + 9];
	iArrReturnedChannels[0] = BUFFERSIZE_;
}


void func3_GPSRAWDATA ()
{

	S32 ByteCounter;

    iNumReturnedFloatChannels = 0;
    bBytesReturned = 1;

    iNumberOfBytesReturned = serial_Buf[iRawSample + 1];

    for (ByteCounter = 0; ByteCounter < iNumberOfBytesReturned; ByteCounter++)
        bArrReturnedByteValues[ByteCounter] = serial_Buf[iRawSample + 2 + ByteCounter];
}


void func4_SECTORTIMEHARDC ()
{
	iNumReturnedFloatChannels = 1;
	dArrReturnedFloatValues[0] = 0;
	iArrReturnedChannels[0] = NODATA_;

	bBytesReturned = 0;
}


void func5_MARKERHARDC ()
{
	//this channel defines a new marker on the track
	iNumReturnedFloatChannels = 1;
	dArrReturnedFloatValues[0] = 0;
	iArrReturnedChannels[0] = NODATA_;
	bBytesReturned = 0;
}


void func6_LOGGERSERIALNUMBERC ()
{
	// Channel, serial number MSB? , serial number LSB?, software version,bootloader version, checksum.
	dArrReturnedFloatValues[0] = (S32)( (F64) ( ((S32)(serial_Buf[iRawSample+1] & 0x3F)<<8) + serial_Buf[iRawSample + 2] ) );  //int LoggerSerialNumber
	dArrReturnedFloatValues[1] = (S32)( serial_Buf[iRawSample + 3] );	//int FirmVersion
	dArrReturnedFloatValues[2] = (S32)( serial_Buf[iRawSample + 4] );	//int BootVersion

    iNumReturnedFloatChannels = 3;

    iArrReturnedChannels[0] = NODATA_;
    iArrReturnedChannels[1] = NODATA_;
    iArrReturnedChannels[2] = NODATA_;

    bBytesReturned = 0;
}


void func7_GPSTIMEMSWEEKC ()
{
	// AJD - no interpolation, is it required?
    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = ExtractU32MsbFirst(serial_Buf, iRawSample + 1);

    iArrReturnedChannels[0] = GPSMSTIME_;
    bBytesReturned = 0;
}


void func8_ACCELSHARDC ()
{

	F32 LongAccel, LatAccel;

	LatAccel =(F32)( (F64)(serial_Buf[iRawSample+1] & 0x7F) + ( (F64)(serial_Buf[iRawSample + 2]) / 0x100 ));

	if (!(serial_Buf[iRawSample+1] & 0x80))
		LatAccel = -(LatAccel);

	LongAccel =(F32)( (F64)(serial_Buf[iRawSample + 3] & 0x7F) + ( (F64)(serial_Buf[iRawSample + 4]) / 0x100 ));

	if (!(serial_Buf[iRawSample + 3] & 0x80))
		LongAccel = -(LongAccel);

    iNumReturnedFloatChannels = 2;
    dArrReturnedFloatValues[0] = LongAccel;
    dArrReturnedFloatValues[1] = LatAccel;
    iArrReturnedChannels[0] = ACCELLONG_;
    iArrReturnedChannels[1] = ACCELLAT_;
    bBytesReturned = 0;
}

void func9_TIMEHARDC ()
{
	S32 NewTimeIndex;

	NewTimeIndex = ((S32)(serial_Buf[iRawSample+1])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+3];

    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = NewTimeIndex / 100;
    iArrReturnedChannels[0] = TIMES_;

    bBytesReturned = 0;
}


void func10_GPSPOSHARDC ()
{
	F32 PosAcc;

	dArrReturnedFloatValues[0] = ExtractI32MsbFirst(serial_Buf, iRawSample+1) * 0.0000001;		//double Longitude
	dArrReturnedFloatValues[1] = ExtractI32MsbFirst(serial_Buf, iRawSample+5) * 0.0000001;		//double Latitude
	PosAcc = ExtractU32MsbFirst(serial_Buf, iRawSample+9) / 1000;		
	dArrReturnedFloatValues[2] = PosAcc;
    iNumReturnedFloatChannels = 3;
    iArrReturnedChannels[0] = POSXM_;
    iArrReturnedChannels[1] = POSXM_;
    iArrReturnedChannels[2] = GPSPOSITIONACC_;
    bBytesReturned = 0;
}


void func11_GPSSPEEDHARDC ()
{
	F32 GpsSpeedOutput, GpsSpeedAccuracyOutput;

	GpsSpeedOutput = ExtractU32MsbFirst(serial_Buf, iRawSample+1) / 100;
	GpsSpeedAccuracyOutput = ExtractU32MsbFirst(serial_Buf, iRawSample+5) / 100;

    iNumReturnedFloatChannels = 2;

	if ( GpsSpeedAccuracyOutput < 2)
        dArrReturnedFloatValues[0] = GpsSpeedOutput * 3.6;  //the factor of 3.6 goes from m/s to kph
    else
        dArrReturnedFloatValues[0] = -1;

    dArrReturnedFloatValues[1] = GpsSpeedAccuracyOutput;
    iArrReturnedChannels[0] = GPSSPEED_;
    iArrReturnedChannels[1] = GPSSPEEDACC_;

    bBytesReturned = 0;
}


void func12_BEACONHARDC ()
{

	F32 BeaconOutput;

	if (serial_Buf[iRawSample+1] == 1)
		BeaconOutput = 1;
	else
		BeaconOutput = 0;

    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = BeaconOutput;
    iArrReturnedChannels[0] = TRACKBEACON_;

    bBytesReturned = 0;
}

void func13_GPSPULSEHARDC ()
{

	F32 DigOutput;

	if (serial_Buf[iRawSample+1] == 1)
		DigOutput = 1;
	else
		DigOutput = 0;

    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = DigOutput;
	iArrReturnedChannels[0] = NODATA_;
    bBytesReturned = 0;
}


void func14_FREQ0HARDC ()
{

	F32 Frequency;

	Frequency = (F64) ( ((S32)(serial_Buf[iRawSample+1])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+3] );
	Frequency = Frequency * TICKPERIOD;
	if (Frequency > 0)
		Frequency = 1 / Frequency;

    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = Frequency;

    switch (iHardwareChannel)
	{
		case FREQ0HARDC:		
			iArrReturnedChannels[0]= DL1_FREQ2_;
			break;
		case FREQ1HARDC:
			iArrReturnedChannels[0]= DL1_FREQ3_;
			break;
		case FREQ2HARDC:
			iArrReturnedChannels[0]= DL1_FREQ4_;
			break;
		case FREQ3HARDC:
			iArrReturnedChannels[0]= DL1_FREQ1_;
			break;
		case RPMHARDC:
			iArrReturnedChannels[0]= RPM_;
			break;
		default:
			iArrReturnedChannels[0]= NODATA_;
			break;
	}
   bBytesReturned = 0;
}



void func19_SERIALDATAHARDC ()
{

	S32 ByteCounter;

    iNumReturnedFloatChannels = 0;
    bBytesReturned = 1;
    iNumberOfBytesReturned = serial_Buf[iRawSample + 1];

	for (ByteCounter = 0; ByteCounter < iNumberOfBytesReturned; ByteCounter++)
          bArrReturnedByteValues[iNumberOfBytesReturned - ByteCounter - 1] = serial_Buf[iRawSample+2 + ByteCounter];
}



void func20_ADC0HARDC ()
{

	F32 AnalogueOutput;

	AnalogueOutput= (F64)(((S32)(serial_Buf[iRawSample+1])<<8) + serial_Buf[iRawSample+2]) / 1000.0;

	iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = AnalogueOutput;

	switch ( serial_Buf[iRawSample] )
	{
		case ADC0HARDC:
			iArrReturnedChannels[0] = DL1_ANAL7_;
			break;
		case ADC1HARDC:
			iArrReturnedChannels[0] = DL1_ANAL5_;
			break;
		case ADC2HARDC:
			iArrReturnedChannels[0] = DL1_ANAL6_;
			break;
		case ADC3HARDC:
			iArrReturnedChannels[0] = DL1_ANAL4_;
			break;
		case ADC4HARDC:
			iArrReturnedChannels[0] = DL1_ANAL3_;
			break;
		case ADC5HARDC:
			iArrReturnedChannels[0] = DL1_ANAL1_;
			break;
		case ADC6HARDC:
			iArrReturnedChannels[0] = DL1_ANAL2_;
			break;
		case ADC7HARDC:
			iArrReturnedChannels[0] = DL1_ANAL0_;
			break;
		case ADC8HARDC:
			iArrReturnedChannels[0] = DL1_ANAL15_;
			break;
		case ADC9HARDC:
			iArrReturnedChannels[0] = DL1_ANAL13_;
			break;
		case ADC10HARDC:
			iArrReturnedChannels[0] = DL1_ANAL14_;
			break;
		case ADC11HARDC:
			iArrReturnedChannels[0] = DL1_ANAL12_;
			break;
		case ADC12HARDC:
			iArrReturnedChannels[0] = DL1_ANAL11_;
			break;
		case ADC13HARDC:
			iArrReturnedChannels[0] = DL1_ANAL9_;
			break;
		case ADC14HARDC:
			iArrReturnedChannels[0] = DL1_ANAL10_;
			break;
		case ADC15HARDC:
			iArrReturnedChannels[0] = DL1_ANAL8_;
			break;
		case ADC16HARDC:
			iArrReturnedChannels[0] = DL1_ANAL16_;
			break;
		case ADC17HARDC:
			iArrReturnedChannels[0] = DL1_ANAL17_;
			break;
		case ADC18HARDC:
			iArrReturnedChannels[0] = DL1_ANAL18_;
			break;
		case ADC19HARDC:
			iArrReturnedChannels[0] = DL1_ANAL19_;
			break;
		case ADC20HARDC:
			iArrReturnedChannels[0] = DL1_ANAL20_;
			break;
		case ADC21HARDC:
			iArrReturnedChannels[0] = DL1_ANAL21_;
			break;
		case ADC22HARDC:
			iArrReturnedChannels[0] = DL1_ANAL22_;
			break;
		case ADC23HARDC:
			iArrReturnedChannels[0] = DL1_ANAL23_;
			break;
		case ADC24HARDC:
			iArrReturnedChannels[0] = DL1_ANAL24_;
			break;
		case ADC25HARDC:
			iArrReturnedChannels[0] = DL1_ANAL25_;
			break;
		case ADC26HARDC:
			iArrReturnedChannels[0] = DL1_ANAL26_;
			break;
		case ADC27HARDC:
			iArrReturnedChannels[0] = DL1_ANAL27_;
			break;
		case ADC28HARDC:
			iArrReturnedChannels[0] = DL1_ANAL28_;
			break;
		case ADC29HARDC:
			iArrReturnedChannels[0] = DL1_ANAL29_;
			break;
		case ADC30HARDC:
			iArrReturnedChannels[0] = DL1_ANAL30_;
			break;
		case ADC31HARDC:
			iArrReturnedChannels[0] = DL1_ANAL31_;
			break;
		default:
			iArrReturnedChannels[0] = NODATA_;
			break;
	}
    bBytesReturned = 0;
}

void func55_GPSDATEHARDC ()
{
    iNumReturnedFloatChannels = 6;

	dArrReturnedFloatValues[0] = serial_Buf[iRawSample];
    dArrReturnedFloatValues[1] = serial_Buf[iRawSample+1];
    dArrReturnedFloatValues[2] = serial_Buf[iRawSample+2];
    dArrReturnedFloatValues[3] = serial_Buf[iRawSample+3];
    dArrReturnedFloatValues[4] = serial_Buf[iRawSample+4];
    dArrReturnedFloatValues[5] = ((S32)(serial_Buf[iRawSample+5])<<8) + serial_Buf[iRawSample+6];
    iArrReturnedChannels[0] = NODATA_;
    iArrReturnedChannels[1] = NODATA_;
    iArrReturnedChannels[2] = NODATA_;
    iArrReturnedChannels[3] = NODATA_;
    iArrReturnedChannels[4] = NODATA_;
    iArrReturnedChannels[5] = NODATA_;

    bBytesReturned = 0;
}



void func56_GPSHEADINGHARDC ()
{
	F32 Heading, HeadAcc;

	Heading = ExtractI32MsbFirst(serial_Buf, iRawSample+1) * 0.00001 / 180 * PI;			// convert from ublox units to rads
	HeadAcc = ExtractU32MsbFirst(serial_Buf, iRawSample+5) * 0.00001 / 180 * PI;	// convert from ublox units to rads
    iNumReturnedFloatChannels = 2;
    dArrReturnedFloatValues[0] = Heading;
    dArrReturnedFloatValues[1] = HeadAcc;
    iArrReturnedChannels[0] = GPSHEADING_;
    iArrReturnedChannels[1] = GPSHEADINGACC_;

    bBytesReturned = 0;
}



void func57_GPSALTHARDC ()
{

	F32 Altitude, AltAcc;

	Altitude = ExtractI32MsbFirst (serial_Buf, iRawSample+1) / 1000;			// convert from mm to m
	AltAcc = ExtractI32MsbFirst (serial_Buf, iRawSample+5) / 1000;	// convert from mm to m

    iNumReturnedFloatChannels = 2;
    dArrReturnedFloatValues[0] = Altitude;
    dArrReturnedFloatValues[1] = AltAcc;
    iArrReturnedChannels[0] = GPSALTITUDE_;
    iArrReturnedChannels[1] = GPSALTACC_;

    bBytesReturned = 0;
}


void func58_EXTENDED_FREQ0HARDC ()
{

	F32 HighPeriod, LowPeriod, PulsePosition;

	PulsePosition	= (F64) ( ((S32)(serial_Buf[iRawSample+1])<<16)	+ ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+3] ) * TICKPERIOD;
	LowPeriod		= (F64) ( ((S32)(serial_Buf[iRawSample+4])<<16)	+ ((S32)(serial_Buf[iRawSample+5])<<8) + serial_Buf[iRawSample+6] ) * TICKPERIOD;
	HighPeriod		= (F64) ( ((S32)(serial_Buf[iRawSample+7])<<16)	+ ((S32)(serial_Buf[iRawSample+8])<<8) + serial_Buf[iRawSample+9] ) * TICKPERIOD;

	iNumReturnedFloatChannels = 3;
	dArrReturnedFloatValues[0] = HighPeriod;
    dArrReturnedFloatValues[1] = LowPeriod;
    dArrReturnedFloatValues[2] = PulsePosition;

	switch ( serial_Buf[iRawSample] )
	{
		case EXTENDED_FREQ0HARDC:
			iArrReturnedChannels[0] = FREQ2_HIGHPERIOD_;
			iArrReturnedChannels[1] = FREQ2_LOWPERIOD_;
			iArrReturnedChannels[2] = FREQ2_PULSEPOSITION_;
			break;

		case EXTENDED_FREQ1HARDC:
			iArrReturnedChannels[0] = FREQ3_HIGHPERIOD_;
			iArrReturnedChannels[1] = FREQ3_LOWPERIOD_;
			iArrReturnedChannels[2] = FREQ3_PULSEPOSITION_;
			break;

		case EXTENDED_FREQ2HARDC:
			iArrReturnedChannels[0] = FREQ4_HIGHPERIOD_;
			iArrReturnedChannels[1] = FREQ4_LOWPERIOD_;
			iArrReturnedChannels[2] = FREQ4_PULSEPOSITION_;
			break;

		case EXTENDED_FREQ3HARDC:
			iArrReturnedChannels[0] = FREQ1_HIGHPERIOD_;
			iArrReturnedChannels[1] = FREQ1_LOWPERIOD_;
			iArrReturnedChannels[2] = FREQ1_PULSEPOSITION_;
			break;

		case EXTENDED_RPMHARDC:
			//NB: Added by nuwan
			iArrReturnedChannels[0] = RPM_HIGHPERIOD_;
			iArrReturnedChannels[1] = RPM_LOWPERIOD_;
			iArrReturnedChannels[2] = RPM_PULSEPOSITION_;
			break;

		default:
			//NB: Added by nuwan
			iArrReturnedChannels[0] = NODATA_;
			iArrReturnedChannels[1] = NODATA_;
			iArrReturnedChannels[2] = NODATA_;
			break;
	}

    bBytesReturned = 0;
}

//Pointed to f_Nodata
void func63_HARDC_START_OF_RUN ()
{
    iNumReturnedFloatChannels = 1;

	if (serial_Buf[iRawSample+1] == 1)
		dArrReturnedFloatValues[0] = 1;
	else
		dArrReturnedFloatValues[0] = 0;

    iArrReturnedChannels[0] = NODATA_;
    bBytesReturned = 0;
}

void func64_PROCESSEDSPEEDHARDC ()
{

	F32 ProcessedSpeed;

	ProcessedSpeed = (F64) ( ((S32)(serial_Buf[iRawSample+1])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+3] );
	ProcessedSpeed *= 0.001379060159;
    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = ProcessedSpeed;
    iArrReturnedChannels[0] = GPSSPEED_;
    bBytesReturned = 0;
}

//Pointed to f_Nodata
void func65_HARDC_GEAR_SETUP_DATA ()
{
	U8 i;

	bBytesReturned = 0;
    iNumReturnedFloatChannels = 14; //7 gears each having Lower and Upper gear check values

	//Upper Gear (N+1) Check value = Data(N+3) + Data(N+4) x 2^8
	//Lower Gear (N+1) check value = Data(N+1) + Data(N+2) x 2^8

	for(i=0;i<14;i++) {
		dArrReturnedFloatValues[i] = (F64) ( ((U16) serial_Buf[iRawSample+1+2*i])
			| (((U16) serial_Buf[iRawSample+2+2*i])<<8) );
		iArrReturnedChannels[i] = NODATA_;
	}
}

//Pointed to f_Nodata
void func67_HARDC_DASHBOARD_SETUP_DATA ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 2;

	dArrReturnedFloatValues[0] = (F64) serial_Buf[iRawSample+1]; //Warning hold time in secs
	iArrReturnedChannels[0] = NODATA_;

	dArrReturnedFloatValues[1] = (F64) serial_Buf[iRawSample+2]; //Sector time hold time in secs
	iArrReturnedChannels[1] = NODATA_;
}

//Pointed to f_Nodata
void func68_HARDC_DASHBOARD_SETUP_DATA_2 ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 2;

	dArrReturnedFloatValues[0] = (F64) serial_Buf[iRawSample+1]; //Highest RPM hold time (ms) 
	dArrReturnedFloatValues[0] = 40*dArrReturnedFloatValues[0];
	iArrReturnedChannels[0] = NODATA_;

	dArrReturnedFloatValues[1] = (F64) serial_Buf[iRawSample+2]; //RPM reduction rate (per 40ms) 
	iArrReturnedChannels[1] = NODATA_;
}

//Pointed to f_Nodata
void func69_HARDC_NEW_TARGET_SECTOR_TIME ()
{
	U8 i;

	bBytesReturned = 0;
	iNumReturnedFloatChannels = 9; //Number of sectors

	//Sector N+1 target time (ms) = Data(4N+1) + Data(4N+2) x 2^8 + Data(4N+3) x 2^16 + Data(4N+4) x 2^24

	for(i=0;i<9;i++) {
		dArrReturnedFloatValues[i] = (F64) ( ((U32) serial_Buf[iRawSample+1+4*i])
			| (((U32) serial_Buf[iRawSample+2+4*i])<<8)
			| (((U32) serial_Buf[iRawSample+3+4*i])<<16)
			| (((U32) serial_Buf[iRawSample+4+4*i])<<24) );
		iArrReturnedChannels[i] = NODATA_;
	}
}

//Pointed to f_Nodata
void func70_HARDC_NEW_TARGET_MARKER_TIME ()
{
	U8 i;

	bBytesReturned = 0;
	iNumReturnedFloatChannels = 9; //Number of markers

	//Marker N+1 target time (ms) = Data(4N+1) + Data(4N+2) x 2^8 + Data(4N+3) x 2^16 + Data(4N+4) x 2^24

	for(i=0;i<9;i++) {
		dArrReturnedFloatValues[i] = (F64) ( ((U32) serial_Buf[iRawSample+1+4*i])
			| (((U32) serial_Buf[iRawSample+2+4*i])<<8)
			| (((U32) serial_Buf[iRawSample+3+4*i])<<16)
			| (((U32) serial_Buf[iRawSample+4+4*i])<<24) );
		iArrReturnedChannels[i] = NODATA_;
	}
}

void func71_HARDC_ECU_MODULETYPE ()
{
    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = 0;
    iArrReturnedChannels[0] = NODATA_;

    bBytesReturned = 0;
}


void func72_HARDC_ECU_TEMP ()
{

#ifndef C6000
	dArrReturnedFloatValues[0] = (S32) ( ((S32)(serial_Buf[iRawSample+3])         <<8) + serial_Buf[iRawSample+2] );
#else
	dArrReturnedFloatValues[0] =		 ((S32)(serial_Buf[iRawSample+3] & 0x107F)<<8) + serial_Buf[iRawSample+2];

	if ((serial_Buf[iRawSample+3] & 0x80) != 0)
		dArrReturnedFloatValues[0] -= 32768;
#endif
	dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] * 0.1;
    iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_TEMP_ - 1;

    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}


void func73_HARDC_ECU_FREQ ()
{
	dArrReturnedFloatValues[0] = ((S32)(serial_Buf[iRawSample+3])<<8) + serial_Buf[iRawSample+2];
    dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] * 0.1;
    iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_FREQ_ - 1;
    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}



void func74_HARDC_ECU_PERC ()
{
	dArrReturnedFloatValues[0] = ((S32)(serial_Buf[iRawSample+3])<<8) + serial_Buf[iRawSample+2];
	dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] * 0.1;
	iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_PERC_ - 1;

	iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;
}

void func75_HARDC_ECU_TIME ()
{
    iArrReturnedChannels[0] = serial_Buf[iRawSample+2] + ECU_FIRST_TIME_ - 1;

	iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;

    if (serial_Buf[iRawSample+2] < 20 && (S8)serial_Buf[iRawSample+2] > -20)
		dArrReturnedFloatValues[0] = ( (serial_Buf[iRawSample+4] + serial_Buf[iRawSample+3]) * pow(10,serial_Buf[iRawSample+2]) );
    else
        dArrReturnedFloatValues[0] = 0;

    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}

//Pointed to f_Nodata
void func76_HARDC_NEW_LCD_DATA ()
{
	U8 i;

	bBytesReturned = 1;
	iNumReturnedFloatChannels = 1;

	dArrReturnedFloatValues[0] = (F64) serial_Buf[iRawSample+1]; //Display location
	iArrReturnedChannels[0] = NODATA_;

	iNumberOfBytesReturned = (serial_Buf[iRawSample+2]>20)?20:serial_Buf[iRawSample+2]; //Number of characters

	for(i=0;i<iNumberOfBytesReturned;i++) {
		bArrReturnedByteValues[i] = serial_Buf[iRawSample+3+i];
	}
}

//Pointed to f_Nodata
void func77_HARDC_NEW_LED_DATA ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 1;

	//this is an 8 bit data value signifying if given LED is on or off
	dArrReturnedFloatValues[0] = (F64) serial_Buf[iRawSample+1];
	iArrReturnedChannels[0] = NODATA_;
}


void func78_PROCESSEDDISTANCEHARDC ()
{

	F32 ProcessedDistance;

	ProcessedDistance = (F32) ( ((S32)(serial_Buf[iRawSample+1])<<24) + ((S32)(serial_Buf[iRawSample+2])<<16) + ((S32)(serial_Buf[+3])<<8) + serial_Buf[iRawSample+4] );
	ProcessedDistance /= 1000000;	// convert from mm to km
    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = (S32)ProcessedDistance;
    iArrReturnedChannels[0] = DISTANCE_;
    bBytesReturned = 0;
}

void func79_YAWGYROHARDC ()
{
	F32 YawRate;

	YawRate = (F32) ( ((S32)(serial_Buf[iRawSample+1])<<8) + serial_Buf[iRawSample+2] );
	YawRate = -((YawRate - 32768) / 100 / 180 * PI);
	iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = YawRate;
    iArrReturnedChannels[0] = GYROYAWRATE_;
    bBytesReturned = 0;
}

void func80_HARDC_GPSYAW ()
{
	dArrReturnedFloatValues[0] = ( (F64) ( ( ( (U16) (0x7F & serial_Buf[iRawSample+1]) ) << 8 )
		 + serial_Buf[iRawSample+2] ) )/100;

	if(serial_Buf[iRawSample+1] & (1<<7))
		dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] - ( ((F64) 256) * 128)/100;

    dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] / 180 * PI;

	iNumReturnedFloatChannels = 1;
    iArrReturnedChannels[0] = GPSRTKYAW_;
    bBytesReturned = 0;
}

void func81_HARDC_PITCH_RATE ()
{
	dArrReturnedFloatValues[0] = (F64) ( ( ((U16) serial_Buf[iRawSample+1]) << 8 ) + 
				( (U16) serial_Buf[iRawSample+2]) );
	dArrReturnedFloatValues[0] = -(dArrReturnedFloatValues[0] - 32768 )*PI/(100*180);
	iArrReturnedChannels[0] = GYROPITCHRATE_;
    iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;
}

void func82_HARDC_PITCH ()
{
	dArrReturnedFloatValues[0] = ( (F64) ( ( ( (U16) (0x7F & serial_Buf[iRawSample+1]) ) << 8 )
		 + serial_Buf[iRawSample+2] ) )/100;

	if(serial_Buf[iRawSample+1] & (1<<7))
		dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] - ( ((F64) 256) * 128)/100;

    dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0] / 180 * PI;

	iNumReturnedFloatChannels = 1;
	iArrReturnedChannels[0] = GPSRTKPITCH_;
	bBytesReturned = 0;
}

void func83_HARDC_ROLL_RATE ()
{
	dArrReturnedFloatValues[0] = (F64) ( ( ((U16) serial_Buf[iRawSample+1]) << 8 ) + 
				( (U16) serial_Buf[iRawSample+2]) );
	dArrReturnedFloatValues[0] = -(dArrReturnedFloatValues[0] - 32768 )*PI/(100*180);
	iArrReturnedChannels[0] = GYROROLLRATE_;
    iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;
}

void func84_HARDC_ROLL_ANGLE ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 2;

	dArrReturnedFloatValues[0] = (F64) ( (((U16) serial_Buf[iRawSample+1])<<8) | ((U16) serial_Buf[iRawSample+2]) );
	dArrReturnedFloatValues[0] = dArrReturnedFloatValues[0]/100;
	iArrReturnedChannels[0] = GPSROLL_;

}

void func85_GPSGRADIENTHARDC ()
{

	F32 Gradient, GradientAcc;
	F64 Lower = -PI;
	F64 Upper = PI;

	// this was added to support the brakebox

	Gradient = ExtractI32MsbFirst(serial_Buf, iRawSample+1) * 0.00001 / 180 * PI;		// convert from ublox units to rads
	GradientAcc = ExtractU32MsbFirst(serial_Buf, iRawSample+5) * 0.00001 / 180 * PI;	// convert from ublox units to rads

	// rescale so it goes from -pi to pi, instead of 0 to 2*pi
	//Constrain(Gradient, -PI, PI);
	if (Lower > Upper)
		Gradient = Lower;
	else
	{
		Gradient -= ((S32)(Gradient / (Upper - Lower)) * (Upper - Lower));

		while (Gradient < Lower)
			Gradient += (Upper - Lower);

		while (Gradient > Upper)
			Gradient -= (Upper - Lower);
	}

    iNumReturnedFloatChannels = 2;
    dArrReturnedFloatValues[0] = Gradient;
    dArrReturnedFloatValues[1] = GradientAcc;
    iArrReturnedChannels[0] = GPSGRADIENT_;
    iArrReturnedChannels[1] = GPSGRADIENTACC_;
    bBytesReturned = 0;
}

void func86_HARDC_PULSE0 ()
{
	/*dArrReturnedFloatValues[0] = ( (F64) serial_Buf[iRawSample+1] ) * (65536) + 
		( (F64) serial_Buf[iRawSample+2] ) * (256) + ( (F64) serial_Buf[iRawSample+3] );*/

	dArrReturnedFloatValues[0] = (F64) ( ((U32) serial_Buf[iRawSample+1])<<16 |
		((U32) serial_Buf[iRawSample+2])<<8 | ((U32) serial_Buf[iRawSample+3]) );

	iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;

	switch (iHardwareChannel)
	{
		case HARDC_PULSE0:
			iArrReturnedChannels[0] = FREQ2_PULSE_COUNT_;
			break;
		case HARDC_PULSE1:
			iArrReturnedChannels[0] = FREQ3_PULSE_COUNT_;
			break;
		case HARDC_PULSE2:
			iArrReturnedChannels[0] = FREQ4_PULSE_COUNT_;
			break;
		case HARDC_PULSE3:
			iArrReturnedChannels[0] = FREQ1_PULSE_COUNT_;
			break;
		default:
			iArrReturnedChannels[0] = NODATA_;
			break;
	}
}

void func90_HARDC_BASELINERTK ()
{
	iArrReturnedChannels[0] = GPSRTKBASELINE_;
	dArrReturnedFloatValues[0] = ( (F64) ( (((U16) serial_Buf[iRawSample+1]) << 8) + serial_Buf[iRawSample+2] ) )/1000;

	iArrReturnedChannels[1] = GPSRTKBASELINEACC_;
	dArrReturnedFloatValues[1] = ( (F64) ( (((U16) serial_Buf[iRawSample+3]) << 8) + serial_Buf[iRawSample+4] ) )/10000;

    iNumReturnedFloatChannels = 2;
	bBytesReturned = 0;
}

//Pointed to f_Nodata
void func91_HARDC_UNIT_CONTROL ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 2;

	dArrReturnedFloatValues[0] = (F64) ( (((U16) serial_Buf[iRawSample+1])<<8) | ((U16) serial_Buf[iRawSample+2]) );
	iArrReturnedChannels[0] = NODATA_;

	dArrReturnedFloatValues[0] = (F64) serial_Buf[iRawSample+3];
	iArrReturnedChannels[0] = NODATA_;
}

void func92_HARDC_Z_ACCEL ()
{
	F32	ZAccel;

	ZAccel = ((F64)(serial_Buf[iRawSample+1] & 0x7F) ) + ( (F64)(serial_Buf[iRawSample+2]) / 0x100);

	if (serial_Buf[iRawSample+1] & 0x80)		
		ZAccel = -ZAccel;

    iNumReturnedFloatChannels = 1;
    dArrReturnedFloatValues[0] = ZAccel;
	iArrReturnedChannels[0] = ACCEL_Z_;
    bBytesReturned = 0;
}


void func93_HARDC_ECU_ANGLE ()
{

	 dArrReturnedFloatValues[0] = ((S32)(serial_Buf[iRawSample+3])<<8) + serial_Buf[iRawSample+2];
     dArrReturnedFloatValues[0] = dArrReturnedFloatValues [0] * 0.1;
	 iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_ANGLE_ - 1;
     iNumReturnedFloatChannels = 1;
     bBytesReturned = 0;
}


void func94_HARDC_ECU_PRESSURE ()
{

	iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_PRESSURE_ - 1;
	iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;

	if( serial_Buf[iRawSample+2] < 20 && (S8)serial_Buf[iRawSample+2] > -20 )
		dArrReturnedFloatValues[0] = (serial_Buf[iRawSample+4] <<8 ) + serial_Buf[iRawSample+3] * 10 ^ serial_Buf[iRawSample+2];
	else
		dArrReturnedFloatValues[0] = 0;

	iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;
}

void func95_HARDC_ECU_MISC ()
{
	dArrReturnedFloatValues[0] = (serial_Buf[iRawSample+3] <<8 ) + serial_Buf[iRawSample+2];
	iArrReturnedChannels[0] = serial_Buf[iRawSample+1] + ECU_FIRST_MISC_ - 1;
	iNumReturnedFloatChannels = 1;
	bBytesReturned = 0;
}

//Pointed to f_Nodata
void func0x65_HARDC_SECTOR_DEFINITION ()
{
	U8 i;

	bBytesReturned = 1;
	iNumReturnedFloatChannels = 1;

	dArrReturnedFloatValues[0] = serial_Buf[iRawSample+1];
	iArrReturnedChannels[0] = NODATA_;

	iNumberOfBytesReturned = 16;
	for(i=0;i<16;i++) {
		bArrReturnedByteValues[i] = serial_Buf[iRawSample+2+i];
	}
}

//Pointed to f_Nodata
void func0x68_HARDC_VIDEO_FRAME_INDEX ()
{
	bBytesReturned = 0;
	iNumReturnedFloatChannels = 1;

	dArrReturnedFloatValues[0] = (F64) ( (((U32) serial_Buf[iRawSample+1])<<24)
		| (((U32) serial_Buf[iRawSample+2])<<16)
		| (((U32) serial_Buf[iRawSample+3])<<8)
		| ((U32) serial_Buf[iRawSample+4]) );
	iArrReturnedChannels[0] = NODATA_;
}

void func0xD0_HARDC_DEBUG_VAR_FP_0 ()
{
	S32 c;
	U8 b[4];
    U8 d[8];
    U8 s;
	F64 Answer;

	s = serial_Buf[iRawSample+1] & 0x80;

	b[0] = serial_Buf[iRawSample+1];
	b[1] = serial_Buf[iRawSample+2];
	b[2] = serial_Buf[iRawSample+3];
	b[3] = serial_Buf[iRawSample+4];

	d[6] = (b[0] & 0x78)>>3;

	d[5] = ((S32)(b[0] & 0x7)<<5) + (S32)((b[1] & 0xF8)>>3);
	d[4] = ((S32)(b[1] & 0x7)<<5) + (S32)((b[2] & 0xF8)>>3);
	d[3] = ((S32)(b[2] & 0x7)<<5);

	c = b[3] + 896;
	d[6] = d[6] + ((S32)(c & 0xF)<<4);

	if (s)	d[7] = 0x80;

	d[7] += c>>4;

	//contruct short arrays
#ifdef C2000
	d[0] = (d[0] << 8) | d[1];
	d[1] = (d[2] << 8) | d[3];
	d[2] = (d[4] << 8) | d[5];
	d[3] = (d[6] << 8) | d[7];
	memmove (&Answer, &d, 4);
#else
	memmove (&Answer, &d, 8);
#endif
	dArrReturnedFloatValues[0] = Answer;
	iArrReturnedChannels[0] = serial_Buf[iRawSample] - HARDC_DEBUG_VAR_FP_0 + DEBUG_VAR_FP_0_;
    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}


void func0xE0_HARDC_DEBUG_VAR_U32_0 ()
{
	dArrReturnedFloatValues[0]  =  (F64) ( ((S32)(serial_Buf[iRawSample+4])<<24) + ((S32)(serial_Buf[iRawSample+3])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+1] );
	iArrReturnedChannels[0] = serial_Buf[iRawSample] - HARDC_DEBUG_VAR_U32_0 + DEBUG_VAR_U32_0_;

    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}



void func0xF0_HARDC_DEBUG_VAR_S32_0 ()
{

	F64 result;

#ifdef C6000
	result = (F64) ( ((S32)(serial_Buf[iRawSample+4] & 0x7F)<<24) + ((S32)(serial_Buf[iRawSample+3])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+1] );
#else	
	result = (F64) ((S32) ( ((S32)(serial_Buf[iRawSample+4]) <<24) + ((S32)(serial_Buf[iRawSample+3])<<16) + ((S32)(serial_Buf[iRawSample+2])<<8) + serial_Buf[iRawSample+1] ) );
#endif
	if ((serial_Buf[iRawSample+4] & 0x80) != 0) {
		//dArrReturnedFloatValues[0] = (-(1U<<31) + result);
		dArrReturnedFloatValues[0] = -(2^31) + result; // ??
	} else {
		dArrReturnedFloatValues[0] = result;
	}

	iArrReturnedChannels[0] = serial_Buf[iRawSample] - HARDC_DEBUG_VAR_S32_0 + DEBUG_VAR_S32_0_;
    iNumReturnedFloatChannels = 1;
    bBytesReturned = 0;
}

void f_NoData () {
	iNumReturnedFloatChannels = 0;
	iNumberOfBytesReturned = 0;
}

//This func is nver called
void f_NotDef ()
{

}

// ***** Array of ponters to functions *****//
CHPF pFuncArray[ARRAY_LEN] = {
	f_NotDef,					// channel #0(0)
	func1_ENCSERIALHARDC,		// channel #1(1)
	func2_STARTSTOPHARDC,		// channel #2(2)
	func3_GPSRAWDATA,			// channel #3(3)
	func4_SECTORTIMEHARDC,		// channel #4(4)
	func5_MARKERHARDC,			// channel #5(5)
	func6_LOGGERSERIALNUMBERC,	// channel #6(6)
	func7_GPSTIMEMSWEEKC,		// channel #7(7)
	func8_ACCELSHARDC,			// channel #8(8)
	func9_TIMEHARDC,			// channel #9(9)
	func10_GPSPOSHARDC,			// channel #10(A)
	func11_GPSSPEEDHARDC,		// channel #11(B)
	func12_BEACONHARDC,			// channel #12(C)
	func13_GPSPULSEHARDC,		// channel #13(D)
	func14_FREQ0HARDC,			// channel #14(E)
	func14_FREQ0HARDC,			// channel #15(F)
	func14_FREQ0HARDC,			// channel #16(10)
	func14_FREQ0HARDC,			// channel #17(11)
	func14_FREQ0HARDC,			// channel #18(12)
	func19_SERIALDATAHARDC,		// channel #19(13)
	func20_ADC0HARDC,			// channel #20(14)
	func20_ADC0HARDC,			// channel #21(15)
	func20_ADC0HARDC,			// channel #22(16)
	func20_ADC0HARDC,			// channel #23(17)
	func20_ADC0HARDC,			// channel #24(18)
	func20_ADC0HARDC,			// channel #25(19)
	func20_ADC0HARDC,			// channel #26(1A)
	func20_ADC0HARDC,			// channel #27(1B)
	func20_ADC0HARDC,			// channel #28(1C)
	func20_ADC0HARDC,			// channel #29(1D)
	func20_ADC0HARDC,			// channel #30(1E)
	func20_ADC0HARDC,			// channel #31(1F)
	func20_ADC0HARDC,			// channel #32(20)
	func20_ADC0HARDC,			// channel #33(21)
	func20_ADC0HARDC,			// channel #34(22)
	func20_ADC0HARDC,			// channel #35(23)
	func20_ADC0HARDC,			// channel #36(24)
	func20_ADC0HARDC,			// channel #37(25)
	func20_ADC0HARDC,			// channel #38(26)
	func20_ADC0HARDC,			// channel #39(27)
	func20_ADC0HARDC,			// channel #40(28)
	func20_ADC0HARDC,			// channel #41(29)
	func20_ADC0HARDC,			// channel #42(2A)
	func20_ADC0HARDC,			// channel #43(2B)
	func20_ADC0HARDC,			// channel #44(2C)
	func20_ADC0HARDC,			// channel #45(2D)
	func20_ADC0HARDC,			// channel #46(2E)
	func20_ADC0HARDC,			// channel #47(2F)
	func20_ADC0HARDC,			// channel #48(30)
	func20_ADC0HARDC,			// channel #49(31)
	func20_ADC0HARDC,			// channel #50(32)
	func20_ADC0HARDC,			// channel #51(33)
	f_NoData,					// channel #52(34)
	f_NoData,					// channel #53(35)
	f_NoData,					// channel #54(36)
	func55_GPSDATEHARDC,		// channel #55(37)
	func56_GPSHEADINGHARDC,		// channel #56(38)
	func57_GPSALTHARDC,			// channel #57(39)
	func58_EXTENDED_FREQ0HARDC,	// channel #58(3A)
	func58_EXTENDED_FREQ0HARDC,	// channel #59(3B)
	func58_EXTENDED_FREQ0HARDC,	// channel #60(3C)
	func58_EXTENDED_FREQ0HARDC,	// channel #61(3D)
	func58_EXTENDED_FREQ0HARDC,	// channel #62(3E)
	f_NoData,					// channel #63(3F)
	func64_PROCESSEDSPEEDHARDC,	// channel #64(40)
	f_NoData,					// channel #65(41)
	f_NoData,					// channel #66(42)
	f_NoData,					// channel #67(43)
	f_NoData,					// channel #68(44)
	f_NoData,					// channel #69(45)
	f_NoData,					// channel #70(46)
	func71_HARDC_ECU_MODULETYPE,	// channel #71(47)
	func72_HARDC_ECU_TEMP,		// channel #72(48)
	func73_HARDC_ECU_FREQ,		// channel #73(49)
	func74_HARDC_ECU_PERC,		// channel #74(4A)
	func75_HARDC_ECU_TIME,		// channel #75(4B)
	f_NoData,					// channel #76(4C)
	f_NoData,					// channel #77(4D)
	func78_PROCESSEDDISTANCEHARDC,	// channel #78(4E)
	func79_YAWGYROHARDC,		// channel #79(4F)
	func80_HARDC_GPSYAW,		// channel #80(50)
	func81_HARDC_PITCH_RATE,	// channel #81(51)
	func82_HARDC_PITCH,			// channel #82(52)
	func83_HARDC_ROLL_RATE,		// channel #83(53)
	func84_HARDC_ROLL_ANGLE,	// channel #84(54)
	func85_GPSGRADIENTHARDC,	// channel #85(55)
	func86_HARDC_PULSE0,		// channel #86(56)
	func86_HARDC_PULSE0,		// channel #87(57)
	func86_HARDC_PULSE0,		// channel #88(58)
	func86_HARDC_PULSE0,		// channel #89(59)
	func90_HARDC_BASELINERTK,	// channel #90(5A)
	f_NoData,					// channel #91(5B)
	func92_HARDC_Z_ACCEL,		// channel #92(5C)
	func93_HARDC_ECU_ANGLE,		// channel #93(5D)
	func94_HARDC_ECU_PRESSURE,	// channel #94(5E)
	func95_HARDC_ECU_MISC,		// channel #95(5F)
	f_NotDef,					// channel #96(60)
	f_NotDef,					// channel #97(61)
	f_NotDef,					// channel #98(62)
	f_NotDef,					// channel #99(63)
	f_NotDef,					// channel #100(64)
	f_NoData,					// channel #101(65)
	f_NoData,					// channel #102(66)
	f_NoData,					// channel #103(67)
	f_NoData,					// channel #104(68)
	f_NotDef,					// channel #105(69)
	f_NotDef,					// channel #106(6A)
	f_NotDef,					// channel #107(6B)
	f_NotDef,					// channel #108(6C)
	f_NotDef,					// channel #109(6D)
	f_NotDef,					// channel #110(6E)
	f_NotDef,					// channel #111(6F)
	f_NotDef,					// channel #112(70)
	f_NotDef,					// channel #113(71)
	f_NotDef,					// channel #114(72)
	f_NotDef,					// channel #115(73)
	f_NotDef,					// channel #116(74)
	f_NotDef,					// channel #117(75)
	f_NotDef,					// channel #118(76)
	f_NotDef,					// channel #119(77)
	f_NotDef,					// channel #120(78)
	f_NotDef,					// channel #121(79)
	f_NotDef,					// channel #122(7A)
	f_NotDef,					// channel #123(7B)
	f_NotDef,					// channel #124(7C)
	f_NotDef,					// channel #125(7D)
	f_NotDef,					// channel #126(7E)
	f_NotDef,					// channel #127(7F)
	f_NotDef,					// channel #128(80)
	f_NotDef,					// channel #129(81)
	f_NotDef,					// channel #130(82)
	f_NotDef,					// channel #131(83)
	f_NotDef,					// channel #132(84)
	f_NotDef,					// channel #133(85)
	f_NotDef,					// channel #134(86)
	f_NotDef,					// channel #135(87)
	f_NotDef,					// channel #136(88)
	f_NotDef,					// channel #137(89)
	f_NotDef,					// channel #138(8A)
	f_NotDef,					// channel #139(8B)
	f_NotDef,					// channel #140(8C)
	f_NotDef,					// channel #141(8D)
	f_NotDef,					// channel #142(8E)
	f_NotDef,					// channel #143(8F)
	f_NotDef,					// channel #144(90)
	f_NotDef,					// channel #145(91)
	f_NotDef,					// channel #146(92)
	f_NotDef,					// channel #147(93)
	f_NotDef,					// channel #148(94)
	f_NotDef,					// channel #149(95)
	f_NotDef,					// channel #150(96)
	f_NotDef,					// channel #151(97)
	f_NotDef,					// channel #152(98)
	f_NotDef,					// channel #153(99)
	f_NotDef,					// channel #154(9A)
	f_NotDef,					// channel #155(9B)
	f_NotDef,					// channel #156(9C)
	f_NotDef,					// channel #157(9D)
	f_NotDef,					// channel #158(9E)
	f_NotDef,					// channel #159(9F)
	f_NotDef,					// channel #160(A0)
	f_NotDef,					// channel #161(A1)
	f_NotDef,					// channel #162(A2)
	f_NotDef,					// channel #163(A3)
	f_NotDef,					// channel #164(A4)
	f_NotDef,					// channel #165(A5)
	f_NotDef,					// channel #166(A6)
	f_NotDef,					// channel #167(A7)
	f_NotDef,					// channel #168(A8)
	f_NotDef,					// channel #169(A9)
	f_NotDef,					// channel #170(AA)
	f_NotDef,					// channel #171(AB)
	f_NotDef,					// channel #172(AC)
	f_NotDef,					// channel #173(AD)
	f_NotDef,					// channel #174(AE)
	f_NotDef,					// channel #175(AF)
	f_NotDef,					// channel #176(B0)
	f_NotDef,					// channel #177(B1)
	f_NotDef,					// channel #178(B2)
	f_NotDef,					// channel #179(B3)
	f_NotDef,					// channel #180(B4)
	f_NotDef,					// channel #181(B5)
	f_NotDef,					// channel #182(B6)
	f_NotDef,					// channel #183(B7)
	f_NotDef,					// channel #184(B8)
	f_NotDef,					// channel #185(B9)
	f_NotDef,					// channel #186(BA)
	f_NotDef,					// channel #187(BB)
	f_NotDef,					// channel #188(BC)
	f_NotDef,					// channel #189(BD)
	f_NotDef,					// channel #190(BE)
	f_NotDef,					// channel #191(BF)
	f_NotDef,					// channel #192(C0)
	f_NotDef,					// channel #193(C1)
	f_NotDef,					// channel #194(C2)
	f_NotDef,					// channel #195(C3)
	f_NotDef,					// channel #196(C4)
	f_NotDef,					// channel #197(C5)
	f_NotDef,					// channel #198(C6)
	f_NotDef,					// channel #199(C7)
	f_NotDef,					// channel #200(C8)
	f_NotDef,					// channel #201(C9)
	f_NotDef,					// channel #202(CA)
	f_NotDef,					// channel #203(CB)
	f_NotDef,					// channel #204(CC)
	f_NotDef,					// channel #205(CD)
	f_NotDef,					// channel #206(CE)
	f_NotDef,					// channel #207(CF)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #208(D0)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #209(D1)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #210(D2)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #211(D3)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #212(D4)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #213(D5)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #214(D6)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #215(D7)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #216(D8)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #217(D9)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #218(DA)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #219(DB)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #220(DC)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #221(DD)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #222(DE)
	func0xD0_HARDC_DEBUG_VAR_FP_0,	// channel #223(DF)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #224(E0)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #225(E1)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #226(E2)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #227(E3)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #228(E4)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #229(E5)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #230(E6)
	func0xE0_HARDC_DEBUG_VAR_U32_0,	// channel #231(E7)
	f_NotDef,						// channel #232(E8)
	f_NotDef,						// channel #233(E9)
	f_NotDef,						// channel #234(EA)
	f_NotDef,						// channel #235(EB)
	f_NotDef,						// channel #236(EC)
	f_NotDef,						// channel #237(ED)
	f_NotDef,						// channel #238(EE)
	f_NotDef,						// channel #239(EF)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #240(F0)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #241(F1)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #242(F2)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #243(F3)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #244(F4)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #245(F5)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #246(F6)
	func0xF0_HARDC_DEBUG_VAR_S32_0,	// channel #247(F7)
	f_NotDef,						// channel #248(F8)
	f_NotDef,						// channel #249(F9)
	f_NotDef,						// channel #250(FA)
	f_NotDef,						// channel #251(FB)
	f_NotDef,						// channel #252(FC)
	f_NotDef,						// channel #253(FD)
	f_NotDef,						// channel #254(FE)
	f_NotDef						// channel #255(FF)
};

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
//								Standard Serial Decoder
///////////////////////////////////////////////////////////////////////////////////////////

void serialDecode (U8 c) {
	U16 iLengthOfRawArray, iChecksum = 0, iCalculatedCheckSum;
	U8 i, bChecksumValid = FALSE;

	serial_Buf[iRawInput++] = c; //insert new data byte in to the serial_Buf[]
	iLengthOfRawArray = iRawInput - iRawSample; //get the number of new(unprocesses) data bytes received

	if(iRawInput >= MAX_MSG_LEN) { //prevent buffer overflow
		if (iLengthOfRawArray) {
			memmove(serial_Buf, serial_Buf+iRawSample, iLengthOfRawArray);
		}
		iRawInput = iLengthOfRawArray;
		iRawSample = 0;
	}


	while (iLengthOfRawArray>0) {
		// the decoder has just complted decoding a message
		// now we're about to start on a completely new packet

		if(!bDecoderRunning) {
			iHardwareChannel = serial_Buf[iRawSample]; //assume start of message current message
			iLengthOfMsg = MessagesLengths[iHardwareChannel]; //assuming the above, fetch the message length

			bDecoderRunning = TRUE; //start reading data

			if(iLengthOfMsg == 0) {
				// the message is an unknown type -- move on to the next byte
				bDecoderRunning = FALSE;
				if(iLengthOfRawArray>0) {
					iRawSample++;
				}
			} else if(iLengthOfMsg == MAX_MSG_LEN) {
				// the message has varibel length --- length is given by second byte of the message
				iLengthOfMsg = serial_Buf[iRawSample+1];
			}
		} else {
			// the decoder is running - i.e. we know the type and length of current message
			if(iLengthOfRawArray >= iLengthOfMsg) {
				//there is sufficient data in the buffer to do a checksum 
				//calculation and subsequent decoding of data

				//Checksum calculation
				if(iHardwareChannel == HARDC_DVR_CMD) {

					if ( (serial_Buf[iRawSample+1] == 0x13) && (serial_Buf[iRawSample+2] == 0x01) ){
						for (i = 0; i < (iLengthOfMsg-2); i++) {
							iChecksum = (iChecksum + serial_Buf[iRawSample + i]);
						}

						iCalculatedCheckSum = (serial_Buf[iRawSample+i]<<8) + serial_Buf[iRawSample+i+1];

						if(iChecksum == iCalculatedCheckSum) {
							func0x67_HARDC_DVR_CMD(iChecksum);

							bChecksumValid = TRUE;
							iRawSample += iLengthOfMsg;
							iLengthOfRawArray = 0;
						}
					}


				} else {

					for (i = 0; i < (iLengthOfMsg-1); i++) {
						iChecksum = 0xFF & (iChecksum + serial_Buf[iRawSample + i]);
					}

					if(iChecksum == serial_Buf[iRawSample+i]) {
						//data ok - calculated checksum and received checksum are equal

						//fprintf(file_out, "HW Channel=%d: ", iHardwareChannel);
						//printf("HW Channel=%d: ", iHardwareChannel);


						if(bReturnDataValues) {
							//decode data
							pFuncArray[iHardwareChannel]();

							/*for (i = 0; i < iNumReturnedFloatChannels; i++){
								fprintf(file_out, "%d=%f ", iArrReturnedChannels[i], dArrReturnedFloatValues[i]);
								printf("%d=%f ", iArrReturnedChannels[i], dArrReturnedFloatValues[i]);
							}
							printf("\r\n", iHardwareChannel);
							fprintf(file_out, "\r\n", iHardwareChannel);*/

						} //else {
							//don't decode data
						//}

						bChecksumValid = TRUE;
						iRawSample += iLengthOfMsg;
						iLengthOfRawArray = 0;
					}
				}

				if(!bChecksumValid) {
					//data corrupt
					if(iLengthOfRawArray>0)
						iRawSample++;				
				}

				bDecoderRunning = FALSE;

			} else {
				//there is insufficient data in the buffer -- fetch more data
				break;
			}
		}
		iLengthOfRawArray = iRawInput - iRawSample; //get the number of unprocessesed data bytes in the buffer
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//									End Standard Serial Decoder
///////////////////////////////////////////////////////////////////////////////////////////

#ifdef PC
void open_log(char *f_name)
{
	file_out = fopen(f_name, "w+");
}

void close_log(void)
{
	fclose(file_out);
}
#endif
