#pragma once
using namespace System;

namespace Antares {
public ref class tf_models
{
public:
static String^ find_model_name(int sysid) {
switch(sysid)
{
case 100: return "TF3000FA";
case 101: return "TF3000CI";
case 102: return "TF3000CIP";
case 103: return "TF3000FE";
case 104: return "TF3100FEP";
case 105: return "TF3100FE";
case 121: return "TF3000CI";
case 134: return "TF3100FEP";
case 145: return "TF3100FE";
case 161: return "TF3000CI";
case 200: return "TF3000FI";
case 201: return "TF3000CIpro";
case 202: return "TF3000CIPpro";
case 203: return "TF3000FEI";
case 204: return "TF3100FEPpro";
case 205: return "TF3100FEI";
case 206: return "TF4000PVR";
case 208: return "TF3000PVR";
case 211: return "TF3000CIpro";
case 223: return "TF3000FEI";
case 233: return "TF3000T(AUS)";
case 240: return "TF3000FI, TF3000FI-N (NTSC)";
case 243: return "TF3000T (EUR)";
case 245: return "TF3100FEI";
case 290: return "TF3030F";
case 293: return "TF3030FE";
case 295: return "TF3100FEI";
case 296: return "TF4000PVR-N";
case 406: return "TF5000PVR, TF5500PVR";
case 407: return "TF5000PVR Cd";
case 416: return "TF5000PVRt (EUR & AUS)";
case 426: return "TF5000PVR-N";
case 436: return "TF5010PVR, TF5510PVR";
case 437: return "TF5020PVR HDMI";
case 439: return "TF800PVR HDMI";
case 446: return "TF5000PVR Black Panther / White Polar";
case 447: return "TF5030PVR HDMI";
case 456: return "TF5800PVR";
case 457: return "TF5810PVRt";
case 458: return "TF5800PVRt";
case 466: return "TF5000PVRt Black Panther / White Polar (AUS)";
case 486: return "TF5010PVR Black Panther / White Polar";
case 500: return "TF4000Fi (ME)";
case 501: return "TF5000CI (ME)";
case 502: return "TF5000CIP";
case 503: return "TF5000T";
case 505: return "TF4000FE, TF5000FE";
case 506: return "TF5000PVR, TF5500PVR";
case 507: return "TF5000Fi";
case 508: return "TF4000C";
case 513: return "TF5000T";
case 520: return "TF4000Fi-NB";
case 523: return "TF4000T";
case 527: return "TF5300FTA";
case 530: return "TF4000Fi (UAE)";
case 533: return "TF4000T (EUR)";
case 540: return "TF4000Fi-N";
case 541: return "TF5300CI";
case 543: return "TF4000T (AUS)";
case 550: return "TF4000Fi-NA";
case 560: return "TF4000Fi (Telran)";
case 563: return "TF4010T";
case 1201: return "TF3000CIC";
case 1205: return "TF3100C";
case 1215: return "TF3100T";
case 1406: return "TF5000PVRt-N";
case 1416: return "TF5000PVR Masterpiece";
case 1417: return "TF5000PVR Masterpiece (JPN)";
case 1426: return "TF5000PVRt Masterpiece (AUS)";
case 1436: return "TF5600PVR";
case 1446: return "TF5400PVR combo (S&T)";
case 1447: return "TF5400PVR";
case 1456: return "TF5010PVR Masterpiece";
case 1466: return "TF5100PVRt Masterpiece (FIN)";
case 1476: return "TF5800PVR Masterpiece";
case 1486: return "TF5100PVRc Masterpiece (FIN)";
case 1496: return "TF4400PVRt (AUS)";
case 1497: return "TF4410PVRt (AUS)";
case 1500: return "TF4000Fi";
case 1501: return "TF5000CI (EUR)";
case 1505: return "TF5000Fe (EUR), TF4000Fe";
case 1516: return "TF5000PVR Masterpiece";
case 2406: return "TF5000PVRc";
case 2416: return "TF6000PVR";
case 2417: return "TF6000PVR WS";
case 2426: return "TF5700PVRt (SWE)";
case 2436: return "TF5700PVRt Masterpiece";
case 2446: return "TF6000PVRt (AUS)";
case 2447: return "TF600PVRt";
case 2456: return "TF6000PVRE";
case 2457: return "TF6000PVR ES";
case 2458: return "TF6000PVRE";
case 2466: return "TF5000PVR";
case 2476: return "TF5900PVR";
case 2477: return "TF5950PVR";
case 2486: return "TF5100PVRc Masterpiece (RUS)";
case 2496: return "TF5410PVR HDMI (C&T)";
case 2500: return "TF4000Fi";
case 2501: return "TF5000CI";
case 2505: return "TF5000Fe";
case 2516: return "TF6000PVR";
case 2556: return "TF6000PVRE";
case 3200: return "TF3200IR";
case 3406: return "TF4000PVR Plus";
case 3416: return "TF6010PVR";
case 3426: return "TF6010PVRE";
case 3436: return "TF4010PVR Plus";
case 3446: return "TF4000PVR Plus";
case 3456: return "TF4100PVRc ,  TF6010PVR WF";
case 3457: return "TF400PVRc";
case 3466: return "TF4100PVRt";
case 3467: return "TF400PVRt";
case 3496: return "TF4100PVRt";
case 3500: return "TF4100Fi";
case 3501: return "TF5000CI (GER)";
case 3506: return "TF4000PVR Plus";
case 4411: return "TF5000CI Plus";
case 4500: return "TF4100Fi";
case 4501: return "TF5000CI Plus";
case 4511: return "TF5000CI Plus (EUR)";
case 4521: return "TF5000CI Plus";
case 4611: return "TF5000CI Plus";
case 5500: return "TF4000Fi";
case 5501: return "TF5050CI";
case 5511: return "TF5050CI";
case 5521: return "TF5050CI";
case 10066: return "TF6060CI";
case 10200: return "TF3500FE";
case 10201: return "TF3000COCI";
case 10203: return "TF3000COT";
case 10206: return "TF4000PVR COCI";
case 10213: return "TF3000COT-d";
case 10215: return "TF3100CO-C";
case 10223: return "TF3000COT-N";
case 10225: return "TF3100COT";
case 10406: return "TF5300d";
case 10416: return "Procaster VF PVR5101C";
case 10426: return "Procaster VF PVR5101T";
case 10436: return "TF5300k";
case 10446: return "TF5200PVRc";
case 10456: return "TF5200PVRc";
case 10500: return "TF4000CO";
case 10503: return "TF4000COT";
case 10508: return "TF4000COC";
case 10510: return "TF4000CO-N";
case 10513: return "TF4000COT-N";
case 10518: return "TF4000COC";
case 10523: return "TF4000COT";
case 10528: return "TF4000COC";
case 10533: return "TF4000COT";
case 10538: return "TF4000COC";
case 11406: return "TF5200PVRt-N";
case 12316: return "TF5100PVRc HDMI-S";
case 12406: return "TF5100PVRc (FIN & RUS)";
case 12416: return "TF5100PVRc HDMI";
case 12417: return "TF500PVRc";
case 12426: return "TF5100PVRcE";
case 12506: return "TF5110PVRc";
case 12516: return "TF5100PVRc HDMI-N";
case 13406: return "TF5100PVRt (FIN)";
case 13416: return "TF5100PVRt HDMI (FIN)";
case 13417: return "TF500PVRt";
case 13426: return "TF5700PVRt HDMI";
case 13436: return "TF5710PVRt HDMI";
case 13446: return "TF5010PVRtH";
case 13516: return "TF5100PVRt HDMI (CZ)";
case 13536: return "TF5720PVRt HDMI";
case 14000: return "TF4000IR Plus";
case 16100: return "TF6100IR";
case 16200: return "TF6000IR, TF6200IR";
case 16300: return "TF6300IR";
case 16310: return "TF6310IR";
case 16400: return "TF6400IR";
case 16410: return "TF6410IR";
case 16700: return "TF6700IRt";
case 20000: return "TF6000F";
case 20001: return "TF6500F";
case 20002: return "TF6000Fe";
case 20200: return "TF6000FT";
case 20201: return "TF6500F";
case 20300: return "TF6000FT";
case 20400: return "TF6000F";
case 20500: return "TF6010FT";
case 20600: return "TF6000FT";
case 20700: return "TF6010FT";
case 21031: return "TF6060CI";
case 21231: return "TF6060CI (RUS)";
case 22010: return "TMS (SR-2100)";
case 23022: return "TF7700HSCI (EUR)";
case 23023: return "TF7710HSCI";
case 23024: return "TF7720HSCI";
case 23025: return "TF7700HSCI(BOE)";
case 23026: return "TF7720HSIR";
case 23031: return "TF7700HDPVR (EUR), TF7710HDPVR";
case 23042: return "TF700HSCI";
case 23122: return "TF7700HSCI";
case 23123: return "TF7710HSCI (GER)";
case 23124: return "TF7720HSCI";
case 23131: return "TF7700HDPVR, TF7710HDPVR (CZ)";
case 23222: return "TF7700HSCI (RUS)";
case 23224: return "TF7720HSCI";
case 23231: return "TF7700HDPVR (GER)";
case 23322: return "TF7700HSCI (POL)";
case 23324: return "TF7720HSCI";
case 23331: return "TF7710HDPVR (NL)";
case 23424: return "TF7720HSCI";
case 23431: return "TF7710HDPVR";
case 23531: return "TF7700HDPVR (AT)";
case 26000: return "TF6000IRC";
case 26300: return "TF6300IRc";
case 26400: return "TF6500IRc";
case 26500: return "TF6400IRc";
case 27000: return "TF8000";
case 30002: return "TF6000T (EUR)";
case 30052: return "TF6000COT";
case 30053: return "TF100T";
case 30062: return "TF6000TS HDMI";
case 30063: return "TF6500T_HDMI";
case 30102: return "TF6000T";
case 30152: return "TF6200COT";
case 30202: return "TF4500T";
case 30252: return "TF6000COT-k";
case 30402: return "TF6000T";
case 30406: return "TF5050PDR";
case 30452: return "TF6100COT";
case 30503: return "TF4000NA";
case 30552: return "TF6000TS";
case 30652: return "TF6000COK";
case 31019: return "TF7000HT";
case 31020: return "TF7010HT";
case 32040: return "TF7000HDPVRt";
case 32340: return "TPR5000";
case 32406: return "TF5050DVR-c";
case 32416: return "TF600PVRc";
case 32420: return "TF650PVR";
case 32430: return "TF550PVR";
case 33022: return "TF7700HTCI";
case 33023: return "TF7710HTCI";
case 33032: return "TF7050HDRt";
case 33050: return "TF7710HTCO";
case 33150: return "TF7700HTCO(ntv)";
case 34010: return "TF5050CI HDMI";
case 34041: return "TF510PVRc";
case 34110: return "TF5050CI HDMI";
case 39321: return "Wildcard";
case 40052: return "TF6000COC";
case 40053: return "TF100C";
case 40152: return "TF6200COC";
case 40252: return "TF6000COC";
case 40452: return "TF6100COC";
case 40453: return "TF6100GRC";
case 40454: return "TF6700COC";
case 40552: return "TF6100COC";
case 40553: return "TF6100GRC(ELOB)";
case 40652: return "TF6100COC";
case 40752: return "TF6100EMC";
case 40753: return "TF6100DCC";
case 40852: return "TF6100COC";
case 40952: return "TF6100COC";
case 40962: return "TF6100COC";
case 43022: return "TF7700HCCI (EU, GER)";
case 43023: return "TF7710HCCI";
case 43030: return "TF7700HCCF (MSAT)";
case 43031: return "TF7700HCCF (Cabletel)";
case 43032: return "TF7700HCCF (DCC)";
case 43122: return "TF7700HCCI";
case 44040: return "TF520PVRc";
case 44041: return "TF510PVRt";
case 44042: return "TF520PVRt";
case 50000: return "TF6000F (ME)";
case 50001: return "TF6500F (ME), TF6400FTA";
case 50002: return "TF6000Fe";
case 50003: return "TF6500F Plus";
case 50010: return "TF6000F";
case 50050: return "TF6100SF";
case 50051: return "TF6000SF";
case 50052: return "TF6000Fe";
case 50053: return "TF6700SF";
case 50070: return "TF6000COCI";
case 50100: return "TF6200F";
case 50101: return "TF6500F";
case 50103: return "TF6800F";
case 50150: return "TF6100SF";
case 50200: return "TF6000F";
case 50201: return "TF6500F";
case 50202: return "TF6000Fe (GER)";
case 50203: return "TF6800F";
case 50212: return "TF6200GR";
case 50251: return "TF6400SF";
case 50312: return "TF6200GR(BBM)";
case 50400: return "TF6100F";
case 50402: return "TF6900Fe";
case 50412: return "TF6200GR(ELOB)";
case 50451: return "TF6000SFP";
case 50500: return "TF6200F";
case 50502: return "TF6000Fe, TF6200CO";
case 50503: return "TF6700CO";
case 50504: return "TF6200CO";
case 50512: return "TF6200Gr";
case 50600: return "TF6000F New OSD";
case 50601: return "TF6500F New OSD";
case 50602: return "TF6000CR";
case 50603: return "TF6700CO";
case 50610: return "TF4000Fi Plus";
case 50612: return "TF6100CR";
case 50700: return "TF6000F";
case 50702: return "TF6000VI";
case 50900: return "TF6100F";
default: return "";
}
}
};
}
