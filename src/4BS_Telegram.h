#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

inline uint8_t handle_4BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex)
{
      union intParts
      {
            uint8_t uint8;
      } _value;

      uint32_t value_4Byte;
      uint16_t lux;
      float luxfloat;
      float temp;
      float hum;
      int8_t value;
      bool bvalue;

      FOURBS_A5_02_TYPE *fourBsA5_02_Tlg_p;
      FOURBS_A5_02_2030TYPE *fourBsA5_02_2030_Tlg_p;
      FOURBS_A5_04_TYPE *fourBsA5_04_Tlg_p;
      FOURBS_A5_04_03_TYPE *fourBsA5_04_03_Tlg_p;
      FOURBS_A5_06_01_TYPE *fourBsA5_06_01_Tlg_p;
      FOURBS_A5_06_01_V2_TYPE *fourBsA5_06_01_V2_Tlg_p;
      FOURBS_A5_06_01_TYPE *fourBsA5_06_02_Tlg_p;
      FOURBS_A5_06_03_TYPE *fourBsA5_06_03_Tlg_p;
      FOURBS_A5_07_01_TYPE *fourBsTlg2_p;
      FOURBS_A5_07_02_TYPE *fourBsTlg_p;
      FOURBS_A5_07_03_TYPE *fourBsTlg3_p;
      FOURBS_A5_08_TYPE *fourBsA5_08_Tlg_p;
      FOURBS_A5_12_01_TYPE *fourBsA5_12_01_Tlg_p;
      FOURBS_A5_14_01_06_TYPE *fourBsA5_17_01_06_Tlg_p;
      FOURBS_A5_14_07_08_TYPE *fourBsA5_17_07_08_Tlg_p;
      FOURBS_A5_14_09_0A_TYPE *fourBsA5_17_09_0A_Tlg_p;
      FOURBS_A5_20_01_TYPE *fourBsA5_20_01_Tlg_p;
      FOURBS_A5_20_04_TYPE *fourBsA5_20_04_Tlg_p;
      FOURBS_A5_20_06_TYPE *fourBsA5_20_06_Tlg_p;

      // Special Profil for a 4BS MSG from a 1BS Device
      ONEBS_TELEGRAM_TYPE_V2 *l1bsV2Tlg_p;

      logDebug("4BS", "Profil: 4BS - %u", ParamENO_CHProfilSelection4BS);
 
//#ifndef EnOceanTEST
      switch (ParamENO_CHProfilSelection4BS) 
//#else
//      switch (profil)
//#endif
      {
      case A5_02:
            logDebug("4BS", "A5-02-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS02))
            switch (ParamENO_CHProfil4BS02)
            {
            //**************************************************************
            // ----------------- Profil: A5-02-01 --------------------------
            //**************************************************************
            case A5_02_01:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375);
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "01 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-02 --------------------------
            //**************************************************************
            case A5_02_02:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 10.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "02 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-03 --------------------------
            //**************************************************************
            case A5_02_03:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 20.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "03 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-04 --------------------------
            //**************************************************************
            case A5_02_04:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 30.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "04 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-05 --------------------------
            //**************************************************************
            case A5_02_05:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 40.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "05 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-06 --------------------------
            //**************************************************************
            case A5_02_06:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 50.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "06 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-07 --------------------------
            //**************************************************************
            case A5_02_07:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 60.0;
                  KoENO_GO_BASE__1.value(temp, Dpt(9, 1));
                  logDebug("4BS", "07 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-08 --------------------------
            //**************************************************************
            case A5_02_08:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 70.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "08 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-09 --------------------------
            //**************************************************************
            case A5_02_09:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 80.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "09 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-0A --------------------------
            //**************************************************************
            case A5_02_0A:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 90.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "0A Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-0B --------------------------
            //**************************************************************
            case A5_02_0B:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 100.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "0B Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-10 --------------------------
            //**************************************************************
            case A5_02_10:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 20.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "10 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-11 --------------------------
            //**************************************************************
            case A5_02_11:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 30.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "11 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-12 --------------------------
            //**************************************************************
            case A5_02_12:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 40.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "12 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-13 --------------------------
            //**************************************************************
            case A5_02_13:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 50.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "13 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-14 --------------------------
            //**************************************************************
            case A5_02_14:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 60.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "14 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-15 --------------------------
            //**************************************************************
            case A5_02_15:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 70.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "15 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-16 --------------------------
            //**************************************************************
            case A5_02_16:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 80.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "16 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-17 --------------------------
            //**************************************************************
            case A5_02_17:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 90.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "17 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-18 --------------------------
            //**************************************************************
            case A5_02_18:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 100.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "18 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-19 --------------------------
            //**************************************************************
            case A5_02_19:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 110.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "19 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-1A --------------------------
            //**************************************************************
            case A5_02_1A:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 120.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "1A Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-1B --------------------------
            //**************************************************************
            case A5_02_1B:
                  fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 130.0;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "1B Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-20 --------------------------
            //**************************************************************
            case A5_02_20:
                  fourBsA5_02_2030_Tlg_p = (FOURBS_A5_02_2030TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(((uint16_t)fourBsA5_02_2030_Tlg_p->TempMSB << 8 | fourBsA5_02_2030_Tlg_p->TempLSB) / -20.0) + 41.2;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "20 Temp: %.1f", temp);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-02-30 --------------------------
            //**************************************************************
            case A5_02_30:
                  fourBsA5_02_2030_Tlg_p = (FOURBS_A5_02_2030TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(((uint16_t)fourBsA5_02_2030_Tlg_p->TempMSB << 8 | fourBsA5_02_2030_Tlg_p->TempLSB) / -10.0) + 62.3;
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  logDebug("4BS", "30 Temp: %.1f", temp);
                  break;
            default:
                  break;
            }
            break; // ENDE A5-02-XX

      case A5_04:
            logDebug("4BS", "A5-04-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS04))
            switch (ParamENO_CHProfil4BS04)
            {
            //**************************************************************
            // ----------------- Profil: A5-04-01 --------------------------
            //**************************************************************
            case A5_04_01:
                  fourBsA5_04_Tlg_p = (FOURBS_A5_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_04_Tlg_p->Temp / 6.25);
                  hum = (float)(fourBsA5_04_Tlg_p->Hum / 2.5);
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
                  logDebug("4BS", "01 Temp: %.1f Hum: %.1f", temp, hum);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-04-02 --------------------------
            //**************************************************************
            case A5_04_02:
                  fourBsA5_04_Tlg_p = (FOURBS_A5_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(fourBsA5_04_Tlg_p->Temp / 3.125) - 20.0;
                  hum = (float)(fourBsA5_04_Tlg_p->Hum / 2.5);
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
                  logDebug("4BS", "02 Temp: %.1f Hum: %.1f", temp, hum);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-04-03 --------------------------
            //**************************************************************
            case A5_04_03:
                  fourBsA5_04_03_Tlg_p = (FOURBS_A5_04_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  temp = (float)(((uint16_t)fourBsA5_04_03_Tlg_p->TempMSB << 8 | fourBsA5_04_03_Tlg_p->TempLSB) / 12.7875) - 20.0;
                  hum = (float)(fourBsA5_04_03_Tlg_p->Hum / 2.55);
                  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
                  logDebug("4BS", "03 Temp: %.1f Hum: %.1f", temp, hum);
                  break;
            default:
                  break;
            }
            break; // ENDE A5-04-XX
      case A5_06:
            logDebug("4BS", "A5-06-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS06))
            switch (ParamENO_CHProfil4BS06)
            {
            //**************************************************************
            // ----------------- Profil: A5-06-01 --------------------------
            //**************************************************************
            case A5_06_01:
                  logDebug("4BS", "01");
                  fourBsA5_06_01_Tlg_p = (FOURBS_A5_06_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  switch (fourBsA5_06_01_Tlg_p->u84BsTelData.RS)
                  {
                  case 0: // Range 600 ... 60000 lux
                        luxfloat = (float)(fourBsA5_06_01_Tlg_p->u8Illumination1 * 232.941176) + 600.0;
                        break;
                  case 1: // Range 300 ... 30000 lux
                        luxfloat = (float)(fourBsA5_06_01_Tlg_p->u8Illumination2 * 116.470588) + 300.0;
                        break;
                  default:
                        luxfloat = 0;
                        break;
                  }
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(fourBsA5_06_01_Tlg_p->u8SupplyVoltage / 50.0, Dpt(9, 1));

                  logDebug("4BS", "RS: %u LUX: %.1f Bat: %.1f", fourBsA5_06_01_Tlg_p->u84BsTelData.RS, luxfloat, fourBsA5_06_01_Tlg_p->u8SupplyVoltage / 50.0);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-06-01 Version 2 ----------------
            //**************************************************************
            case A5_06_01_V2:
                  logDebug("4BS", "01*");
                  fourBsA5_06_01_V2_Tlg_p = (FOURBS_A5_06_01_V2_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

                  if (fourBsA5_06_01_V2_Tlg_p->u8byte0 == 0x09)
                  {
                        if (fourBsA5_06_01_V2_Tlg_p->u8Illumbyte2 == 0) // Helligkeit 0..100LUX
                        {
                              // Range 0 ... 50 lux
                              luxfloat = (float)(fourBsA5_06_01_V2_Tlg_p->u8Illumbyte3);
                        }
                        else
                        {
                              // Range 300 ... 30000 lux
                              luxfloat = (float)(fourBsA5_06_01_V2_Tlg_p->u8Illumbyte2 * 116.470588) + 300.0;
                        }
                  }
                  else
                  {
                        luxfloat = 0;
                  }
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));

                  logDebug("4BS", "LUX: %.1f", luxfloat);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-06-02 --------------------------
            //**************************************************************
            case A5_06_02:
                  logDebug("4BS", "02");
                  fourBsA5_06_02_Tlg_p = (FOURBS_A5_06_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  switch (fourBsA5_06_02_Tlg_p->u84BsTelData.RS)
                  {
                  case 0: // Range 0 ... 1020 lux
                        luxfloat = (float)(fourBsA5_06_02_Tlg_p->u8Illumination1 / 0.25);
                        break;
                  case 1: // Range 0 ... 510 lux
                        luxfloat = (float)(fourBsA5_06_02_Tlg_p->u8Illumination2 / 0.5);
                        break;
                  default:
                        luxfloat = 0;
                        break;
                  }
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(fourBsA5_06_02_Tlg_p->u8SupplyVoltage / 50.0, Dpt(9, 1));

                  logDebug("4BS", "LUX: %.1f Bat: %.1f", luxfloat, fourBsA5_06_02_Tlg_p->u8SupplyVoltage / 50.0);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-06-03 --------------------------
            //**************************************************************
            case A5_06_03:
                  logDebug("4BS", "03");
                  fourBsA5_06_03_Tlg_p = (FOURBS_A5_06_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

                  // ........Illumination.......................................
                  lux = (uint16_t)fourBsA5_06_03_Tlg_p->u8LuxMSB << 2 | fourBsA5_06_03_Tlg_p->u84BsTelData.LUX;
                  luxfloat = (float)lux * 1.0;

                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(fourBsA5_06_03_Tlg_p->u8SupplyVoltage / 51.0, Dpt(9, 1));

                  logDebug("4BS", "LUX: %.1f Bat: %.1f", luxfloat, fourBsA5_06_03_Tlg_p->u8SupplyVoltage / 51.0);
                  break;
            default:
                  break;
            }
            break; // ENDE A5-06-XX
      case A5_07:
            logDebug("4BS", "A5-07-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS07))
            switch (ParamENO_CHProfil4BS07)
            {
            //**************************************************************
            // ----------------- Profil: A5-07-01 --------------------------
            //**************************************************************
            case A5_07_01:
                  fourBsTlg2_p = (FOURBS_A5_07_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  // .......PIR Status.........................................
                  if (fourBsTlg2_p->PIR < 128)
                        KoENO_GO_BASE__1.value(false, Dpt(1, 1));
                  else
                        KoENO_GO_BASE__1.value(true, Dpt(1, 1));
                  logDebug("4BS", "01 PIR: %u", fourBsTlg2_p->PIR);
                  break; // ENDE A5-07-01
            //**************************************************************
            // ----------------- Profil: A5-07-02 --------------------------
            //**************************************************************
            case A5_07_02:
                  fourBsTlg_p = (FOURBS_A5_07_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "02");
                  // .......PIR Status.........................................
                  KoENO_GO_BASE__1.value(fourBsTlg_p->u84BsTelData.PIR, Dpt(1, 1));
                  logDebug("4BS", "PIR: %u", fourBsTlg_p->u84BsTelData.PIR);
                  // ...................  Supply Voltage ......................
                  KoENO_GO_BASE__3.value(fourBsTlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsTlg_p->u8SupplyVoltage / 50.0);
                  break; // ENDE A5-07-02

            //**************************************************************
            // ----------------- Profil: A5-07-03 --------------------------
            //**************************************************************
            case A5_07_03:
                  fourBsTlg3_p = (FOURBS_A5_07_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "03");
                  // .......PIR Status.........................................
                  KoENO_GO_BASE__1.value(fourBsTlg3_p->u84BsTelData.PIR, Dpt(1, 1));
                  logDebug("4BS", "PIR:%u", fourBsTlg3_p->u84BsTelData.PIR);
                  // ........Illumination.......................................
                  lux = (uint16_t)fourBsTlg3_p->u8LuxMSB << 2 | fourBsTlg3_p->u84BsTelData2.LUX;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__3.value(fourBsTlg3_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsTlg3_p->u8SupplyVoltage / 50.0);
                  break; // ENDE A5-07-03

            default:
                  logDebug("4BS", "ERROR");
                  break;
            }
            break; // ENDE A5-07-XX
      case A5_08:
            logDebug("4BS", "A5-08-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS08))
            switch (ParamENO_CHProfil4BS08)
            {
            //**************************************************************
            // ----------------- Profil: A5-08-01 --------------------------
            //**************************************************************
            case A5_08_01:
                  fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "01");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination..............................................
                  lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 2;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Temperature..............................................
                  KoENO_GO_BASE__7.value(fourBsA5_08_Tlg_p->TMP / 5.0, Dpt(9, 1));
                  logDebug("4BS", "Temp: %.1f", fourBsA5_08_Tlg_p->TMP / 5.0);
                  // .......PIR Status.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
                  logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
                  // .......Occupancy Button .........................................
                  KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
                  logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);

                  break; // ENDE A5-08-01
            //**************************************************************
            // ----------------- Profil: A5-08-02 --------------------------
            //**************************************************************
            case A5_08_02:
                  fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "02");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination..............................................
                  lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 4;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Temperature..............................................
                  KoENO_GO_BASE__7.value(fourBsA5_08_Tlg_p->TMP / 5.0, Dpt(9, 1));
                  logDebug("4BS", "Temp: %.1f", fourBsA5_08_Tlg_p->TMP / 5.0);
                  // .......PIR Status.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
                  logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
                  // .......Occupancy Button .........................................
                  KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
                  logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);

                  break; // ENDE A5-08-02

            //**************************************************************
            // ----------------- Profil: A5-08-03 --------------------------
            //**************************************************************
            case A5_08_03:
                  fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "03");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination..............................................
                  lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 6;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Temperature..............................................
                  KoENO_GO_BASE__7.value((fourBsA5_08_Tlg_p->TMP / 3.1875) - 30, Dpt(9, 1));
                  logDebug("4BS", "Temp: %.1f", (fourBsA5_08_Tlg_p->TMP / 3.1875) - 30);
                  // .......PIR Status.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
                  logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
                  // .......Occupancy Button .........................................
                  KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
                  logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);

                  break; // ENDE A5-08-03
            }
            break; // ENDE A5-08-XX
      case A5_12:
            logDebug("4BS", "A5-12-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS12))
            switch (ParamENO_CHProfil4BS12)
            {
            //**************************************************************
            // ----------------- Profil: A5-12-01 --------------------------
            //**************************************************************
            case A5_12_01:
                  fourBsA5_12_01_Tlg_p = (FOURBS_A5_12_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "01");

                  value_4Byte = ((uint32_t)fourBsA5_12_01_Tlg_p->Meterreading_MSB) << 16 | fourBsA5_12_01_Tlg_p->Meterreading_MID << 8 | fourBsA5_12_01_Tlg_p->Meterreading_LSB;

                  switch (fourBsA5_12_01_Tlg_p->u84BsTelData.DIV)
                  {
                  case 0:
                        value_4Byte = value_4Byte / 1;
                        break;
                  case 1:
                        value_4Byte = value_4Byte / 10;
                        break;
                  case 2:
                        value_4Byte = value_4Byte / 100;
                        break;
                  case 3:
                        value_4Byte = value_4Byte / 1000;
                        break;

                  default:
                        break;
                  }
                  // check unit
                  if (fourBsA5_12_01_Tlg_p->u84BsTelData.DT == 1) // WATT
                  {
                        KoENO_GO_BASE__8.value(value_4Byte, Dpt(14, 1)); // WATT
                  }
                  else // KWh
                  {
                        KoENO_GO_BASE__7.value(value_4Byte, Dpt(13, 1)); // KWH
                  }

                  // Tarif
                  KoENO_GO_BASE__10.value(fourBsA5_12_01_Tlg_p->u84BsTelData.TI, Dpt(5, 1));
                  logDebug("4BS", "Energy: %u DT: %u DIV: %u Tarif: %u %u %u %u", value_4Byte, fourBsA5_12_01_Tlg_p->u84BsTelData.DT, fourBsA5_12_01_Tlg_p->u84BsTelData.DIV, fourBsA5_12_01_Tlg_p->u84BsTelData.TI, fourBsA5_12_01_Tlg_p->Meterreading_MSB, fourBsA5_12_01_Tlg_p->Meterreading_MID, fourBsA5_12_01_Tlg_p->Meterreading_LSB);
                  break;
            }
            break; // ENDE A5-12-XX

      case A5_14:
            logDebug("4BS", "A5-14-");
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BS14))
            switch (ParamENO_CHProfil4BS14)
            {
            //**************************************************************
            // ----------------- Profil: A5-14-01 --------------------------
            //**************************************************************
            case A5_14_01:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "01");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Contact..............................................
                  if (fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = true;
                        else
                              bvalue = false;
                  }
                  else // open
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                  }
                  KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                  logDebug("4BS", "detected: State: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-02 --------------------------
            //**************************************************************
            case A5_14_02:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "02");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination.......................................
                  lux = (uint16_t)fourBsA5_17_01_06_Tlg_p->u8Lux * 4;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Contact..............................................
                  if (fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = true;
                        else
                              bvalue = false;
                  }
                  else // open
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                  }
                  KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                  logDebug("4BS", "detected: State: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-03 --------------------------
            //**************************************************************
            case A5_14_03:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "03");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Contact..............................................
                  if (fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = true;
                        else
                              bvalue = false;
                  }
                  else // open
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                  }
                  KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                  logDebug("4BS", "detected: State: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT);
                  // ........Vibration Det.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibration det: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-04 --------------------------
            //**************************************************************
            case A5_14_04:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "04");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination.......................................
                  lux = (uint16_t)fourBsA5_17_01_06_Tlg_p->u8Lux * 4;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Contact..............................................
                  if (fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = true;
                        else
                              bvalue = false;
                  }
                  else // open
                  {
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                  }
                  KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                  logDebug("4BS", "detected: State: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.CT);
                  // ........Vibration Det.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibration det: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-05 --------------------------
            //**************************************************************
            case A5_14_05:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "05");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Vibration Det.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibration det: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-06 --------------------------
            //**************************************************************
            case A5_14_06:
                  fourBsA5_17_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "06");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_01_06_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Illumination.......................................
                  lux = (uint16_t)fourBsA5_17_01_06_Tlg_p->u8Lux * 4;
                  luxfloat = (float)lux * 1.0;
                  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
                  logDebug("4BS", "LUX: %u", lux);
                  // ........Vibration Det.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibration det: %u", fourBsA5_17_01_06_Tlg_p->u84BsTelData.VIB);
                  break;

            //**************************************************************
            // ----------------- Profil: A5-14-07 --------------------------
            //**************************************************************
            case A5_14_07:
                  fourBsA5_17_07_08_Tlg_p = (FOURBS_A5_14_07_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "07");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_07_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_07_08_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........DOOR Contact..............................................
                  KoENO_GO_BASE__2.value(fourBsA5_17_07_08_Tlg_p->u84BsTelData.DCT, Dpt(1, 1));
                  logDebug("4BS", "Door State: %u", fourBsA5_17_07_08_Tlg_p->u84BsTelData.DCT);
                  // ........Lock Contact.........................................
                  KoENO_GO_BASE__4.value(fourBsA5_17_07_08_Tlg_p->u84BsTelData.LCT, Dpt(1, 1));
                  logDebug("4BS", "Lock State: %u", fourBsA5_17_07_08_Tlg_p->u84BsTelData.LCT);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-08 --------------------------
            //**************************************************************
            case A5_14_08:
                  fourBsA5_17_07_08_Tlg_p = (FOURBS_A5_14_07_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "08");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_07_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_07_08_Tlg_p->u8SupplyVoltage / 50.0);
                  // ........Vibration Det.........................................
                  KoENO_GO_BASE__1.value(fourBsA5_17_07_08_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibration det: %u", fourBsA5_17_07_08_Tlg_p->u84BsTelData.VIB);
                  // ........DOOR Contact..............................................
                  KoENO_GO_BASE__2.value(fourBsA5_17_07_08_Tlg_p->u84BsTelData.DCT, Dpt(1, 1));
                  logDebug("4BS", "Door State: %u", fourBsA5_17_07_08_Tlg_p->u84BsTelData.DCT);
                  // ........Lock Contact.........................................
                  KoENO_GO_BASE__4.value(fourBsA5_17_07_08_Tlg_p->u84BsTelData.LCT, Dpt(1, 1));
                  logDebug("4BS", "Lock State: %u", fourBsA5_17_07_08_Tlg_p->u84BsTelData.LCT);
                  break;
            //**************************************************************
            // ----------------- Profil: A5-14-09 --------------------------
            //**************************************************************
            case A5_14_09:
                  fourBsA5_17_09_0A_Tlg_p = (FOURBS_A5_14_09_0A_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  //logDebug("4BS", "09 %u", firstParameter);
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_09_0A_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_09_0A_Tlg_p->u8SupplyVoltage / 50.0);

                  switch (fourBsA5_17_09_0A_Tlg_p->u84BsTelData.CT)
                  {
                  case 0x00: // close
                        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = true;
                        else
                              bvalue = false;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

                        logDebug("4BS", "State: close");
                        break;
                  case 0x01: // tilt
                        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(true, Dpt(1, 1));
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

                        logDebug("4BS", "State: gekippt");
                        break;
                  case 0x03: // open
                        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
                        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
                              bvalue = false;
                        else
                              bvalue = true;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                        logDebug("4BS", "State: open");
                        break;

                  default:
                        break;
                  }
                  break;

                  //**************************************************************
                  // ----------------- Profil: A5-14-0A --------------------------
                  //**************************************************************
            case A5_14_0A:
                  fourBsA5_17_09_0A_Tlg_p = (FOURBS_A5_14_09_0A_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "0A");
                  // ...................  Supply Voltage .......................
                  KoENO_GO_BASE__5.value(fourBsA5_17_09_0A_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
                  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_17_09_0A_Tlg_p->u8SupplyVoltage / 50.0);
                  // ...................  Fenster Status .......................
                  switch (fourBsA5_17_09_0A_Tlg_p->u84BsTelData.CT)
                  {
                  case 0x00: // close
                        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
                        if (ParamENO_CHWindowcloseValue)
                              bvalue = true;
                        else
                              bvalue = false;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

                        logDebug("4BS", "State: close");
                        break;
                  case 0x01: // tilt
                        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(true, Dpt(1, 1));
                        if (ParamENO_CHWindowcloseValue)
                              bvalue = false;
                        else
                              bvalue = true;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

                        logDebug("4BS", "State: gekippt");
                        break;
                  case 0x03: // open
                        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
                        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
                        if (ParamENO_CHWindowcloseValue)
                              bvalue = false;
                        else
                              bvalue = true;
                        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
                        logDebug("4BS", "State: open");
                        break;

                  default:
                        break;
                  }
                  // ...................  Vibration detected .......................
                  KoENO_GO_BASE__1.value(fourBsA5_17_09_0A_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
                  logDebug("4BS", "Vibr: %u", fourBsA5_17_09_0A_Tlg_p->u84BsTelData.VIB);

                  break; // A5_14_0A

            default:
                  logDebug("4BS", "ERROR");
                  break;
            }
            break; // ENDE A5-14-XX
      case A5_20:
            logDebug("4BS", "A5-20-");
//#ifndef EnOceanTEST
            switch (ParamENO_CHProfil4BS20)
//#else
//            switch (profil2nd)
//#endif
            {
            //**************************************************************
            // ----------------- Profil: A5-20-01 --------------------------
            //**************************************************************
            case A5_20_01:
                  fourBsA5_20_01_Tlg_p = (FOURBS_A5_20_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "01");
                  // check TeachIn-Bit
                  if ((fourBsA5_20_01_Tlg_p->u84BsTelData.LRNstatus) == 0)
                  {
                        logDebug("4BS", "TeachIn: %u", fourBsA5_20_01_Tlg_p->u84BsTelData.LRNstatus);
                        // check LRN TYP Bit (bit8) and  LRN Status (Bit4)
                        if ((fourBsA5_20_01_Tlg_p->u84BsTelData.LRNtype == 1) && (fourBsA5_20_01_Tlg_p->u84BsTelData.LRNstatus == 0))
                        {
                              logDebug("4BS", " OK");
                              return TEACHIN_A52001;
                        }
                        else
                        {
                              logDebug("4BS", " NOK");
                        }
                  }
                  else
                  {
                        // ......Current Position.........................................
                        _value.uint8 = fourBsA5_20_01_Tlg_p->u8CurrentPos * 2.55; // Value 0...100 = 0...100%  --> Value 0...255 = 0...100%
                        KoENO_GO_BASE__8.value(_value.uint8, Dpt(5, 1));
                        logDebug("4BS", "current Pos: %u%%", fourBsA5_20_01_Tlg_p->u8CurrentPos);
                        // ......Status Bits.........................................
                        KoENO_GO_BASE__10.value(fourBsA5_20_01_Tlg_p->u8StatusBits, Dpt(5, 1));
                        logDebug("4BS", "Status Bits (MSB-LSB): %u", fourBsA5_20_01_Tlg_p->u8StatusBits);
                        // Temperature
                        temp = (float)fourBsA5_20_01_Tlg_p->u8Temp / 6.375;
                        KoENO_GO_BASE__7.value(temp, Dpt(9, 1));
                        logDebug("4BS", "Temperatur: %.1f", temp);
                  }
                  return msg_A5_20_01;
                  break;
            //**************************************************************
            // ----------------- Profil: A5-20-04 --------------------------
            //**************************************************************
            case A5_20_04:
                  fourBsA5_20_04_Tlg_p = (FOURBS_A5_20_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "04");
                  // check TeachIn-Bit
                  if ((fourBsA5_20_04_Tlg_p->u84BsTelData.LRNstatus) == 0)
                  {
                        logDebug("4BS", "TeachIn: %u", fourBsA5_20_04_Tlg_p->u84BsTelData.LRNstatus);
                        // check LRN TYP Bit (bit8) and  LRN Status (Bit4)
                        if ((fourBsA5_20_04_Tlg_p->u84BsTelData.MSTLRNtype) == 1 && (fourBsA5_20_04_Tlg_p->u84BsTelData.LRNstatus == 0))
                        {
                              logDebug("4BS", " OK");
                              return TEACHIN_A52004;
                        }
                        else
                        {
                              logDebug("4BS", " NOK");
                        }
                  }
                  else
                  {
                        // ......Current Position.........................................
                        _value.uint8 = fourBsA5_20_04_Tlg_p->u8CurrentPos * 2.55; // Value 0...100 = 0...100%  --> Value 0...255 = 0...100%
                        KoENO_GO_BASE__8.value(_value.uint8, Dpt(5, 1));
                        logDebug("4BS", "current Pos: %u%%", fourBsA5_20_04_Tlg_p->u8CurrentPos);
                        // ......Temperature SetPoint or FeeD Temp ......................................
                        if (fourBsA5_20_04_Tlg_p->u84BsTelData.TS == 1)
                        {
                              // Temperature Setpoint
                              temp = (float)fourBsA5_20_04_Tlg_p->u8Temp / 12.75 + 10.0;
                              KoENO_GO_BASE__7.value(temp, Dpt(9, 1));
                              logDebug("4BS", "Temp SetPoint: %.1f", temp);
                        }
                        else // FEED Temperature
                        {
                              temp = (float)fourBsA5_20_04_Tlg_p->u8Temp / 4.25 + 20.0;
                              KoENO_GO_BASE__7.value(fourBsA5_20_04_Tlg_p->u8Temp, Dpt(9, 1));
                              logDebug("4BS", "Feed-Temp: %u", fourBsA5_20_04_Tlg_p->u8Temp);
                        }
                        // ...... Room Temperature or Failure Code ......................................
                        if (fourBsA5_20_04_Tlg_p->u84BsTelData.FL == 1)
                        {
                              // Failure Code
                              KoENO_GO_BASE__6.value(fourBsA5_20_04_Tlg_p->u8TempError, Dpt(5, 1));
                              logDebug("4BS", "Failure Code: %u", fourBsA5_20_04_Tlg_p->u8TempError);
                        }
                        else // Room Temperature
                        {
                              temp = (float)fourBsA5_20_04_Tlg_p->u8TempError / 12.75 + 10.0;
                              KoENO_GO_BASE__9.value(temp, Dpt(9, 1));
                              logDebug("4BS", "Room-Temp: %.1f", temp);
                        }

                        // ......Status Bits.........................................
                        KoENO_GO_BASE__10.value(f_Pkt_st->u8DataBuffer[4], Dpt(5, 1));
                        logDebug("4BS", "Status Bits (MSB-LSB): %u", f_Pkt_st->u8DataBuffer[4]);
                  }

                  return msg_A5_20_04;
                  break;
            //**************************************************************
            // ----------------- Profil: A5-20-06 --------------------------
            //**************************************************************
            case A5_20_06:
                  fourBsA5_20_06_Tlg_p = (FOURBS_A5_20_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("4BS", "06");

                  // check TeachIn-Bit
                  if (((fourBsA5_20_06_Tlg_p->u8StatusBits >> 3) & 1) == 0)
                  {
                        logDebug("4BS", "TeachIn: %u", fourBsA5_20_06_Tlg_p->u8StatusBits);
                        // check LRN TYP Bit (bit8)
                        if (((fourBsA5_20_06_Tlg_p->u8StatusBits >> 7) & 1) == 1 && ((fourBsA5_20_06_Tlg_p->u8StatusBits >> 4) & 1) == 0)
                        {
                              logDebug("4BS", " OK");
                              return TEACHIN_A52006;
                        }
                        else
                        {
                              logDebug("4BS", " NOK");
                        }
                  }
                  else
                  {
                        // ......Current Position.........................................
#ifndef EnOceanTEST
                        _value.uint8 = fourBsA5_20_06_Tlg_p->u8CurrentPos * 2.55; // Value 0...100 = 0...100%  --> Value 0...255 = 0...100%
                        KoENO_GO_BASE__8.value(_value.uint8, Dpt(5, 1));
#endif
                        logDebug("4BS", "current Pos: %u%%", fourBsA5_20_06_Tlg_p->u8CurrentPos);

                        // ......Temperature Offset.........................................
                        if (fourBsA5_20_06_Tlg_p->u84BsTelData.LOM == 1)
                        {
#ifndef EnOceanTEST
                              KoENO_GO_BASE__9.value(fourBsA5_20_06_Tlg_p->u84BsTelData.Offset / 2.0, Dpt(9, 1));
#endif
                              logDebug("4BS", "LOM = 1 Local offset: %.1f°C", fourBsA5_20_06_Tlg_p->u84BsTelData.Offset / 2.0);
                        }
                        else if (fourBsA5_20_06_Tlg_p->u84BsTelData.LOM == 0)
                        {
                              switch (fourBsA5_20_06_Tlg_p->u84BsTelData.Offset)
                              {
                              case 0x0:
                                    value = 0;
                                    break;
                              case 0x1:
                                    value = 1;
                                    break;
                              case 0x2:
                                    value = 2;
                                    break;
                              case 0x3:
                                    value = 3;
                                    break;
                              case 0x4:
                                    value = 4;
                                    break;
                              case 0x5:
                                    value = 5;
                                    break;
                              case 0x7B:
                                    value = -5;
                                    break;
                              case 0x7C:
                                    value = -4;
                                    break;
                              case 0x7D:
                                    value = -3;
                                    break;
                              case 0x7E:
                                    value = -2;
                                    break;
                              case 0x7F:
                                    value = -1;
                                    break;

                              default:
                                    value = 0;
                                    break;
                              }
#ifndef EnOceanTEST
                              KoENO_GO_BASE__9.value(value, Dpt(9, 1));
#endif
                              logDebug("4BS", "LOM = 0 Local offset: %d°C", value);
                        }
#ifndef EnOceanTEST
                        // ......Temperature.........................................
                        KoENO_GO_BASE__7.value(fourBsA5_20_06_Tlg_p->u8Temp / 2.0, Dpt(9, 1));
                        // ......Status Bits.........................................
                        KoENO_GO_BASE__10.value(fourBsA5_20_06_Tlg_p->u8StatusBits, Dpt(5, 1));
#endif

                        logDebug("4BS", "Temperature: %.1f Status Bits (MSB-LSB): %u", fourBsA5_20_06_Tlg_p->u8Temp / 2.0, fourBsA5_20_06_Tlg_p->u8StatusBits);
                        break; // ENDE A5-20-06
                  default:
                        break;
                        logDebug("4BS", "ERROR");
                        break;
                  }
            }
            return msg_A5_20_06;
            break; // ENDE A5-20-XX

      case A5_special:
            //switch (knx.paramWord(firstParameter + ENO_CHProfil4BSAdd))
            switch (ParamENO_CHProfil4BSAdd)
            {
            case D5_00_01_V2:
                  logDebug("4BS", "D5-00-01 V2");
                  l1bsV2Tlg_p = (ONEBS_TELEGRAM_TYPE_V2 *)&(f_Pkt_st->u8DataBuffer[1]);

                  // Energy storage
                  KoENO_GO_BASE__8.value(l1bsV2Tlg_p->u8EnergyVoltage * 19.608, Dpt(9, 1)); // umgerechnet in 5V in 5000mV
                  // Energy storage
                  KoENO_GO_BASE__5.value(l1bsV2Tlg_p->u8BatteryVoltage * 19.608, Dpt(9, 1)); // umgerechnet in 5V in 5000mV

                  logDebug("4BS", "Enr: %.1f Bat: %.1f", l1bsV2Tlg_p->u8EnergyVoltage / 51.0, l1bsV2Tlg_p->u8BatteryVoltage / 51.0);
                  break;

            default:
                  logDebug("4BS", "ERROR");
                  break;
            }

            break; // ENDE A5-Special

      //**************************************************************
      default:
            logDebug("4BS", "Profil: ERROR");
            break;
      }

      return 0;
}
