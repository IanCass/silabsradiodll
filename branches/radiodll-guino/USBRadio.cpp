// USBRadio.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "USBRadio.h"
#include "FMRadioDevice.h"

static CFMRadioDevice fmRadioDevice(true);
static RadioData radioData;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

bool
OpenFMRadio (CFMRadioDevice* fmDevice)
{
	if (fmDevice->OpenFMRadio(&radioData) == STATUS_OK ) {
		fmDevice->StreamAudio();
		fmDevice->CreateRadioTimer();
		return (true);
	}

	return (false);
}

bool
CloseFMRadio (CFMRadioDevice* fmDevice)
{
	if (fmDevice->DestroyRadioTimer())
	{
		fmDevice->CloseFMRadio();
		return (true);
	}

	return (false);
}

bool
GetRDSInfo (CFMRadioDevice* fmDevice, RDSData* rdsData)
{
	return (fmDevice->GetRDSData(rdsData));
}

USBRADIO_API long __stdcall
GetCurrStation ()
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		return (rds_data.currentStation * 1000);
	} else {
		return (0);
	}
}

USBRADIO_API long __stdcall
SeekStation (bool SeekUp)
{
	fmRadioDevice.Seek(SeekUp);

	return (GetCurrStation());
}

USBRADIO_API bool __stdcall 
FMTune (long frequency)
{
	return (fmRadioDevice.Tune((double)frequency/1000));
}

//
// Radiator Interface support
//
USBRADIO_API char* __stdcall
GetModuleName ()
{
	return ("SiLabs USB FM Radio (http://silabsradiodll.googlecode.com)");
}

USBRADIO_API unsigned long __stdcall
GetModuleInfo ()
{
	return (1+4+8+16+32+128+512+2048+4096);
}

USBRADIO_API bool __stdcall
HWInit ()
{
	bool ret = OpenFMRadio(&fmRadioDevice);

	return (ret);
}

USBRADIO_API bool __stdcall
HWDeInit ()
{
	return (CloseFMRadio(&fmRadioDevice));
}

USBRADIO_API void __stdcall
TuneFreq (long frequency)
{
	fmRadioDevice.Tune((double)frequency/1000);
}

USBRADIO_API void __stdcall
SetMute (bool mute)
{
	fmRadioDevice.Mute(mute);
}

USBRADIO_API long __stdcall
ScanStation (bool directionUpDown, long startFrequency)
{
	RDSData rds_data;

	if (fmRadioDevice.Tune((double)startFrequency/1000)) {
		fmRadioDevice.Seek(directionUpDown);
	}

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		return (rds_data.currentStation * 1000);
	} else {
		return startFrequency;
	}
}

USBRADIO_API unsigned int __stdcall
GetSignal ()
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		return (rds_data.recievedSignalStrength);
	} else {
		return (0);
	}
}

USBRADIO_API bool __stdcall
IsStereo ()
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		return rds_data.isStereo;
	} else {
		return (false);
	}
}

USBRADIO_API unsigned int __stdcall
GetVolume ()
{
	return (fmRadioDevice.GetWaveOutVolume());
}

USBRADIO_API void __stdcall
SetVolume (unsigned int left, unsigned int right)
{
	fmRadioDevice.SetWaveOutVolume(left);
}

USBRADIO_API void __stdcall
VolumeUpDown (int step)
{
	int curr = (int)GetVolume();

	// Make sure we don't roll over 100% or under 0%
	if ((curr + step) > 100) {
		curr = 100; step = 0;
	} else if ((curr + step) < 0) {
		curr = 0; step = 0;
	}

	SetVolume(curr + step, curr + step);
}

USBRADIO_API const char* __stdcall
GetRDS ()
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		return (rds_data.rdsText.c_str());
	} else {
		return ("");
	}
}

USBRADIO_API bool __stdcall
FMTuneUp ()
{
	return (fmRadioDevice.Tune(true));
}

USBRADIO_API bool __stdcall
FMTuneDown ()
{
	return (fmRadioDevice.Tune(false));
}

USBRADIO_API bool __stdcall VB_GetModuleName (char szReturnModuleName[256], short *iSize)
{

	*iSize=strlen("Silicon Labs USB FM Radio Reference Design (Appy v0.1)");
	strncpy(szReturnModuleName, "Silicon Labs USB FM Radio Reference Design (Appy v0.1)", *iSize);

	return true;
}

USBRADIO_API bool __stdcall VB_GetRDS (char szRetRDS[256], short *iRetSize)
{
	return VB_GetRDSText(szRetRDS, iRetSize);
}

USBRADIO_API bool __stdcall VB_GetRDSText (char szRetRDS[256], short *iRetSize)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*iRetSize=strlen(rds_data.rdsText.c_str());
		strncpy(szRetRDS, rds_data.rdsText.c_str(), *iRetSize);
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetFrequency (double frequency)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		frequency = rds_data.currentStation;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPS (char szRetRDS[8], short *iRetSize)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*iRetSize=strlen(rds_data.rdsPS.c_str());
		strncpy(szRetRDS, rds_data.rdsPS.c_str(), *iRetSize);
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPIRegion (char szRetRDS[256], short *iRetSize)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*iRetSize=strlen(rds_data.rdsPIRegion.c_str());
		strncpy(szRetRDS, rds_data.rdsPIRegion.c_str(), *iRetSize);
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPICountry (char szRetRDS[256], short *iRetSize)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*iRetSize=strlen(rds_data.rdsPICountry.c_str());
		strncpy(szRetRDS, rds_data.rdsPICountry.c_str(), *iRetSize);
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPI (int *rdsPI)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*rdsPI = rds_data.rdsPI;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPTY (short *rdsPTY)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*rdsPTY = rds_data.rdsPTY;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSPTYString (char szReturnString[256], short *iRetSize)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*iRetSize=strlen(rds_data.rdsPTYString.c_str());
		strncpy(szReturnString, rds_data.rdsPTYString.c_str(), *iRetSize);
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSMS (bool *res)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*res = rds_data.rdsMS;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSTP (bool *res)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*res = rds_data.rdsTP;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRDSTA (bool *res)
{
	RDSData rds_data;

	if (fmRadioDevice.GetRDSData(&rds_data)) {
		*res = rds_data.rdsTA;
		return true;
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetAFList (float* ary, int* arysize) {

	RDSData rds_data;

	if (*arysize < 1) return false;

	if (fmRadioDevice.GetRDSData(&rds_data)) {

		if (rds_data.AFMap.size() > 0) {
	
			std::map<float, float>::iterator iter; // our i for looping
			int cnt = 0;
			for(iter = rds_data.AFMap.begin(); iter != rds_data.AFMap.end(); iter++) {
				ary[cnt] = iter->first;
				cnt++;
				if (cnt > *arysize - 1) break;
			}
			*arysize = rds_data.AFMap.size();
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

USBRADIO_API bool __stdcall VB_GetRadioRegisters (char szRetBuf[256], short *iRetBufSize)
{
	FMRADIO_REGISTER	allRegisters[FMRADIO_REGISTER_NUM];

	//Read all the registers and fill the buffer
	if (fmRadioDevice.GetRegisterReport(ENTIRE_REPORT, allRegisters, FMRADIO_REGISTER_NUM)) {

		//Get the RDS report from the device
		if (fmRadioDevice.GetRegisterReport(RDS_REPORT, &allRegisters[STATUSRSSI], RDS_REGISTER_NUM)) {

			//Assign returned data to the szRetBuf
			for (BYTE i = 0; i < FMRADIO_REGISTER_NUM; i++)
			{
				szRetBuf[(i * 2)] = (allRegisters[i] & 0xFF00) >> 8;
				szRetBuf[(i * 2) + 1] = allRegisters[i] & 0x00FF;
			}
			*iRetBufSize=FMRADIO_REGISTER_NUM*FMRADIO_REGISTER_SIZE;

			return true;

		} else {

			return false;
		}
	} else {

		return false;
	}

}

USBRADIO_API bool __stdcall RegisterTAStart (char windowName[256], short dwData, char lpData[256])
{
	return fmRadioDevice.RTAStart(windowName, dwData, lpData);
}

USBRADIO_API bool __stdcall RegisterTAStop (char windowName[256], short dwData, char lpData[256])
{
	return fmRadioDevice.RTAStop(windowName, dwData, lpData);
}

USBRADIO_API bool __stdcall RegisterRadioText (char windowName[256], short dwData, char lpData[256])
{
	return fmRadioDevice.RRadioText(windowName, dwData, lpData);
}

USBRADIO_API bool __stdcall VB_GetRDSRegisters (char szRetBuf[256], short *iRetBufSize)
{
	FMRADIO_REGISTER	allRegisters[FMRADIO_REGISTER_NUM];

	//Get the RDS report from the device
	if (fmRadioDevice.GetRegisterReport(RDS_REPORT, &allRegisters[STATUSRSSI], RDS_REGISTER_NUM)) {

		//Assign returned data to the szRetBuf
		for (BYTE i = 0; i < FMRADIO_REGISTER_NUM; i++)
		{
			szRetBuf[(i * 2)] = (allRegisters[i] & 0xFF00) >> 8;
			szRetBuf[(i * 2) + 1] = allRegisters[i] & 0x00FF;
		}
		*iRetBufSize=FMRADIO_REGISTER_NUM*FMRADIO_REGISTER_SIZE;

		return true;

	} else {

		return false;
	}

}
