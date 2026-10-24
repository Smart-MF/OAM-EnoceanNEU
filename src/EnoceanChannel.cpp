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

// Wird einmalig beim Start des Kanals aufgerufen: startet die Ausfallüberwachung ab dem Neustart.
void EnoceanChannel::setup() {
  logDebugP("setup");
  _lastSeen = millis();
}

// Das Gerät dieses Kanals hat gesendet: Ausfallüberwachung neu starten und einen gemeldeten Ausfall zurücknehmen.
void EnoceanChannel::deviceSeen() {
  _lastSeen = millis();
  if (_failed) {
    _failed = false;
    logInfoP("Device available again");
    KoENO_CHFailure.value(false, Dpt(1, 5));
  }
}

// Meldet über KO_10 einen Ausfall, wenn das Gerät länger als die per ETS eingestellte Zeit nichts gesendet hat.
void EnoceanChannel::checkFailure() {
  uint8_t hours = ParamENO_CHMonitorTime;
  if (hours == 0 || _failed || ParamENO_CHProductsel == CH_inaktive)
    return;
  if (delayCheck(_lastSeen, hours * 3600000UL)) {
    _failed = true;
    logInfoP("Device failure: no telegram for %u h", hours);
    KoENO_CHFailure.value(true, Dpt(1, 5));
  }
}

// Wird zyklisch aufgerufen: sendet bei der Taster-Emulation (KNX -> EnOcean) das ausstehende Loslass-Telegramm und
// erkennt bei gedrücktem Taster (EnOcean -> KNX) den langen Tastendruck (Zeit per ETS-Parameter in 10 ms).
void EnoceanChannel::loop() {
  checkFailure();

  // Antwort an ein bidirektionales Gerät: wird beim Empfang vorbereitet und hier gesendet, weil der Empfangspuffer
  // beim Senden für die Antwort des Transceivers wiederverwendet wird (Gerät hört ca. 1 s nach seinem Telegramm zu)
  sendPendingHvacReply();

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

// Sendet ein Lerntelegramm mit der Sender-ID dieses Kanals (Base-ID + Kanalindex), damit ein EnOcean-Aktor im
// Lernmodus den Kanal einlernen kann (ETS-Button / Konsole "eno learn N"). Derzeit für den Taster in Richtung
// KNX -> EnOcean: Tastendruck oben (AO), danach Loslassen. Liefert false, wenn der Kanal nicht sendet.
bool EnoceanChannel::sendLearnTelegram() {
  if (ParamENO_CHProductsel != EEP_Profil || ParamENO_CHProfilSelection != u8RORG_Rocker || ParamENO_CHDirectionKnxEnocean != 1)
    return false;

  logInfoP("Send learn telegram (rocker AO)");
  knxRockerPress(0x30, ROCKER_TAP_MS);
  return true;
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
  deviceSeen();

  // Bidirektionale Geräte (EEP A5-20-01 oder Eltako FKS-SV) brauchen eine Antwort im Empfangsfenster
  if (isHvacA52001()) {
    handleHvac(pPacket);
    return true;
  }

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

// Kanal ist ein bidirektionaler Heizkörper-Stellantrieb A5-20-01: als EEP-Profil oder als Eltako FKS-SV.
bool EnoceanChannel::isHvacA52001() const {
  switch (ParamENO_CHProductsel) {
  case EEP_Profil:
    return ParamENO_CHProfilSelection == u8RORG_4BS && ParamENO_CHProfilSelection4BS == A5_20 && ParamENO_CHProfil4BS20 == A5_20_01;
  case Hersteller_Eltako:
    return ParamENO_CHProductselType == Eltako_Heizung && ParamENO_CHProductselTypeHei == Eltako_Heizung_FKS_SV;
  default:
    return false;
  }
}

// Telegramm des Antriebs: KOs aktualisieren und die Antwort vorbereiten (Teach-In-Antwort oder Sollwert).
void EnoceanChannel::handleHvac(PACKET_SERIAL_TYPE_ *pPacket) {
  if (pPacket->u8DataBuffer[0] != u8RORG_4BS) {
    logDebugP("A5-20-01: R-ORG %02X ignored", pPacket->u8DataBuffer[0]);
    return;
  }

  _hvacRxTime = millis();
  EnoceanModule::extractSenderId(pPacket, _hvacDest);

  switch (handle4BS_A5_20_01(pPacket, _channelIndex)) {
  case TEACHIN_A52001:
    prepareHvacTeachResponse(pPacket);
    break;
  case msg_A5_20_01:
    prepareHvacReply(pPacket);
    break;
  default:
    break;
  }
}

// Das Gateway hat im Teach-In-Fenster ein Lerntelegramm diesem Kanal zugeordnet. Bidirektionale Geräte erwarten
// darauf eine Teach-In-Antwort, sonst brechen sie das Einlernen ab. Sie wird hier sofort gesendet (der Aufrufer
// verarbeitet das Telegramm danach nicht weiter, der Empfangspuffer darf also überschrieben werden).
void EnoceanChannel::teachInReceived(PACKET_SERIAL_TYPE_ *pPacket) {
  if (!isHvacA52001()) {
    logInfoP("Teach-in: channel is not configured as A5-20-01/FKS-SV, no teach-in response");
    return;
  }
  handleHvac(pPacket);
  sendPendingHvacReply();
}

// Sendet eine vorbereitete Antwort an das bidirektionale Gerät. Die Teach-In-Antwort geht adressiert an den Antrieb.
void EnoceanChannel::sendPendingHvacReply() {
  if (!_hvacReplyPending)
    return;
  _hvacReplyPending = false;

  bool ok = _hvacReplyIsTeach ? openknxEnoceanModule.send4BSTelegram(_channelIndex, _hvacReply, "A5-20-01 teach-in response", _hvacDest)
                              : openknxEnoceanModule.send4BSTelegram(_channelIndex, _hvacReply, "A5-20-01 reply");
  logInfoP("A5-20-01 %s %02X %02X %02X %02X sent %u ms after receive%s", _hvacReplyIsTeach ? "teach-in response" : "reply",
           _hvacReply[0], _hvacReply[1], _hvacReply[2], _hvacReply[3], (uint32_t)(millis() - _hvacRxTime), ok ? "" : " (FAILED)");
  if (ok && !_hvacReplyIsTeach)
    _hvacOneShot = 0;
}

// Teach-In-Antwort (4BS, Variante 3): EEP und Hersteller wie in der Anfrage, DB0 = LRN-Typ 1, EEP unterstützt,
// Einlernen erfolgreich, Antwort, LRN-Bit 0. Sender-ID ist die Kanal-ID (Base-ID + Kanalindex).
void EnoceanChannel::prepareHvacTeachResponse(PACKET_SERIAL_TYPE_ *pPacket) {
  _hvacReply[0] = pPacket->u8DataBuffer[1];
  _hvacReply[1] = pPacket->u8DataBuffer[2];
  _hvacReply[2] = pPacket->u8DataBuffer[3];
  _hvacReply[3] = 0xF0;
  _hvacReplyIsTeach = true;
  _hvacReplyPending = true;
  logInfoP("A5-20-01 teach-in request, sending response");
}

// Antwort auf ein Datentelegramm: DB3 Sollwert, DB2 Raumtemperatur (von KNX, sonst die gemessene Temperatur des
// Antriebs zurück), DB1 Befehle, DB0 = Datentelegramm. Ohne Sollwert von KNX (z.B. nach einem Neustart) wird nicht
// geantwortet, der Antrieb behält dann sein eigenes Verhalten.
void EnoceanChannel::prepareHvacReply(PACKET_SERIAL_TYPE_ *pPacket) {
  if (!_hvacSetpointValid) {
    logDebugP("A5-20-01: no setpoint from KNX yet, no reply");
    return;
  }

  _hvacReply[0] = _hvacSetpoint;
  // DB2 ist in Richtung 2 invertiert (255..0 = 0..40 °C), die gemessene Temperatur (DB1) in Richtung 1 linear
  _hvacReply[1] = _hvacRoomTempValid ? _hvacRoomTemp : (uint8_t)(255 - pPacket->u8DataBuffer[3]);
  _hvacReply[2] = _hvacFlags | _hvacOneShot | (ParamENO_CHA52001SPS ? A52001_DB1_SPS : 0);
  _hvacReply[3] = 0x08;
  _hvacReplyIsTeach = false;
  _hvacReplyPending = true;
}

// Temperatur in °C auf den A5-20-01-Wertebereich 0..255 = 0..40 °C umrechnen.
static uint8_t hvacTempToRaw(double temp) {
  if (temp < 0.0)
    temp = 0.0;
  if (temp > 40.0)
    temp = 40.0;
  return (uint8_t)round(temp * 255.0 / 40.0);
}

// KNX -> Antrieb (A5-20-01): Werte nur merken, gesendet werden sie mit der nächsten Antwort (prepareHvacReply).
// KO_0 Sollwert (Temperatur DPT 9.001 oder Ventilstellung DPT 5.001, je nach ETS), KO_1 Sommerbetrieb, KO_2
// Referenzfahrt, KO_3 Raumtemperatur, KO_4 Service (1 = Ventil schließen, 2 = Ventil öffnen, 3 = Hub einstellen).
// Liefert false, wenn der Kanal kein A5-20-01 ist.
bool EnoceanChannel::processKnxHvac(int koIndex, GroupObject &ko) {
  if (!isHvacA52001())
    return false;

  switch (koIndex) {
  case ENO_KoGO_BASE__1:
    if (ParamENO_CHA52001SPS) {
      double temp = ko.value(Dpt(9, 1));
      _hvacSetpoint = hvacTempToRaw(temp);
      logInfoP("A5-20-01 setpoint %.1f °C (sent with next reply)", temp);
    } else {
      _hvacSetpoint = (uint8_t)ko.value(Dpt(5, 1)); // 0..100 %
      logInfoP("A5-20-01 valve position %u %% (sent with next reply)", _hvacSetpoint);
    }
    _hvacSetpointValid = true;
    break;

  case ENO_KoGO_BASE__2:
    if ((bool)ko.value(Dpt(1, 1)))
      _hvacFlags |= A52001_DB1_SB;
    else
      _hvacFlags &= ~A52001_DB1_SB;
    logInfoP("A5-20-01 summer mode %u", (_hvacFlags & A52001_DB1_SB) != 0);
    break;

  case ENO_KoGO_BASE__3:
    if ((bool)ko.value(Dpt(1, 1))) {
      _hvacOneShot |= A52001_DB1_RIN;
      logInfoP("A5-20-01 reference run requested");
    }
    break;

  case ENO_KoGO_BASE__4: {
    double temp = ko.value(Dpt(9, 1));
    _hvacRoomTemp = 255 - hvacTempToRaw(temp); // Richtung 2: 255..0 = 0..40 °C
    _hvacRoomTempValid = true;
    logDebugP("A5-20-01 room temperature %.1f °C", temp);
    break;
  }

  case ENO_KoGO_BASE__5: {
    uint8_t cmd = (uint8_t)ko.value(Dpt(5, 10));
    _hvacOneShot &= ~(A52001_DB1_VC | A52001_DB1_VO | A52001_DB1_LFS);
    switch (cmd) {
    case 1:
      _hvacOneShot |= A52001_DB1_VC;
      break;
    case 2:
      _hvacOneShot |= A52001_DB1_VO;
      break;
    case 3:
      _hvacOneShot |= A52001_DB1_LFS;
      break;
    default:
      break;
    }
    logInfoP("A5-20-01 service command %u", cmd);
    break;
  }

  default:
    break;
  }
  return true;
}
