// RDSData.h: interface for the CRDSData class.
//
//////////////////////////////////////////////////////////////////////



#if !defined(AFX_RDSDATA_H__A14C0C6B_E19E_4099_AD73_0EC6272CB271__INCLUDED_)
#define AFX_RDSDATA_H__A14C0C6B_E19E_4099_AD73_0EC6272CB271__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <map>
#include <fstream>


//Default text string for the RDS
//#define DEFAULT_RDS_TEXT	"STAY IN TUNE WITH SILICON LABS FM TUNER TECHNOLOGY - "
#define DEFAULT_RDS_TEXT	""

typedef struct rdsFifo_struct_tag {
	WORD a;
	WORD b;
	WORD c;
	WORD d;
} rdsFifo_struct;

typedef std::map<float, float> tAFMap;

#define RDS_TYPE_0A     ( 0 * 2 + 0)
#define RDS_TYPE_0B     ( 0 * 2 + 1)
#define RDS_TYPE_1A     ( 1 * 2 + 0)
#define RDS_TYPE_1B     ( 1 * 2 + 1)
#define RDS_TYPE_2A     ( 2 * 2 + 0)
#define RDS_TYPE_2B     ( 2 * 2 + 1)
#define RDS_TYPE_3A     ( 3 * 2 + 0)
#define RDS_TYPE_3B     ( 3 * 2 + 1)
#define RDS_TYPE_4A     ( 4 * 2 + 0)
#define RDS_TYPE_4B     ( 4 * 2 + 1)
#define RDS_TYPE_5A     ( 5 * 2 + 0)
#define RDS_TYPE_5B     ( 5 * 2 + 1)
#define RDS_TYPE_6A     ( 6 * 2 + 0)
#define RDS_TYPE_6B     ( 6 * 2 + 1)
#define RDS_TYPE_7A     ( 7 * 2 + 0)
#define RDS_TYPE_7B     ( 7 * 2 + 1)
#define RDS_TYPE_8A     ( 8 * 2 + 0)
#define RDS_TYPE_8B     ( 8 * 2 + 1)
#define RDS_TYPE_9A     ( 9 * 2 + 0)
#define RDS_TYPE_9B     ( 9 * 2 + 1)
#define RDS_TYPE_10A    (10 * 2 + 0)
#define RDS_TYPE_10B    (10 * 2 + 1)
#define RDS_TYPE_11A    (11 * 2 + 0)
#define RDS_TYPE_11B    (11 * 2 + 1)
#define RDS_TYPE_12A    (12 * 2 + 0)
#define RDS_TYPE_12B    (12 * 2 + 1)
#define RDS_TYPE_13A    (13 * 2 + 0)
#define RDS_TYPE_13B    (13 * 2 + 1)
#define RDS_TYPE_14A    (14 * 2 + 0)
#define RDS_TYPE_14B    (14 * 2 + 1)
#define RDS_TYPE_15A    (15 * 2 + 0)
#define RDS_TYPE_15B    (15 * 2 + 1)

#define BLOCK_A 6
#define BLOCK_B 4
#define BLOCK_C 2
#define BLOCK_D 0
#define CORRECTED_NONE          0
#define CORRECTED_ONE_TO_TWO    1
#define CORRECTED_THREE_TO_FIVE 2
#define UNCORRECTABLE           3
#define ERRORS_CORRECTED(data,block) ((data>>block)&0x30)

#define RDS_FIFO_SIZE	8

#define RT_VALIDATE_LIMIT 2
#define RDS_PI_VALIDATE_LIMIT  4
#define RDS_PTY_VALIDATE_LIMIT 4
#define PS_VALIDATE_LIMIT 4
#define TA_VALIDATE_LIMIT 4

class CRDSData  
{
	void InitRDS();

	std::ofstream outfile;
	BYTE m_RdsReadPtr;
	BYTE m_RdsWritePtr;
	BYTE m_RdsFifoEmpty;
	rdsFifo_struct	m_RdsFifo[RDS_FIFO_SIZE];
	int validation_limit;
	int ta_validate_count;
	
	// RDS Radio Text
    BYTE m_rtDisplay[64];   // Displayed Radio Text
	BYTE m_rtTmp0[64];      // Temporary Radio Text (high probability)
	BYTE m_rtTmp1[64];      // Temporary radio text (low probability)
	BYTE m_rtTmp2[64];      // Temporary radio text (low probability)
	BYTE m_rtCnt[64];       // Hit count of high probabiltiy radio text
	bool m_rtFlag;          // Radio Text A/B flag
	bool m_OldabFlag;
	BYTE m_rtFlagValid;     // Radio Text A/B flag is valid
	bool m_rtTog;

	// RDS Program Service
	BYTE m_psDisplay[8];    // Displayed Program Service text
	BYTE m_psTmp0[8];       // Temporary PS text (high probability)
	BYTE m_psTmp1[8];       // Temporary PS text (low probability)
	BYTE m_psCnt[8];        // Hit count of high probability PS text

	// RDS flags and counters
	BYTE m_RdsDataAvailable;	// Count of unprocessed RDS Groups
	BYTE m_RdsIndicator;		// If true, RDS was recently detected
	WORD m_RdsDataLost;			// Number of Groups lost
	WORD m_RdsBlocksValid;		// Number of valid blocks received
	WORD m_RdsBlocksTotal;		// Total number of blocks expected



	// Debug information storing number of each kind of group received
	WORD m_debug_group_counters[32];

	void UpdateRDSFifo(WORD* registers);
	void SendToXPort(char *windowName, ULONG ulMsg, char *pszData, ULONG ulLength);
	void update_pi(WORD current_pi);
	void update_pty(BYTE current_pty);
	//void update_alt_freq(WORD current_alt_freq);
	void update_ps(BYTE addr, BYTE byte);
	void update_rt(bool abFlag, BYTE count, BYTE addr, BYTE* byte, BYTE errorFlags);
	void display_rt();
	void LogRDSDataStream(WORD* registers);
	float ConvertAFFrequency(BYTE freq);

public:
	CRDSData();
	virtual ~CRDSData();

	void UpdateRDSText(WORD* registers);
	void ResetRDSText();
	std::string m_RDSText;
	std::string m_RDSPS;
	
	// RDS Program Identifier
	WORD m_piDisplay;              // Displayed Program Identifier
	std::string m_piCountry;
	std::string m_piRegion;

	// RDS Program Type
	BYTE m_ptyDisplay;             // Displayed Program Type
	std::string m_ptyDisplayString;
	bool m_tp;
	bool m_ta;
	bool m_ms;
	tAFMap AFMap;				// Alternate Frequencies

	std::string TACallbackStartCommand;
	short TACallbackStartDwData;
	std::string TACallbackStartWindowName;

	std::string TACallbackStopCommand;
	short TACallbackStopDwData;
	std::string TACallbackStopWindowName;

	std::string RTCallbackWindowName;
	short RTCallbackDwData;
	std::string RTCallbackCommand;


	bool TANowPlaying;

};

#endif // !defined(AFX_RDSDATA_H__A14C0C6B_E19E_4099_AD73_0EC6272CB271__INCLUDED_)
