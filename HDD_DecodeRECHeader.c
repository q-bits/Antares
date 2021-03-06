#include <string.h>
#include <stdio.h>
#include "types.h"
#include "FBLib_rec.h"
#include "tf_bytes.h"
#include "mjd.h"
//--------------------------------------- HDD_DecodeRECHeader --------------------------------
//
// Taken from the FirebirdLib, but modified and extended for use in Antares by H. Haselgrove.


dword get_dword(void* addr)
{
	return get_u32(addr);
}

word get_word(void* addr)
{
	return get_u16(addr);
}

byte get_byte(void* addr)
{
	return *( (__u8*) addr);
}

void memcpy_and_process(char * dest, char *src, size_t size)
// Do a memcpy-like operation, but skip any characters in the range 0<x<32.
// Intended to get rid of non-printing characters from program description.
{
	size_t si, di;
	for (si=0, di=0; si<size; si++)
	{
		if (src[si]<32 && src[si]>0) continue;
		dest[di] = src[si];

		di++;
	}


}


#define PF(n) if(verbose) printf(#n "        =   %d\n",RECHeaderInfo->##n);


int test_event_block(char *Buffer, dword p, tRECHeaderInfo * RECHeaderInfo)
{
	// Trial-decode an event block, to see if we have guessed its starting location correctly (DVB-C versus -S/-T)

	struct tf_datetime start_time;
	struct tf_datetime end_time;
	double y1,y2;

	start_time.second=0;
	end_time.second=0;

	RECHeaderInfo->EventDurationHour   = get_byte(&Buffer [p +  2]);
	RECHeaderInfo->EventDurationMin    = get_byte(&Buffer [p +  3]);
	RECHeaderInfo->EventEventID        = get_dword(&Buffer [p +  4]);
	memcpy (&start_time, &Buffer [p +  8], 4);
	memcpy (&end_time  , &Buffer [p + 12], 4);
	RECHeaderInfo->EventRunningStatus  = get_byte(&Buffer [p + 16]);
	RECHeaderInfo->EventTextLength     = get_byte(&Buffer [p + 17]);
	RECHeaderInfo->EventParentalRate   = get_byte(&Buffer [p + 18]);

	if (start_time.hour > 24 || end_time.hour > 24 || start_time.minute>60 || end_time.minute>60) return 0;

	y1 = tfdt_to_year(&start_time);
	y2 = tfdt_to_year(&end_time);


	//printf("st  H:M =%d:%d  (%f)   end  H:M =%d:%d  (%f)  \n",(int) start_time.hour, (int) start_time.minute, (double) y1, (int) end_time.hour, (int) end_time.minute, (double) y2);

	if (y1<98 || y2<98 || y1>130 || y2>130) return 0;


	return 1;
}


void HDD_DecodeRECHeader (char *Buffer, tRECHeaderInfo *RECHeaderInfo)
{
	dword                 Frq, SR;
	byte                  Modulation, Polarization, BandWidth, LPHPStream;
	dword                 p;
	dword                 sPoints = 0, tPoints = 0, cPoints = 0;

	int verbose=0;
	//if (!LibInitialized) InitTAPex ();
	//if (!LibInitialized) return;

	memset (RECHeaderInfo, 0, sizeof (tRECHeaderInfo));
	//RECHeaderInfo->HeaderSvcNumber = -1;

	//Is this a REC header?
	if (get_dword(&Buffer [0]) != 0x54467263) return;

	//Header block
	p = 0;
	RECHeaderInfo->HeaderMagic     = get_dword(&Buffer [p +  0]);
	RECHeaderInfo->HeaderVersion   = get_word(&Buffer [p +  4]);
	memcpy (RECHeaderInfo->HeaderReserved1, &Buffer [p +  6], 2);
	RECHeaderInfo->HeaderDuration  = get_word(&Buffer [p +  8]);
	RECHeaderInfo->HeaderSvcNumber = get_word(&Buffer [p + 10]);
	RECHeaderInfo->HeaderSvcType   = get_word(&Buffer [p + 12]);
	p += 14;



	if (verbose) printf("---------\n");
	PF(HeaderMagic);
	PF(HeaderVersion);
	PF(HeaderDuration);
	PF(HeaderSvcNumber);
	PF(HeaderSvcType);


	//Service Info block
	RECHeaderInfo->SISatIndex  = get_byte(&Buffer [p +  0]);
	RECHeaderInfo->SIReserved1 = get_byte(&Buffer [p +  1]);
	RECHeaderInfo->SITPIdx     = get_word(&Buffer [p +  2]) >> 6;
	RECHeaderInfo->SITunerNum  = (get_word(&Buffer [p +  2]) >> 4) & 3;
	RECHeaderInfo->SIDelFlag   = (get_word(&Buffer [p +  2]) >> 3) & 1;
	RECHeaderInfo->SICASFlag   = (get_word(&Buffer [p +  2]) >> 2) & 1;
	RECHeaderInfo->SILockFlag  = (get_word(&Buffer [p +  2]) >> 1) & 1;
	RECHeaderInfo->SISkipFlag  = (get_word(&Buffer [p +  2])     ) & 1;
	RECHeaderInfo->SIServiceID = get_word(&Buffer [p +  4]);
	RECHeaderInfo->SIPMTPID    = get_word(&Buffer [p +  6]);
	RECHeaderInfo->SIPCRPID    = get_word(&Buffer [p +  8]);
	RECHeaderInfo->SIVideoPID  = get_word(&Buffer [p + 10]);
	RECHeaderInfo->SIAudioPID  = get_word(&Buffer [p + 12]);

	//printf("Service ID = %d,     Service Number = %d\n",(int)RECHeaderInfo->HeaderSvcNumber ,(int) RECHeaderInfo->SIServiceID);

	PF(SISatIndex);
	PF(SIReserved1);
	PF(SITPIdx); 
	PF(SITunerNum);
	PF(SIDelFlag);
	PF(SICASFlag);
	PF(SILockFlag);
	PF(SISkipFlag);
	PF(SIServiceID);
	PF(SIPMTPID);
	PF(SIPCRPID);
	PF(SIVideoPID);
	PF(SIAudioPID);


	if (RECHeaderInfo->HeaderVersion == 0x5010)
	{
		memcpy_and_process (RECHeaderInfo->SISvcName, &Buffer [p +  14], 28);
		p += 42;
	}
	else
	{
		memcpy_and_process (RECHeaderInfo->SISvcName, &Buffer [p +  14], 24);
		p += 38;
	}
	if (verbose) printf("SISvcName = @%s@    (HeaderVersion=%x)\n",RECHeaderInfo->SISvcName, RECHeaderInfo->HeaderVersion );

	//Use the following rules for the type decision (thx to jkIT)
	//Count 1 point for every matching rule
	//for dvb-s:
	//  Transponder_Info.Polarity & 0x6F == 0 ?
	//  Transponder_Info.Symbol_Rate in [2000...30000] ?
	//  Transponder_Info.Frequency in [10700...12800] ?
	//for dvb-t
	//  Transponder_Info.Bandwidth in [6..8] ?
	//  Transponder_Info.Frequency in [174000...230000] or [470000...862000] ?
	//  Transponder_Info.LP_HP_Stream & 0xFE == 0 ?
	//for dvb-c
	//  Transponder_Info.Frequency in [47000...862000] ?
	//  Transponder_Info.Symbol_Rate in [2000...30000] ?
	//  Transponder_Info.Modulation <= 4 ?

	//Count cable points
	Frq = get_dword(&Buffer [p + 0]);
	SR = *(word*)(&Buffer [p + 4]);
	Modulation = *(byte *) (&Buffer [p + 10]);
	if ((Frq >= 47000) && (Frq <= 862000)) cPoints++;
	if ((SR >= 2000) && (SR <= 30000)) cPoints++;
	if (Modulation <= 4) cPoints++;

	if (verbose) {printf("Cable: Frq=%d  SR=%d  Modulation=%d\n",(int)Frq, (int) SR, (int) Modulation);};

	//Count sat points
	Frq = get_dword(&Buffer [p + 4]);
	SR = *(word*)(&Buffer [p + 8]);
	Polarization = *(byte *) (&Buffer [p + 1]);
	if ((Frq >= 10700) && (Frq <= 12800)) sPoints++;
	if ((SR >= 2000) && (SR <= 30000)) sPoints++;
	if ((Polarization & 0x6F) == 0) sPoints++;

	if (verbose) {printf("Sat: Frq=%d  SR=%d  Modulation=%d\n",(int)Frq, (int) SR, (int) Modulation);};


	//Count terrestrial points
	BandWidth = *(byte *) (&Buffer [p + 2]);
	LPHPStream = *(byte *) (&Buffer [p + 10]);
	if (((Frq >= 174000) && (Frq <= 230000)) || ((Frq >= 470000) && (Frq <= 862000))) tPoints++;
	if ((BandWidth >= 6) && (BandWidth <= 8)) tPoints++;
	if ((LPHPStream & 0xFE) == 0) tPoints++;

	if (verbose) {printf("Terrest.: Frq=%d  LPHPStream & 0xFE = %d\n",(int)Frq, (int) (LPHPStream & 0xFE) );};


	//If one system has 3 points and all other have less than 3, use that DVB system
	RECHeaderInfo->HeaderType = unknown;
	if ((sPoints == 3) && (tPoints  < 3) && (cPoints  < 3)) RECHeaderInfo->HeaderType = HT_DVBS;
	if ((sPoints <  3) && (tPoints == 3) && (cPoints  < 3)) RECHeaderInfo->HeaderType = DVBT;//(SysID == 2426 || SysID == 13426 ? HT_DVBT5700 : DVBT);
	if ((sPoints <  3) && (tPoints  < 3) && (cPoints == 3)) RECHeaderInfo->HeaderType = HT_DVBC;

	if (verbose) printf("** HeaderType = %d   sPoints=%d  tPoints=%d    cPoints=%d\n",RECHeaderInfo->HeaderType,sPoints,tPoints,cPoints);

	if (RECHeaderInfo->HeaderType == unknown)
	{

		dword p_old = p;
		int ok16=0;
		int ok12=0;

		//// Try S/T  (skip 16 bytes)
		p = p_old + 16;

		ok16 = test_event_block(Buffer, p, RECHeaderInfo);

		//// Try C  (skip 12 bytes)
		p = p_old + 12;

		ok12 = test_event_block(Buffer, p, RECHeaderInfo);


		if (ok12 || !ok16) 
			RECHeaderInfo->HeaderType =  HT_DVBC;

		if (ok12 && ok16)
		{
			if (cPoints > sPoints && cPoints > tPoints)
				RECHeaderInfo->HeaderType =  HT_DVBC;
		}



		p = p_old;


	}


	//Transponder block
	switch (RECHeaderInfo->HeaderType)
	{
	case HT_DVBS:
		{
			RECHeaderInfo->TPSatIndex     = get_byte(&Buffer [p +  0]);
			RECHeaderInfo->TPPolarization = get_byte(&Buffer [p +  1]) >> 7;
			RECHeaderInfo->TPMode         = (get_byte(&Buffer [p +  1]) >> 4) & 7;
			RECHeaderInfo->TPReserved3    = (get_byte(&Buffer [p +  1])     ) & 15;
			memcpy (RECHeaderInfo->TPReserved1,     &Buffer [p +  2], 2);
			RECHeaderInfo->TPFrequency    = get_dword(&Buffer [p +  4]);
			RECHeaderInfo->TPSymbolRate   = get_word(&Buffer [p +  8]);
			RECHeaderInfo->TPTSID         = get_word(&Buffer [p + 10]);
			memcpy (RECHeaderInfo->TPReserved2,     &Buffer [p + 12], 2);
			RECHeaderInfo->TPNetworkID    = get_word(&Buffer [p + 14]);

			p += 16;
			break;
		}

	case HT_DVBT:
	case HT_DVBT5700:
		{
			RECHeaderInfo->TPChannelNumber = get_word(&Buffer [p +  0]);
			RECHeaderInfo->TPBandwidth     = get_byte(&Buffer [p +  2]);
			RECHeaderInfo->TPReserved3     = get_byte(&Buffer [p +  3]);
			RECHeaderInfo->TPFrequency     = get_dword(&Buffer [p +  4]);
			RECHeaderInfo->TPTSID          = get_word(&Buffer [p +  8]);
			RECHeaderInfo->TPLPHPStream    = get_byte(&Buffer [p + 10]);
			RECHeaderInfo->TPReserved4     = get_byte(&Buffer [p + 11]);
			RECHeaderInfo->TPNetworkID     = get_word(&Buffer [p + 12]);
			memcpy (RECHeaderInfo->TPUnknown1,     &Buffer [p + 14], 2);

			p += 16;

			if (RECHeaderInfo->HeaderType == HT_DVBT5700)
			{
				memcpy(RECHeaderInfo->TPUnknown2, &Buffer[p + 16], 8);
				p += 8;
			}
			break;
		}

	case HT_DVBC:
		{
			RECHeaderInfo->TPFrequency  = get_dword(&Buffer [p +  0]);
			RECHeaderInfo->TPSymbolRate = get_word(&Buffer [p +  4]);
			RECHeaderInfo->TPTSID       = get_word(&Buffer [p +  6]);
			RECHeaderInfo->TPNetworkID  = get_word(&Buffer [p +  8]);
			RECHeaderInfo->TPModulation = get_byte(&Buffer [p + 10]);
			RECHeaderInfo->TPReserved5  = get_byte(&Buffer [p + 11]);

			p += 12;
			break;
		}

	default:
		{
			p += 16;
			break;
		}
	}

	//Event block
	memcpy (RECHeaderInfo->EventUnknown2, &Buffer [p], 2);
	RECHeaderInfo->EventDurationHour   = get_byte(&Buffer [p +  2]);
	RECHeaderInfo->EventDurationMin    = get_byte(&Buffer [p +  3]);
	RECHeaderInfo->EventEventID        = get_dword(&Buffer [p +  4]);

	RECHeaderInfo->EventStartTime.second=0;
	RECHeaderInfo->EventEndTime.second=0;

	memcpy (&RECHeaderInfo->EventStartTime, &Buffer [p +  8], 4);
	memcpy (&RECHeaderInfo->EventEndTime  , &Buffer [p + 12], 4);
	RECHeaderInfo->EventRunningStatus  = get_byte(&Buffer [p + 16]);
	RECHeaderInfo->EventTextLength     = get_byte(&Buffer [p + 17]);
	RECHeaderInfo->EventParentalRate   = get_byte(&Buffer [p + 18]);
	memcpy_and_process (RECHeaderInfo->EventEventName, &Buffer [p + 19], RECHeaderInfo->EventTextLength);
	memcpy_and_process (RECHeaderInfo->EventEventDescription, &Buffer [p + 19 + RECHeaderInfo->EventTextLength], 257 - RECHeaderInfo->EventTextLength);
	memcpy_and_process (RECHeaderInfo->EventUnknown1, &Buffer [p + 276], 18);

	PF(EventDurationHour);
	PF(EventDurationMin);
	PF(EventEventID);
	PF(EventRunningStatus);
	PF(EventTextLength);
	PF(EventParentalRate);


	p += 294;

	//Extended Event block
	RECHeaderInfo->ExtEventTextLength  = get_word(&Buffer [p +  0]);
	RECHeaderInfo->ExtEventEventID     = get_dword(&Buffer [p +  2]);
	memcpy_and_process (RECHeaderInfo->ExtEventText, &Buffer [p +  6], 1024);

	PF(ExtEventTextLength);


	p += 1030;

	//Crypt Flag block
	memcpy (RECHeaderInfo->CryptReserved1, &Buffer [p], 4);
	RECHeaderInfo->CryptFlag =   get_byte(&Buffer [p +  4]);
	memcpy (RECHeaderInfo->CryptReserved2, &Buffer [p + 5], 3);

	p += 8;

	//Bookmark block
	memcpy (RECHeaderInfo->Bookmark, &Buffer [p], 64 * sizeof (dword));

	p += 256;

	RECHeaderInfo->Resume = *( dword*)(&Buffer [p + 0]);
}
