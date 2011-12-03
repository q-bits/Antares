#ifndef FBLIB_REC_H
#define FBLIB_REC_H

#include "mjd.h"
#include "types.h"

typedef __u8 byte;
typedef __u16 word;
typedef __u32 dword;

  typedef enum
  {
    HT_UNKNOWN,
    HT_DVBS,
    HT_DVBT,
    HT_DVBC,
    HT_DVBT5700
  } HEADER_TYPE;


    typedef enum
  {
    unknown,
    DVBS,
    DVBT,
    DVBC
  } FLASH_TYPE;


  typedef struct
  {
	  int readsize;  // how many bytes were read from the file before trying to decode the header?
    HEADER_TYPE         HeaderType;

    dword               HeaderMagic;
    word                HeaderVersion;
    char                HeaderReserved1 [2];
    word                HeaderDuration;
    word                HeaderSvcNumber;
    word                HeaderSvcType;

    byte                SISatIndex;
    byte                SIReserved1;
    word                SITPIdx;
    byte                SITunerNum;
    byte                SIDelFlag;
    byte                SICASFlag;
    byte                SILockFlag;
    byte                SISkipFlag;
    word                SIServiceID;
    word                SIPMTPID;
    word                SIPCRPID;
    word                SIVideoPID;
    word                SIAudioPID;
    char                SISvcName [28];

    byte                TPSatIndex;         //S
    byte                TPPolarization;     //S
    byte                TPMode;             //S
    word                TPChannelNumber;    //T
    byte                TPBandwidth;        //T
    byte                TPLPHPStream;       //T
    byte                TPModulation;       //C
    dword               TPFrequency;        //STC
    word                TPSymbolRate;       //SC
    word                TPTSID;             //STC
    word                TPNetworkID;        //STC
    byte                TPReserved1 [2];    //S
    byte                TPReserved2 [2];    //S
    byte                TPReserved3;        //ST
    byte                TPReserved4;        //T
    byte                TPUnknown1 [2];     //T
    byte                TPUnknown2 [8];     //T (only 5700)
    byte                TPReserved5;        //C

    byte                EventDurationHour;
	byte                EventDurationMin;
	dword              EventEventID;
	struct    tf_datetime              EventStartTime;
	struct    tf_datetime              EventEndTime;
	byte                EventRunningStatus;
	byte                EventTextLength;
    byte                EventParentalRate;
    char                EventEventName [257];
    char                EventEventDescription [257];
    char                EventUnknown1 [18];
    byte                EventUnknown2 [2];

    word                ExtEventTextLength;
    dword               ExtEventEventID;
    char                ExtEventText [1024];

    byte                CryptReserved1 [4];
    byte                CryptFlag;
    byte                CryptReserved2 [3];

    dword               Bookmark [64];

    dword               Resume;
  } tRECHeaderInfo;

  void      HDD_DecodeRECHeader (char *Buffer, tRECHeaderInfo *RECHeaderInfo);

#endif
