
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the USBRADIO_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// USBRADIO_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#include "FMRadioDevice.h"
//#include "oleauto.h"

#ifdef USBRADIO_EXPORTS
#define USBRADIO_API __declspec(dllexport)
#else
#define USBRADIO_API __declspec(dllimport)
#endif

//Tested
USBRADIO_API bool __stdcall         HWInit();
USBRADIO_API bool __stdcall         HWDeInit();
USBRADIO_API bool __stdcall         FMTune (long frequency);
USBRADIO_API unsigned int __stdcall GetSignal();
USBRADIO_API long __stdcall         GetCurrStation();
USBRADIO_API long __stdcall         SeekStation(bool SeekUp);
USBRADIO_API bool __stdcall         IsStereo();
USBRADIO_API const char* __stdcall  GetRDS();

USBRADIO_API long __stdcall ScanStation (bool directionUpDown, long startFrequency);

USBRADIO_API bool __stdcall FMTuneUp ();
USBRADIO_API bool __stdcall FMTuneDown ();
USBRADIO_API char* __stdcall GetModuleName();
USBRADIO_API unsigned long __stdcall GetModuleInfo();

USBRADIO_API void __stdcall TuneFreq (long frequency);

USBRADIO_API void __stdcall SetMute(bool mute);



USBRADIO_API unsigned int __stdcall GetVolume();

// Doesn't support different volume levels per side, ignores right side
USBRADIO_API void __stdcall SetVolume(unsigned int left, unsigned int right);

USBRADIO_API void __stdcall VolumeUpDown(int step);

//VB calls!
USBRADIO_API bool __stdcall     VB_GetModuleName (char szReturnModuleName[256], short *iSize);
USBRADIO_API bool __stdcall     VB_GetFrequency (double frequency);
USBRADIO_API bool __stdcall     VB_GetRDSText (char szRetRDS[256], short *iRetSize);
USBRADIO_API bool __stdcall     VB_GetRDSPS (char szRetRDS[8], short *iRetSize);
USBRADIO_API bool __stdcall		VB_GetRDSPIRegion (char szRetRDS[8], short *iRetSize);
USBRADIO_API bool __stdcall		VB_GetRDSPICountry (char szRetRDS[8], short *iRetSize);
USBRADIO_API bool __stdcall		VB_GetRDSPI (int *rdsPI);
USBRADIO_API bool __stdcall		VB_GetRDSPTY (short *rdsPTY);
USBRADIO_API bool __stdcall		VB_GetRDSPTY (bool *rdsTA);
USBRADIO_API bool __stdcall		VB_GetRDSPTY (bool *rdsTP);
USBRADIO_API bool __stdcall		VB_GetRDSPTY (bool *rdsMS);

USBRADIO_API bool __stdcall     VB_GetRadioRegisters (char szRetBuf[256], short *iRetBufSize);
USBRADIO_API bool __stdcall VB_GetAFList (float* ary, int* arysize);
USBRADIO_API bool __stdcall     VB_GetRDSRegisters (char szRetBuf[256], short *iRetBufSize);
USBRADIO_API bool __stdcall RegisterTAStart (char windowName[256], short dwData, char lpData[256]);
USBRADIO_API bool __stdcall RegisterTAStop (char windowName[256], short dwData, char lpData[256]);
USBRADIO_API bool __stdcall RegisterRadioText (char windowName[256], short dwData, char lpData[256]);
USBRADIO_API bool __stdcall SetExFlags(long Flags);






