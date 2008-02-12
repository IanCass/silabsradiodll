// FMRadioDevice.cpp: implementation of the CFMRadioDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FMRadioDevice.h"
#include <string>
#include <vector>

//////////// DEBUG ////////////
//#define DOLOG
#ifdef DOLOG
#include <fstream>
static std::ofstream outfile;
#endif
//////////////////////////////

#define RADIO_TIMER_PERIOD 30	/* Call StreamAudio every RADIO_TIMER_PERIOD ms */
#define RDS_TIMER_PERIOD 40		/* Call updateRDSData every RDS_TIMER_PERIOD ms */

static RDSData rdsTimerData;	/* Variable Outside Class for Direct Thread access */
static bool ShouldQuit;			/* Variable Outside Class for Direct Thread access */
static int CurrFreq;			/* Variable Outside Class for Direct Thread access */
static int QueFreq;				/* Variable Outside Class for Direct Thread access */

DWORD WINAPI RadioThread( LPVOID lpParam )
{
	while(!ShouldQuit) {
		//If new freq waiting to be tuned, do it
		if (QueFreq != CurrFreq) ((CFMRadioDevice*)lpParam)->DoTune((double)QueFreq/10);
		// Process Audio
		((CFMRadioDevice*)lpParam)->StreamAudio();
		// Wait next turn
		Sleep (RADIO_TIMER_PERIOD);
	}

	// Got here, then we've been requested to quit (Shouldquit=true)
	return 0;
}

DWORD WINAPI RDSThread( LPVOID lpParam )
{
	while(!ShouldQuit) {
		// Updat RDS
		((CFMRadioDevice*)lpParam)->updateRDSData(&rdsTimerData);
		// Wait next turn
		Sleep (RDS_TIMER_PERIOD);
	}

	// Got here, then we've been requested to quit (Shouldquit=true)
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFMRadioDevice::CFMRadioDevice(bool GetRDSText)
{

#ifdef DOLOG
	char output [100];
	outfile.open("c:\\log.txt", std::ofstream::app);
	outfile << "FM Log Started\n";
#endif

	//Initialize the critical section variable used for exclusivity
	InitializeCriticalSection(&gWaveCriticalSection);

	//Make the handles NULL to begin with
	m_FMRadioAudioHandle = NULL;
	m_FMRadioDataHandle = NULL;
	m_FMRadioRDSHandle = NULL;
	m_SoundCardHandle = NULL;

	//Set the input buffer pointers to NULL, and size to 0
	m_pEndpoint0ReportBuffer = NULL;
	m_pEndpoint1ReportBuffer = NULL;
	m_pEndpoint2ReportBuffer = NULL;
	m_Endpoint0ReportBufferSize = 0;
	m_Endpoint1ReportBufferSize = 0;
	m_Endpoint2ReportBufferSize = 0;

	//The radio is not streaming or tuning initially
	m_StreamingAllowed = false;
	m_Streaming = false;
	m_Tuning = false;
	m_AudioAllowed = true;

	//Setup the wave format based on our defined audio data
	m_FMRadioWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_FMRadioWaveFormat.nSamplesPerSec = SAMPLES_PER_SECOND;
	m_FMRadioWaveFormat.wBitsPerSample = BITS_PER_SAMPLE;
	m_FMRadioWaveFormat.nChannels = CHANNELS;

	// Opening the audio stream from a DLL requires this?
	m_FMRadioWaveFormat.nBlockAlign= 0xcccc;
	m_FMRadioWaveFormat.nAvgBytesPerSec = 0xcccccccc;
	m_FMRadioWaveFormat.cbSize = 0xcccc;

	//Set the last known radio index to negative since there hasnt been a radio attached yet
	m_LastKnownRadioIndex = -1;

	//The current block starts at zero, and all blocks are initially free
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;

	//Allocate memory for the blocks of audio data to stream
	m_OutputHeader = AllocateBlocks(BLOCK_SIZE, BLOCK_COUNT);

	m_WaveformBuffer = (char*)malloc(BLOCK_SIZE);
	m_InputHeader.lpData = m_WaveformBuffer;
	m_InputHeader.dwBufferLength = BLOCK_SIZE;
	m_InputHeader.dwFlags = 0;

	// Initialize our radio timer to NULL
	h_radioTimer = NULL;

	// Initialize the previous process priority to 0
	m_previous_process_priority = 0;
	m_process_priority_set = false;

	// We may or may not want to change the process priority
	change_process_priority = true;

	m_GetRDSText = GetRDSText;

	m_OldRegister = 0;
	m_RDSCleared = true;
}

CFMRadioDevice::~CFMRadioDevice()
{
	//Free all allocated memory when destroyed
	FreeBlocks(m_OutputHeader);
	free(m_WaveformBuffer);
}

BYTE CFMRadioDevice::OpenFMRadio(RadioData* radioData)
{
	//Check that radio data is not NULL
	if (radioData == NULL) {
		return (STATUS_ERROR);
	}

	//Try opening all valid pipes
	if (!OpenFMRadioData()) {
		return (STATUS_FMRADIODATA_ERROR);
	}

	//Get the radio data from the device's scratch page
	if (!GetRadioData(radioData)) {
		return (STATUS_ERROR);
	}

	//Initialize the radio with the current radio data
	if (!InitializeRadioData(radioData)) {
		return (STATUS_ERROR);
	}

	//Open the FM Radio audio input
	if (!OpenFMRadioAudio()) {
		return (STATUS_FMRADIOAUDIO_ERROR);
	}

	//Open the sound card
	if (!OpenSoundCard()) {
		return (STATUS_OUTPUTAUDIO_ERROR);
	}

	//Tune to the current station
	Tune(radioData->currentStation);

	return (STATUS_OK);
}

bool CFMRadioDevice::CloseFMRadio()
{
	bool status = false;
	
	//Close all pipes
	CloseFMRadioAudio();
	CloseSoundCard();
	CloseFMRadioData();
	
	status = true;

	return status;
}

bool CFMRadioDevice::RTAStart (char windowName[256], short dwData, char lpData[256])
{	
	m_RDS.TACallbackStartWindowName = windowName;
	m_RDS.TACallbackStartDwData = dwData;
	m_RDS.TACallbackStartCommand = lpData;
	return true;
}

bool CFMRadioDevice::RTAStop (char windowName[256], short dwData, char lpData[256])
{	
	m_RDS.TACallbackStopWindowName = windowName;
	m_RDS.TACallbackStopDwData = dwData;
	m_RDS.TACallbackStopCommand = lpData;
	return true;
}

bool CFMRadioDevice::RRadioText (char windowName[256], short dwData, char lpData[256])
{	
	m_RDS.RTCallbackWindowName = windowName;
	m_RDS.RTCallbackDwData = dwData;
	m_RDS.RTCallbackCommand = lpData;
	return true;
}


bool CFMRadioDevice::GetRDSData(RDSData* rdsData) {

	if (&rdsTimerData) {

		//Store all the current RDS data in the rds Data structure
		rdsData->currentStation = (double)QueFreq/10; // CalculateStationFrequency(m_Register[READCHAN] & READCHAN_READCHAN);
		rdsData->recievedSignalStrength = m_Register[STATUSRSSI] & STATUSRSSI_RSSI;
		if(rdsData->recievedSignalStrength > 15) {
			rdsData->isStereo = (((m_Register[STATUSRSSI] & STATUSRSSI_ST) >> 8) == DATA_MONOSTEREO_STEREO)||(m_RDS.m_ptyDisplay!=0)?true:false;
		}
		else 
			rdsData->isStereo = (m_RDS.m_ptyDisplay!=0);

		// Radio Text
		if (rdsData->rdsText != m_RDS.m_RDSText) {
			rdsData->rdsText = m_RDS.m_RDSText;
		}

		// Program Stream Name
		if (rdsData->rdsPS != m_RDS.m_RDSPS)
		{
			rdsData->rdsPS = m_RDS.m_RDSPS;
		}

		// PI
		if (rdsData->rdsPI != m_RDS.m_piDisplay) {
			rdsData->rdsPI = m_RDS.m_piDisplay;
			rdsData->rdsPIRegion = m_RDS.m_piRegion;
			rdsData->rdsPICountry = m_RDS.m_piCountry;
		}

		// PTY
		if (rdsData->rdsPTY != m_RDS.m_ptyDisplay) {
			rdsData->rdsPTY = m_RDS.m_ptyDisplay;
			rdsData->rdsPTYString = m_RDS.m_ptyDisplayString;
		}

		// TA, TP, MS
		rdsData->rdsTA = m_RDS.m_ta;
		rdsData->rdsTP = m_RDS.m_tp;
		rdsData->rdsMS = m_RDS.m_ms;

		// AF
		rdsData->AFMap = m_RDS.AFMap;

		return true;
	} else {
		return false;
	}
}

bool CFMRadioDevice::updateRDSData(RDSData* rdsData)
{
	bool status = false;
	char op[20];

	//Check that rdbsData is not NULL
	if (rdsData)
	{
		//Call the update function and if it succeeds, fill the return structure with the current RDBS data
		if (UpdateRDS())
		{
			/*
			// Deduplicate
			if(memcmp(&m_Register[STATUSRSSI], m_OldRDSRegister, sizeof(m_OldRDSRegister))==0)
			{	//registers identicle
				if(m_RDSCleared)//first time we have seen this string
				{//already seen ignore
					return(status);
				}
			}
			else
			{
				memcpy(m_OldRDSRegister, &m_Register[STATUSRSSI], sizeof(m_OldRDSRegister)); 
				if((m_Register[STATUSRSSI] & STATUSRSSI_RDSR)==0)
					m_RDSCleared = true;
				else
					m_RDSCleared = false;
				return(status);
			}
			*/
										
			if ((m_Register[STATUSRSSI] & STATUSRSSI_RDSR))
			{
				sprintf(op, "\r\nRDS Status:%04X", (unsigned int)m_Register[STATUSRSSI]);
				OutputDebugString(op);
				if (m_GetRDSText) 
				{
					m_RDS.UpdateRDSText(m_Register);
					status = true;
				}
			}
			else
				OutputDebugString(".");
		}
	}
	return status;
}

void CFMRadioDevice::ResetRDSText()
{
	//Resets the RDS text in the RDS Data (used when switching channels)
	m_RDS.ResetRDSText();
}

bool CFMRadioDevice::SaveRadioSettings(RadioData* radioData)
{
	bool status = false;

	//Check that radio data is not NULL
	if (radioData)
	{
		//Initialize the radio with the current radio data
		if (InitializeRadioData(radioData))
		{
			//Save the settings passed through
			if (SetRadioData(radioData))
				status = true;
		}
	}

	return status;
}

bool CFMRadioDevice::OpenFMRadioAudio()
{
	bool status = false;

	//Check the last known radio index, if it is not negative attemp to
	//open this device, it will probably be the radio
	if (m_LastKnownRadioIndex >= 0)
	{
		//If the open succeeds, then return status true, otherwise set the radio index to -1 to try again
		if (waveInOpen(&m_FMRadioAudioHandle,
					   m_LastKnownRadioIndex,
					   &m_FMRadioWaveFormat, 
					   NULL, NULL, CALLBACK_NULL) == MMSYSERR_NOERROR)
		{
			status = true;
		}
		else
		{
			m_LastKnownRadioIndex = -1;
		}
	}

	//If status isn't true, then the radio didn't open successfully, or we are opening
	//the radio for the first time
	if (!status)
	{
		//Get the index of the audio device
		m_LastKnownRadioIndex = GetAudioDeviceIndex();

		//If a valid index is returned open the audio device
		if (m_LastKnownRadioIndex >= 0)
		{
			DWORD ret = waveInOpen(&m_FMRadioAudioHandle,
									m_LastKnownRadioIndex,
									&m_FMRadioWaveFormat,
									NULL, NULL, CALLBACK_NULL);

			//If the audio device opens successfully, then return true
			if (ret == MMSYSERR_NOERROR)
			{
				status = true;
			}
		}
	}

	return status;
}

int CFMRadioDevice::GetAudioDeviceIndex()
{
	//This function is designed to open up the audio handle to the USB Radio. In
	//Windows XP, the audio portion of the USB Radio will come up as "FM Radio". If
	//this string is found, the radio will open the first one immediately, and begin
	//to play it. If it isn't found then it will go through the device list and look
	//for "USB Audio Device", which shows up in Windows 2000. If only one is found, then
	//it will go ahead and open it immediately as well since the device will be the radio.
	//However if more devices (or none) are found then it will prompt with a list for the 
	//user to select the correct USB Audio device from the list.

	int index = -1;
	std::vector<std::string> deviceList;

	//Obtain the number of input devices on the system
	DWORD numWaveInputDevices = waveInGetNumDevs();

	//Scan through each input device to see if we can find the FM Radio
	for (DWORD i = 0; i < numWaveInputDevices; i++)
	{
		WAVEINCAPS waveInputCapabilities;

		//Get the device capabilities of the currently indexed device
		if (waveInGetDevCaps(i, &waveInputCapabilities, sizeof(waveInputCapabilities)) == MMSYSERR_NOERROR)
		{
			//If FM Radio is found, open the radio immediately, this is our device
			if (!strcmp(waveInputCapabilities.szPname, "FM Radio")
				|| !strcmp(waveInputCapabilities.szPname, "ADS InstantFM Music"))
			{
				//Set the current index to i, and set i to numWaveInputDevices to break
				//out of the for loop
				index = i;
				i = numWaveInputDevices;
			}
			else
			{
				//Otherwise push back the string of the device on the list
				deviceList.push_back(waveInputCapabilities.szPname);
			}		
		}
	}

	//If we haven't found a valid index, then start looking at the strings
	if (index < 0)
	{
		DWORD usbAudioDeviceNum = 0;

		//Go through the list of device strings
		for (int i = 0; i < deviceList.size(); i++)
		{
			//See if a "USB Audio Device" is found
			if (deviceList[i].find("USB Audio Device", 0))
			{
				//Increment the usb audio device number, and set our current index
				usbAudioDeviceNum++;
				index = i;
			}
		}
#if 0
		//If more than one (or no deivices) are found, then the user needs
		//to select their audio input device from the list
		if (usbAudioDeviceNum != 1)
		{
			//Reset the index to -1, invalid
			index = -1;
			//Bring up the device select dialog by passing it the audio input
			//device list
			CDeviceSelectDlg deviceSelectDlg(&deviceList);

			if (deviceSelectDlg.DoModal() == IDOK)
			{
				//If OK is pressed, get the index selected
				index = deviceSelectDlg.GetIndex();
			}
		}
#endif
	}

	//Return the index that will be used
	return index;
}

int	CFMRadioDevice::GetLastKnownRadioIndex()
{
	//Gets the last known radio index variable
	return m_LastKnownRadioIndex;
}
	
void CFMRadioDevice::SetNewRadioIndex(int index)
{
	//Disable audio
	bool reEnableAudio = m_Streaming;
	m_Tuning = true;
	CloseFMRadioAudio();

	//Set the new index
	m_LastKnownRadioIndex = index;

	//Open the audio again (the new index will be used)
	OpenFMRadioAudio();

	//Enable audio again if we were streaming
	if (reEnableAudio)
	{
		InitializeStream();
	}

	m_Tuning = false;
}

bool CFMRadioDevice::OpenSoundCard()
{
	bool status = false;
	DWORD res;

	//Open a handle to the default wave output device (sound card)
	if ((res = waveOutOpen(&m_SoundCardHandle, WAVE_MAPPER,
						   &m_FMRadioWaveFormat, (DWORD)waveOutProc,
						   (DWORD)&gWaveFreeBlockCount, CALLBACK_FUNCTION)) == MMSYSERR_NOERROR)
	{
		status = true;
	}

	return status;
}

void CFMRadioDevice::InitializeStream()
{
	waveInStart(m_FMRadioAudioHandle);

	//Reset block status to "empty" the buffer (simply starting over and refilling)
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;

	//Fill the audio buffer to initialize the stream
	while (gWaveFreeBlockCount > (BLOCK_COUNT - BUFFER_PADDING))
	{
		StreamAudioIn();
		Sleep(50);
	}

	//Skip the first chunk (at least half) of the padded buffer to eliminate any audio glitches
	if (BUFFER_PADDING > 10)
	{
		m_CurrentBlock = 5;
		gWaveFreeBlockCount = 5;
	}

	//Set streaming to true
	m_Streaming = true;
}

void CFMRadioDevice::StreamAudio()
{
	//If our timer thread is being stopped, then don't do this
	if (!m_StreamingAllowed) {
		return;
	}

	//If a tune isn't being performed, then stream in and out
	if (!m_Tuning)
	{
		//If we are not already streaming, initialize the stream
		if (!m_Streaming)
			InitializeStream();		

		//Check that the handles arent NULL
		if ((m_FMRadioAudioHandle) && (m_SoundCardHandle))
		{
			//If there are any free blocks, then stream audio in.
			if (gWaveFreeBlockCount) {
				StreamAudioIn();
			}

			//If there are any blocks ready for output, then stream audio out
			if (gWaveFreeBlockCount < BLOCK_COUNT) {
				StreamAudioOut();
			}
		}
	}
}

bool CFMRadioDevice::IsStreaming()
{
	return m_Streaming;
}

bool CFMRadioDevice::StreamAudioIn()
{
	bool status = false;

	//Unprepare header to begin preperation process
	waveInUnprepareHeader(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock]));

	//Prepare the header for streaming in
	if (waveInPrepareHeader(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock])) == MMSYSERR_NOERROR)
	{
		//Get the buffer of audio in the input header
		if (waveInAddBuffer(m_FMRadioAudioHandle, &m_OutputHeader[m_FreeBlock], sizeof(m_OutputHeader[m_FreeBlock])) == MMSYSERR_NOERROR)
		{				
			//Enter the critical section to decrement the free block count
			EnterCriticalSection(&gWaveCriticalSection);
			gWaveFreeBlockCount--;
			LeaveCriticalSection(&gWaveCriticalSection);

			//Increment the free block index, and scale it
			m_FreeBlock++;
			m_FreeBlock %= BLOCK_COUNT;

			status = true;
		}
	}

	return status;
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	//If a WaveOut Done message comes back, then enter the critical section to
	//increment the free block counter
	if (uMsg == WOM_DONE)
	{
		EnterCriticalSection(&gWaveCriticalSection);
		int* freeBlockCounter = (int*)dwInstance;
		(*freeBlockCounter)++;
		LeaveCriticalSection(&gWaveCriticalSection);
	}
}

bool CFMRadioDevice::StreamAudioOut()
{
	bool status = false;

	//Unprepare header to begin preparation to stream out
	waveOutUnprepareHeader(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR));

	//Prepare the header for streaming out
	if (waveOutPrepareHeader(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
	{
		if (m_AudioAllowed)
			status = (waveOutWrite(m_SoundCardHandle, &m_OutputHeader[m_CurrentBlock], sizeof(WAVEHDR)) == MMSYSERR_NOERROR);
		else
			status = true;

		//Write the sound to the sound card
		if (status)
		{
			//waveOutProc callback function will get called, and free block counter gets incremented
			if (!m_AudioAllowed)
				if (gWaveFreeBlockCount == 1) waveOutProc(0, WOM_DONE, (DWORD)&gWaveFreeBlockCount, 0, 0);

			//Increment the index of the current block to be played
			m_CurrentBlock++;
			m_CurrentBlock %= BLOCK_COUNT;

			status = true;
		}
	}

	return status;
}

BYTE CFMRadioDevice::GetWaveOutVolume()
{
	DWORD level = VOLUME_MIN;

	//This gets the current wave output volume
	waveOutGetVolume(m_SoundCardHandle, &level);

	//This determines the level of one channel
	level = level & 0xFFFF;
	
	//If the level is above 0, then calculate it's percentage (0-100%)
	if (level)	
		level = (DWORD)((double)(level / (double)0xFFFF) * 100.0);

	//Return the percentage level of volume
	return (BYTE)(level & 0xFF);
}

bool CFMRadioDevice::SetWaveOutVolume(BYTE level)
{
	bool status = false;
	DWORD setLevel = 0x00000000;

	//Don't set the volume to anything greater than the max level
	if (level > VOLUME_MAX)
		level = VOLUME_MAX;
	
	//Calculate a value based on the percentage input of one channel
	if (level)
		setLevel = (DWORD)(((double)level / 100.0) * (double)0xFFFF);

	//Set the volume for L and R channels
	setLevel = (setLevel << 16) | setLevel;

	//Set the volume
	if (waveOutSetVolume(m_SoundCardHandle, setLevel) == MMSYSERR_NOERROR)
		status = true;

	return status;
}

bool CFMRadioDevice::CloseFMRadioAudio()
{
	bool status = false;

	//Stop the input from the device
	waveInStop(m_FMRadioAudioHandle);

	//Reset the device
	waveInReset(m_FMRadioAudioHandle);

	//Close the device
	waveInClose(m_FMRadioAudioHandle);

	//Reset handles and variables
	m_FMRadioAudioHandle = NULL;
	m_CurrentBlock = 0;
	m_FreeBlock = 0;
	gWaveFreeBlockCount = BLOCK_COUNT;
	m_Streaming = false;
	status = true;
			
	return status;
}

bool CFMRadioDevice::CloseSoundCard()
{
	bool status = false;

	//Reset the device
	waveOutReset(m_SoundCardHandle);
	
	//Close the device
	waveOutClose(m_SoundCardHandle);

	//Reset the handle
	m_SoundCardHandle = NULL;
	status = true;
				
	return status;
}

WAVEHDR* CFMRadioDevice::AllocateBlocks(int size, int count)
{
	char* buffer;
	WAVEHDR* blocks = NULL;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

	//Allocate zero initialized memory the size of our total buffer
	if (buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBufferSize))
	{
		blocks = (WAVEHDR*)buffer;
		buffer += sizeof(WAVEHDR) * count;

		//Fill the headers out based on our allocated space
		for (int i = 0; i < count; i++)
		{
			blocks[i].dwBufferLength = size;
			blocks[i].lpData = buffer;
			buffer += size;
		}
	}

	return blocks;
}

void CFMRadioDevice::FreeBlocks(WAVEHDR* blockArray)
{
	//Free the heap memory from the pointer provided
	HeapFree(GetProcessHeap(), 0, blockArray);
}

bool CFMRadioDevice::OpenFMRadioData()
{
	bool status = false;

	HANDLE		hHidDeviceHandle = NULL;
	HANDLE		hHidDeviceHandleR = NULL;
	GUID		hidGuid;
	HDEVINFO	hHidDeviceInfo = NULL;

	//Obtain the HID GUID
	HidD_GetHidGuid(&hidGuid);

	//Use the HID GUID to get a handle to a list of all HID devices connected
	hHidDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hHidDeviceInfo != INVALID_HANDLE_VALUE)
	{
		SP_DEVICE_INTERFACE_DATA hidDeviceInterfaceData;
		hidDeviceInterfaceData.cbSize = sizeof(hidDeviceInterfaceData);

		DWORD i = 0;
		BOOL hidResult = 1;

		//Loop through devices until the hidResult fails, the max USB devices are reached, or status is true
		while ((hidResult) && (i < MAX_USB_DEVICES) && (!status))
		{
			//Query the device using the index to get the interface data
			hidResult = SetupDiEnumDeviceInterfaces(hHidDeviceInfo, 0, &hidGuid, i, &hidDeviceInterfaceData);

			//If a successful query was made, use it to get the detailed data of the device
			if (hidResult)
			{
				BOOL detailResult;
				DWORD length, required;
				PSP_DEVICE_INTERFACE_DETAIL_DATA hidDeviceInterfaceDetailData;

				//Obtain the length of the detailed data structure, then allocate space and retrieve it
				SetupDiGetDeviceInterfaceDetail(hHidDeviceInfo, &hidDeviceInterfaceData, NULL, 0, &length, NULL);
				hidDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
				hidDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				detailResult = SetupDiGetDeviceInterfaceDetail(hHidDeviceInfo, &hidDeviceInterfaceData, hidDeviceInterfaceDetailData, length, &required, NULL);
				
				//If another successful query to the device detail was made, open a handle to
				//determine if the VID and PID are a match as well
				if (detailResult)
				{
					//Open the device				
					hHidDeviceHandle = CreateFile(hidDeviceInterfaceDetailData->DevicePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
					//hHidDeviceHandle = CreateFile(hidDeviceInterfaceDetailData->DevicePath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, NULL, NULL);

					if (hHidDeviceHandle != INVALID_HANDLE_VALUE)
					{
						HIDD_ATTRIBUTES	hidDeviceAttributes;

						//If it is a valid open, then get the attributes of the HID device
						if (HidD_GetAttributes(hHidDeviceHandle, &hidDeviceAttributes))
						{
							//Check that the VID and PID match
							if (((hidDeviceAttributes.VendorID == SILABS_VID) && (hidDeviceAttributes.ProductID == SILABS_PID))
								|| ((hidDeviceAttributes.VendorID == PCEAR_VID) && (hidDeviceAttributes.ProductID == PCEAR_PID))
								|| ((hidDeviceAttributes.VendorID == ADSTECH_VID) && (hidDeviceAttributes.ProductID == ADSTECH_PID))
								)
							{
								m_FMRadioDataHandle = hHidDeviceHandle;

								PHIDP_PREPARSED_DATA preparsedData;

								//Get the preparsed data structure
								if (HidD_GetPreparsedData(hHidDeviceHandle, &preparsedData))
								{
									HIDP_CAPS capabilities;

									//Used the preparsed data structure to get the device capabilities
									if (HidP_GetCaps(preparsedData, &capabilities))
									{
										//Check that the feature report length is more than 2
										if (capabilities.FeatureReportByteLength > (FMRADIO_REGISTER_NUM * FMRADIO_REGISTER_SIZE))
										{
											//Allocate the right amount of space for the control feature reports (1-17)
											m_Endpoint0ReportBufferSize = capabilities.FeatureReportByteLength;
											m_pEndpoint0ReportBuffer = (BYTE*)malloc(m_Endpoint0ReportBufferSize);
											memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);

											//Allocate the right amout of space for endpoint 1 feature report (18)
											m_Endpoint1ReportBufferSize = RDS_REPORT_SIZE;
											m_pEndpoint1ReportBuffer = (BYTE*)malloc(m_Endpoint1ReportBufferSize);
											memset(m_pEndpoint1ReportBuffer, 0, m_Endpoint1ReportBufferSize);

											m_Endpoint2ReportBufferSize = FMTUNERDATA_REPORT_SIZE;
											m_pEndpoint2ReportBuffer = (BYTE*)malloc(m_Endpoint2ReportBufferSize);
											memset(m_pEndpoint2ReportBuffer, 0, m_Endpoint2ReportBufferSize);
											
											if (ReadAllRegisters(m_Register))
											{
												//Set the LED to the connected state
												ChangeLED(CONNECT_STATE);
												status = true;
											}
										}
									}

									//Cleanup the preparesed data
									HidD_FreePreparsedData(preparsedData);
								}
							}
							else
							{
								//If they dont match, close the handle and continue the search
								CloseHandle(hHidDeviceHandle);
							}
						}
					}
				}

				//Deallocate space for the detailed data structure
				free(hidDeviceInterfaceDetailData);
			}

			//Increment i for the next device
			i++;
		}
	}

	return status;
}
	
bool CFMRadioDevice::InitializeRadioData(RadioData* radioData)
{
	bool status = false;

	//Check that radio data is not a NULL pointer
	if (radioData)
	{
		//Always init the RDBS to be enabled
		m_Register[SYSCONFIG1] |= SYSCONFIG1_RDS;

		//Check the band
		switch (radioData->band & DATA_BAND)
		{
		case DATA_BAND_875_108MHZ	:	m_Register[SYSCONFIG2] &= ~SYSCONFIG2_BAND;	break;
		case DATA_BAND_76_90MHZ		:	m_Register[SYSCONFIG2] |= SYSCONFIG2_BAND;	break;
		default						:	m_Register[SYSCONFIG2] &= ~SYSCONFIG2_BAND;	break;
		}

		//Check the spacing
		m_Register[SYSCONFIG2] &= ~SYSCONFIG2_SPACE;
		switch (radioData->spacing & DATA_SPACING)
		{
		case DATA_SPACING_200KHZ	:	m_Register[SYSCONFIG2] |= SYSCONFIG2_SPACE_200KHZ;	break;
		case DATA_SPACING_100KHZ	:	m_Register[SYSCONFIG2] |= SYSCONFIG2_SPACE_100KHZ;	break;
		case DATA_SPACING_50KHZ		:	m_Register[SYSCONFIG2] |= SYSCONFIG2_SPACE_50KHZ;	break;
		default						:	m_Register[SYSCONFIG2] |= SYSCONFIG2_SPACE_200KHZ;	break;
		}

		//Check the de-emphasis
		switch (radioData->deemphasis & DATA_DEEMPHASIS)
		{
		case DATA_DEEMPHASIS_75		:	m_Register[SYSCONFIG1] &= ~SYSCONFIG1_DE;	break;
		case DATA_DEEMPHASIS_50		:	m_Register[SYSCONFIG1] |= SYSCONFIG1_DE;	break;
		default						:	m_Register[SYSCONFIG1] &= ~SYSCONFIG1_DE;	break;
		}

		//Check mono/stereo selection
		switch (radioData->monoStereo & DATA_MONOSTEREO)
		{
		case DATA_MONOSTEREO_STEREO	:	m_Register[POWERCFG] &= ~POWERCFG_MONO;	break;
		case DATA_MONOSTEREO_MONO	:	m_Register[POWERCFG] |= POWERCFG_MONO;	break;
		default						:	m_Register[POWERCFG] &= ~POWERCFG_MONO;	break;
		}

		//Check the seek threshold
		m_Register[SYSCONFIG2] &= ~SYSCONFIG2_SEEKTH;
		m_Register[SYSCONFIG2] |= (WORD)radioData->seekThreshold << 8;

		//Write the registers
		if (WriteRegister(SYSCONFIG1_REPORT, m_Register[SYSCONFIG1]) &&
			WriteRegister(SYSCONFIG2_REPORT, m_Register[SYSCONFIG2]) &&
			WriteRegister(POWERCFG_REPORT, m_Register[POWERCFG]))
			status = true;
	}
	
	return status;
}

bool CFMRadioDevice::BootloadDevice(RadioData* radioData)
{
	return true;
}

bool CFMRadioDevice::CloseFMRadioData()
{
	bool status = false;

	//Since the radio is being closed display the disconnet LED
	//only if the handle is still valid
	if (m_FMRadioDataHandle)
		ChangeLED(DISCONNECT_STATE);

	//Free the endpoint buffers
	if (m_pEndpoint0ReportBuffer)
		free(m_pEndpoint0ReportBuffer);
	if (m_pEndpoint1ReportBuffer)
		free(m_pEndpoint1ReportBuffer);
	if (m_pEndpoint2ReportBuffer)
		free(m_pEndpoint2ReportBuffer);

	//Set the endpoint buffer sizes back to zero
	m_Endpoint0ReportBufferSize = 0;
	m_Endpoint1ReportBufferSize = 0;
	m_Endpoint2ReportBufferSize = 0;

	m_pEndpoint0ReportBuffer = NULL;
	m_pEndpoint1ReportBuffer = NULL;
	m_pEndpoint2ReportBuffer = NULL;

	//Close the FM Radio handle and make it NULL
	CloseHandle(m_FMRadioDataHandle);
	m_FMRadioDataHandle = NULL;

	status = true;

	return status;
}

bool CFMRadioDevice::IsTuning()
{
	//Return the tuning status
	return m_Tuning;
}

bool CFMRadioDevice::StopStream(bool stop)
{
	//To stop the stream, set tune to true to and "mimic the tune"
	m_Tuning = stop;

	return true;
}

bool CFMRadioDevice::Mute(bool mute)
{

	if (mute)
	{
		m_AudioAllowed = false;
		waveOutReset(m_SoundCardHandle);
	}
	else
	{
		m_AudioAllowed = true;
		for(int i=0;i++;i<BLOCK_COUNT)
			StreamAudioOut();
	}

	return true;
}

// Buggy SHIT from who knows -- Don't use
bool CFMRadioDevice::Tune(bool tuneUp)
{
	bool status = false;

	FMRADIO_REGISTER channel = m_Register[READCHAN] & READCHAN_READCHAN;

	//If tuning up, add one to the channel, if tuning down, subtract 1
	//Also, check for overflow and underflow after the channel changes
	if (tuneUp)
	{
		channel++;
		if (channel == CHANNEL_CHAN + 1)
			channel = 0x0000;
	}
	else
	{
		channel--;
		if (channel == 0xFFFF)
			channel = CHANNEL_CHAN;
	}

	//After the frequency is set, check to make sure it is not over 90.0/108.0 MHz, if so
	//treat it like an overflow and set the channel bits back to 0 tuning up, otherwise 
	//tune down until the range is reached
	while (CalculateStationFrequency(channel) > ((m_Register[SYSCONFIG2] & SYSCONFIG2_BAND) ? 90.0 : 108.0))
	{
		if (tuneUp)
			channel = 0x0000;
		else
			channel--;
	}
	
	//Once the proper frequency is attained, tune the channel
	if (Tune(CalculateStationFrequency(channel)))
		status = true;

	if (status) {
		
		ResetRDSText();
	}

	return status;
}

bool CFMRadioDevice::Tune(double frequency)
{

	QueFreq = (int)((frequency+0.001)*10);

	return true;
}

bool CFMRadioDevice::DoTune(double frequency)
{
	bool status = false;

	//char output [100];
	//sprintf(output, "TuneFreq: %f\nChanBits: %d\nCalc: %d", frequency, (int)CalculateStationFrequencyBits(frequency), (int)((frequency+0.001)*10));
	//MessageBox(0, output, "Test",0);

	//Check that the frequency is in range for the current band
	if (((m_Register[SYSCONFIG2] & SYSCONFIG2_BAND) && (frequency >= 76.0) && (frequency <= 90.0)) || 
		(!(m_Register[SYSCONFIG2] & SYSCONFIG2_BAND) && (frequency >= 87.5) && (frequency <= 108.0)))
	{
		WORD channel;
		
		//Determine the frequency bits
		channel = CalculateStationFrequencyBits(frequency);
		m_Register[CHANNEL] &= ~CHANNEL_CHAN;
		m_Register[CHANNEL] |= channel | CHANNEL_TUNE;
		
		m_Tuning = true;

		//Use set feature to set the channel
		if (SetRegisterReport(CHANNEL_REPORT, &m_Register[CHANNEL], 1))
		{
			//Read in the status register to poll STC and see when the tune is complete
			bool stc = false, error = false;

			//Get the current time to check for polling timeout
			SYSTEMTIME systemTime;
			GetSystemTime(&systemTime);
			WORD startTime = systemTime.wSecond + POLL_TIMEOUT_SECONDS;

			//Poll the RSSI register to see if STC gets set
			while (!stc && !error)
			{	
				if (GetRegisterReport(STATUSRSSI_REPORT, &m_Register[STATUSRSSI], 1))
				{
					if (m_Register[STATUSRSSI] & STATUSRSSI_STC)
					{
						stc = true;
					}
				}
				else
				{
					error = true;
				}

				//Get current time and see if timout has occurred
				GetSystemTime(&systemTime);
				if ((systemTime.wSecond - startTime) > POLL_TIMEOUT_SECONDS)
					error = true;
				//Sleep(3);
			}

			//Once we are out of the polling loop, if there was no error and tune completed, clear 
			//the channel bit and get the current channel
			if (stc && !error)
			{
				m_Register[CHANNEL] &= ~CHANNEL_TUNE;

				if (SetRegisterReport(CHANNEL_REPORT, &m_Register[CHANNEL], 1))
					status = true;

				GetRegisterReport(READCHAN_REPORT, &m_Register[READCHAN], 1);
				
			}
		}
		else
		{
			//If the write failed, set our tune bit back
			m_Register[CHANNEL] &= ~CHANNEL_TUNE;
		}

		CurrFreq = (int)((frequency+0.001)*10);

		//Set tuning back to false
		m_Tuning = false;
	}	

	if (status) {
		ResetRDSText();

		RDSData rds_data;
		if (GetRDSData(&rds_data)) {
			RadioData radioData;
			GetRadioData(&radioData);
			radioData.currentStation = rds_data.currentStation;
			SetRadioData(&radioData);
		}
	}

	return status;
}

bool CFMRadioDevice::Seek(bool seekUp)
{

	bool status = false;

	//Set the seekUp bit in the Power Config register
	if (seekUp)
		m_Register[POWERCFG] |= POWERCFG_SEEKUP;
	else
		m_Register[POWERCFG] &= ~POWERCFG_SEEKUP;

	//Set the seek bit in the Power Config register
	m_Register[POWERCFG] |= POWERCFG_SEEK;

	m_Tuning = true;

	//Use set feature to set the channel
	if (SetRegisterReport(POWERCFG_REPORT, &m_Register[POWERCFG], 1))
	{
		//Read in the status register to poll STC and see when the seek is complete
		bool stc = false, error = false;

		//Get the current time to check for polling timeout
		SYSTEMTIME systemTime;
		GetSystemTime(&systemTime);
		WORD startTime = systemTime.wSecond + POLL_TIMEOUT_SECONDS;

		//Poll the RSSI register to see if STC gets set
		while (!stc && !error)
		{	
			if (GetRegisterReport(STATUSRSSI_REPORT, &m_Register[STATUSRSSI], 1))
			{
				if (m_Register[STATUSRSSI] & STATUSRSSI_STC)
				{
					stc = true;
				}
			}
			else
				error = true;

			//Get current time and see if timout has occurred
			GetSystemTime(&systemTime);
			if ((systemTime.wSecond - startTime) > POLL_TIMEOUT_SECONDS)
				error = true;
			//Sleep(3);
		}

		//Once we are out of the polling loop, if there was no error and tune completed, clear 
		//the channel bit and get the current channel
		if (stc && !error)
		{

			m_Register[POWERCFG] &= ~POWERCFG_SEEK;


			if (SetRegisterReport(POWERCFG_REPORT, &m_Register[POWERCFG], 1))
				status = true;

			GetRegisterReport(READCHAN_REPORT, &m_Register[READCHAN], 1);
		}
	}
	else
	{
		//If the write failed, set our seek bit back
		m_Register[POWERCFG] &= ~POWERCFG_SEEK;
	}


	//Set tuning back to false
	m_Tuning = false;

	ResetRDSText();

	// Save new Freq
	CurrFreq = (int)(CalculateStationFrequency(m_Register[READCHAN] & READCHAN_READCHAN)*10);
	QueFreq = CurrFreq;

	RDSData rds_data;
	if (GetRDSData(&rds_data)) {
		RadioData radioData;
		GetRadioData(&radioData);
		radioData.currentStation = rds_data.currentStation;
		SetRadioData(&radioData);
	}

	return status;
}

double CFMRadioDevice::CalculateStationFrequency(FMRADIO_REGISTER hexChannel)
{
	double frequency = 0;

	double band = 87.5, spacing = 0.2, channel = (double)hexChannel;
	
	//Determine the band and spacing
	band = (m_Register[SYSCONFIG2] & SYSCONFIG2_BAND) ? 76 : 87.5;

	switch (m_Register[SYSCONFIG2] & SYSCONFIG2_SPACE)
	{
	case SYSCONFIG2_SPACE_200KHZ :	spacing = 0.2;	break;
	case SYSCONFIG2_SPACE_100KHZ :	spacing = 0.1;	break;
	case SYSCONFIG2_SPACE_50KHZ :	spacing = 0.05;	break;
	}
	
	//Calculate the frequency and add .0001 to round up numbers not quite close enough to the frequency
	frequency = (int)(((band + (spacing * channel)) + .0001) * 100.0) / 100.0;

	return frequency;
}

WORD CFMRadioDevice::CalculateStationFrequencyBits(double frequency)
{
	WORD hexChannel;

	double band = 87.5, spacing = 0.2;

	//Determine the band and spacing
	band = (m_Register[SYSCONFIG2] & SYSCONFIG2_BAND) ? 76 : 87.5;

	switch (m_Register[SYSCONFIG2] & SYSCONFIG2_SPACE)
	{
	case SYSCONFIG2_SPACE_200KHZ :	spacing = 0.2;	break;
	case SYSCONFIG2_SPACE_100KHZ :	spacing = 0.1;	break;
	case SYSCONFIG2_SPACE_50KHZ :	spacing = 0.05;	break;
	}

	//When calculating the channel, add .0001 to the double to round numbers that don't quite get up to the frequency
	hexChannel = (WORD)(((frequency - band) / spacing) + .0001);

	return hexChannel;
}

bool CFMRadioDevice::GetRadioData(RadioData* radioData)
{
	bool status = false;

	if (GetScratchReport(SCRATCH_REPORT, m_ScratchPage, SCRATCH_PAGE_SIZE))
	{	
		//Get all the generic data
		radioData->swVersion = m_ScratchPage[1];
		radioData->hwVersion = m_ScratchPage[2];
		radioData->partNumber = (m_Register[DEVICEID] & DEVICEID_PN) >> 12;
		radioData->manufacturerID = m_Register[DEVICEID] & DEVICEID_MFGID;
		radioData->chipVersion = (m_Register[CHIPID] & CHIPID_REV) >> 10;
		radioData->deviceVersion = (m_Register[CHIPID] & CHIPID_DEV) >> 9;
		radioData->firmwareVersion = m_Register[CHIPID] & CHIPID_FIRMWARE;

		//If the scratch page's first byte is 0, then it is new, set all options to default
		if (m_ScratchPage[0] == 0x00)
		{	
			radioData->firstRun = true;
			radioData->preset[0] = 90.5;
			radioData->preset[1] = 92.1;
			radioData->preset[2] = 93.3;
			radioData->preset[3] = 93.7;
			radioData->preset[4] = 94.7;
			radioData->preset[5] = 96.7;
			radioData->preset[6] = 100.7;
			radioData->preset[7] = 101.5;
			radioData->preset[8] = 102.3;
			radioData->preset[9] = 103.5;
			radioData->preset[10] = 105.9;
			radioData->preset[11] = 107.1;
			radioData->currentStation = 102.3;
			radioData->seekThreshold = PREFERRED_SEEK_THRESHOLD;
			radioData->band = DATA_BAND_875_108MHZ;
			radioData->spacing = DATA_SPACING_100KHZ;
			radioData->deemphasis = DATA_DEEMPHASIS_75;
			radioData->monoStereo = DATA_MONOSTEREO_STEREO;
			radioData->alwaysOnTop = false;
			radioData->showInTray = true;
			radioData->showInTitleBar = true;
			radioData->muteOnStartup = false;
			radioData->scanTime = 4;
			radioData->bufferSize = 0;
		}
		else
		{
			//Radio data is read in from the scratch page, and is decoded as follows:
			//[0] = 0x00/First run radio (only 0, 1, 2 are valid spaces), 0x01/Radio with valid settings
			//[1] = Software version
			//[2] = Hardware version
			//[3] = Preset channels, 10 bits per channel starting at 3 [3]&0xFF|[4]&0xC0 = P1, 
			//  													   [4]&0x3F|[5]&0xF0 = P2, etc.
			//[..] = ..
			//[17] = Last register containing preset channel bits
			//[18] = &0xFF = Current station hi
			//[19] = &0xC0 = Current station low,
			//		 &0x20 = Band,
			//		 &0x10 = Mono/Stereo,
			//		 &0x0C = Spacing,
			//		 &0x02 = De-emphasis,
			//		 &0x01 UNDEFINED
			//[20] = Seek threshold
			//[21] = &0x80 = Always on top,
			//		 &0x40 = Show in tray,
			//		 &0x20 = Show in title bar,
			//		 &0x10 = Mute on statup,
			//		 &0x0F = Scan time

			radioData->firstRun = false;
			radioData->preset[0] = CalculateStationFrequency((WORD)(((m_ScratchPage[3] & 0xFF) << 2) | ((m_ScratchPage[4] & 0xC0) >> 6)));
			radioData->preset[1] = CalculateStationFrequency((WORD)(((m_ScratchPage[4] & 0x3F) << 4) | ((m_ScratchPage[5] & 0xF0) >> 4)));
			radioData->preset[2] = CalculateStationFrequency((WORD)(((m_ScratchPage[5] & 0x0F) << 6) | ((m_ScratchPage[6] & 0xFC) >> 2)));
			radioData->preset[3] = CalculateStationFrequency((WORD)(((m_ScratchPage[6] & 0x03) << 8) | (m_ScratchPage[7] & 0xFF)));
			radioData->preset[4] = CalculateStationFrequency((WORD)(((m_ScratchPage[8] & 0xFF) << 2) | ((m_ScratchPage[9] & 0xC0) >> 6)));
			radioData->preset[5] = CalculateStationFrequency((WORD)(((m_ScratchPage[9] & 0x3F) << 4) | ((m_ScratchPage[10] & 0xF0) >> 4)));
			radioData->preset[6] = CalculateStationFrequency((WORD)(((m_ScratchPage[10] & 0x0F) << 6) | ((m_ScratchPage[11] & 0xFC) >> 2)));
			radioData->preset[7] = CalculateStationFrequency((WORD)(((m_ScratchPage[11] & 0x03) << 8) | (m_ScratchPage[12] & 0xFF)));
			radioData->preset[8] = CalculateStationFrequency((WORD)(((m_ScratchPage[13] & 0xFF) << 2) | ((m_ScratchPage[14] & 0xC0) >> 6)));
			radioData->preset[9] = CalculateStationFrequency((WORD)(((m_ScratchPage[14] & 0x3F) << 4) | ((m_ScratchPage[15] & 0xF0) >> 4)));
			radioData->preset[10] = CalculateStationFrequency((WORD)(((m_ScratchPage[15] & 0x0F) << 6) | ((m_ScratchPage[16] & 0xFC) >> 2)));
			radioData->preset[11] = CalculateStationFrequency((WORD)(((m_ScratchPage[16] & 0x03) << 8) | (m_ScratchPage[17] & 0xFF)));
			radioData->currentStation = CalculateStationFrequency((WORD)(((m_ScratchPage[18] & 0xFF) << 2) | ((m_ScratchPage[19] & 0xC0) >> 6)));
			radioData->band = m_ScratchPage[19] & DATA_BAND;
			radioData->spacing = DATA_SPACING_100KHZ; // m_ScratchPage[19] & DATA_SPACING;
			radioData->deemphasis = m_ScratchPage[19] & DATA_DEEMPHASIS;
			radioData->monoStereo = DATA_MONOSTEREO_STEREO; // m_ScratchPage[19] & DATA_MONOSTEREO;
			radioData->seekThreshold = PREFERRED_SEEK_THRESHOLD; // m_ScratchPage[20];
			radioData->alwaysOnTop = (m_ScratchPage[21] & DATA_ALWAYSONTOP) ? true : false;
			radioData->showInTray = (m_ScratchPage[21] & DATA_SHOWINTRAY) ? true : false;
			radioData->showInTitleBar = (m_ScratchPage[21] & DATA_SHOWINTITLEBAR) ? true : false;
			radioData->muteOnStartup = (m_ScratchPage[21] & DATA_MUTEONSTARTUP) ? true : false;
			radioData->scanTime = m_ScratchPage[21] & DATA_SCANTIME;
			radioData->bufferSize = 0;
		}

		status = true;
	}
		
	return status;
}

bool CFMRadioDevice::SetRadioData(RadioData* radioData)
{
	bool status = false;
	
	//Check that radio data is not NULL
	if (radioData)
	{
		//Clear the entire scratch page to FFs
		for (int i = 0; i < SCRATCH_PAGE_SIZE; i++)
			m_ScratchPage[i] = 0xFF;

		//Clear the scratch page of used data to 00s
		for (i = 0; i < SCRATCH_PAGE_USED_SIZE; i++)
			m_ScratchPage[i] = 0x00;

		//See GetRadioData for the format of the scratch page
		m_ScratchPage[0] |= (radioData->firstRun ? 0x00 : 0x01);
		m_ScratchPage[1] |= radioData->swVersion;
		m_ScratchPage[2] |= radioData->hwVersion;
		m_ScratchPage[3] |= (CalculateStationFrequencyBits(radioData->preset[0]) >> 2) & 0xFF;
		m_ScratchPage[4] |= (CalculateStationFrequencyBits(radioData->preset[0]) << 6) & 0xC0;
		m_ScratchPage[4] |= (CalculateStationFrequencyBits(radioData->preset[1]) >> 4) & 0x3F;
		m_ScratchPage[5] |= (CalculateStationFrequencyBits(radioData->preset[1]) << 4) & 0xF0;
		m_ScratchPage[5] |= (CalculateStationFrequencyBits(radioData->preset[2]) >> 6) & 0x0F;
		m_ScratchPage[6] |= (CalculateStationFrequencyBits(radioData->preset[2]) << 2) & 0xFC;
		m_ScratchPage[6] |= (CalculateStationFrequencyBits(radioData->preset[3]) >> 8) & 0x03;
		m_ScratchPage[7] |= CalculateStationFrequencyBits(radioData->preset[3]) & 0xFF;
		m_ScratchPage[8] |= (CalculateStationFrequencyBits(radioData->preset[4]) >> 2) & 0xFF;
		m_ScratchPage[9] |= (CalculateStationFrequencyBits(radioData->preset[4]) << 6) & 0xC0;
		m_ScratchPage[9] |= (CalculateStationFrequencyBits(radioData->preset[5]) >> 4) & 0x3F;
		m_ScratchPage[10] |= (CalculateStationFrequencyBits(radioData->preset[5]) << 4) & 0xF0;
		m_ScratchPage[10] |= (CalculateStationFrequencyBits(radioData->preset[6]) >> 6) & 0x0F;
		m_ScratchPage[11] |= (CalculateStationFrequencyBits(radioData->preset[6]) << 2) & 0xFC;
		m_ScratchPage[11] |= (CalculateStationFrequencyBits(radioData->preset[7]) >> 8) & 0x03;
		m_ScratchPage[12] |= CalculateStationFrequencyBits(radioData->preset[7]) & 0xFF;
		m_ScratchPage[13] |= (CalculateStationFrequencyBits(radioData->preset[8]) >> 2) & 0xFF;
		m_ScratchPage[14] |= (CalculateStationFrequencyBits(radioData->preset[8]) << 6) & 0xC0;
		m_ScratchPage[14] |= (CalculateStationFrequencyBits(radioData->preset[9]) >> 4) & 0x3F;
		m_ScratchPage[15] |= (CalculateStationFrequencyBits(radioData->preset[9]) << 4) & 0xF0;
		m_ScratchPage[15] |= (CalculateStationFrequencyBits(radioData->preset[10]) >> 6) & 0x0F;
		m_ScratchPage[16] |= (CalculateStationFrequencyBits(radioData->preset[10]) << 2) & 0xFC;
		m_ScratchPage[16] |= (CalculateStationFrequencyBits(radioData->preset[11]) >> 8) & 0x03;
		m_ScratchPage[17] |= CalculateStationFrequencyBits(radioData->preset[11]) & 0xFF;
		m_ScratchPage[18] |= (CalculateStationFrequencyBits(radioData->currentStation) >> 2) & 0xFF;
		m_ScratchPage[19] |= ((CalculateStationFrequencyBits(radioData->currentStation) << 6) & 0xC0) | radioData->band | radioData->monoStereo | radioData->spacing | radioData->deemphasis;
		m_ScratchPage[20] |= radioData->seekThreshold;
		if (radioData->alwaysOnTop) m_ScratchPage[21] |= DATA_ALWAYSONTOP;
		if (radioData->showInTray) m_ScratchPage[21] |= DATA_SHOWINTRAY;
		if (radioData->showInTitleBar) m_ScratchPage[21] |= DATA_SHOWINTITLEBAR;
		if (radioData->muteOnStartup) m_ScratchPage[21] |= DATA_MUTEONSTARTUP;
		m_ScratchPage[21] |= radioData->scanTime & 0x0F;

		if (SetScratchReport(SCRATCH_REPORT, m_ScratchPage, SCRATCH_PAGE_SIZE))
			status = true;
	}	

	return status;
}

bool CFMRadioDevice::UpdateRDS()
{
	bool status = false;

	//Get the RDS report from the device
	if (GetRegisterReport(RDS_REPORT, &m_Register[STATUSRSSI], RDS_REGISTER_NUM))
		status = true;
		
	return status;
}

bool CFMRadioDevice::WriteRegister(BYTE report, FMRADIO_REGISTER registers)
{
	bool status = false;

	m_Register[report - 1] = registers;

	//Write the report submitted
	if (SetRegisterReport(report, &m_Register[report - 1], FMRADIO_REGISTER_NUM))
		status = true;

	return status;
}

bool CFMRadioDevice::ReadAllRegisters(FMRADIO_REGISTER *registers)
{
	bool status = false;

	//Read all the registers and fill the buffer
	if (GetRegisterReport(ENTIRE_REPORT, registers, FMRADIO_REGISTER_NUM))
		status = true;

	return status;
}

bool CFMRadioDevice::ChangeLED(BYTE ledState)
{
	bool status = false;

	//Set the LED report with the state of the LED
	BYTE ledReport[LED_REPORT_SIZE] = {LED_COMMAND, ledState, 0xFF};

	//Use set report to send the new LED state
	if (SetLEDReport(LED_REPORT, ledReport, LED_REPORT_SIZE))
		status = true;

	return status;
}

bool CFMRadioDevice::SetRegisterReport(BYTE report, FMRADIO_REGISTER* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= ((m_Endpoint0ReportBufferSize - 1) / FMRADIO_REGISTER_SIZE))
		{
			//Check to see that the report to write is a writeable register
			if ((report == POWERCFG_REPORT) || (report == CHANNEL_REPORT) ||
				(report == SYSCONFIG1_REPORT) || (report == SYSCONFIG2_REPORT) ||
				(report == SYSCONFIG3_REPORT) || (report == TEST1_REPORT) ||
				(report == TEST2_REPORT) || (report == BOOTCONFIG_REPORT))
			{
				//Clear out the endpoint 0 buffer
				memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

				//Assign the first item in the array to the report number to write
				m_pEndpoint0ReportBuffer[0] = report;

				//Assign the rest of the buffer with the data to write
				for (BYTE i = 0; i < dataBufferSize; i++)
				{
					m_pEndpoint0ReportBuffer[(i * 2) + 1] = (dataBuffer[i] & 0xFF00) >> 8;
					m_pEndpoint0ReportBuffer[(i * 2) + 2] = dataBuffer[i] & 0x00FF;
				}

				//Call set feature to write the data
				if (HidD_SetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
					status = true;
			}
		}
	}
	
	return status;
}

bool CFMRadioDevice::GetRegisterReport(BYTE report, FMRADIO_REGISTER* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;
	char op[256];

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= ((m_Endpoint0ReportBufferSize - 1) / FMRADIO_REGISTER_SIZE))
		{
			//Check to see if the report to read is a single register, or the entire group, or the RDS data
			if ((report >= DEVICEID_REPORT) && (report <= ENTIRE_REPORT))
			{
				//Clear out the endpoint 0 buffer
				memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

				//Assign the first item in the array to the report number to read
				m_pEndpoint0ReportBuffer[0] = report;

				//Call get feature to get the data
				if (HidD_GetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
				{
					//Assign returned data to the dataBuffer
					for (BYTE i = 0; i < dataBufferSize; i++)
					{
						dataBuffer[i] = (m_pEndpoint0ReportBuffer[(i * 2) + 1] << 8) | m_pEndpoint0ReportBuffer[(i * 2) + 2];
					}

					status = true;
				}
			}
			else if (report == RDS_REPORT)
			{
				DWORD bytesRead;
				OVERLAPPED o;

				memset(&o, 0, sizeof(OVERLAPPED));

				o.Offset     = 0; 
				o.OffsetHigh = 0; 
				o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

				//Clear out endpoint 1 buffer
				memset(m_pEndpoint1ReportBuffer, 0, m_Endpoint1ReportBufferSize);

				//Assign the first item in the array to the report number to read
				m_pEndpoint1ReportBuffer[0] = RDS_REPORT;

				ReadFile(m_FMRadioDataHandle, m_pEndpoint1ReportBuffer, m_Endpoint1ReportBufferSize, &bytesRead, &o);
				status = (bytesRead == m_Endpoint1ReportBufferSize);
#ifdef DOLOG
	outfile << "Bytes Read: " << bytesRead << " (of " << m_Endpoint1ReportBufferSize << ")\n";
#endif

/*
				//Sleep(30); // FUCKING Overlapped IO doesn't work which makes this necessary

				//Call a read file on the data handle to read in from endpoint 1
				//if (!ReadFile(m_FMRadioDataHandle, m_pEndpoint1ReportBuffer, m_Endpoint1ReportBufferSize, &bytesRead, &o))
				if (!ReadFile(m_FMRadioDataHandle, m_pEndpoint1ReportBuffer, m_Endpoint1ReportBufferSize, &bytesRead, NULL))

				{
					// If it didn't go through, then wait on the object to complete the read
					DWORD error = GetLastError();
					if (error == ERROR_IO_PENDING) 
					{

						if (WaitForSingleObject(o.hEvent, 3000))
							status = true;

						GetOverlappedResult(m_FMRadioDataHandle, &o, &bytesRead, TRUE);
					
						//sprintf(op, "Overlapped Result, len = %d %s\n", bytesRead, m_pEndpoint1ReportBuffer);
						//OutputDebugString(op);
					}
				}
				else 
				{
					//sprintf(op, "Non Overlapped Result, len = %d %s\n", bytesRead, m_pEndpoint1ReportBuffer);
					//OutputDebugString(op);
					status = true;
				}
*/
				//Close the object
				CloseHandle(o.hEvent);
				

				//If the read succeeded, assign returned data to the dataBuffer
				if (status)
					for (BYTE i = 0; i < dataBufferSize; i++)
					{
						dataBuffer[i] = (m_pEndpoint1ReportBuffer[(i * 2) + 1] << 8) | m_pEndpoint1ReportBuffer[(i * 2) + 2];
					}
			}
		}
	}
	
	return status;
}

bool CFMRadioDevice::SetScratchReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= (m_Endpoint0ReportBufferSize - 1))
		{
			//Clear out the endpoint 0 buffer
			memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

			//Assign the first item in the array to the report number to read
			m_pEndpoint0ReportBuffer[0] = report;

			//Assign the rest of the buffer with the data to write
			for (BYTE i = 0; i < dataBufferSize; i++)
			{
				m_pEndpoint0ReportBuffer[i + 1] = dataBuffer[i];
			}

			//Call set feature to write the data
			if (HidD_SetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
				status = true;
		}
	}
	
	return status;
}

bool CFMRadioDevice::GetScratchReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= (m_Endpoint0ReportBufferSize - 1))
		{
			//Clear out the endpoint 0 buffer
			memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

			//Assign the first item in the array to the report number to read
			m_pEndpoint0ReportBuffer[0] = report;

			//Call get feature to get the data
			if (HidD_GetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
			{
				for (BYTE i = 0; i < dataBufferSize; i++)
				{
					dataBuffer[i] = m_pEndpoint0ReportBuffer[i + 1];
				}

				status = true;
			}
		}
	}
	
	return status;
}

bool CFMRadioDevice::SetLEDReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= (m_Endpoint0ReportBufferSize - 1))
		{
			//Check to see that the report to write is a writeable register
			if (report == LED_REPORT)
			{
				//Clear out the endpoint 0 buffer
				memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

				//Assign the first item in the array to the report number to write
				m_pEndpoint0ReportBuffer[0] = report;

				//Assign the rest of the buffer with the data to write
				for (BYTE i = 0; i < dataBufferSize; i++)
				{
					m_pEndpoint0ReportBuffer[i + 1] = dataBuffer[i];
				}

				//Call set feature to write the data
				if (HidD_SetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
					status = true;
			}
		}
	}
	
	return status;
}

bool CFMRadioDevice::SetStreamReport(BYTE report, BYTE* dataBuffer, DWORD dataBufferSize)
{
	bool status = false;

	//Make sure our handle isn't NULL
	if (m_FMRadioDataHandle)
	{
		//Ensure there will be room in the endpoint buffer for the data requested
		if (dataBufferSize <= (m_Endpoint0ReportBufferSize - 1))
		{
			//Check to see that the report to write is a writeable register
			if (report == STREAM_REPORT)
			{
				//Clear out the endpoint 0 buffer
				memset(m_pEndpoint0ReportBuffer, 0, m_Endpoint0ReportBufferSize);	

				//Assign the first item in the array to the report number to write
				m_pEndpoint0ReportBuffer[0] = report;

				//Assign the rest of the buffer with the data to write
				for (BYTE i = 0; i < dataBufferSize; i++)
				{
					m_pEndpoint0ReportBuffer[i + 1] = dataBuffer[i];
				}

				//Call set feature to write the data
				if (HidD_SetFeature(m_FMRadioDataHandle, m_pEndpoint0ReportBuffer, m_Endpoint0ReportBufferSize))
					status = true;
			}
		}
	}
	
	return status;
}


bool CFMRadioDevice::CreateRadioTimer()
{
	bool ret;
	DWORD ThreadID;

	if (h_radioTimer) {
		// Didn't destroy the old one first!
		return (false);
	}

	// Make sure we just don't quit
	ShouldQuit = false;

	// Create our Radio & RDS threads
	h_radioTimer = CreateThread(NULL, 0, RadioThread, this, 0, &ThreadID); 
	h_rdsTimer = CreateThread(NULL, 0, RDSThread, this, 0, &ThreadID); 
	SetThreadPriority(h_radioTimer, THREAD_PRIORITY_TIME_CRITICAL);
	SetThreadPriority(h_rdsTimer, THREAD_PRIORITY_TIME_CRITICAL);

	m_StreamingAllowed = true;

	return true;
}

bool CFMRadioDevice::DestroyRadioTimer()
{
	bool ret;
	HANDLE thread_handle;

	if (!h_radioTimer) {
		// Already destroyed, or not created!
		return (false);
	}

    // destroy the timer
	m_StreamingAllowed = false;

	// Quit Threads
	ShouldQuit = true;
	Sleep (200);

	// Set they've been terminated
	h_radioTimer = NULL;

	h_rdsTimer = NULL;

	return true;
}
