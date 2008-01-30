// RDSData.cpp: implementation of the CRDSData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "USBRadio.h"
#include "RDSData.h"
#include "FMRadioDevice.h"
#include <map>
#include <fstream>
#include <bitset>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRDSData::CRDSData()
{
	//Initialize the RDS
	InitRDS();	

	//outfile.open("c:\\log.txt", std::ofstream::app);
	outfile << "Log File\r\n" << std::flush;

}	

CRDSData::~CRDSData()
{

}

void CRDSData::InitRDS()
{
    BYTE i = 0;

	//Set the RDS text to the default
	m_RDSText = DEFAULT_RDS_TEXT;
	m_RDSPS = "";
	m_tp = false;
	m_ms = false;
	m_ta = false;

    // Reset RDS variables
	m_RdsDataAvailable = 0;
	m_RdsDataLost      = 0;
	m_RdsIndicator     = 0;
	m_RdsBlocksValid   = 0;
	m_RdsBlocksTotal   = 0;

    // Clear RDS Fifo
    m_RdsFifoEmpty = 1;
    m_RdsReadPtr = 0;
    m_RdsWritePtr = 0;

    // Clear Radio Text
	m_rtFlagValid     = 0;
	m_rtFlag = 0;
    for (i=0; i<sizeof(m_rtDisplay); i++)
	{
        m_rtDisplay[i] = 0;
		m_rtTmp0[i]    = 0;
		m_rtTmp1[i]    = 0;
		m_rtCnt[i]     = 0;
    }

    // Clear Program Service
	for (i=0;i<sizeof(m_psDisplay);i++)
	{
		m_psDisplay[i] = 0;
		m_psTmp0[i]    = 0;
		m_psTmp1[i]    = 0;
		m_psCnt[i]     = 0;
	}

    // Clear Program ID
	m_piDisplay = 0;
	m_ptyDisplay = 0;
	m_ptyDisplayString = "";

    // Reset Debug Group counters
    for (i=0; i<32; i++)
    {
        m_debug_group_counters[i] = 0;   
    }
}

void CRDSData::UpdateRDSText(WORD* registers)
{
	BYTE group_type;      // bits 4:1 = type,  bit 0 = version
    BYTE addr;
	BYTE errorCount;
    BYTE errorFlags;
	bool abflag;
	char op[20];

	// Check errors
	errorCount = (registers[STATUSRSSI] & 0x0E00) >> 9;
	errorFlags = registers[SYSCONFIG3] & 0xFF;

	sprintf(op, "-Config:%04X", (unsigned int)registers[SYSCONFIG3]);
	OutputDebugString(op);

	if (errorCount < 4)
	{
		m_RdsBlocksValid += (4 - errorCount);
	}

    // Drop the data if there are any errors
    if (errorCount)
	{
		return;
	}

	LogRDSDataStream(registers);

	m_RdsDataAvailable = 0;

    //UpdateRDSFifo(registers);

	m_RdsIndicator = 100; // Reset RdsIndicator
    
    group_type = registers[RDSB] >> 11;  // upper five bits are the group type and version

	m_debug_group_counters[group_type] += 1;

    // Update pi code.  Version B formats always have the pi code in words A and C
    update_pi(registers[RDSA]);

    if (group_type & 0x01)
	{
        update_pi(registers[RDSC]);
	}
    
    // update pty code.  
    update_pty((registers[RDSB]>>5) & 0x1f); 

	char* szString;

    switch (group_type) {

		// 0A, Basic tuning and switching info
		case RDS_TYPE_0A:
			addr = (registers[RDSB]&0x3)*2;
			m_ms = ((registers[RDSB] & 0x08) == 0x08)?true:false;
			m_ta = ((registers[RDSB] & 0x10) == 0x10)?true:false;
			m_tp = ((registers[RDSB] & 0x400) == 0x400)?true:false;
			OutputDebugString("---");
	        update_ps(addr+0, registers[RDSD] >> 8  );
		    update_ps(addr+1, registers[RDSD] & 0xff);
		    break;
		
		// 0B, Basic tuning and switching info
	    case RDS_TYPE_0B:
	        addr = (registers[RDSB]&0x3)*2;
			m_ms = (registers[RDSB] & 0x08 == 0x08)?true:false;
			m_ta = (registers[RDSB] & 0x10 == 0x10)?true:false;
			m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
	        update_ps(addr+0, registers[RDSD] >> 8  );
		    update_ps(addr+1, registers[RDSD] & 0xff);
		    break;

		// 1A, Program Item Number and Slow Labelling Codes
	    case RDS_TYPE_1A:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;

		// 1B, Program Item Number and Slow Labelling Codes
	    case RDS_TYPE_1B:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;

		// 2A, Radio Text
	    case RDS_TYPE_2A:
	        addr = (registers[RDSB] & 0xf) * 4;
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
			//abflag = (registers[RDSB] & 0x0010) >> 4;  // this is wrong
			abflag = (registers[RDSB] & 0x10) == 0x10 ? true : false;
			if(registers[RDSB] & 0x10)
			{
				abflag = true;
				OutputDebugString("-A-");
			}
			else
			{
				abflag = false;
				OutputDebugString("-B-");
			}
			update_rt(abflag, 4, addr, (BYTE*)&(registers[RDSC]), errorFlags);
		    break;

		// 2B, Radio Text
	    case RDS_TYPE_2B:
	    //    addr = (registers[RDSB] & 0xf) * 4;
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
			//abflag = (registers[0xb] & 0x0010) >> 4; // this is wrong
			//abflag = (registers[RDSB] & 0x10) >> 4;
		//	abflag = (registers[RDSB] & 0x10) == 0x10 ? true : false;
	        // The last 32 bytes are unused in this format
		//	m_rtTmp0[32]    = 0x0d;
		//	m_rtTmp1[32]    = 0x0d;
		//	m_rtCnt[32]     = RT_VALIDATE_LIMIT;
		//	update_rt(abflag, 2, addr, (BYTE*)&(registers[RDSD]), errorFlags);
	        break;

		// 4A, Clock
	    case RDS_TYPE_4A:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;

		// 8A, TMC
	    case RDS_TYPE_8A:
			//szString = (char*)registers;
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
			//SendToXPort(1, NULL, 0); // Pause real GPS before sending
			//SendToXPort(2, "?", 1 ); // Send data
			//SendToXPort(2, szString, 8 ); // Send data
			//SendToXPort(2, "?", 1 ); // Send data

			//SendToXPort(0, NULL, 0); // Unpause real GPS when done sending
		    break;

		// 10A, Programme Type Name
	    case RDS_TYPE_10A:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;

		// 14A, Enhanced Other Networks
	    case RDS_TYPE_14A:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;

		// 15B, Fast Basic Tuning
	    case RDS_TYPE_15B:
			//m_tp = (registers[RDSB] & 0x400 == 0x400)?true:false;
		    break;


	    default:
	        break;                
    }
}

void CRDSData::LogRDSDataStream(WORD* registers)
{
	char op[30];

	switch (registers[RDSB] >> 11) {
		case RDS_TYPE_0A:
					OutputDebugString("-0A-");
					break;
		case RDS_TYPE_0B:
					OutputDebugString("-0B-");
					break;
		case RDS_TYPE_1A:
					OutputDebugString("-1A-");
					break;
		case RDS_TYPE_1B:
					OutputDebugString("-1B-");
					break;
		case RDS_TYPE_2A:
					OutputDebugString("-2A-");
					break;
		case RDS_TYPE_2B:
					OutputDebugString("-2B-");
					break;
		case RDS_TYPE_3A:
					OutputDebugString("-3A-");
					break;
		case RDS_TYPE_3B:
					OutputDebugString("-3B-");
					break;
		case RDS_TYPE_4A:
					OutputDebugString("-4A-");
					break;
		case RDS_TYPE_4B:
					OutputDebugString("-4B-");
					break;
		case RDS_TYPE_5A:
					OutputDebugString("-5A-");
					break;
		case RDS_TYPE_5B:
					OutputDebugString("-5B-");
					break;		
		case RDS_TYPE_6A:
					OutputDebugString("-6A-");
					break;
		case RDS_TYPE_6B:
					OutputDebugString("-6B-");
					break;
		case RDS_TYPE_7A:
					OutputDebugString("-7A-");
					break;
		case RDS_TYPE_7B:
					OutputDebugString("-7B-");
					break;
		case RDS_TYPE_8A:
					OutputDebugString("-8A-");
					break;
		case RDS_TYPE_8B:
					OutputDebugString("-8B-");
					break;
		case RDS_TYPE_9A:
					OutputDebugString("-9A-");
					break;
		case RDS_TYPE_9B:
					OutputDebugString("-9B-");
					break;
		case RDS_TYPE_10A:
					OutputDebugString("-10A-");
					break;
		case RDS_TYPE_10B:
					OutputDebugString("-10B-");
					break;
		case RDS_TYPE_11A:
					OutputDebugString("-11A-");
					break;
		case RDS_TYPE_11B:
					OutputDebugString("-11B-");
					break;
		case RDS_TYPE_12A:
					OutputDebugString("-12A-");
					break;
		case RDS_TYPE_12B:
					OutputDebugString("-12B-");
					break;
		case RDS_TYPE_13A:
					OutputDebugString("-13A-");
					break;
		case RDS_TYPE_13B:
					OutputDebugString("-13B-");
					break;
		case RDS_TYPE_14A:
					OutputDebugString("-14A-");
					break;
		case RDS_TYPE_14B:
					OutputDebugString("-14B-");
					break;
		case RDS_TYPE_15A:
					OutputDebugString("-15A-");
					break;
		case RDS_TYPE_15B:
					OutputDebugString("-15B-");
					break;
	}

	sprintf(op, "%04X %04X %04X %04X", (unsigned int)registers[RDSA], (unsigned int)registers[RDSB], 
								(unsigned int)registers[RDSC], (unsigned int)registers[RDSD]);
	OutputDebugString(op);

}


void CRDSData::SendToXPort(ULONG ulMsg, char* pszData, ULONG ulLength)
{
     HWND hWnd = FindWindow( NULL, "XPortMsgWnd" );
     if( !hWnd ) return;
     COPYDATASTRUCT CD;
     CD.dwData = ulMsg;
     CD.lpData = pszData;
     CD.cbData = ulLength;
     SendMessage( hWnd, WM_COPYDATA, 0, (LPARAM)&CD );
}

void CRDSData::UpdateRDSFifo(WORD* group)
{
    m_RdsFifo[m_RdsWritePtr].a = group[0xC];
    m_RdsFifo[m_RdsWritePtr].b = group[0xD];
    m_RdsFifo[m_RdsWritePtr].c = group[0xE];
    m_RdsFifo[m_RdsWritePtr].d = group[0xF];
    if(m_RdsWritePtr == m_RdsReadPtr && !m_RdsFifoEmpty)
    {
		m_RdsDataLost++;
		m_RdsReadPtr++;
		m_RdsReadPtr %= RDS_FIFO_SIZE;
    }
    m_RdsFifoEmpty = 0;
	m_RdsWritePtr++;
	m_RdsWritePtr %= RDS_FIFO_SIZE;
}

void CRDSData::update_pi(WORD current_pi)
{
    static BYTE rds_pi_validate_count = 0;
    static WORD rds_pi_nonvalidated = 0;
    
    // if the pi value is the same for a certain number of times, update
    // a validated pi variable
    if (rds_pi_nonvalidated != current_pi)
	{
        rds_pi_nonvalidated =  current_pi;
        rds_pi_validate_count = 1;
    }
	else
	{
        rds_pi_validate_count++;
    }

    if (rds_pi_validate_count > validation_limit)
	{
        m_piDisplay = rds_pi_nonvalidated;
	}
}

void CRDSData::update_pty(BYTE current_pty)
{
    static BYTE rds_pty_validate_count = 0;
    static BYTE rds_pty_nonvalidated = 0;
    
    // if the pty value is the same for a certain number of times, update
    // a validated pty variable
    if (rds_pty_nonvalidated != current_pty)
	{
        rds_pty_nonvalidated =  current_pty;
        rds_pty_validate_count = 1;
    }
	else
	{
        rds_pty_validate_count++;
    }

    if (rds_pty_validate_count > validation_limit)
	{
        m_ptyDisplay = rds_pty_nonvalidated;

		switch (m_ptyDisplay) {
			case 1: m_ptyDisplayString = "News";break;
			case 2: m_ptyDisplayString = "Current Affairs";break;
			case 3: m_ptyDisplayString = "Information";break;
			case 4: m_ptyDisplayString = "Sport";break;
			case 5: m_ptyDisplayString = "Education";break;
			case 6: m_ptyDisplayString = "Drama";break;
			case 7: m_ptyDisplayString = "Culture";break;
			case 8: m_ptyDisplayString = "Science";break;
			case 9: m_ptyDisplayString = "Varied";break;
			case 10: m_ptyDisplayString = "Pop Music";break;
			case 11: m_ptyDisplayString = "Rock Music";break;
			case 12: m_ptyDisplayString = "Easy Listening";break;
			case 13: m_ptyDisplayString = "Light Classic Music";break;
			case 14: m_ptyDisplayString = "Serious Classic Music";break;
			case 15: m_ptyDisplayString = "Other Music";break;
			case 16: m_ptyDisplayString = "Weather & Metr";break;
			case 17: m_ptyDisplayString = "Finance";break;
			case 18: m_ptyDisplayString = "Children's Progs";break;
			case 19: m_ptyDisplayString = "Social Affairs";break;
			case 20: m_ptyDisplayString = "Religion";break;
			case 21: m_ptyDisplayString = "Phone In";break;
			case 22: m_ptyDisplayString = "Travel & Touring";break;
			case 23: m_ptyDisplayString = "Leisure & Hobby";break;
			case 24: m_ptyDisplayString = "Jazz Music";break;
			case 25: m_ptyDisplayString = "Country Music";break;
			case 26: m_ptyDisplayString = "National Music";break;
			case 27: m_ptyDisplayString = "Oldies Music";break;
			case 28: m_ptyDisplayString = "Folk Music";break;
			case 29: m_ptyDisplayString = "Documentary";break;
			case 30: m_ptyDisplayString = "Alarm Test";break;
			case 31: m_ptyDisplayString = "Alarm - Alarm !";break;
			default: m_ptyDisplayString = "";break;
		}
	}
}

void CRDSData::update_ps(BYTE addr, BYTE byte)
{
    BYTE i = 0;
	BYTE textChange = 0; // indicates if the PS text is in transition
	BYTE psComplete = 1; // indicates that the PS text is ready to be displayed
	char op[2];

	op[0] = byte;
	op[1] = 0;
	OutputDebugString(op);
		
	if(isprint(byte)==0) //non printable - exit
		return;

	if(m_psTmp0[addr] == byte)
	{
        // The new byte matches the high probability byte
		if(m_psCnt[addr] < validation_limit)
		{
			m_psCnt[addr]++;
		}
		else
		{
            // we have recieved this byte enough to max out our counter
            // and push it into the low probability array as well
			m_psCnt[addr] = validation_limit;
			m_psTmp1[addr] = byte;
		}
	}
	else if(m_psTmp1[addr] == byte)
	{
        // The new byte is a match with the low probability byte. Swap
        // them, reset the counter and flag the text as in transition.
        // Note that the counter for this character goes higher than
        // the validation limit because it will get knocked down later
		if(m_psCnt[addr] >= validation_limit)
		{
			textChange = 1;
		}
		m_psCnt[addr] = validation_limit + 1;
		m_psTmp1[addr] = m_psTmp0[addr];
		m_psTmp0[addr] = byte;
	}
	else if(!m_psCnt[addr])
	{
        // The new byte is replacing an empty byte in the high
        // proability array
		m_psTmp0[addr] = byte;
		m_psCnt[addr] = 1;
	}
	else
	{
        // The new byte doesn't match anything, put it in the
        // low probablity array.
		m_psTmp1[addr] = byte;
	}

	if(textChange)
	{
        // When the text is changing, decrement the count for all 
        // characters to prevent displaying part of a message
        // that is in transition.
		for(i=0;i<sizeof(m_psCnt);i++)
		{
			if(m_psCnt[i] > 1)
			{
				m_psCnt[i]--;
			}
		}
	}

    // The PS text is incomplete if any character in the high
    // probability array has been seen fewer times than the
    // validation limit.
	for (i=0;i<sizeof(m_psCnt);i++)
	{
		if(m_psCnt[i] < validation_limit)
		{
			psComplete = 0;
			break;
		}
	}

    // If the PS text in the high probability array is complete
    // copy it to the display array
	if (psComplete)
	{
		m_RDSPS = "";
		for (i=0;i<sizeof(m_psDisplay); i++)
		{
			m_psDisplay[i] = m_psTmp0[i];
			m_RDSPS += m_psDisplay[i];
		}
		//std::map<WORD, std::string>::iterator it = m_psTable.find(m_piDisplay);
		//if( it != m_psTable.end() ) {
		//	m_psTable.erase(it);
		//}
		//m_psTable.insert(std::pair<WORD, std::string>(m_piDisplay, m_RDSPS));
	}
}

void CRDSData::display_rt()
{
    BYTE rtComplete = 1;
	BYTE i;

    // The Radio Text is incomplete if any character in the high
    // probability array has been seen fewer times than the
    // validation limit.
	for (i=0; i<sizeof(m_rtTmp0);i++)
	{
		if(m_rtCnt[i] < RT_VALIDATE_LIMIT)
		{
			rtComplete = 0;
			break;
		}
		if(m_rtTmp0[i] == 0x0d)
		{
            // The array is shorter than the maximum allowed
			break;
		}
	}

    // If the Radio Text in the high probability array is complete
    // copy it to the display array
	if (1)//rtComplete)
	{
		m_RDSText = "";

		for (i=0; i < sizeof(m_rtDisplay); i += 2)
		{
			if ((m_rtDisplay[i] != 0x0d) && (m_rtDisplay[i+1] != 0x0d))
			{
				m_rtDisplay[i] = m_rtTmp0[i+1];
				m_rtDisplay[i+1] = m_rtTmp0[i];
			}
			else
			{
				m_rtDisplay[i] = m_rtTmp0[i];
				m_rtDisplay[i+1] = m_rtTmp0[i+1];
			}

			if (m_rtDisplay[i] != 0x0d)
				m_RDSText += m_rtDisplay[i];
			
			if (m_rtDisplay[i+1] != 0x0d)
				m_RDSText += m_rtDisplay[i+1];

			if ((m_rtDisplay[i] == 0x0d) || (m_rtDisplay[i+1] == 0x0d))
				i = sizeof(m_rtDisplay);
		}

        // Wipe out everything after the end-of-message marker
		for (i++;i<sizeof(m_rtDisplay);i++)
		{
			m_rtDisplay[i] = 0;
			m_rtCnt[i]     = 0;
			m_rtTmp0[i]    = 0;
			m_rtTmp1[i]    = 0;
		}

		//std::map<WORD, std::string>::iterator it = m_textTable.find(m_piDisplay);
		//if( it != m_textTable.end() ) {
		//	m_textTable.erase(it);
		//}
		//m_textTable.insert(std::pair<WORD, std::string>(m_piDisplay, m_RDSText));
	}
}


void CRDSData::update_rt(bool abFlag, BYTE count, BYTE addr, BYTE* byte, BYTE errorFlags)
{
    BYTE i;
	BYTE textChange = 0; // indicates if the Radio Text is changing
	char op[2];

	if(m_OldabFlag != abFlag)
	{
        //ab event - clear string
		for(i=0;i<sizeof(m_rtTmp0);i++)
		{
			m_rtTmp0[i] = m_rtTmp1[i] = m_rtTmp2[i] = ' ';
		}
	}
	m_OldabFlag = abFlag;

	for(i=0;i<count;i++)
	{
		if(isprint(byte[i])==0)
		{
			byte[i] = ' '; // translate nulls to spaces
		}

		op[0] = byte[i];
		op[1] = 0;
		OutputDebugString(op);
		
        /*// The new byte matches the high probability byte
		if(m_rtTmp0[addr+i] == byte[i])
		{
			if(m_rtCnt[addr+i] < RT_VALIDATE_LIMIT)
			{
				m_rtCnt[addr+i]++;
			}
			else
			{
                // we have recieved this byte enough to max out our counter
                // and push it into the low probability array as well
				m_rtCnt[addr+i] = RT_VALIDATE_LIMIT;
				m_rtTmp1[addr+i] = byte[i];
			}
		}
		else if(m_rtTmp1[addr+i] == byte[i])
		{

            // The new byte is a match with the low probability byte. Swap
            // them, reset the counter and flag the text as in transition.
            // Note that the counter for this character goes higher than
            // the validation limit because it will get knocked down later
			if(m_rtCnt[addr+i] >= RT_VALIDATE_LIMIT)
			{
				textChange = 1;
			}
			m_rtCnt[addr+i] = RT_VALIDATE_LIMIT + 1;
			m_rtTmp1[addr+i] = m_rtTmp0[addr+i];
			m_rtTmp0[addr+i] = byte[i];
		}
//		else if(!m_rtCnt[addr+i])
//		{
//            // The new byte is replacing an empty byte in the high
//            // proability array
//			m_rtTmp0[addr+i] = byte[i];
//			m_rtCnt[addr+i] = 1;
//		}
		else
		{
            // The new byte doesn't match anything, put it in the
            // low probablity array.
			m_rtTmp1[addr+i] = byte[i];
		}*/
		if(byte[i] == m_rtTmp1[addr+i]) 
			m_rtTmp0[addr+i] = byte[i];
		else if(byte[i] == m_rtTmp2[addr+i])
			m_rtTmp0[addr+i] = byte[i];
		else
		{
			if(m_rtTog)
				m_rtTmp1[addr+i] = byte[i];
			else
				m_rtTmp2[addr+i] = byte[i];
			m_rtTog = !m_rtTog;
		}

	}

    // Display the Radio Text
	display_rt();
}



void CRDSData::ResetRDSText()
{
	//Re-initialize the RDS
	InitRDS();
}