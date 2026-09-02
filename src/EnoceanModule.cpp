#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "knxprod.h"
#include "EnOceanESP3.h"
#include "EnOceanTeachIn.h"

#define KDEBUG_Received

// CRC8-Tabelle sowie die ESP3-Paket-Statemachine (Senden/Empfangen) leben jetzt in EnOceanESP3.

// Konstruktor: verdrahtet _teachIn mit dem eigenen ESP3-Treiber und registriert die KO-Rückmeldung für dessen
// Lernmodus-Änderungen, Rest übernimmt die Basisklasse.
EnoceanModule::EnoceanModule() : _teachIn(_esp3) { _teachIn.onLearnModeChanged(&EnoceanModule::onTeachInLearnModeChanged); }

// Liefert den Modulnamen "Enocean" für Log-/Konsolenausgaben.
const std::string EnoceanModule::name() { return "Enocean"; }

// Liefert bewusst einen leeren String, damit das Modul nicht separat in der Versionsausgabe der Konsole erscheint.
const std::string EnoceanModule::version() {
  // hides the module in the version output on the console, because the firmware
  // version is sufficient.
  return "";
}

// Merkt sich den zu verwendenden UART-Stream für die Kommunikation mit dem EnOcean-Transceiver
// und initialisiert den ESP3-Treiber darauf.
void EnoceanModule::initSerial(Stream &serial) { _esp3.begin(serial); }

// Initialisiert die UART-Schnittstelle zum EnOcean-Transceiver und legt bei vorhandener Konfiguration die Kanäle an.
void EnoceanModule::setup(bool configured) {
  logInfoP("Setup0");
  logIndentUp();

  OPENKNX_ENO_SERIAL.setRX(OPENKNX_ENO_UART_RX_PIN);
  OPENKNX_ENO_SERIAL.setTX(OPENKNX_ENO_UART_TX_PIN);

  OPENKNX_ENO_SERIAL.begin(57600);
  initSerial(OPENKNX_ENO_SERIAL);

  SERIAL_PORT.begin(9600);

  if (configured) {
    // setupCustomFlash();
    setupChannels();
  }

  begin();

  logIndentDown();
}

// Erzeugt für jeden in der ETS sichtbaren Kanal eine EnoceanChannel-Instanz und ruft deren setup() auf.
void EnoceanModule::setupChannels() {
  for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
    _channels[i] = new EnoceanChannel(i);
    _channels[i]->setup();
  }
}

// void EnoceanModule::setupCustomFlash() {
//   logDebugP("initialize Enocean flash");
//   OpenKNX::Flash::Driver _EnoceanStorage;
// #ifdef ARDUINO_ARCH_ESP32
//   _EnoceanStorage.init("Enocean");
// #else
//   _EnoceanStorage.init("Enocean", Enocean_FLASH_OFFSET, Enocean_FLASH_SIZE);
// #endif
//
//   logTraceP("write Enocean data");
//   // _EnoceanStorage.writeByte(0, 0x11);
//   // _EnoceanStorage.writeWord(1, 0xFFFF);
//   // _EnoceanStorage.writeInt(3, 6666666);
//   // for (size_t i = 0; i < 4095; i++)
//   // {
//   //     _EnoceanStorage.writeByte(i, i);
//   // }
//   // _EnoceanStorage.commit();
//
//   logDebugP("read Enocean data");
//   logIndentUp();
//   // logHexDebugP(_EnoceanStorage.flashAddress(), 4095);
//   // logDebugP("byte: %02X", _EnoceanStorage.readByte(0)); // UINT8
//   // logDebugP("word: %i", _EnoceanStorage.readWord(1));   // UINT16
//   // logDebugP("int: %i", _EnoceanStorage.readInt(3));     // UINT32
//
//   logIndentDown();
// }

// Zyklischer Aufruf: loggt periodisch den Status und verarbeitet Round-Robin ein Telegramm pro konfiguriertem Kanal.
void EnoceanModule::loop(bool configured) {
  if (delayCheck(_timer1, 30000)) {
    logInfoP("Loop0");
    logInfoP("CH %u", ParamENO_VisibleChannels);

    _timer1 = millis();
  }

  if (configured) {
    if (ParamENO_VisibleChannels == 0)
      return;

    uint8_t processed = 0;
    do {
      task();

      _channels[_currentChannel]->loop();
    } while (openknx.freeLoopIterate(ParamENO_VisibleChannels, _currentChannel, processed));
  }
}

#ifdef OPENKNX_DUALCORE

// Setup für den zweiten RP2040-Kern (Dual-Core); aktuell nur eine Startverzögerung.
void EnoceanModule::setup1(bool configured) {
  delay(1000);
  // logInfoP("Setup1");
}

// Loop für den zweiten RP2040-Kern (Dual-Core); aktuell ohne Funktion.
void EnoceanModule::loop1(bool configured) {
  if (delayCheck(_timer2, 7200)) {
    // logInfoP("Loop1");
    _timer2 = millis();
  }
}
#endif

// Empfängt und verarbeitet ein einzelnes EnOcean-Telegramm vom UART und protokolliert periodisch ein Lebenszeichen.
void EnoceanModule::task() {
  static uint32_t lastTime = 0;
  if (millis() - lastTime > 30000) {
    logInfoP("Alive, runtime: %u", lastTime);
    lastTime = millis();
  }

  // EnOcean IN -> KNX OUT
  uint8_t ret = _esp3.poll();
  getEnOceanMSG(ret, &_esp3.packet());
}

// Einmalige Initialisierung: liest die Transceiver-Base-ID aus, gleicht sie ggf. mit der ETS-Parametrierung ab
// und konfiguriert die Repeater-Funktion. Das eigentliche ESP3-Request/Response-Handling steckt jetzt in EnOceanESP3.
void EnoceanModule::begin() {
  if (isInited)
    return;

  //****************** Read, Check & Set EnOcean Gateway Base ID ************************************
  delay(2000);
  logInfoP("read BaseID");

  if (!_esp3.readBaseId(lui8_BaseID_p)) {
    logInfoP("Base-ID: read failed!");
  } else if ((knx.paramByte(ENO_SetBaseIdFunc) >> ENO_SetBaseIdFuncShift) & 1) {
    if (checkBaseID()) { // alte != neue Base-ID laut ETS-Parametrierung
      logInfoP("Base-ID: OLD != NEW -> change! ");
      if (0x00 == _esp3.setBaseId(lui8_BaseID_p)) {
        // Base-ID erneut lesen, um die Aenderung zu bestaetigen und den Modul-Cache zu aktualisieren.
        _esp3.readBaseId(lui8_BaseID_p);
      } else {
        // Genauer Grund (RET_NOT_SUPPORTED, FLASH_HW_ERROR, BASEID_MAX_REACHED, ...) wurde bereits von
        // setBaseId() geloggt; hier nur die Zusammenfassung fuer diesen Ablauf.
        logInfoP("Base-ID: change FAILED, keeping previous value.");
      }
    } else {
      logDebugP("Base-ID: OLD == NEW -> NO change! ");
    }
  }

  for (int i = 0; i < BASEID_BYTES; i++) {
    logInfoP("Base-ID: %i", lui8_BaseID_p[i]);
  }

  //****************** Repeater Function ************************************
  logDebugP("----------------------");
  _esp3.setRepeater(ParamENO_RepeaterFunc, ParamENO_RepeaterLevel);
  // prueft ob Aenderungen umgesetzt wurden
  uint8_t repEnable = 0, repLevel = 0;
  _esp3.readRepeater(repEnable, repLevel);
  logDebugP("----------------------");

  isInited = true;
}

// Vergleicht die zuletzt gelesene Base-ID mit der in der ETS parametrierten (ParamENO_Id2/4/6).
bool EnoceanModule::checkBaseID()
{
  if (lui8_BaseID_p[0] != 0xFF)
    return 1;
  else if (lui8_BaseID_p[1] != ParamENO_Id2)
    return 1;
  else if (lui8_BaseID_p[2] != ParamENO_Id4)
    return 1;
  else if (lui8_BaseID_p[3] != ParamENO_Id6)
    return 1;
  else
    return 0;
}

// Bildet das Detailergebnis des letzten activateLearnMode()/readLearnMode()-Aufrufs auf einen der beiden
// Fehler-Sonderwerte des KO IsTeachChannel ab (siehe EnoceanModule.h).
uint8_t EnoceanModule::teachChannelErrorCode() const {
  return (_esp3.lastLearnModeResult() == LEARNMODE_RESULT_REJECTED) ? ENO_TEACHCHANNEL_ERROR_REJECTED
                                                                      : ENO_TEACHCHANNEL_ERROR_NO_RESPONSE;
}

// Statischer Trampolin-Aufruf (siehe Deklaration in EnoceanModule.h) auf die eigentliche, nicht-statische
// Implementierung, da logDebugP/logInfoP/KoENO_* ein gueltiges 'this' (logPrefix()) brauchen.
void EnoceanModule::onTeachInLearnModeChanged(uint8_t channel, EnOceanLearnModeEvent event) {
  openknxEnoceanModule.reportTeachInLearnModeChanged(channel, event);
}

// Von EnOceanTeachIn::teachInOnChannel() (Secure-Teach-in) bei jeder Lernmodus-Statusaenderung aufgerufen (siehe
// Registrierung im Konstruktor), damit dieser Weg genauso auf KoENO_IsTeachChannel sichtbar wird wie der Weg
// ueber processInputKo()/startDisableAllChannels().
void EnoceanModule::reportTeachInLearnModeChanged(uint8_t channel, EnOceanLearnModeEvent event) {
  switch (event) {
  case LEARNMODE_EVENT_OPENED:
    logDebugP("Reporting teach channel opened (via TeachIn): %u", channel);
    KoENO_IsTeachChannel.value(channel, Dpt(5, 10));
    break;
  case LEARNMODE_EVENT_CLOSED:
    logDebugP("Reporting teach channel closed (via TeachIn).");
    KoENO_IsTeachChannel.value(ENO_TEACHCHANNEL_CLOSED, Dpt(5, 10));
    break;
  case LEARNMODE_EVENT_ACTIVATE_FAILED: {
    uint8_t errorCode = teachChannelErrorCode();
    logInfoP("TeachIn: activating learn mode for channel %u FAILED (result=%u).", channel,
             _esp3.lastLearnModeResult());
    KoENO_IsTeachChannel.value(errorCode, Dpt(5, 10));
    break;
  }
  }
}

// Schliesst den Lernmodus, egal auf welchem Kanal er gerade offen ist: fragt per CO_RD_LEARNMODE den
// tatsaechlichen Zustand des Transceivers ab (statt blind alle 30 Kanäle durchzuschalten - da dank der
// Ein-Kanal-Garantie in EnOceanESP3::activateLearnMode() ohnehin nie mehr als einer aktiv sein kann, und ein
// blindes 1..30-Sweep den wirklich aktiven Kanal weder zuverlässig trifft, wenn er z. B. durch einen
// MCU-Reset ohne Transceiver-Reset undokumentiert offen blieb, noch einen Kanal außerhalb 1..30 erreichen
// würde) und deaktiviert genau diesen einen Kanal. Meldet den geschlossenen Zustand per KO IsTeachChannel = 255.
void EnoceanModule::startDisableAllChannels() {
  uint8_t enable = 0;
  uint8_t activeChannel = 0;
  if (!_esp3.readLearnMode(enable, activeChannel)) {
    uint8_t errorCode = teachChannelErrorCode();
    logInfoP("Closing learn mode FAILED: could not read current state (result=%u).", _esp3.lastLearnModeResult());
    KoENO_IsTeachChannel.value(errorCode, Dpt(5, 10));
    return;
  }

  if (enable) {
    logDebugP("Disabling learn mode for active channel %u.", activeChannel);
    if (!_esp3.activateLearnMode(activeChannel, false)) {
      uint8_t errorCode = teachChannelErrorCode();
      logInfoP("Closing learn mode for channel %u FAILED (result=%u).", activeChannel, _esp3.lastLearnModeResult());
      KoENO_IsTeachChannel.value(errorCode, Dpt(5, 10));
      return;
    }
  } else {
    logDebugP("No channel currently in learn mode.");
  }

  logDebugP("Reporting teach channel closed.");
  KoENO_IsTeachChannel.value(ENO_TEACHCHANNEL_CLOSED, Dpt(5, 10));
}

// Fragt den aktuell aktiven Lernmodus-Kanal direkt per CO_RD_LEARNMODE ab (ausgelöst über SetTeachChannel = 100)
// und meldet ihn per KO IsTeachChannel - ein einziger Request statt eines 1..30-Scans, da readLearnMode() den
// tatsächlich aktiven Kanal ohnehin direkt liefert (Spec Tab. 54), inklusive Werten außerhalb 1..30 (z. B. ein
// Reststand vom Transceiver selbst).
void EnoceanModule::startReportTeachChannel() {
  uint8_t enable = 0;
  uint8_t activeChannel = 0;
  if (!_esp3.readLearnMode(enable, activeChannel)) {
    uint8_t errorCode = teachChannelErrorCode();
    logInfoP("Reading teach channel FAILED (result=%u).", _esp3.lastLearnModeResult());
    KoENO_IsTeachChannel.value(errorCode, Dpt(5, 10));
    return;
  }

  if (enable) {
    logDebugP("Reporting teach channel: %u", activeChannel);
    KoENO_IsTeachChannel.value(activeChannel, Dpt(5, 10));
  } else {
    logDebugP("No channel currently in learn mode.");
    KoENO_IsTeachChannel.value(ENO_TEACHCHANNEL_CLOSED, Dpt(5, 10));
  }
}







// Rückweg KNX→EnOcean: ermittelt Kanal und lokalen KO-Index des beschriebenen Gruppenobjekts über die ENO_KoCalc*-Makros.
void EnoceanModule::processInputKo(GroupObject &iKo) {
  if (iKo.asap() == ENO_KoSetTeachChannel) {
    uint8_t teachChannel = iKo.value(Dpt(5, 10));
    logDebugP("processInputKo: SetTeachChannel = %u", teachChannel);
    if (teachChannel > 0 && teachChannel <= 30) {
      if (_esp3.activateLearnMode(teachChannel, true)) {
        logDebugP("Reporting teach channel opened: %u", teachChannel);
        KoENO_IsTeachChannel.value(teachChannel, Dpt(5, 10));
      } else {
        uint8_t errorCode = teachChannelErrorCode();
        logInfoP("Activating learn mode for channel %u FAILED (result=%u).", teachChannel, _esp3.lastLearnModeResult());
        KoENO_IsTeachChannel.value(errorCode, Dpt(5, 10));
      }
    } else if (teachChannel == 255) {
      startDisableAllChannels();
    } else if (teachChannel == 100) {
      startReportTeachChannel();
    }
    return;
  }

  int channel = ENO_KoCalcChannel(iKo.asap());
  if (channel < 0 || channel >= ParamENO_VisibleChannels)
    return; // Gruppenobjekt gehört zu keinem aktiven EnOcean-Kanal

  uint8_t _channelIndex = (uint8_t)channel;
  int koIndex = ENO_KoCalcIndex(iKo.asap());

  // logDebugP("processInputKo: Kanal %u, KO-Index %d", _channelIndex+1, koIndex);
  handleKnxEvent(_channelIndex, koIndex, iKo);
}







// Wird aufgerufen, wenn ein Gruppenobjekt eines EnOcean-Kanals von der KNX-Seite beschrieben wurde; _channelIndex ist der betroffene Kanal, koIndex die lokale
// Position (0-basiert) innerhalb des Kanal-Blocks, ko der Wert.
void EnoceanModule::handleKnxEvent(uint8_t _channelIndex, int koIndex, GroupObject &iKo) {
  //logDebugP("handleKnxEvent: KO-Index %d, Value: %he", koIndex);
  //logHexDebugP(iKo.valueRef(), iKo.valueSize());
  // TODO: je nach konfiguriertem EEP-Profil und koIndex das passende EnOcean-Funktelegramm senden.

  uint8_t teachinCH = 0;

//  if (koIndex != _channelIndex) // prüft ob KO zum CHannel passt
//  {
//    return;
//  }

  // SERIAL_PORT.print(F("Profil: "));
  // SERIAL_PORT.println(knx.paramByte(ENO_CHProfilSelection + firstParameter));

  switch (ParamENO_CHProfilSelection) {
  case u8RORG_4BS:
    logInfoP("4BS %u", ParamENO_CHProfil4BS20); //knx.paramWord(ENO_CHProfil4BS20 + firstParameter));
    switch (ParamENO_CHProfil4BS20) {
    // *************** A5-20-01 ***********************************************************
    case A5_20_01:
//      switch (koNr) {
//      case KO_0: // SET Temp / SET Pos
//        if ((knx.paramByte(ENO_CHA52001SPS + firstParameter) >> ENO_CHA52001SPSShift) & 1) {
//          union1.val_A5_20_01[0] = (float)iKo.value(getDPT(VAL_DPT_9)) * 2.5; // Set Point Temp#
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Temp to: "));
//          SERIAL_PORT.print(union1.val_A5_20_01[0] / 2.5);
//          SERIAL_PORT.println(F("°C"));
//#endif
//        } else {
//          union1.val_A5_20_01[0] = (uint8_t)iKo.value(getDPT(VAL_DPT_5)); // Set Point Pos
//          union1.val_A5_20_01[0] = round(union1.val_A5_20_01[0] / 2.55);
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Pos to: "));
//          SERIAL_PORT.print(union1.val_A5_20_01[0]);
//          SERIAL_PORT.println(F("%"));
//#endif
//        }
//        break;
//      case KO_1: // Summer Bit
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_01[2] = (uint8_t)union1.val_A5_20_01[2] | (1 << 3); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: aktiv"));
//#endif
//        } else {
//          union1.val_A5_20_01[2] = (uint8_t)union1.val_A5_20_01[2] & ~(1 << 3); // clear Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: inaktiv"));
//#endif
//        }
//        break;
//      case KO_2: // Run Init
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_01[2] = (uint8_t)union1.val_A5_20_01[2] | (1 << 7); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Run Init"));
//#endif
//        } else {
//          union1.val_A5_20_01[2] = (uint8_t)union1.val_A5_20_01[2] & ~(1 << 7); // clear Bit
//        }
//
//        break;
//      case KO_3: // Raum Temp
//        union1.val_A5_20_01[1] = (uint8_t)iKo.value(getDPT(VAL_DPT_9)) * 6.375;
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Raum-Temp: "));
//        SERIAL_PORT.print(union1.val_A5_20_01[1] / 6.375);
//        SERIAL_PORT.println(F("°C"));
//#endif
//        break;
//      case KO_4: // Service CMD
//
//        union3.A52001_CMD = iKo.value(getDPT(VAL_DPT_5));
//        if (union3.A52001_CMD < 8) {
//          if (union3.A52001_CMD == 0) {
//            union1.val_A5_20_01[2] &= ~(1 << 4); // clear Bit4
//            union1.val_A5_20_01[2] &= ~(1 << 5); // clear Bit5
//            union1.val_A5_20_01[2] &= ~(1 << 6); // clear Bit6
//          } else if (union3.A52001_CMD == 1) {
//            union1.val_A5_20_01[2] |= (1 << 4);  // Set   Bit4
//            union1.val_A5_20_01[2] &= ~(1 << 5); // clear Bit5
//            union1.val_A5_20_01[2] &= ~(1 << 6); // clear Bit6
//          } else if (union3.A52001_CMD == 2) {
//            union1.val_A5_20_01[2] &= ~(1 << 4); // clear Bit4
//            union1.val_A5_20_01[2] |= (1 << 5);  // set   Bit5
//            union1.val_A5_20_01[2] &= ~(1 << 6); // clear Bit6
//          } else if (union3.A52001_CMD == 3) {
//            union1.val_A5_20_01[2] &= ~(1 << 4); // clear Bit4
//            union1.val_A5_20_01[2] &= ~(1 << 5); // clear Bit5
//            union1.val_A5_20_01[2] |= (1 << 6);  // set   Bit6
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("Service CMD: "));
//          SERIAL_PORT.println(union3.A52001_CMD);
//#endif
//        } else {
//          union1.val_A5_20_01[2] &= ~(1 << 4); // clear Bit4
//          union1.val_A5_20_01[2] &= ~(1 << 5); // clear Bit5
//          union1.val_A5_20_01[2] &= ~(1 << 6); // clear Bit6
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("Service CMD: Wrong Value"));
//#endif
//        }
//        break;
//      }
      break;
    // *************** A5-20-04 ***********************************************************
    case A5_20_04:
//      switch (koNr) {
//      case KO_0: //  SET Pos
//        union1.val_A5_20_04[0] = (uint8_t)iKo.value(getDPT(VAL_DPT_5));
//        union1.val_A5_20_04[0] = round(union1.val_A5_20_04[0] / 2.55);
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("SET Pos to: "));
//        SERIAL_PORT.print(union1.val_A5_20_04[0]);
//        SERIAL_PORT.println(F("%"));
//#endif
//        break;
//      case KO_1: //  SET Temp
//        union1.val_A5_20_04[1] = ((float)iKo.value(getDPT(VAL_DPT_9)) - 10.0) / 0.078;
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("SET Temp to: "));
//        SERIAL_PORT.print(union1.val_A5_20_04[1] * 0.078 + 10.0);
//        SERIAL_PORT.println(F("°C"));
//#endif
//        break;
//      case KO_2: // Wake UP cycle
//        union3.A52004 = iKo.value(getDPT(VAL_DPT_5));
//        if (union3.A52004 <= 63) {
//          union1.val_A5_20_04[2] = ((uint8_t)union3.A52004); // Set WCU
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("WCU: "));
//          SERIAL_PORT.println(union3.A52004);
//          SERIAL_PORT.print(F("DB1: "));
//          SERIAL_PORT.println(union1.val_A5_20_04[2]);
//#endif
//        } else {
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("WCU: wrong Value"));
//#endif
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("MC: "));
//        SERIAL_PORT.println((knx.paramByte(ENO_CHA52004MC + firstParameter) >> ENO_CHA52004MCShift) & 1);
//#endif
//        break;
//      case KO_3: // Button Lock
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_04[3] |= (1 << 2); // set   Bit2
//        } else {
//          union1.val_A5_20_04[3] &= ~(1 << 2); // clear Bit2
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("BLC: "));
//        SERIAL_PORT.println((bool)iKo.value(getDPT(VAL_DPT_1)));
//#endif
//        break;
//      case KO_4: // Service CMD
//        union3.A52004 = iKo.value(getDPT(VAL_DPT_5));
//
//        switch (union3.A52004) {
//        case 0:                                // No Change
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        case 1:                                // open Value
//          union1.val_A5_20_04[3] |= (1 << 0);  // set   Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        case 2:                                // RUN initilistation
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] |= (1 << 1);  // set   Bit1
//          break;
//        case 3:                               // RUN initilistation
//          union1.val_A5_20_04[3] |= (1 << 0); // set   Bit0
//          union1.val_A5_20_04[3] |= (1 << 1); // set   Bit1
//          break;
//        default:
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Service : "));
//        SERIAL_PORT.println(union3.A52004);
//#endif
//        break;
//      }
      break;
    // *************** A5-20-06 ***********************************************************
    case A5_20_06:
//
//      switch (koNr) {
//      case KO_Teachin: // Teach-in MSG
//        teachinCH = iKo.value(getDPT(VAL_DPT_5));
//#ifdef KDEBUG_min
//        SERIAL_PORT.println(teachinCH);
//        SERIAL_PORT.println(index + 1);
//#endif
//        if (teachinCH != index + 1) {
//          return;
//        }
//#ifdef KDEBUG_min
//        SERIAL_PORT.println(F("ready to send"));
//#endif
//        union1.val_A5_20_06[0] = 0x80;
//        union1.val_A5_20_06[1] = 0x30;
//        union1.val_A5_20_06[2] = 0x49;
//        union1.val_A5_20_06[3] = 0x80;
//        send_4BS_Msg(lui8_SendeID_p, index, union1.val_A5_20_06, 0);
//        break;
//
//      case KO_0: // SET Temp oder SET Pos
//        if ((knx.paramByte(ENO_CHA52006SPS + firstParameter) >> ENO_CHA52006SPSShift) & 1) {
//          union1.val_A5_20_06[0] = (float)iKo.value(getDPT(VAL_DPT_9)) * 2.0; // Set Point Temp#
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Temp to: "));
//          SERIAL_PORT.print(union1.val_A5_20_06[0] / 2.0);
//          SERIAL_PORT.println(F("°C"));
//#endif
//        } else {
//          union1.val_A5_20_06[0] = (uint8_t)iKo.value(getDPT(VAL_DPT_5)); // Set Point Pos
//          union1.val_A5_20_06[0] = round(union1.val_A5_20_06[0] / 2.55);
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Pos to: "));
//          SERIAL_PORT.print(union1.val_A5_20_06[0]);
//          SERIAL_PORT.println(F("%"));
//#endif
//        }
//        break;
//
//      case KO_1: // Sommer Umschaltung
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] | (1 << 3); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: aktiv"));
//#endif
//        } else {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] & ~(1 << 3); // clear Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: inaktiv"));
//#endif
//        }
//        break;
//
//      case KO_2: // Set Communications Interval
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Com Interval: "));
//        SERIAL_PORT.println((uint8_t)iKo.value(getDPT(VAL_DPT_5)));
//#endif
//        union1.val_A5_20_06[2] &= ~(1 << ENO_CHA52006RFCShift);                                    // clear Bit4
//        union1.val_A5_20_06[2] &= ~(1 << (ENO_CHA52006RFCShift + 1));                              // clear Bit5
//        union1.val_A5_20_06[2] &= ~(1 << (ENO_CHA52006RFCShift + 2));                              // clear Bit6
//        union1.val_A5_20_06[2] |= ((uint8_t)iKo.value(getDPT(VAL_DPT_5)) << ENO_CHA52006RFCShift); // Set Bit
//        break;
//
//      case KO_3:                                                              // Raum Temperatur
//        union1.val_A5_20_06[1] = (uint8_t)iKo.value(getDPT(VAL_DPT_9)) * 4.0; // Raumtemperatur
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Raum-Temp: "));
//        SERIAL_PORT.print(union1.val_A5_20_06[1]);
//        SERIAL_PORT.println(F("°C"));
//#endif
//        break;
//
//      case KO_4: // Standby
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] | (1 << 0); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Standby"));
//#endif
//        } else {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] & ~(1 << 0); // clear Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("normal Mode"));
//#endif
//        }
//        break;
//
//      default:
//        break;
//      }
//      // gesendet wird in der TASK() da das Device nur nachrichten Empfangen kann, 1sek nachdem es eine Nachrticht
//      // geschickt hat send_4BS_Msg(lui8_SendeID_p, index, union1.val_A5_20_06);
//      break;
//    } // ENDE 2BS
    break;
  case u8RORG_VLD:
    switch (ParamENO_CHProfilSelectionVLD) {
    case D2_01:
//      switch (knx.paramWord(ENO_CHProfilVLD01 + firstParameter)) {
//      case D2_01_0E:
//        switch (koNr) {
//        case KO_0: // schalten Aktor
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//            buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = true;
//          } else {
//            buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = false;
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_0 handled"));
//#endif
//          break;
//        case KO_1: // Anfrage Messwerte
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_1 handled"));
//#endif
//          getActorsMeasurmentValue(lui8_SendeID_p, index, union1.val_D2_01_0E_Energy,
//                                   iKo.value(getDPT(VAL_DPT_1))); // Energy = 0  / Power = 1
//          break;
//        case KO_2: // Anfrage Statuswerte SWITCH
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_2 handled"));
//#endif
//          getStatusActors(lui8_SendeID_p, index); // Request Aktor Status
//          break;
//        }
//      default:
        break; // ENDE VLD D2_01_0E
      case D2_01_12:
//        switch (koNr) {
//        case KO_0: // schalten Aktor CH1
//          setStatusActors(lui8_SendeID_p, index, iKo.value(getDPT(VAL_DPT_1)));
//
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//
//            /*buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = true;*/
//#ifdef KDEBUG
//            SERIAL_PORT.println(F("KO value = HIGH"));
//#endif
//          } else {
//            /*buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = false;*/
//#ifdef KDEBUG
//            SERIAL_PORT.println(F("KO value = LOW"));
//#endif
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_0 handled"));
//#endif
//          break;
//        case KO_1: //  Schalten Aktor CH2
//          // union4.koIndex2 = index + 2;
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//            buttonStateSimulation2 = SIMULATE_PUSH;
//            buttonMessage2 = true;
//          } else {
//            buttonStateSimulation2 = SIMULATE_PUSH;
//            buttonMessage2 = false;
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_1 handled"));
//#endif
//          break;
//        case KO_4:
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_4 handled"));
//#endif
//          getStatusActors(lui8_SendeID_p, index); // Request Aktor Status
//          break;
//        default:
//          break;
//        }
        break; // ENDE VLD D2_01_12
      }
      break; // ENDE VLD D2_01
    }
    break; // ENDE VLD

  case u8RORG_Rocker:
    switch (ParamENO_CHDirectionKnxEnocean) {
    case 1:
      logInfoP("send Rocker MSG");
      if (iKo.value(Dpt(1, 1))) {
        send_RPS_Taster(lui8_BaseID_p, false, true, 0); // BaseID_CH = 0
      } else {
        send_RPS_Taster(lui8_BaseID_p, true, true, 0); // BaseID_CH = 0
      }
      break;
    }
    break;

  default:
    return;
    break; // ENDE ROCKER
  }
}

// Registriert den Hilfetext für den Konsolenbefehl "dummy".
void EnoceanModule::showHelp() { openknx.console.printHelpLine("dummy", "Print a dummy text"); }

// Verarbeitet Konsolenbefehle; derzeit nur ein Debug-Ausgabeblock für den (wegen eines Vergleichsfehlers nie erreichbaren) Befehl "Enocean".
bool EnoceanModule::processCommand(const std::string cmd, bool diagnoseKo) {
  if (cmd.substr(0, 5) == "Enocean") {
    logInfoP("Enocean Info");
    logIndentUp();
    logInfoP("Info 1");
    logInfoP("Info 2");
    logIndentUp();
    logInfoP("Info 2a");
    logInfoP("Info 2b");
    logIndentDown();
    logInfoP("Info 3");
    logIndentDown();
    return true;
  }

  return false;
}

#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
// Registriert Beispiel-Callbacks für den USB-Dateiaustausch (Demo-Datei "Dummy.txt"); wird aktuell nirgends aufgerufen.
void EnoceanModule::registerUsbExchangeCallbacks() {
  // Sample
  openknxUsbExchangeModule.onLoad("Dummy.txt", [](UsbExchangeFile *file) -> void { file->write("Demo"); });
  openknxUsbExchangeModule.onEject("Dummy.txt", [](UsbExchangeFile *file) -> bool {
    // File is required
    if (file == nullptr) {
      logError("EnoceanModule", "File Dummy.txt was deleted but is mandatory");
      return false;
    }
    return true;
  });
}
#endif
#endif

// ESP3-Paketversand/-Empfang (Sync/Header/CRC8H/Data/OptionalData/CRC8D) steckt jetzt vollständig in EnOceanESP3
// (siehe EnOceanESP3::sendPacket()/poll()).

// Wertet ein empfangenes ERP1-Funktelegramm aus und reicht es an alle konfigurierten Kanäle zur ID-Prüfung/Verarbeitung weiter.
void EnoceanModule::getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  if (u8RetVal == ENOCEAN_OK) {
    
    logDebugP("Received Data:");
    logHexDebugP(f_Pkt_st->u8DataBuffer, f_Pkt_st->u16DataLength + (uint16_t)f_Pkt_st->u8OptionLength);

    if (f_Pkt_st->u8Type == u8RADIO_ERP1) {

      /*
      if (f_Pkt_st->u8DataBuffer[0] == u8RORG_RPS) {
        logDebugP("Typ: RPS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X", f_Pkt_st->u8DataBuffer[2],
                  f_Pkt_st->u8DataBuffer[3], f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[1]);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_VLD) {
        logDebugP("Typ: VLD, Eno-ID: %02X-%02X-%02X-%02X", f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 5],
                  f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 4], f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 3],
                  f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2]);
        logHexDebugP(&f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u16DataLength - 6);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_4BS) {
        logDebugP("Typ: 4BS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X %02X %02X %02X", f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[6], f_Pkt_st->u8DataBuffer[7], f_Pkt_st->u8DataBuffer[8],
                  f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u8DataBuffer[2], f_Pkt_st->u8DataBuffer[3],
                  f_Pkt_st->u8DataBuffer[4]);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_1BS) {
        logDebugP("Typ: 1BS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X", f_Pkt_st->u8DataBuffer[2],
                  f_Pkt_st->u8DataBuffer[3], f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[1]);
      }
      */
      bool packetWasHandled = false;
      for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
        if (_channels[i]->check_Eno_ID(f_Pkt_st)) {
          packetWasHandled = true;
        }
      } 

      // Senden Unknown ID 
      if(packetWasHandled == false && ParamENO_NewID_OnOff == true)
      {
        checkAndReportUnknownId(f_Pkt_st);
      }

      logDebugP(packetWasHandled ? "Data handled :-)" : "Data not handled!");
    }
  }
}

// Ermittelt anhand des RORG-Typs die Position der 4-Byte Sender-ID im empfangenen Telegramm.
// Liefert false, wenn der RORG-Typ nicht unterstützt wird bzw. das Telegramm dafür zu kurz ist.
bool EnoceanModule::extractSenderId(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t out8SenderId[4]) {
  switch (f_Pkt_st->u8DataBuffer[0]) {
  case u8RORG_RPS:
  case u8RORG_1BS:
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[2];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[3];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[4];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[5];
    return true;

  case u8RORG_4BS:
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[5];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[6];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[7];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[8];
    return true;

  case u8RORG_VLD:
    if (f_Pkt_st->u16DataLength < 5)
      return false;
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 5];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 4];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 3];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2];
    return true;

  default:
    return false;
  }
}

// Prüft kanalübergreifend, ob die Sender-ID des empfangenen Telegramms an einem Kanal parametriert ist.
// Ist sie an keinem Kanal bekannt, wird sie sofort über das KO "NewId" auf den KNX-Bus gemeldet.
void EnoceanModule::checkAndReportUnknownId(PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  uint8_t senderId[4];
  if (!extractSenderId(f_Pkt_st, senderId))
    return;

  char idStr[15];
  snprintf(idStr, sizeof(idStr), "%02X%02X%02X%02X", senderId[0], senderId[1], senderId[2], senderId[3]);
  logInfoP("Unknown Enocean-ID: %s", idStr);
  KoENO_NewId.value(idStr, Dpt(16, 1));
}



void EnoceanModule::send_4BS_Msg(uint8_t *fui8_BaseID_p, uint8_t Index, uint8_t *inputs, uint8_t baseID_CH)
{
    PACKET_SERIAL_TYPE_ l_TestPacket_st;
    uint8_t l_TestBuf_p[10];
    l_TestPacket_st.u16DataLength = 0x000A;
    l_TestPacket_st.u8OptionLength = 0x00;
    l_TestPacket_st.u8Type = u8RADIO_ERP1;
    l_TestPacket_st.u8DataBuffer = &l_TestBuf_p[0];

    l_TestBuf_p[0] = u8RORG_4BS;

    l_TestBuf_p[1] = inputs[0];
    l_TestBuf_p[2] = inputs[1];
    l_TestBuf_p[3] = inputs[2];
    l_TestBuf_p[4] = inputs[3];

    for (int i = 0; i < 4; i++)
    {
        l_TestBuf_p[i + 5] = fui8_BaseID_p[i];
    }
    if (baseID_CH <= 4) // anpassen der Sende ID -> baseID_CH max 4 !!!
        l_TestBuf_p[8] = l_TestBuf_p[8] + baseID_CH;

    logInfoP("ID: %u", l_TestBuf_p[8]);

    _esp3.sendPacket(&l_TestPacket_st);
}

void EnoceanModule::send_RPS_Taster(uint8_t *fui8_BaseID_p, boolean state, boolean pressed, uint8_t baseID_CH)
{
    PACKET_SERIAL_TYPE_ l_TestPacket_st;
    uint8_t l_TestBuf_p[7];
    l_TestPacket_st.u16DataLength = 0x0007;
    l_TestPacket_st.u8OptionLength = 0x00;
    l_TestPacket_st.u8Type = u8RADIO_ERP1;
    l_TestPacket_st.u8DataBuffer = &l_TestBuf_p[0];

    l_TestBuf_p[0] = u8RORG_RPS;

    if (state == true)
    {
        if (pressed == true)
        {
            l_TestBuf_p[1] = 0x10;
            l_TestBuf_p[6] = 0x30;
        }
        else
        {
            l_TestBuf_p[1] = 0x00;
            l_TestBuf_p[6] = 0x20;
        }
    }
    else
    {
        if (pressed == true)
        {
            l_TestBuf_p[1] = 0x30;
            l_TestBuf_p[6] = 0x30;
        }
        else
        {
            l_TestBuf_p[1] = 0x00;
            l_TestBuf_p[6] = 0x20;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        l_TestBuf_p[i + 2] = fui8_BaseID_p[i];
    }

    if (baseID_CH <= 4) // anpassen der Sende ID -> baseID_CH max 4 !!!
        l_TestBuf_p[5] = l_TestBuf_p[5] + baseID_CH;

    logInfoP("Send-ID: %02X %02X %02X %02X", l_TestBuf_p[2], l_TestBuf_p[3], l_TestBuf_p[4], l_TestBuf_p[5]);

    _esp3.sendPacket(&l_TestPacket_st);
}

void EnoceanModule::setStatusActors(uint8_t *mySenderId, uint8_t idExtra, bool state)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[16];

    lRdBaseIDPkt_st.u16DataLength = 0x0009;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_01;
    lu8SndBuf[2] = 0x1E; // Channel
    if (state == true)
        lu8SndBuf[3] = 0x01;
    else
        lu8SndBuf[3] = 0x00;
    lu8SndBuf[4] = mySenderId[0];
    lu8SndBuf[5] = mySenderId[1];
    lu8SndBuf[6] = mySenderId[2];
    lu8SndBuf[7] = mySenderId[3] + idExtra;
    lu8SndBuf[8] = 0x00;
    // optional data
    lu8SndBuf[9] = 0x03;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0xFF;
    lu8SndBuf[14] = 0x00;
    lu8SndBuf[15] = 0x00;

    if (!_esp3.sendPacket(&lRdBaseIDPkt_st))
    {
        logInfoP("Sending telegram failed.");
    }
    else
    {
        logInfoP("Requested status");
    }
}

void EnoceanModule::getStatusActors(uint8_t *mySenderId, uint8_t idExtra)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[15];

    lRdBaseIDPkt_st.u16DataLength = 0x0008;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_03;
    lu8SndBuf[2] = 0x1e;

    lu8SndBuf[3] = mySenderId[0];
    lu8SndBuf[4] = mySenderId[1];
    lu8SndBuf[5] = mySenderId[2];
    lu8SndBuf[6] = mySenderId[3] + idExtra;
    lu8SndBuf[7] = 0x00;
    // optional data
    lu8SndBuf[8] = 0x03;
    lu8SndBuf[9] = 0xFF;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0x00;
    lu8SndBuf[14] = 0x00;

    if (!_esp3.sendPacket(&lRdBaseIDPkt_st))
    {
        logInfoP("Sending telegram failed.");
    }
    else
    {
        logInfoP("Requested status");
    }
}

void EnoceanModule::setActorsMeasurment(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[19];

    lRdBaseIDPkt_st.u16DataLength = 0x000C;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_05;
    lu8SndBuf[2] = inputs[0];
    lu8SndBuf[3] = inputs[1];
    lu8SndBuf[4] = inputs[2];
    lu8SndBuf[5] = inputs[3];
    lu8SndBuf[6] = inputs[4];
    lu8SndBuf[7] = mySenderId[0];
    lu8SndBuf[8] = mySenderId[1];
    lu8SndBuf[9] = mySenderId[2];
    lu8SndBuf[10] = mySenderId[3] + idExtra;
    lu8SndBuf[11] = 0x00;
    // optional data
    lu8SndBuf[12] = 0x03;
    lu8SndBuf[13] = 0xFF;
    lu8SndBuf[14] = 0xFF;
    lu8SndBuf[15] = 0xFF;
    lu8SndBuf[16] = 0xFF;
    lu8SndBuf[17] = 0x00;
    lu8SndBuf[18] = 0x00;

    if (!_esp3.sendPacket(&lRdBaseIDPkt_st))
    {
        logInfoP("Sending telegram failed.");
    }
    else
    {
        logInfoP("Requested Meas Setup");
    }
}

void EnoceanModule::getActorsMeasurmentValue(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs, bool unit)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[15];

    lRdBaseIDPkt_st.u16DataLength = 0x0008;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_06;
    lu8SndBuf[2] = 0x00; // 0x00 = Energy
    if (unit == 1)
        lu8SndBuf[2] = 0x20; // 0x20 = Power
    lu8SndBuf[2] = lu8SndBuf[2] + 0x1E;
    lu8SndBuf[3] = mySenderId[0];
    lu8SndBuf[4] = mySenderId[1];
    lu8SndBuf[5] = mySenderId[2];
    lu8SndBuf[6] = mySenderId[3] + idExtra;
    lu8SndBuf[7] = 0x00;
    // optional data
    lu8SndBuf[8] = 0x03;
    lu8SndBuf[9] = 0xFF;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0x00;
    lu8SndBuf[14] = 0x00;

    if (!_esp3.sendPacket(&lRdBaseIDPkt_st))
    {
        logInfoP("Sending telegram failed.");
    }
    else
    {
        logInfoP("Requested Meas Value");
    }
}


// setRepeaterFunc()/readRepeaterFunc() -> EnOceanESP3::setRepeater()/readRepeater(), aufgerufen aus begin().

EnoceanModule openknxEnoceanModule;
