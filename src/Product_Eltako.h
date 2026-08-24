#pragma once

#include "EnoceanModule.h"
#include "knxprod.h"

#include "1BS_Telegram.h"
#include "4BS_Telegram.h"
#include "RPS_Telegram.h"

#define Eltako_inaktive 0
#define Eltako_Taster 1
#define Eltako_Kontakte 2
#define Eltako_Umwelt 3
#define Eltako_Helligkeit 4
#define Eltako_Bewegung 5
#define Eltako_Melder 6
#define Eltako_Heizung 7
#define Eltako_Wasser 8

// ParamENO_CHProductselTypeKon
#define ELtako_Kontakte_inaktive 0
#define ELtako_Kontakte_FFG7B 1
#define ELtako_Kontakte_FFKB 2
#define ELtako_Kontakte_FTK 3
#define ELtako_Kontakte_FTKB 4
#define ELtako_Kontakte_FTKE 5
#define ELtako_Kontakte_FFTE 6

// ParamENO_CHProductselTypeUmw
#define Eltako_Umwelt_inaktive 0
#define Eltako_Umwelt_FFT60SB 1
#define Eltako_Umwelt_FTFB 2
#define Eltako_Umwelt_FTFSB 3
#define Eltako_Umwelt_FFT55EB 4
#define Eltako_Umwelt_FLGTF55E 5
#define Eltako_Umwelt_FCO2TF65 6
#define Eltako_Umwelt_FLT58 7

// ParamENO_CHProductselTypeHell
#define Eltako_Helligkeit_inaktive 0
#define Eltako_Helligkeit_FHD60SB 1

// ParamENO_CHProductselTypeBew
#define Eltako_Bewegung_inaktive 0
#define Eltako_Bewegung_FB55EB 1
#define Eltako_Bewegung_FBHT55ESB 2
#define Eltako_Bewegung_FABH65S 3
#define Eltako_Bewegung_FABH130 4

// ParamENO_CHProductselTypeMel
#define Eltako_Melder_inaktive 0
#define Eltako_Melder_FRWB 1
#define Eltako_Melder_FHMB 2

// ParamENO_CHProductselTypeHei
#define Eltako_Heizung_inaktive 0
#define Eltako_Heizung_FKS_SV 1
#define Eltako_Heizung_FKS_B 2

// ParamENO_CHProductselTypeWas
#define Eltako_Wasser_inaktive 0
#define Eltako_Wasser_FWS81 1

inline void handleProductEltako(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {

  switch (ParamENO_CHProductselType) {
  case Eltako_inaktive:
    logDebug("Eltako", "Typen: inaktive");
    break;

  case Eltako_Taster:
    /* code */
    break;

  case Eltako_Kontakte:
    switch (ParamENO_CHProductselTypeKon) {
    case ELtako_Kontakte_inaktive:
      logDebug("Eltako Kontakte", "Profil: inaktive");
      break;

    case ELtako_Kontakte_FFG7B:
      // FFG7B sendet je nach Ereignis entweder ein 4BS-Statustelegramm (A5-14-09) oder ein RPS-Fenstergriff-Telegramm
      // (F6-10-00) - welches der beiden gerade vorliegt, entscheidet das tatsächlich empfangene RORG-Byte, nicht ein
      // fester ETS-Parameter.
      if (f_Pkt_st->u8DataBuffer[0] == u8RORG_4BS && ParamENO_CHSelectionFFG7B == 0) {
        handle4BS_A5_14_09(f_Pkt_st, _channelIndex);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_RPS && ParamENO_CHSelectionFFG7B == 1) {
        handle_F6_10_00(f_Pkt_st, _channelIndex);
      }
      break;

    case ELtako_Kontakte_FFKB: //(EEP: D5-00-01)
    case ELtako_Kontakte_FTK:  //(EEP: D5-00-01)
    case ELtako_Kontakte_FTKB: //(EEP: D5-00-01)
      handle_1BS(f_Pkt_st, _channelIndex);
      break;

    case ELtako_Kontakte_FTKE: // EEP F6-10-00
    case ELtako_Kontakte_FFTE: // EEP F6-10-00
      /* Funk-Telegramme nach EEP F6-10-00
            ORG = 0x05
            Data_byte3 = 0xF0 = Fenster geschlossen
                         0xE0 = Fenster offen */
      handle_F6_10_00(f_Pkt_st, _channelIndex);
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeKon)
    break;

  case Eltako_Umwelt:
    switch (ParamENO_CHProductselTypeUmw) {
    case Eltako_Umwelt_inaktive:
      logDebug("Eltako Umweltsensoren", "Profil: inaktive");
      break;

    case Eltako_Umwelt_FFT60SB: // A5-04-02 oder A5-04-03
    case Eltako_Umwelt_FTFB:    // A5-04-02 oder A5-04-03
    case Eltako_Umwelt_FTFSB:   // A5-04-02 oder A5-04-03
    case Eltako_Umwelt_FFT55EB: // A5-04-02 oder A5-04-03
      handle4BS_A5_04_02(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Umwelt_FLGTF55E: // A5-09-0C & A5-04-02
      handle4BS_A5_04_02(f_Pkt_st, _channelIndex);
      handle4BS_A5_09_0C(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Umwelt_FCO2TF65: // A5-09-04
      handle4BS_A5_09_04(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Umwelt_FLT58: // A5-09-05 & A5-04-02
      handle4BS_A5_04_02(f_Pkt_st, _channelIndex);
      handle4BS_A5_09_05(f_Pkt_st, _channelIndex);
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeUmw)
    break;

  case Eltako_Helligkeit:
    switch (ParamENO_CHProductselTypeHell) {
    case Eltako_Helligkeit_inaktive:
      logDebug("Eltako Helligkeitssensoren", "Profil: inaktive");
      break;

    case Eltako_Helligkeit_FHD60SB:
      /* EEP A5-06-01 */
      /*Data_byte3 = Helligkeit 0-100 lux, linear n = 0x00-0x64 (nur gültig wenn DB2 = 0x00)
        Data_byte2 = Helligkeit 300-30.000 lux, linear n = 0x00-0xFF
        Data_byte1 = -
        Data_byte0 = 0x09*/
      handle4BS_A5_06_01_V2(f_Pkt_st, _channelIndex);
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeHell)
    break;

  case Eltako_Bewegung:
    switch (ParamENO_CHProductselTypeBew) {
    case Eltako_Bewegung_inaktive:
      logDebug("Eltako BWM", "Profil: inaktive");
      break;

    case Eltako_Bewegung_FB55EB: //EEP A5-07-01
      handle4BS_A5_07_01(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Bewegung_FBHT55ESB:  // EEP: A5-08-01 oder  A5-07-01
      handle4BS_A5_08_01(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Bewegung_FABH65S: // EEP A5-08-01  mit Eltako Anpassung 
      handle4BS_A5_08_01(f_Pkt_st, _channelIndex);
      break;

    case Eltako_Bewegung_FABH130: // EEP F6-02-01 (Rocker Taster)
      /* code */
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeBew)
    break;

  case Eltako_Melder:
    switch (ParamENO_CHProductselTypeMel) {
    case Eltako_Melder_inaktive:
      logDebug("Eltako RM", "Profil: inaktive");
      break;

    case Eltako_Melder_FRWB:
    case Eltako_Melder_FHMB:
      handle4BS_A5_30_03(f_Pkt_st, _channelIndex);
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeMel)
    break;

  case Eltako_Heizung:
    switch (ParamENO_CHProductselTypeHei) {
    case Eltako_Heizung_inaktive:
      logDebug("Eltako Heizung", "Profil: inaktive");
      break;

    case Eltako_Heizung_FKS_SV:
      /* code */
      break;

    case Eltako_Heizung_FKS_B:
      /* code */
      break;

    default:
      break;
    } // ENDE switch (ParamENO_CHProductselTypeHei)
    break;

  case Eltako_Wasser:
    switch (ParamENO_CHProductselTypeWas) {
    case Eltako_Wasser_inaktive:
      logDebug("Eltako Wassersensoren", "Profil: inaktive");
      break;

    case Eltako_Wasser_FWS81:
      /* code */
      break;

    default:

      break;
    } // ENDE switch (ParamENO_CHProductselTypeWas)
    break;

  default:
    break;
  } // ENDE switch (ParamENO_CHProductselType)
}