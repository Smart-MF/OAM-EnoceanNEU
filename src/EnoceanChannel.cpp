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

// Wird zyklisch aufgerufen: sendet bei der Taster-Emulation (KNX -> EnOcean) das ausstehende Loslass-Telegramm und
// erkennt bei gedrücktem Taster (EnOcean -> KNX) den langen Tastendruck (Zeit per ETS-Parameter in 10 ms).
void EnoceanChannel::loop() {
  if (_knxRockerReleaseAt != 0 && (int32_t)(millis() - _knxRockerReleaseAt) >= 0) {
    _knxRockerReleaseAt = 0;
    openknxEnoceanModule.sendRockerTelegram(_channelIndex, 0x00);
  }

  if (_rockerWippe == ROCKER_NONE || _rockerLong)
    return;

  if (!rockerNeedsLongPress(rockerFunction(_rockerWippe, _channelIndex)))
    return;

  if (delayCheck(_rockerPressTime, ParamENO_CHRockerLongPressWaitTime * 10)) {
    _rockerLong = true;
    rockerLongStart(_rockerWippe, _rockerUpper, _channelIndex);
  }
}

// Sendet "Taste gedrückt" und plant das Loslass-Telegramm nach releaseAfterMs (0 = Taste bleibt gedrückt, bis ein
// Loslassen explizit angefordert wird).
void EnoceanChannel::knxRockerPress(uint8_t db0, uint32_t releaseAfterMs) {
  _knxRockerLastDb0 = db0;
  openknxEnoceanModule.sendRockerTelegram(_channelIndex, db0);
  _knxRockerReleaseAt = millis() + releaseAfterMs;
  if (_knxRockerReleaseAt == 0)
    _knxRockerReleaseAt = 1; // 0 ist "kein Loslassen ausstehend"
}

// Taster-Emulation KNX -> EnOcean für das Eingangs-KO KO_0, Funktion per ETS (ProfilRockerFunktionKNX):
// 1 = Schalten (DPT 1.001): EIN = Taste oben (AO), AUS = Taste unten (AI), jeweils kurz getippt.
// 2 = Dimmen (DPT 3.007): Start = Taste halten (heller oben / dunkler unten), Stopp = loslassen.
// 3 = Jalousie (DPT 3.008): Fahren = kurz tippen (AB unten / AUF oben), Stopp = erneut in letzter Richtung tippen.
void EnoceanChannel::processKnxRocker(GroupObject &ko) {
  switch (ParamENO_CHProfilRockerFunktionKNX) {
  case 1: { // Schalten
    bool on = ko.value(Dpt(1, 1));
    logDebugP("KNX -> EnOcean: Schalten %u", on);
    knxRockerPress(on ? 0x30 : 0x10, ROCKER_TAP_MS);
    break;
  }
  case 2: { // Dimmen
    bool up = ko.value(Dpt(3, 7, 0));
    uint8_t step = ko.value(Dpt(3, 7, 1));
    logDebugP("KNX -> EnOcean: Dimmen %s, Step %u", up ? "heller" : "dunkler", step);
    if (step == 0)
      _knxRockerReleaseAt = millis(); // Stopp: sofort loslassen (in loop())
    else
      knxRockerPress(up ? 0x30 : 0x10, ROCKER_HOLD_MAX_MS);
    break;
  }
  case 3: { // Jalousie: DPT 3.008 Richtungsbit 1 = AB
    bool down = ko.value(Dpt(3, 8, 0));
    uint8_t step = ko.value(Dpt(3, 8, 1));
    logDebugP("KNX -> EnOcean: Jalousie %s, Step %u", down ? "AB" : "AUF", step);
    if (step == 0) {
      if (_knxRockerLastDb0 != 0)
        knxRockerPress(_knxRockerLastDb0, ROCKER_TAP_MS); // Stopp: in letzter Fahrtrichtung tippen
    } else
      knxRockerPress(down ? 0x10 : 0x30, ROCKER_TAP_MS);
    break;
  }
  }
}

// Verarbeitet ein Rocker-Telegramm (F6-02-xx) dieses Kanals: Drücken merkt sich Wippe/Seite und löst Schalten/Szene
// sofort aus, Loslassen wertet kurzen bzw. langen Tastendruck aus (langer Druck wird in loop() erkannt).
void EnoceanChannel::handleRocker(PACKET_SERIAL_TYPE_ *pPacket) {
  uint8_t db0 = pPacket->u8DataBuffer[1];
  bool pressed = db0 & 0x10; // EB: Energy Bow

  if (pressed) {
    bool upper = false;
    uint8_t wippe = rockerDecode(db0, &upper, _channelIndex);
    logDebugP("Rocker pressed: DB0 %02X, Wippe %u, %s", db0, wippe, upper ? "oben" : "unten");
    if (wippe == ROCKER_NONE)
      return;

    _rockerWippe = wippe;
    _rockerUpper = upper;
    _rockerLong = false;
    _rockerPressTime = millis();
    rockerPress(wippe, upper, _channelIndex);
    return;
  }

  // losgelassen
  logDebugP("Rocker released");
  if (_rockerWippe == ROCKER_NONE)
    return;

  if (_rockerLong)
    rockerLongEnd(_rockerWippe, _channelIndex);
  else
    rockerShort(_rockerWippe, _rockerUpper, _channelIndex);

  _rockerWippe = ROCKER_NONE;
}

// Prüft anhand der EnOcean-ID und des RORG-Typs, ob ein empfangenes Paket zu diesem Kanal gehört,
// und leitet es je nach Profil (1BS/RPS/4BS/VLD) an den passenden Telegramm-Handler weiter.
bool EnoceanChannel::check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket) {
  // init parameter
  unionMSG.msg_sent_after_receive = 0;

  // Get rid of messages we can't handle
  if (0 == ParamENO_CHProductsel) {
    return false;
  }

  // Get rid of messages not intended for us: die Sender-ID (Position abhängig vom RORG, siehe
  // EnoceanModule::extractSenderId) muss mit der parametrierten ENOCEAN-ID übereinstimmen - für EEP-Profile
  // wie für Hersteller-Produkte.
  uint8_t senderId[4];
  if (!EnoceanModule::extractSenderId(pPacket, senderId) || !matchesDeviceId(senderId)) {
    return false;
  }

  logDebugP("ENO-ID:");
  logHexDebugP(senderId, 4);

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
      // Verarbeitet ein empfangenes 1BS-EnOcean-Telegramm (D5-00-01 Kontakt) und schreibt den Open/Close-Zustand auf
      // das KNX-Objekt
      handle_1BS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_RPS:
      handle_RPS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_4BS:
      unionMSG.msg_sent_after_receive = handle_4BS(pPacket, _channelIndex);
      return true;

      break;

    case u8RORG_VLD:
      handle_VLD(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_Rocker:
      // Taster: nur RPS-Telegramme, nur Richtung EnOcean -> KNX (0); KNX -> EnOcean siehe handleKnxEvent()
      if (pPacket->u8DataBuffer[0] != u8RORG_RPS || ParamENO_CHDirectionKnxEnocean != 0)
        return false;
      handleRocker(pPacket);
      return true;
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

// Vergleicht die EnOcean-ID dieses Kanals mit der übergebenen ID (unabhängig vom gewählten Profil), um
// kanalübergreifend feststellen zu können, ob eine empfangene EnOcean-ID bereits bekannt ist.
// ID-Quelle: die ETS-Parameter, wenn dort eine ID steht; sonst im Teach-In-Modus die im Gateway eingelernte ID
// (Brücke bis zum nächsten ETS-Download, siehe "Ergebnis übernehmen").
bool EnoceanChannel::matchesDeviceId(const uint8_t id[4]) const {
  if (ParamENO_CHProductsel == CH_inaktive)
    return false;

  uint8_t configuredId[4];
  configuredId[0] = (ParamENO_CHId0 << 4) | ParamENO_CHId1;
  configuredId[1] = (ParamENO_CHId2 << 4) | ParamENO_CHId3;
  configuredId[2] = (ParamENO_CHId4 << 4) | ParamENO_CHId5;
  configuredId[3] = (ParamENO_CHId6 << 4) | ParamENO_CHId7;

  bool paramIdSet = configuredId[0] | configuredId[1] | configuredId[2] | configuredId[3];
  if (!paramIdSet) {
    if (ParamENO_CHSetupMode != ENO_SETUP_TEACHIN || !openknxEnoceanModule.learnedId(_channelIndex, configuredId))
      return false;
  }

  return memcmp(configuredId, id, 4) == 0;
}
