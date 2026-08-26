#include "EnoceanChannel.h"
#include "1BS_Telegram.h"
#include "4BS_Telegram.h"
#include "Arduino.h"
#include "EnoceanModule.h"
#include "Product_Eltako.h"
#include "RPS_Telegram.h"
#include "VLD_Telegram.h"
#include "knxprod.h"

EnoceanHandle unionMSG;

// Konstruktor: speichert den Kanal-Index dieses EnoceanChannel-Objekts.
EnoceanChannel::EnoceanChannel(uint8_t index) { _channelIndex = index; }

// Liefert den Anzeigenamen des Kanals für Logging/Debug-Ausgaben.
const std::string EnoceanChannel::name() { return "EnoceanChannel"; }

// Wird einmalig beim Start des Kanals aufgerufen, aktuell nur für Log-Testausgaben genutzt.
void EnoceanChannel::setup() {
  logInfoP("setup");
  logIndentUp();
  logDebugP("debug setup");
  logTraceP("trace setup");
  logIndentDown();
}

// Wird zyklisch aufgerufen; aktuell ohne Funktion (Platzhalter für spätere Logik).
void EnoceanChannel::loop() {
  // Dummy Action
  // delayMicroseconds(100);
}

// Prüft anhand der EnOcean-ID und des RORG-Typs, ob ein empfangenes Paket zu diesem Kanal gehört,
// und leitet es je nach Profil (1BS/RPS/4BS/VLD) an den passenden Telegramm-Handler weiter.
bool EnoceanChannel::check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket) {
  // pPacket->u8DataBuffer holds RORG + data + sender-ID + status.
  // Position of the 4 sender-ID bytes depends on u8DataBuffer[0] (RORG):
  //  - RPS/1BS: bytes [2..5]
  //  - 4BS:     bytes [5..8]
  //  - VLD:     bytes [u16DataLength-5 .. u16DataLength-2]

  // init parameter
  unionMSG.msg_sent_after_receive = 0;

  // Get rid of messages we can't handle
  if (0 == ParamENO_CHProductsel) {
    return false;
  }

  // Read Parameter: ENOCEAN-ID
  deviceId_Arr[0] = (ParamENO_CHId0 << 4) | ParamENO_CHId1;
  deviceId_Arr[1] = (ParamENO_CHId2 << 4) | ParamENO_CHId3;
  deviceId_Arr[2] = (ParamENO_CHId4 << 4) | ParamENO_CHId5;
  deviceId_Arr[3] = (ParamENO_CHId6 << 4) | ParamENO_CHId7;

  logDebugP("ENO-ID:");
  logHexDebugP(deviceId_Arr, 4);
  // logHexDebugP(&pPacket->u8DataBuffer[2], 4);

  // Prüft wie der CH konfiguriert ist. Über EEP-Profil oder über Hersteller
  switch (ParamENO_CHProductsel) {
  case CH_inaktive: // Kein Profil oder Hersteller gewählt
    return false;
    break;

  case EEP_Profil: // EEP Profile

    // Get rid of messages we can't handle
    if (pPacket->u8DataBuffer[0] != ParamENO_CHProfilSelection && ParamENO_CHProfilSelection != u8RORG_Rocker) {
      logDebugP("wrong Profil");
      return false;
    }

    switch (ParamENO_CHProfilSelection) {
    case u8RORG_1BS:
      // Get rid of messages not intended for us
      if (pPacket->u8DataBuffer[2] != deviceId_Arr[0])
        return false;
      if (pPacket->u8DataBuffer[3] != deviceId_Arr[1])
        return false;
      if (pPacket->u8DataBuffer[4] != deviceId_Arr[2])
        return false;
      if (pPacket->u8DataBuffer[5] != deviceId_Arr[3])
        return false;

      logHexDebugP(&pPacket->u8DataBuffer[2], 4);

      // Verarbeitet ein empfangenes 1BS-EnOcean-Telegramm (D5-00-01 Kontakt) und schreibt den Open/Close-Zustand auf
      // das KNX-Objekt
      handle_1BS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_RPS:
      // Get rid of messages not intended for us
      if (pPacket->u8DataBuffer[2] != deviceId_Arr[0])
        return false;
      if (pPacket->u8DataBuffer[3] != deviceId_Arr[1])
        return false;
      if (pPacket->u8DataBuffer[4] != deviceId_Arr[2])
        return false;
      if (pPacket->u8DataBuffer[5] != deviceId_Arr[3])
        return false;

      logHexDebugP(&pPacket->u8DataBuffer[2], 4);

      handle_RPS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_4BS:

      // Get rid of messages not intended for us
      if (pPacket->u8DataBuffer[5] != deviceId_Arr[0])
        return false;
      if (pPacket->u8DataBuffer[6] != deviceId_Arr[1])
        return false;
      if (pPacket->u8DataBuffer[7] != deviceId_Arr[2])
        return false;
      if (pPacket->u8DataBuffer[8] != deviceId_Arr[3])
        return false;

      logHexDebugP(&pPacket->u8DataBuffer[5], 4);

      unionMSG.msg_sent_after_receive = handle_4BS(pPacket, _channelIndex);
      return true;

      break;

    case u8RORG_VLD:

      // Get rid of messages not intended for us
      if (pPacket->u8DataBuffer[pPacket->u16DataLength - 5] != deviceId_Arr[0])
        return false;
      if (pPacket->u8DataBuffer[pPacket->u16DataLength - 4] != deviceId_Arr[1])
        return false;
      if (pPacket->u8DataBuffer[pPacket->u16DataLength - 3] != deviceId_Arr[2])
        return false;
      if (pPacket->u8DataBuffer[pPacket->u16DataLength - 2] != deviceId_Arr[3])
        return false;

      logHexDebugP(&pPacket->u8DataBuffer[pPacket->u16DataLength - 5], 4);

      handle_VLD(pPacket, _channelIndex);
      return true;
      break;

      //    case u8RORG_Rocker:
      //      // Get rid of messages not intended for us
      //      if (pPacket->u8DataBuffer[2] != deviceId_Arr[0])
      //        return false;
      //      if (pPacket->u8DataBuffer[3] != deviceId_Arr[1])
      //        return false;
      //      if (pPacket->u8DataBuffer[4] != deviceId_Arr[2])
      //        return false;
      //      if (pPacket->u8DataBuffer[5] != deviceId_Arr[3])
      //        return false;
      //
      // #ifdef KDEBUG_Rocker
      //      SERIAL_PORT.print(pPacket->u8DataBuffer[2], HEX);
      //      SERIAL_PORT.print(".");
      //      SERIAL_PORT.print(pPacket->u8DataBuffer[3], HEX);
      //      SERIAL_PORT.print(".");
      //      SERIAL_PORT.print(pPacket->u8DataBuffer[4], HEX);
      //      SERIAL_PORT.print(".");
      //      SERIAL_PORT.println(pPacket->u8DataBuffer[5], HEX);
      // #endif
      //      // uint8_t stateRocker = handle_RPS_Rocker(pPacket, profil,
      //      firstComObj, firstParameter, index); uint8_t stateRocker =
      //      pPacket->u8DataBuffer[1];
      // #ifdef KDEBUG
      //      // SERIAL_PORT.println(stateRocker, HEX);
      // #endif
      //      switch (stateRocker)
      //      {
      //      case AI_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case AI_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case AO_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case AO_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case BI_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case BI_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case BO_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case BO_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case CI_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case CI_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case CO_pressed:
      //        unionMSG.rockerState_pressed = stateRocker;
      //        break;
      //      case CO_release:
      //        union2.rockerState_Release = stateRocker;
      //        break;
      //      case Contact_pressed:
      //        if (knx.paramByte(firstParameter + ENO_CHRockerProfil) == Wippen1)
      //          handle_RPS_Rocker(pPacket, _channelIndex);
      //        break;
      //      case Contact_release:
      //        if (knx.paramByte(firstParameter + ENO_CHRockerProfil) == Wippen1)
      //          handle_RPS_Rocker(pPacket, _channelIndex);
      //        break;
      //      default:
      //        union2.rockerState_Release = stateRocker; // Für Wert = 0
      //        break;
      //      }
      //      break;
    } // ENDE switch (ParamENO_CHProfilSelection)
    break; // ENDE EEP-Profil

  case Hersteller_Eltako: // Hersteller Eltako
    handleProductEltako(pPacket, _channelIndex);
    return true;
    break; // ENDE Hersteller ELTAKO

  default:
    break;
  } // ENDE SWITCH(ParamENO_CHProductsel)

  return false;
}
