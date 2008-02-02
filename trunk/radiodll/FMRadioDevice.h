// FMRadioDevice.h: interface for the CFMRadioDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FMRADIODEVICE_H__6F0D8129_C34F_4CBC_93A0_EC3F2F68A8D1__INCLUDED_)
#define AFX_FMRADIODEVICE_H__6F0D8129_C34F_4CBC_93A0_EC3F2F68A8D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT 0x0500

#include <windows.h>


//ULONG/DWORD pointer types defined for DDK if not already
#ifndef ULONG_PTR
#define ULONG_PTR	ULONG
#endif

#ifndef DWORD_PTR
#define DWORD_PTR	DWORD
#endif

#ifndef DDK_SERVER_2003_DDK
//#pragma message ("Using Windows Server 2003 DDK")
// Use the setupapi.lib file that comes with the compiler
// (even though the DDK also has a setupapi.lib library)
#pragma comment (lib, "setupapi.lib")
// Use the hid.lib file from the appropriate DDK directory
#pragma comment (lib, "hid.lib")
#pragma comment (lib, "kernel32.lib")
#endif

#include "/winddk/3790.1830/inc/wxp/initguid.h"

#define MAX_LOADSTRING 256

extern "C" {
#include "/winddk/3790.1830/inc/wxp/hidsdi.h"
}
#include "/winddk/3790.1830/inc/wxp/setupapi.h"

#include <dbt.h>

//Multimedia header
#include "mmsystem.h"

#include "RDSData.h"

//Max number of USB Devices allowed
#define MAX_USB_DEVICES	64

#define FMTUNERDATA_REPORT_SIZE	61

//Device data
#define SILABS_VID	0x10C4
#define SILABS_PID	0x818A
#define PCEAR_VID	0x10C5
#define PCEAR_PID	0x819A
#define ADSTECH_VID	0x06E1
#define ADSTECH_PID	0xA155

#define FMRADIO_SW_VERSION		7
#define FMRADIO_HW_VERSION		1
#define FMRADIO_FMTUNER_VERSION	10

//Audio data
#define SAMPLES_PER_SECOND	96000
#define BITS_PER_SAMPLE		16
#define CHANNELS			2

//Our stream data should be in 20 blocks at 32KB each
#define BLOCK_SIZE		32768
#define BLOCK_COUNT		20
#define BUFFER_PADDING	19

//Return Codes
#define STATUS_OK					0x00
#define STATUS_ERROR				0x01
#define STATUS_OUTPUTAUDIO_ERROR	0x02
#define STATUS_FMRADIOAUDIO_ERROR	0x03
#define STATUS_FMRADIODATA_ERROR	0x04
#define STATUS_BOOTLOAD_ERROR		0x05

//Register definitions
#define FMRADIO_REGISTER_NUM	16
#define RDS_REGISTER_NUM		6

#define FMRADIO_REGISTER	WORD
#define FMRADIO_REGISTER_SIZE	sizeof(FMRADIO_REGISTER)

#define DEVICEID		0
#define CHIPID			1
#define POWERCFG		2
#define CHANNEL			3
#define SYSCONFIG1		4
#define SYSCONFIG2		5
#define SYSCONFIG3		6
#define TEST1			7
#define TEST2			8
#define BOOTCONFIG		9
#define STATUSRSSI		10
#define READCHAN		11
#define RDSA			12
#define RDSB			13
#define RDSC			14
#define RDSD			15

//Bit definitions
#define DEVICEID_PN		0xF000
#define DEVICEID_MFGID	0x0FFF

#define CHIPID_REV		0xFC00
#define CHIPID_DEV		0x0200
#define CHIPID_FIRMWARE	0x01FF

#define POWERCFG_DMUTE		0x4000
#define POWERCFG_MONO		0x2000
#define POWERCFG_SEEKUP		0x0200
#define POWERCFG_SEEK		0x0100
#define POWERCFG_DISABLE	0x0040
#define POWERCFG_ENABLE		0x0001

#define CHANNEL_TUNE	0x8000
#define CHANNEL_CHAN	0x03FF

#define SYSCONFIG1_DE		0x0800
#define SYSCONFIG1_RDS		0x1000

#define SYSCONFIG2_SEEKTH	0xFF00
#define SYSCONFIG2_BAND		0x0080
#define SYSCONFIG2_SPACE	0x0030
#define SYSCONFIG2_VOLUME	0x000F

#define SYSCONFIG2_SPACE_200KHZ	0x0000
#define SYSCONFIG2_SPACE_100KHZ	0x0010
#define SYSCONFIG2_SPACE_50KHZ	0x0020

#define STATUSRSSI_RDSR		0x8000
#define STATUSRSSI_STC		0x4000
#define STATUSRSSI_SF		0x2000
#define STATUSRSSI_ST		0x0100
#define STATUSRSSI_RSSI		0x00FF

#define READCHAN_READCHAN	0x03FF

//Scratch page
#define SCRATCH_PAGE_SIZE		63
#define SCRATCH_PAGE_USED_SIZE	22
typedef BYTE	SCRATCH_PAGE[SCRATCH_PAGE_SIZE];

#define SCRATCH_PAGE_SW_VERSION	1
#define SCRATCH_PAGE_HW_VERSION	2

//Report definitions
#define DEVICEID_REPORT		DEVICEID + 1
#define CHIPID_REPORT		CHIPID + 1
#define POWERCFG_REPORT		POWERCFG + 1
#define CHANNEL_REPORT		CHANNEL + 1
#define SYSCONFIG1_REPORT	SYSCONFIG1 + 1
#define SYSCONFIG2_REPORT	SYSCONFIG2 + 1
#define SYSCONFIG3_REPORT	SYSCONFIG3 + 1
#define TEST1_REPORT		TEST1 + 1
#define TEST2_REPORT		TEST2 + 1
#define BOOTCONFIG_REPORT	BOOTCONFIG + 1
#define STATUSRSSI_REPORT	STATUSRSSI + 1
#define READCHAN_REPORT		READCHAN + 1
#define RDSA_REPORT			RDSA + 1
#define RDSB_REPORT			RDSB + 1
#define RDSC_REPORT			RDSC + 1
#define RDSD_REPORT			RDSD + 1
#define LED_REPORT			19
#define STREAM_REPORT		19
#define SCRATCH_REPORT		20

#define ENTIRE_REPORT		FMRADIO_REGISTER_NUM + 1
#define RDS_REPORT			FMRADIO_REGISTER_NUM + 2

#define REGISTER_REPORT_SIZE	(FMRADIO_REGISTER_SIZE + 1)
#define ENTIRE_REPORT_SIZE		((FMRADIO_REGISTER_NUM * FMRADIO_REGISTER_SIZE) + 1)
#define RDS_REPORT_SIZE			((RDS_REGISTER_NUM * FMRADIO_REGISTER_SIZE) + 1)
#define LED_REPORT_SIZE			3
#define STREAM_REPORT_SIZE		3
#define SCRATCH_REPORT_SIZE		(SCRATCH_PAGE_SIZE + 1)

//LED State definitions
#define LED_COMMAND			0x35

#define NO_CHANGE_LED		0x00
#define ALL_COLOR_LED		0x01
#define BLINK_GREEN_LED		0x02
#define BLINK_RED_LED		0x04
#define BLINK_ORANGE_LED	0x10
#define SOLID_GREEN_LED		0x20
#define SOLID_RED_LED		0x40
#define SOLID_ORANGE_LED	0x80

#define CONNECT_STATE		BLINK_GREEN_LED
#define DISCONNECT_STATE	BLINK_ORANGE_LED
#define BOOTLOAD_STATE		SOLID_RED_LED
#define STREAMING_STATE		ALL_COLOR_LED
#define TUNING_STATE		SOLID_GREEN_LED
#define SEEKING_STATE		SOLID_GREEN_LED

//Stream State definitions
#define STREAM_COMMAND	0x36

#define STREAM_VIDPID	0x00
#define STREAM_AUDIO	0xFF

//Function definitions
#define SEEK_UP		true
#define SEEK_DOWN	false

#define TUNE_UP		true
#define TUNE_DOWN	false

//Volume Definitions
#define VOLUME_MIN	0		//0% - 100%
#define VOLUME_MAX	100

//Timeout definitions
#define	POLL_TIMEOUT_SECONDS		3
#define DISCONNECT_TIMEOUT_SECONDS	15

//Seek Threshold Definitions
#define MAX_SEEK_THRESHOLD			63
#define PREFERRED_SEEK_THRESHOLD	31

//Number of presets definition
#define PRESET_NUM	12

//Global variables for the critical section and 
//free block count, used by callback functions
static CRITICAL_SECTION gWaveCriticalSection;
static volatile BYTE gWaveFreeBlockCount;

//Global callback function for when wave out terminates
static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//Structure that contains all useful data about the radio - filled from the scratch page
typedef struct RadioData
{
	bool firstRun;
	BYTE swVersion;
	BYTE hwVersion;
	BYTE partNumber;
	WORD manufacturerID;
	BYTE chipVersion;
	BYTE deviceVersion;
	BYTE firmwareVersion;
	double currentStation;
	double preset[PRESET_NUM];
	BYTE seekThreshold;
	BYTE band;
	BYTE spacing;
	BYTE deemphasis;
	BYTE monoStereo;
	bool alwaysOnTop;
	bool showInTray;
	bool showInTitleBar;
	bool muteOnStartup;
	BYTE scanTime;
	BYTE bufferSize;
} RadioData;

// This is also added to the header for the DLL, protect from multiple inclusions
#ifndef _RDSDATA_
//Structure that contains the RDS data
typedef struct RDSData
{
	double currentStation;
	BYTE recievedSignalStrength;
	bool isStereo;
	std::string rdsText;
	std::string rdsPS;
	WORD rdsPI;
	BYTE rdsPTY;
	bool rdsTA;
	bool rdsTP;
	bool rdsMS;
	std::string rdsPTYString;
} RDSData;
#define _RDSDATA_
#endif

//Values used within the RadioData structure to assign values to the band, spacing, deemphasis, and gui options
#define DATA_BAND				0x20
#define DATA_BAND_875_108MHZ	0x00
#define DATA_BAND_76_90MHZ		0x20

#define DATA_SPACING		0x0C
#define DATA_SPACING_200KHZ	0x00
#define DATA_SPACING_100KHZ	0x04
#define DATA_SPACING_50KHZ	0x08

#define DATA_DEEMPHASIS		0x02
#define DATA_DEEMPHASIS_75	0x00
#define DATA_DEEMPHASIS_50	0x02

#define DATA_MONOSTEREO			0x10
#define DATA_MONOSTEREO_STEREO	0x01
#define DATA_MONOSTEREO_MONO	0x10

#define DATA_ALWAYSONTOP		0x80
#define DATA_SHOWINTRAY			0x40
#define DATA_SHOWINTITLEBAR		0x20
#define DATA_MUTEONSTARTUP		0x10
#define DATA_SCANTIME			0x0F

class CFMRadioDevice  
{
public:
	CFMRadioDevice(bool GetRDSText = false);
	virtual ~CFMRadioDevice();

//////////////////////
//General Functionality

public:
	bool    change_process_priority;
	DWORD   m_previous_process_priority;
	bool    m_process_priority_set;
	bool    m_GetRDSText;

	BYTE	OpenFMRadio(RadioData* radioData);
	bool	CloseFMRadio();

//////////////////////
//////////////////////

///////////////////////
//USB HID Functionality

public:
	bool	BootloadDevice(RadioData* radioData);
	
	bool	StopStream(bool stop);
	bool	Mute(bool mute);
	bool	Tune(bool tuneUp);
	bool	Tune(double frequency);
	bool	Seek(bool seekUp);
	bool	GetRDSData(RDSData* radioData);
	bool	updateRDSData(RDSData* radioData);
	void	ResetRDSText();
	bool	SaveRadioSettings(RadioData* radioData);	
	bool	WriteRegister(BYTE report, FMRADIO_REGISTER registers);
	bool	ReadAllRegisters(FMRADIO_REGISTER *registers);
	bool	UpdateRDS();

	bool	SetRegisterReport(BYTE report, FMRADIO_REGISTER* dataBuffer, DWORD dataBufferSize);
	bool	GetRegisterReport(BYTE report, FMRADIO_REGISTER* dataBuffer, DWORD dataBufferSize);

private:
	bool	OpenFMRadioData();
	int		GetAudioDeviceIndex();
	bool	GetRadioData(RadioData* radioData);
	bool	SetRadioData(RadioData* radioData);
	bool	InitializeRadioData(RadioData* radioData);
	bool	CloseFMRadioData();

	HANDLE	m_FMRadioDataHandle;

	BYTE*	m_pEndpoint0ReportBuffer;
	DWORD	m_Endpoint0ReportBufferSize;

	BYTE*	m_pEndpoint1ReportBuffer;
	DWORD	m_Endpoint1ReportBufferSize;

	BYTE*	m_pEndpoint2ReportBuffer;
	DWORD	m_Endpoint2ReportBufferSize;

	FMRADIO_REGISTER	m_Register[FMRADIO_REGISTER_NUM];
	SCRATCH_PAGE		m_ScratchPage;

	CRDSData	m_RDS;
	bool		m_RDSCleared;

	double	CalculateStationFrequency(FMRADIO_REGISTER hexChannel);
	WORD	CalculateStationFrequencyBits(double frequency);

	bool	SetScratchReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize);
	bool	GetScratchReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize);
	bool	SetLEDReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize);
	bool	SetStreamReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize);

////////////////////////
////////////////////////

//////////////////////////
//USB Audio  Functionality

public:
	void	StreamAudio();
	bool	IsStreaming();
	bool	IsTuning();
	BYTE	GetWaveOutVolume();
	bool	SetWaveOutVolume(BYTE level);

	int		GetLastKnownRadioIndex();
	void	SetNewRadioIndex(int index);

	bool CreateRadioTimer();
    bool DestroyRadioTimer();

	bool CreateRDSTimer();
    bool DestroyRDSTimer();
	
private:
	bool	OpenFMRadioAudio();
	bool	OpenSoundCard();
	void	InitializeStream();
	bool	CloseFMRadioAudio();	
	bool	CloseSoundCard();

	HWAVEIN		m_FMRadioAudioHandle;
	HWAVEOUT	m_SoundCardHandle;

	int			m_LastKnownRadioIndex;

	WAVEFORMATEX	m_FMRadioWaveFormat;

	WAVEHDR		m_InputHeader;
	WAVEHDR*	m_OutputHeader;
	char*		m_WaveformBuffer;

	bool        m_StreamingAllowed;
	bool		m_Streaming;
	bool		m_Tuning;
	int			m_CurrentBlock;
	int			m_FreeBlock;

	bool	StreamAudioIn();
	bool	StreamAudioOut();
	bool	ChangeLED(BYTE ledState);

	WAVEHDR*	AllocateBlocks(int size, int count);
	void		FreeBlocks(WAVEHDR*);

	HANDLE h_radioTimer;
	HANDLE h_rdsTimer;

////////////////////////////
////////////////////////////
};

#endif // !defined(AFX_FMRADIODEVICE_H__6F0D8129_C34F_4CBC_93A0_EC3F2F68A8D1__INCLUDED_)

