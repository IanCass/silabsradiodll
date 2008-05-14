// USBRadio.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "USBRadio.h"
#include "FMRadioDevice.h"
#include <time.h>

static CFMRadioDevice fmRadioDevice(true);
static CFMRadioDevice fmRadioDevice2(false);
static RadioData radioData, radioData2;
static bool shouldQuit = false;
static bool SSThreadAlive = false;
static double SecEonFreq = 89.3;
static bool EnableTA = false;
static bool ScanComplete = false;
static bool RRDetected = false;
	

//typedef std::map<WORD, float> tEONMap;
//static tEONMap EONMap;	

typedef struct tPIStruct
{
	double Frequency;
	BYTE SignalStrength;
	int Rating;
	bool FoundLastScan;
}tPIStruct;

typedef std::map<WORD, tPIStruct> tPIMap;
tPIMap PIMap;
	
#define SCAN_SIG_STRENGTH 25 //min signal strength before considered worth looking for PI
#define STATION_PAUSE_PERIOD 20 //time * 100ms to wait for PI to come in
#define PI_RATING_INC 4 //increment for PI rating when it is used foe EON TA

//AFthread tick
#define AF_THREAD_TIMER 100
//periods for EON & AF scanning
#define STATION_SCAN_PERIOD 1200 //time in sec between band scans

#define P_AF_SCAN_PERIOD 600//time in sec between AF checks
#define P_LOW_SIG_AF_SCAN_PERIOD 60 //time in sec between band scans if sig is <20

#define S_AF_SCAN_PERIOD 600//time in sec between AF checks

#define TA_TIMEOUT 120 //max time allowed for TA before reverting in sec

//Minimum signal strength for EON station scanning
//#define EON_SIG_STRENGTH 25 //min signal strength before considered worth looking for PI
//#define EON_SCAN_PERIOD 20
//Time to wait on strong stations to allow PI to be decoded
//#define AF_EON_DELAY 4000


HANDLE h_afTimer;
HANDLE h_SSThread;

//function to scan all EOF freqs to fin strongest
//Disabled, EON seems to send a lot of crap that is not worth processing
//why do I want a EON list that covers the whole country, just want to know what's local
/*void EON_Scan(RDSData *EonData)
{
	// tune each EON station to find stongest freq for each PI
	WORD EonPi = 0;
	WORD BestPIMatchSigStrengh;
	RDSData afData2;
	char c[265];
		
	tEONPIMap::iterator PI_iter, BestPI_iter;
	std::pair <tEONPIMap::iterator, tEONPIMap::iterator> Ret;
	
	if (EonData->EONPIMap.size() > 0) 
	{
		do
		{
			PI_iter = EonData->EONPIMap.upper_bound(EonPi);
			Ret = EonData->EONPIMap.equal_range(PI_iter->first);
			EonPi = Ret.first->first;
			BestPI_iter = NULL;
			BestPIMatchSigStrengh = 0;
	
			for(PI_iter=Ret.first;PI_iter!=Ret.second;PI_iter++)
			{
				double freq = PI_iter->second;
	
				fmRadioDevice2.DoQuickTune(freq);
	
				Sleep(100);
				// get signal strength
				fmRadioDevice2.GetRDSData(&afData2);
				if(afData2.recievedSignalStrength > EON_SIG_STRENGTH)
				{//no point checking PI if sig strength lower than current
					// Let the station settle, RDS updates will be goig on in background
					Sleep(AF_EON_DELAY);
		
					// get RDS data
					fmRadioDevice2.GetRDSData(&afData2);
	
					if(afData2.rdsPI == PI_iter->first)
					{
						//if PI matches
						if(afData2.recievedSignalStrength > BestPIMatchSigStrengh)
						{
							BestPI_iter = PI_iter;
							BestPIMatchSigStrengh = afData2.recievedSignalStrength;
						}
					}
					//sprintf(c, "EON Freq = %.2f, signal strength = %d, stereo = %d, PI = %d\n", freq, afData2.recievedSignalStrength, afData2.isStereo, afData2.rdsPI);
				}
				//else
					//sprintf(c, "EON Freq = %.2f, signal strength = %d, stereo = %d\n", freq, afData2.recievedSignalStrength, afData2.isStereo);
				//OutputDebugString(c);	
			}
			if(BestPIMatchSigStrengh != NULL)
			{
				EONMap[BestPI_iter->first] = BestPI_iter->second;			
				sprintf(c, "EON Freq for PI %d set to %.2f\n", BestPI_iter->first, BestPI_iter->second);
				OutputDebugString(c);
			}
		}
		while(PI_iter!= EonData->EONPIMap.end());

		// tune to our background EON channel
		fmRadioDevice2.DoQuickTune(BGROUND_EON);
	}
}*/

bool SavePIListToFlash(tPIMap *PIMap)
{
	tPIMap::iterator it_locPIMap;
	int i;
	bool Status = true;
	
	//get latest radio data - could have been a tune since read
	if(!fmRadioDevice.pubGetRadioData(&radioData))
		return false;
	if(!fmRadioDevice2.pubGetRadioData(&radioData2))
		return false;

	//update PI list and save
	std::multimap<int, WORD> PI_RankedList;
	PI_RankedList.empty();	
	//create a list ordered in terms of ranking
	for(it_locPIMap = PIMap->begin(); it_locPIMap != PIMap->end(); it_locPIMap++)
	{
		PI_RankedList.insert(std::pair<int, WORD>(it_locPIMap->second.Rating, it_locPIMap->first));
	}
	std::multimap<int, WORD>::iterator it_RankedList;
	if((it_RankedList = PI_RankedList.begin()) != PI_RankedList.end())
	{
		for(i=0;((i<FLASH_PI_NUM)&&(it_RankedList != PI_RankedList.end()));i++)
		{
			radioData.PI[i] = it_RankedList->second;
			radioData.PI_Freq[i] = PIMap->find(it_RankedList->second)->second.Frequency;
			it_RankedList++;
		}
		for(;(i<FLASH_PI_NUM);i++)//fill rest with 0
		{
			radioData.PI[i] = 0;
			radioData.PI_Freq[i] = 0.0;
		}
		for(i=0;((i<FLASH_PI_NUM)&&(it_RankedList != PI_RankedList.end()));i++)
		{
			radioData2.PI[i] = it_RankedList->second;
			radioData2.PI_Freq[i] = PIMap->find(it_RankedList->second)->second.Frequency;
			it_RankedList++;
		}
		for(;(i<FLASH_PI_NUM);i++)//fill rest with 0
		{
			radioData2.PI[i] = 0;
			radioData2.PI_Freq[i] = 0.0;
		}
		
	}
	//is a mutex needed around this? =- No just put it around HID access functions
	if(!fmRadioDevice.pubSetRadioData(&radioData))
		Status = false;
	if(!fmRadioDevice2.pubSetRadioData(&radioData2))
		Status = false;

	return Status;
}



DWORD WINAPI StationScanThread( LPVOID lpParam )
{
	tPIMap *PIMap;
	WORD EonPi = 0;
	WORD BestPIMatchSigStrengh;
	double oldStation;
	RDSData LocalData;
	char c[265];
	tPIStruct localPIStruct;
	tPIMap::iterator it_locPIMap;
	int i;

	PIMap = (tPIMap *)lpParam;
			
	tPIMap::iterator PI_iter;
	std::pair <tPIMap::iterator, tPIMap::iterator> Ret;

	OutputDebugString("Starting Station Sweep...\n");

	for(it_locPIMap = PIMap->begin(); it_locPIMap != PIMap->end();it_locPIMap++)
	{
		it_locPIMap->second.FoundLastScan = false;
		//get current sig strenghts for PIs, otherwise could be using out of data info
		fmRadioDevice2.DoQuickTune(it_locPIMap->second.Frequency);
		Sleep(20);
		fmRadioDevice2.GetRDSData(&LocalData);
		it_locPIMap->second.SignalStrength = LocalData.recievedSignalStrength;
	}	
	//tune to bottom of range
	if(radioData2.band == DATA_BAND_875_108MHZ)
		oldStation = 87.5;
	else
		oldStation = 76.0;

	fmRadioDevice2.DoQuickTune(oldStation);
	LocalData.currentStation = oldStation;
		
	do
	{
		//find next station
		oldStation = LocalData.currentStation;
		fmRadioDevice2.QuickSeek(true);
		Sleep(100);
		// get signal strength
		fmRadioDevice2.GetRDSData(&LocalData);
		
		if(LocalData.recievedSignalStrength > SCAN_SIG_STRENGTH)
		{//no point checking PI if sig strength too low
			for(i=0;i<STATION_PAUSE_PERIOD;i++)
			{
				// Let the station settle, RDS updates will be goig on in background
				Sleep(100);
				// get RDS data
				fmRadioDevice2.GetRDSData(&LocalData);
				if(LocalData.rdsPI != 0)
					break;
			}
			sprintf(c, "Freq = %.2f, PI = %d, signal strength = %d, Search time = %d\n", LocalData.currentStation, LocalData.rdsPI, LocalData.recievedSignalStrength, i);
			OutputDebugString(c);
			if(LocalData.rdsPI != 0)
			{
				localPIStruct.Frequency = LocalData.currentStation;
				localPIStruct.SignalStrength = LocalData.recievedSignalStrength;
				localPIStruct.Rating = 0;
				localPIStruct.FoundLastScan = true;
				if((it_locPIMap = PIMap->find(LocalData.rdsPI)) == PIMap->end())
				{//no match so add
					typedef std::pair <WORD, tPIStruct> tPIStructPair;
					PIMap->insert ( tPIStructPair ( LocalData.rdsPI, localPIStruct ) );
				}
				else //existing PI
				{
					if(localPIStruct.Frequency == it_locPIMap->second.Frequency)
					{//just update sig strength
						it_locPIMap->second.SignalStrength = localPIStruct.SignalStrength;
						it_locPIMap->second.FoundLastScan = true;
					}
					else 
					{
						if(localPIStruct.SignalStrength > it_locPIMap->second.SignalStrength)
						{
							it_locPIMap->second.Frequency = LocalData.currentStation;
							it_locPIMap->second.SignalStrength = LocalData.recievedSignalStrength;
							it_locPIMap->second.FoundLastScan = true;
						}
						else
							it_locPIMap->second.FoundLastScan = true;
					}
				}
			}
		}
		else
		{
			sprintf(c, "Freq = %.2f, signal strength = %d, Too low - skipped\n", LocalData.currentStation, LocalData.recievedSignalStrength);
			OutputDebugString(c);
		}
	}
	while((oldStation < LocalData.currentStation) && !shouldQuit);

	if(shouldQuit)
		return(1);

	for(it_locPIMap = PIMap->begin(); it_locPIMap != PIMap->end();it_locPIMap++)
	{
		if(it_locPIMap->second.FoundLastScan)
			it_locPIMap->second.Rating--;
		else
			it_locPIMap->second.Rating++;
	}

	fmRadioDevice2.DoQuickTune(SecEonFreq);

	SavePIListToFlash(PIMap);

	OutputDebugString("Station Sweep Complete\n");
	SSThreadAlive = false;
	ScanComplete = true;

	return(0);
}


/**
* Thread to Poll the primary radio device & see if it
* needs switching frequency
*/
DWORD WINAPI AFThread( LPVOID lpParam )
{
	RDSData afData, afData2;
	char c[265];
	int loopTime = 1000, i;
	WORD PriEONCount = 0;
	WORD P_AFCount = 0, S_AFCount = 0;//(9*(AF_SCAN_PERIOD * (1000/AF_THREAD_TIMER)))/10;
	WORD ScanCount = (9*(STATION_SCAN_PERIOD * (1000/AF_THREAD_TIMER)))/10;
	tPIStruct localPIStruct;
	typedef std::pair <WORD, tPIStruct> tPIStructPair;
	tPIMap::iterator it_PIMap;
	int TA_Timeout = 0;
	int OldStation = 0;
	double NewStation = 0;
	double SecOldStation = 0;
	BYTE SecOldSigStrength = 0;
			

	WORD TA_PI;
	enum t_State {IDLE, CHECK_AF_SIG, SEC_AF_CHECK, SEC_CHECK_AF_SIG, PRIMARY_TA, SECONDARY_TA, TA_RETUNE, SECONDARY_EON, PRIMARY_EON} State;

	//update our PI list from primary radio memory
	for(i=0;i<FLASH_PI_NUM;i++)
	{
		if(radioData.PI[i] != 0)
		{
			localPIStruct.Frequency = radioData.PI_Freq[i];
			localPIStruct.SignalStrength = 0;
			localPIStruct.Rating = 0;
			localPIStruct.FoundLastScan = true;
			PIMap.insert ( tPIStructPair ( radioData.PI[i], localPIStruct ) );
			sprintf(c, "Primary Flash PI = %d, Frequency = %.2f\n", radioData.PI[i], radioData.PI_Freq[i]);
			OutputDebugString(c);
		}
		else
			break;
	}

	//update our PI list from secondary radio memory
	for(i=0;i<FLASH_PI_NUM;i++)
	{
		if(radioData2.PI[i] != 0)
		{
			localPIStruct.Frequency = radioData2.PI_Freq[i];
			localPIStruct.SignalStrength = 0;
			localPIStruct.Rating = 0;
			localPIStruct.FoundLastScan = true;
			PIMap.insert ( tPIStructPair ( radioData2.PI[i], localPIStruct ) );
			sprintf(c, "Secondary Flash PI = %d, Frequency = %.2f\n", radioData2.PI[i], radioData2.PI_Freq[i]);
			OutputDebugString(c);
		}
		else
			break;
	}

	if(radioData2.EonPI==0)
	{
		radioData2.EonPI = 49666; //BBCR2
		sprintf(c, "No Secondary EON Staion Specified, set to BBC R2\n");
	}

	//find freq for EON PI
	if((it_PIMap = PIMap.find(radioData2.EonPI)) != PIMap.end())
	{//we have a valid frequency switch main tuner
		SecEonFreq = it_PIMap->second.Frequency;
		sprintf(c, "Secondary EON set to PI = %d, Freq = %.2f\n", radioData2.EonPI, SecEonFreq);
	}
	else //use first preset
	{
		SecEonFreq = 89.3;
		sprintf(c, "No EON Frequency Found, set to 89.3MHz\n");
	}
	OutputDebugString(c);
						
	
	// tune to our background EON channel
	fmRadioDevice2.DoQuickTune(SecEonFreq);

	State = IDLE;
	
	while(!shouldQuit) {
		//int seconds = time(NULL);

		switch(State)
		{
		case IDLE :
			//check to see if primary station is in PI list
			fmRadioDevice.GetRDSData(&afData);
			if(afData.rdsPI != 0)
				if(PIMap.find(afData.rdsPI) == PIMap.end())
				{
					localPIStruct.Frequency = afData.currentStation;
					localPIStruct.SignalStrength = afData.recievedSignalStrength;
					localPIStruct.Rating = 0;
					localPIStruct.FoundLastScan = true;
					PIMap.insert ( tPIStructPair ( afData.rdsPI, localPIStruct ) );
				}
			if(fmRadioDevice.m_TrafficStartFlag && EnableTA) //primary TA
			{
				OutputDebugString("Primary TA \n");
				//let FE know
				fmRadioDevice.Send_TA_Start();
				//Send_TA_Start();
				TA_Timeout = (TA_TIMEOUT * (1000/AF_THREAD_TIMER));
				State = PRIMARY_TA;
			}
			else if(fmRadioDevice2.m_TrafficStartFlag && EnableTA) //secondary TA
			{
				//switch primary radio station
				OutputDebugString("Secondary TA \n");
				OldStation = fmRadioDevice.CurrFreq;
				//let FE know
				fmRadioDevice.Send_TA_Start();
				fmRadioDevice.DoQuickTune(fmRadioDevice2.CurrFreq/10.0);
				TA_Timeout = (TA_TIMEOUT * (1000/AF_THREAD_TIMER));
				State = SECONDARY_TA;
			}
			if(fmRadioDevice2.m_EONTAStart && !SSThreadAlive && EnableTA) //secondary EON
			{	//TA in progress and not scaning
				if((it_PIMap = PIMap.find(fmRadioDevice2.m_EONTAPIStart)) != PIMap.end())
				{//we have a valid frequency switch main tuner
					sprintf(c, "Secondary EON TA, PI = %d, Freq = %.2f\n", fmRadioDevice2.m_EONTAPIStart, it_PIMap->second.Frequency);
					//increase rating - this PI is usefull
					TA_PI = fmRadioDevice2.m_EONTAPIStart;
					it_PIMap->second.Rating -= PI_RATING_INC;
					OldStation = fmRadioDevice.CurrFreq;
					fmRadioDevice.DoQuickTune(it_PIMap->second.Frequency);
					//fet FE know
					fmRadioDevice.Send_TA_Start();
					TA_Timeout = (TA_TIMEOUT * (1000/AF_THREAD_TIMER));
					State = SECONDARY_EON;
				}
				else
					sprintf(c, "Secondary EON TA with PI of %d, no freq available\n", fmRadioDevice2.m_EONTAPIStart);
				OutputDebugString(c);
			}
			if(fmRadioDevice.m_EONTAStart && EnableTA) //primary EON
			{	//TA in progress
				if((it_PIMap = PIMap.find(fmRadioDevice.m_EONTAPIStart)) != PIMap.end())
				{//we have a valid frequency switch main tuner
					sprintf(c, "Primary EON TA, PI = %d, Freq = %.2f\n", fmRadioDevice.m_EONTAPIStart, it_PIMap->second.Frequency);
					//increase rating - this PI is usefull
					TA_PI = fmRadioDevice.m_EONTAPIStart;
					it_PIMap->second.Rating -= PI_RATING_INC;
					OldStation = fmRadioDevice.CurrFreq;
					fmRadioDevice.DoQuickTune(it_PIMap->second.Frequency);
					//fet FE know
					fmRadioDevice.Send_TA_Start();
					TA_Timeout = (TA_TIMEOUT * (1000/AF_THREAD_TIMER));
					State = PRIMARY_EON;
				}
				else
					sprintf(c, "Primary EON TA with PI of %d, no freq available\n", fmRadioDevice.m_EONTAPIStart);
				OutputDebugString(c);
			}
			else if(ScanCount>=(STATION_SCAN_PERIOD* (1000/AF_THREAD_TIMER)) ||
				((P_AFCount>=(P_LOW_SIG_AF_SCAN_PERIOD * (1000/AF_THREAD_TIMER))) && (afData.recievedSignalStrength < 20) && (afData.rdsPI != 0)))
			{ //force early scan if primary is strength low, need a PI though to identify new station
				ScanCount = 0;
				P_AFCount = 0;
				if(!SSThreadAlive)
				{
					SSThreadAlive = true;
					h_SSThread = CreateThread(NULL, 0, StationScanThread, (LPVOID*)&PIMap , 0, NULL); 
					SetThreadPriority(h_SSThread, THREAD_PRIORITY_LOWEST);
				}
			}
			else if(ScanComplete)
			{ //new PI data available so see if we are on best frew
			ScanComplete = false;
				OutputDebugString("CHECK_AF\n");
				if((it_PIMap = PIMap.find(afData.rdsPI)) != PIMap.end())
				{//have an entry in PI list
					if(it_PIMap->second.FoundLastScan && 
						(it_PIMap->second.Frequency != afData.currentStation))
					{//alternative freq available, available on last scan
						NewStation = it_PIMap->second.Frequency;
						sprintf(c, "AF Re-tune, Old Freq = %.2f, New Freq = %.2f\n", afData.currentStation, NewStation);
						OutputDebugString(c);
						fmRadioDevice2.DoQuickTune(it_PIMap->second.Frequency);
						State = CHECK_AF_SIG;
					}
					else
						State = SEC_AF_CHECK;
				}
				else
					State = SEC_AF_CHECK;
			}
			break;
		case CHECK_AF_SIG:
			OutputDebugString("CHECK_AF_SIG\n");
			fmRadioDevice2.GetRDSData(&afData2);
			//check current sig strength and switch if better
			if(afData2.recievedSignalStrength > (afData.recievedSignalStrength+4))
			{
				fmRadioDevice.QueFreq = (int)((NewStation+0.01)*10);
				//fmRadioDevice.DoTune(NewStation);
				OutputDebugString("Tuned to new frequency\n");
			}
			fmRadioDevice2.DoQuickTune(SecEonFreq);
			State = SEC_AF_CHECK;
			break;
		case SEC_AF_CHECK:
			OutputDebugString("SEC_AF_CHECK\n");
			fmRadioDevice2.GetRDSData(&afData2);
			if((it_PIMap = PIMap.find(radioData2.EonPI)) != PIMap.end())
			{//have an entry in PI list
				if(it_PIMap->second.FoundLastScan && 
					(it_PIMap->second.Frequency != afData2.currentStation))
				{//alternative freq available, available on last scan
					SecOldStation = afData2.currentStation;
					SecOldSigStrength = afData2.recievedSignalStrength;
					sprintf(c, "Sec AF Re-tune, Old Freq = %.2f, New Freq = %.2f\n", afData2.currentStation, it_PIMap->second.Frequency);
					OutputDebugString(c);
					SecEonFreq = it_PIMap->second.Frequency;
					fmRadioDevice2.DoQuickTune(SecEonFreq);
					State = SEC_CHECK_AF_SIG;
				}
				else
					State = IDLE;
			}
			else 
				State = IDLE;
			break;
		case SEC_CHECK_AF_SIG:
			OutputDebugString("SEC_CHECK_AF_SIG\n");
			fmRadioDevice2.GetRDSData(&afData2);
			if(afData2.recievedSignalStrength < SecOldSigStrength)
			{ //check current sig strength and revert if worse
				SecEonFreq = SecOldStation;
				fmRadioDevice2.DoQuickTune(SecEonFreq);
				OutputDebugString("Secondary Reverted to old frequency\n");
			}
			State = IDLE;
			break;
		case PRIMARY_TA:
			OutputDebugString("PRIMARY_TA\n");
			//sit here till timeout or end
			if(!fmRadioDevice.m_TrafficEndFlag || (TA_Timeout == 0))
			{
				//let FE know
				fmRadioDevice.Send_TA_Stop();
				State = IDLE;
			}
			break;
		case SECONDARY_TA:
			OutputDebugString("SECONDARY_TA\n");
			//sit here till timeout or end
			if(fmRadioDevice.m_TrafficEndFlag || (TA_Timeout == 0))
			{
				//let FE know
				fmRadioDevice.Send_TA_Stop();
				State = TA_RETUNE;
			}
			break;
		case TA_RETUNE:
			OutputDebugString("TA_RETUNE\n");
			//let FE know 
			fmRadioDevice.Send_TA_Stop();
			fmRadioDevice.DoQuickTune(OldStation/10.0);
			State = IDLE;
			break;
		case SECONDARY_EON:
			OutputDebugString("SEC_EON\n");
			if(fmRadioDevice.m_TrafficStartFlag)
			{//RDS stream found on new station on primary so rely on this
				State = SECONDARY_TA;
			}
			else if(((fmRadioDevice2.m_EONTAStop) && (TA_PI == fmRadioDevice2.m_EONTAPIStop)) || (TA_Timeout == 0))
				{//EON TA end flag found for this station on secondary or timeout
					State = TA_RETUNE;
				}
			break;
		case PRIMARY_EON:
			OutputDebugString("PRIMARY_EON\n");
			if(fmRadioDevice.m_TrafficStartFlag)
			{//RDS stream found on new station on primary so rely on this
				State = SECONDARY_TA;
			}
			else if(TA_Timeout == 0)
				{
					State = TA_RETUNE;
				}
			break;
		}

		//abortive early version, leave here for now just in case

		/*fmRadioDevice2.GetRDSData(&afData);
		if(afData.mEON_TrafficPI != 0) 
		{	//TA in progress
			//if we have a valid freuwncy switch main tuner
			if((it_PIMap = PIMap.find(afData.mEON_TrafficPI)) != PIMap.end())
			{
				//increase rating - this PI is usefull
				it_PIMap->second.Rating -= PI_RATING_INC;
				fmRadioDevice.DoQuickTune(it_PIMap->second.Frequency);
			}
			else
			{
				sprintf(c, "Secondary EON TA with PI of %d, no freq available\n", afData.mEON_TrafficPI);
				OutputDebugString(c);
			}
		}*/


		//EON scan for Secondary EON freqs - do first as scan will destroy EON info
		/*if(SecEONCount>=EON_SCAN_PERIOD)
		{
			SecEONCount = 0;
			//OutputDebugString("Start Secondary EON Sweep...\n");
			fmRadioDevice2.GetRDSData(&afData);
			//EON_Scan(&afData);
		}

		//EON scan for Primary EON freqs
		if(PriEONCount>=EON_SCAN_PERIOD)
		{
			PriEONCount = 0;
			//OutputDebugString("Start Primary EON Sweep...\n");
			fmRadioDevice.GetRDSData(&afData);
			//EON_Scan(&afData);
		}
		
		if(AFCount>=AF_SCAN_PERIOD)
		{
			AFCount = 0;
			//OutputDebugString("Start AF Sweep...\n");
		
			//sprintf(c, "Got RDS Data, Freq = %.2f, Signal strength = %d\n", afData.currentStation, afData.recievedSignalStrength);
			OutputDebugString(c);
			int cf = fmRadioDevice.CurrFreq;
	
			// tune each AF station to check if the signal is
			// better than current
			if (afData.AFMap.size() > 0) 
			{
				std::map<float, float>::iterator iter; // our i for looping
				for(iter = afData.AFMap.begin(); iter != afData.AFMap.end(); iter++) 
				{
					double freq = iter->first;

					// tune to our candidate freq
					fmRadioDevice2.DoQuickTune(freq);
	
					Sleep(10);
					// get signal strength
					fmRadioDevice2.GetRDSData(&afData2);
					if(afData2.recievedSignalStrength > afData.recievedSignalStrength)
					{//no point checking PI if sig strength lower than current
						// Let the station settle, RDS updates will be goig on in background
						Sleep(AF_EON_DELAY);
		
						// Update teh stats
						// repeat lots of times in case we have crud
						//for (int i = 0; i < 100; i++) {
						//	fmRadioDevice2.updateRDSData();
						//}
		
						// get RDS data
						fmRadioDevice2.GetRDSData(&afData2);
	
						//sprintf(c, "AF Freq = %.2f, signal strength = %d, stereo = %d, PI = %d\n", freq, afData2.recievedSignalStrength, afData2.isStereo, afData2.rdsPI);
					}
					else
						//sprintf(c, "AF Freq = %.2f, signal strength = %d, stereo = %d\n", freq, afData2.recievedSignalStrength, afData2.isStereo);
					OutputDebugString(c);	
	
					
					if (shouldQuit) break;
					if (cf != fmRadioDevice.CurrFreq) {
						//OutputDebugString("Aborting AF Sweep = channel changed...\n");
						break;
					}
				}
			}
			// tune to our background EON channel
			fmRadioDevice2.DoQuickTune(BGROUND_EON);
			//OutputDebugString("End AF Sweep...\n");
		}*/

		ScanCount++;
		P_AFCount++;
		S_AFCount++;
		if(TA_Timeout)
			TA_Timeout--;
		Sleep(AF_THREAD_TIMER);
	}

	// Got here, then we've been requested to quit (Shouldquit=true)
	return 0;
}




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
OpenFMRadio (CFMRadioDevice* fmDevice, RadioData* RadioData)
{
	BYTE RetVal;
	if ((RetVal = fmDevice->OpenFMRadio(RadioData)) == STATUS_OK ) //{
		fmDevice->InitializeStream();
	if(RetVal != STATUS_FMRADIODATA_ERROR)
		fmDevice->CreateRadioTimer();
	//}

	return (true);
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
	shouldQuit = false;
	SSThreadAlive = false;
	EnableTA = true;
	ScanComplete = false;

	bool ret = OpenFMRadio(&fmRadioDevice, &radioData);
	ret = OpenFMRadio(&fmRadioDevice2, &radioData2);

	h_afTimer = CreateThread(NULL, 0, AFThread, NULL, 0, NULL); 
	SetThreadPriority(h_afTimer, THREAD_PRIORITY_LOWEST);

	return (ret);
}

USBRADIO_API bool __stdcall
HWDeInit ()
{
	SavePIListToFlash(&PIMap);

	CloseFMRadio(&fmRadioDevice);
	CloseFMRadio(&fmRadioDevice2);

	// Quit Threads
	shouldQuit = true;
	Sleep (200);

	// Set they've been terminated
	h_afTimer = NULL;
	h_SSThread = NULL;

	return true;

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

	if(!RRDetected)
	{
		if (fmRadioDevice.GetRDSData(&rds_data)) {
			return rds_data.isStereo;
		} else {
			return (false);
		}
	}
	else
	{
		if (fmRadioDevice.GetRDSData(&rds_data)) {
			return !rds_data.isStereo;
		} else {
			return (true);
		}
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

//Temp mod to return PI list, (can't mod VB app to report both)
USBRADIO_API bool __stdcall VB_GetAFList (float* ary, int* arysize) {

	RDSData rds_data;

	if (*arysize < 1) return false;

	/*if (fmRadioDevice.GetRDSData(&rds_data)) {

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
	*/

	tPIMap::iterator it_locPIMap;
	int cnt = 0;
	for(it_locPIMap = PIMap.begin(); it_locPIMap != PIMap.end(); it_locPIMap++)
	{
		ary[cnt] = (float)it_locPIMap->first;
		cnt++;
		if (cnt > *arysize - 1) break;
	}
	if(cnt>0)
	{
		*arysize = cnt;// - 1;
		return true;
	}
	else
	{
		*arysize = 0;
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
	EnableTA = true;
	return fmRadioDevice.RTAStart(windowName, dwData, lpData);
}

USBRADIO_API bool __stdcall RegisterTAStop (char windowName[256], short dwData, char lpData[256])
{
	EnableTA = true;
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
	/*if (fmRadioDevice.GetRegisterReport(RDS_REPORT, &allRegisters[STATUSRSSI], RDS_REGISTER_NUM)) {

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
	}*/

	//backward compatibility for RR and ohers
	RDSData rds_data;
	static int muxCount = 0;
	static int RDSOffset = 0;
	static int dispCount = 0;
	std::string myString;
	char string1[4];

	RRDetected = true;
	
	if (fmRadioDevice.GetRDSData(&rds_data)) 
	{
		allRegisters[DEVICEID] = 4674;
		allRegisters[CHIPID] = 2575;
		allRegisters[POWERCFG] = 16897;
		allRegisters[CHANNEL] = 134;
		allRegisters[SYSCONFIG1] = 6144;
		allRegisters[SYSCONFIG2] = 4127;
		allRegisters[SYSCONFIG3] = 0;
		allRegisters[TEST1] = 15364;
		allRegisters[TEST2] = 8;
		allRegisters[BOOTCONFIG] = 1;
		allRegisters[STATUSRSSI] = STATUSRSSI_RDSR | rds_data.recievedSignalStrength;
		allRegisters[RDSA] = rds_data.rdsPI;

		if(muxCount<4)
		{
			myString = rds_data.rdsPS;
			allRegisters[RDSB] = muxCount;
			if(rds_data.rdsTP) allRegisters[RDSB] |= 0x0400; //RR ignores this!
			if(rds_data.rdsTA) allRegisters[RDSB] |= 0x0010;
			allRegisters[RDSC] = 0x00;
			while(myString.size()<8)
				myString.append(" ");
			allRegisters[RDSD] = (((WORD)myString[muxCount*2])<<8) | myString[(muxCount*2)+1];
			if(muxCount == 3)
			{
				if(dispCount++>3)
				{
					dispCount=0;
					if(RDSOffset == 0)
						RDSOffset = 8;
					else
						RDSOffset = 0;
				}
			}
		}
		else
		{
			int locCount = (muxCount - 4) + RDSOffset;
			myString = rds_data.rdsText;
			if(myString.size()<32)//F(*(*^ RR can only take 32 chars!
			{
				myString.append("\r");
				while((myString.size()%4)!=0)
					myString.append(" ");
			}
			//string is now correctly terminated and an exact multiple of 4
			if(myString.size() >= ((locCount+1) * 4))
			{
				allRegisters[RDSB] = 0x2000 | (locCount&0x07);
				if(rds_data.rdsTP) allRegisters[RDSB] |= 0x0400;
				allRegisters[RDSC] = (((WORD)myString[locCount*4])<<8) | myString[(locCount*4)+1];
				allRegisters[RDSD] = (((WORD)myString[(locCount*4)+2])<<8) | myString[(locCount*4)+3];
				
				string1[0] = myString[locCount*4];
				string1[1] = myString[locCount*4+1];
				string1[2] = myString[locCount*4+2];
				string1[3] = myString[locCount*4+3];
			}
			else
				locCount = 20;//string complete, restart
		}

		muxCount++;
		if(muxCount>11)
			muxCount = 0;

		for (BYTE i = 0; i < FMRADIO_REGISTER_NUM; i++)
		{
			szRetBuf[(i * 2)] = (allRegisters[i] & 0xFF00) >> 8;
			szRetBuf[(i * 2) + 1] = allRegisters[i] & 0x00FF;
		}
		*iRetBufSize=FMRADIO_REGISTER_NUM*FMRADIO_REGISTER_SIZE;
	} 
	else 
	{
		return false;
	}
	return false;
}

USBRADIO_API bool __stdcall SetExFlags(long Flags)
{
	// Set Ex Flags -- Some featueres may only have effect if called before HWInit
	fmRadioDevice.ExFlags = Flags;
	return true;
}
